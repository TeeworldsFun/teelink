#include "http_downloader.h"
#include <base/math.h>
#include <algorithm>

// TODO: Clean duplicate code and HTTP1.1 support
// TODO 2: Add regex or ssl support? ummm if yes best use libcurl

bool CHttpDownloader::GetToFile(const char *url, const char *dest, NETDOWNLOAD *pNetDownload, unsigned timeOut, unsigned downloadSpeed)
{
	unsigned int FileSize = 0u;
	unsigned char *pFileData = GetToMemory(url, &FileSize, pNetDownload, timeOut, downloadSpeed);

	if (pFileData)
	{
		if (FileSize > 0u)
		{
			IOHANDLE dstFile = io_open(dest, IOFLAG_WRITE);
			if(!dstFile)
			{
				dbg_msg("HttpDownloader", "Error creating '%s'...", dest);
				pNetDownload->m_Status = ERROR;
				return false;
			}
			if (!io_write(dstFile, pFileData, FileSize))
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
		return pNetDownload->m_Status != ERROR;
	}

	pNetDownload->m_Status = ERROR;
	return false;
}

unsigned char* CHttpDownloader::GetToMemory(const char *url, unsigned *size, NETDOWNLOAD *pNetDownload, unsigned timeOut, unsigned downloadSpeed)
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
    pNetDownload->m_NAddr.port = 80;

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
    str_format(aBuff, sizeof(aBuff), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", pNetDownload->m_NetURL.m_aSlug, pNetDownload->m_NetURL.m_aHost);
	net_tcp_send(pNetDownload->m_Socket, aBuff, str_length(aBuff));

	*size = 0;
	std::string NetData;
	unsigned TotalBytes = 0;
	int CurrentRecv = 0;
	unsigned nlCount = 0;
	unsigned destCursor = 0;
	char aNetBuff[1024] = {0};
	do
	{
		// Limit Speed
		if (downloadSpeed > 0)
		{
			int64 ctime = time_get();
			if (ctime - downloadTime <= time_freq())
			{
				if (chunkBytes >= downloadSpeed)
				{
					int tdff = (time_freq() - (ctime - downloadTime)) / 1000;
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
			if (nlCount < 2)
			{
				if (aNetBuff[i] == '\r' || aNetBuff[i] == '\n')
				{
				    ++nlCount;
					if (NetData.size() > 0)
					{
                        std::transform(NetData.begin(), NetData.end(), NetData.begin(), ::tolower);
                        if (NetData.find("404 not found") != std::string::npos)
                        {
                            dbg_msg("HttpDownloader", "ERROR 404: '%s' not found...", pNetDownload->m_NetURL.m_aFile);
                            net_tcp_close(pNetDownload->m_Socket);
                            pNetDownload->m_Status = ERROR;
                            return 0x0;
                        }
                        else if (NetData.find("content-length:") != std::string::npos)
                        {
							char aFileSize[64];
							str_copy(aFileSize, NetData.substr(15).c_str(), sizeof(aFileSize));
							str_trim(aFileSize);
							TotalBytes = atoi(aFileSize);
							if (TotalBytes <= 0)
							{
							   dbg_msg("HttpDownloader", "Error downloading '%s'...", pNetDownload->m_NetURL.m_aFile);
							   net_tcp_close(pNetDownload->m_Socket);
							   pNetDownload->m_Status = ERROR;
							   return 0x0;
							}

                            pData = (unsigned char*)mem_alloc(TotalBytes, 1);
                            if (!pData)
                            {
                            	dbg_msg("HttpDownloader", "Error allocating memory for download '%s'...", pNetDownload->m_NetURL.m_aFile);
                            	net_tcp_close(pNetDownload->m_Socket);
                            	pNetDownload->m_Status = ERROR;
                            	return 0x0;
                            }

                            pNetDownload->m_Status = DOWNLOADING;
                        }

                        NetData.clear();
					}

					if (aNetBuff[i] == '\r')
						++i;
				}
				else
				{
					nlCount = 0;
					NetData += aNetBuff[i];
				}
			}
			else
			{
			    pData[destCursor++] = aNetBuff[i];
			    pNetDownload->m_Received = destCursor;
				if (destCursor >= TotalBytes)
					break;
			}
		}
	} while (CurrentRecv > 0 && !pNetDownload->m_ForceStop);

	net_tcp_close(pNetDownload->m_Socket);
	pNetDownload->m_Status = DOWNLOADED;

	*size = TotalBytes;
    return pData;
}

unsigned CHttpDownloader::GetFileSize(const char *url, unsigned timeOut)
{
    NETSOCKET sock;
    NETADDR nadd, bindAddr;
    mem_zero(&nadd, sizeof(nadd));
    mem_zero(&bindAddr, sizeof(bindAddr));
    bindAddr.type = NETTYPE_IPV4;

    NETURL NetUrl = CreateUrl(url);


    if (net_host_lookup(NetUrl.m_aHost, &nadd, NETTYPE_IPV4) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't found '%s'...", NetUrl.m_aHost);
        return false;
    }
    nadd.port = 80;

    sock = net_tcp_create(bindAddr);
    net_socket_rcv_timeout(sock, timeOut);
    if (net_tcp_connect(sock, &nadd) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't connect with '%s'...", NetUrl.m_aHost);
        net_tcp_close(sock);
        return false;
    }

    char aBuff[512] = {0};
    str_format(aBuff, sizeof(aBuff), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", NetUrl.m_aSlug, NetUrl.m_aHost);
	net_tcp_send(sock, aBuff, str_length(aBuff));

	std::string NetData;
	int TotalBytes = 0;
	int CurrentRecv = 0;
	int nlCount = 0;
	char aNetBuff[1024] = {0};
	do
	{
		CurrentRecv = net_tcp_recv(sock, aNetBuff, sizeof(aNetBuff));
		for (int i=0; i<CurrentRecv ; i++)
		{
			if (nlCount < 2)
			{
				if (aNetBuff[i] == '\r' || aNetBuff[i] == '\n')
				{
				    ++nlCount;
					if (NetData.size() > 0)
					{
                        std::transform(NetData.begin(), NetData.end(), NetData.begin(), ::tolower);
                        if (NetData.find("404 not found") != std::string::npos)
                        {
                            dbg_msg("HttpDownloader", "ERROR 404: '%s' not found...", NetUrl.m_aFile);
                            net_tcp_close(sock);
                            return false;
                        }
                        else if (NetData.find("content-length:") != std::string::npos)
                        {
                        	char aFileSize[64];
                        	str_copy(aFileSize, NetData.substr(15).c_str(), sizeof(aFileSize));
                        	str_trim(aFileSize);
                            TotalBytes = atoi(aFileSize);

                            if (TotalBytes <= 0)
                            {
                                dbg_msg("HttpDownloader", "Error getting size of '%s'...", NetUrl.m_aFile);
                                net_tcp_close(sock);
                                return 0;
                            }

                            net_tcp_close(sock);
                            return TotalBytes;
                        }

                        NetData.clear();
					}

					if (aNetBuff[i] == '\r') ++i;
					continue;
				}

                nlCount = 0;
                NetData += aNetBuff[i];
			}
			else
			{
				dbg_msg("HttpDownloader", "Error getting size of '%s'...", NetUrl.m_aFile);
				net_tcp_close(sock);
				return 0;
			}
		}
	} while (CurrentRecv > 0);

    return 0;
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

    // Get Host, Slug and File
    charpos = url.find_first_of("/\\");
    str_copy(re.m_aHost, charpos == std::string::npos?url.c_str():url.substr(0, charpos).c_str(), sizeof(re.m_aHost));
    str_copy(re.m_aSlug, charpos == std::string::npos?"\0":url.substr(charpos).c_str(), sizeof(re.m_aSlug));
    if (re.m_aSlug[0] != 0)
    {
    	url = re.m_aSlug;
    	charpos = url.find_last_of("/\\");
    	str_copy(re.m_aFile, charpos == std::string::npos?"\0":url.substr(charpos+1).c_str(), sizeof(re.m_aFile));
    }

    return re;
}
