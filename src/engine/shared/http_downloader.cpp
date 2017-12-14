/* Alexandre Díaz - GET HTTP Support */
#include "http_downloader.h"
#include <base/math.h>
#include <algorithm>

// TODO: Add regex or ssl support? ummm if yes best use libcurl

bool CHttpDownloader::GetToFile(const char *url, const char *dest, NETDOWNLOADINFO *pNetDownload, unsigned timeOut, unsigned downloadSpeed)
{
	unsigned char *pFileData = GetToMemory(url, pNetDownload, timeOut, downloadSpeed);

	if (pFileData)
	{
		if (pNetDownload->m_FileSize > 0u)
		{
			IOHANDLE dstFile = io_open(dest, IOFLAG_WRITE);
			if(!dstFile)
			{
				dbg_msg("HttpDownloader", "Error creating '%s'...", dest);
				pNetDownload->m_Status = ERROR;
				return false;
			}
			if (!io_write(dstFile, pFileData, pNetDownload->m_FileSize))
			{
				dbg_msg("HttpDownloader", "Error writing '%s'...", dest);
				io_close(dstFile);
				fs_remove(dest);
				mem_free(pFileData);
				pNetDownload->m_Status = ERROR;
				return false;
			}
			io_close(dstFile);
			pNetDownload->m_Status = DOWNLOADED;
		}
		else
			pNetDownload->m_Status = ERROR;

		mem_free(pFileData);
	}
	else
		pNetDownload->m_Status = ERROR;

	return pNetDownload->m_Status != ERROR;
}

unsigned char* CHttpDownloader::GetToMemory(const char *url, NETDOWNLOADINFO *pNetDownload, unsigned timeOut, unsigned downloadSpeed, bool onlyInfo)
{
	unsigned char *pData = 0x0;
	int64 downloadTime = time_get();
	unsigned chunkBytes = 0;

	pNetDownload->m_NetURL = CreateUrl(url);

    if (net_host_lookup(pNetDownload->m_NetURL.m_aHost, &pNetDownload->m_NAddr, NETTYPE_IPV4) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't found '%s'...", pNetDownload->m_NetURL.m_aHost);
        pNetDownload->m_Status = ERROR;
        return 0x0;
    }
    pNetDownload->m_NAddr.port = pNetDownload->m_NetURL.m_Port;

    pNetDownload->m_Socket = net_tcp_create(pNetDownload->m_BindAddr);
    if (net_tcp_connect(pNetDownload->m_Socket, &pNetDownload->m_NAddr) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't connect with '%s'...", pNetDownload->m_NetURL.m_aHost);
        net_tcp_close(pNetDownload->m_Socket);
        pNetDownload->m_Status = ERROR;
        return 0x0;
    }

    net_socket_rcv_timeout(pNetDownload->m_Socket, timeOut);
    pNetDownload->m_Status = CONNECTING;

    char aBuff[512] = {0};
    str_format(aBuff, sizeof(aBuff), "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", pNetDownload->m_NetURL.m_aFullPath, pNetDownload->m_NetURL.m_aHost);
	net_tcp_send(pNetDownload->m_Socket, aBuff, str_length(aBuff));

	pNetDownload->m_FileSize = 0u;
	std::string NetData;
	int CurrentRecv = 0;
	bool isHeader = true;
	const unsigned buffSize = onlyInfo?128:1024;
	char aNetBuff[buffSize];
	do
	{
		// Limit Speed
		if (downloadSpeed > 0)
		{
			const int64 ctime = time_get();
			if (ctime - downloadTime <= time_freq())
			{
				if (chunkBytes >= downloadSpeed)
				{
					const int tdff = (time_freq() - (ctime - downloadTime)) / 1000;
					thread_sleep(tdff);
					continue;
				}
			}
			else
			{
				chunkBytes = 0;
				downloadTime = time_get();
			}
		}
		//

		CurrentRecv = net_tcp_recv(pNetDownload->m_Socket, aNetBuff, sizeof(aNetBuff));
		chunkBytes += CurrentRecv;
		for (int i=0; i<CurrentRecv; i++)
		{
			if (isHeader)
			{
				if (aNetBuff[i] == '\n')
				{ // Is end of line
					if (NetData.size() > 0)
					{
                        std::transform(NetData.begin(), NetData.end(), NetData.begin(), ::tolower);
                        if (NetData.find("404 not found") != std::string::npos)
                        {
                            dbg_msg("HttpDownloader", "ERROR 404: '%s' not found...", pNetDownload->m_NetURL.m_aFile);
                            pNetDownload->m_Status = ERROR;
                            break;
                        }
                        else if (NetData.find("content-length:") != std::string::npos)
                        {
							char aFileSize[10];
							str_copy(aFileSize, NetData.substr(15).c_str(), sizeof(aFileSize)); // 15 is the length of 'content-length:' string
							str_trim(aFileSize);
							pNetDownload->m_FileSize = atoi(aFileSize);

							if (!onlyInfo)
							{
								if (pNetDownload->m_FileSize <= 0)
								{
								   dbg_msg("HttpDownloader", "Error downloading '%s'...", pNetDownload->m_NetURL.m_aFile);
								   pNetDownload->m_Status = ERROR;
								   break;
								}

								pData = (unsigned char*)mem_alloc(pNetDownload->m_FileSize, 1);
								if (!pData)
								{
									dbg_msg("HttpDownloader", "Error allocating memory for download '%s'...", pNetDownload->m_NetURL.m_aFile);
									pNetDownload->m_Status = ERROR;
									break;
								}
							}
                        }

                        NetData.clear();
					}
					else
					{ // Empty Line: End of Header
						isHeader = false;
						if (onlyInfo)
						{
							pNetDownload->m_Status = DOWNLOADED;
							break;
						}

						if (!pData)
						{
							dbg_msg("HttpDownloader", "Unexpected http error, 'content-length' not found!");
							pNetDownload->m_Status = ERROR;
							break;
						}

						pNetDownload->m_Status = DOWNLOADING;
					}
				}
				else if (aNetBuff[i] != '\r')
				{ // Fill line buffer
					NetData += aNetBuff[i];
				}
			}
			else
			{ // Read Body
				const unsigned int numBytes = CurrentRecv-i;
				if (pNetDownload->m_Received+numBytes > pNetDownload->m_FileSize)
				{
					dbg_msg("HttpDownloader", "Unexpected error... server try send more bytes that declared in 'content-length'!");
					pNetDownload->m_Status = ERROR;
					mem_free(pData);
					pData = 0x0;
					break;
				}

				mem_copy(pData+pNetDownload->m_Received, aNetBuff+i, numBytes);
				pNetDownload->m_Received += numBytes;
				if (pNetDownload->m_Received == pNetDownload->m_FileSize)
				{
					pNetDownload->m_Status = DOWNLOADED;
					break;
				}
				i += CurrentRecv;
			}
		}
	} while (CurrentRecv > 0 && !pNetDownload->m_ForceStop);

	net_tcp_close(pNetDownload->m_Socket);
    return pData;
}

CHttpDownloader::NETURL CHttpDownloader::CreateUrl(std::string url)
{
	NETURL re;
	mem_zero(&re, sizeof(NETURL));

    // Get Protocol
	size_t charpos = url.find("://");
	if (charpos != std::string::npos)
	{
		str_copy(re.m_aProtocol, url.substr(0, charpos).c_str(), sizeof(re.m_aProtocol));
		url = url.substr(charpos+3);
	}
	else
		str_copy(re.m_aProtocol, "http", sizeof(re.m_aProtocol)); // Default

    // Get Host, FullPath and File
    charpos = url.find_first_of("/\\");
    str_copy(re.m_aHost, charpos == std::string::npos?url.c_str():url.substr(0, charpos).c_str(), sizeof(re.m_aHost));
    str_copy(re.m_aFullPath, charpos == std::string::npos?"\0":url.substr(charpos).c_str(), sizeof(re.m_aFullPath));
    if (re.m_aFullPath[0] != 0)
    {
    	url = re.m_aFullPath;
    	charpos = url.find_last_of("/\\");
    	str_copy(re.m_aFile, charpos == std::string::npos?"\0":url.substr(charpos+1).c_str(), sizeof(re.m_aFile));
    }

    // Get Port?
    url = re.m_aHost;
    charpos = url.find(":");
    if (charpos != std::string::npos)
    {
		re.m_Port = atoi(url.substr(charpos).c_str());
		str_copy(re.m_aHost, url.substr(0, charpos).c_str(), sizeof(re.m_aHost));
    }
    else
    	re.m_Port = 80; // Default

    return re;
}
