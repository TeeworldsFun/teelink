#include "http_downloader.h"
#include <base/math.h>
#include <base/system.h>
#include <algorithm>

// TODO: Clean duplicate code and HTTP1.1 support
// TODO 2: Add regex or ssl support? ummm if yes best use libcurl

bool CHttpDownloader::GetToFile(const char *url, const char *dest, unsigned timeOut, unsigned downloadSpeed)
{
	if (!dest || dest[0] == 0)
		return false;

	int64 downloadTime = time_get();
	unsigned chunkBytes = 0;
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
    if (net_tcp_connect(sock, &nadd) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't connect with '%s'...", NetUrl.m_aHost);
        net_tcp_close(sock);
        return false;
    }

    net_socket_rcv_timeout(sock, timeOut);

    char aBuff[512] = {0};
    str_format(aBuff, sizeof(aBuff), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", NetUrl.m_aSlug, NetUrl.m_aHost);
	net_tcp_send(sock, aBuff, str_length(aBuff));

	std::string NetData;
	unsigned TotalRecv = 0;
	unsigned TotalBytes = 0;
	int CurrentRecv = 0;
	unsigned nlCount = 0;
	char aNetBuff[1024] = {0};
	IOHANDLE dstFile = NULL;
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

		CurrentRecv = net_tcp_recv(sock, aNetBuff, sizeof(aNetBuff));
		chunkBytes += CurrentRecv;
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
			    if (nlCount == 2)
                {
                    if (TotalBytes <= 0)
                    {
                        dbg_msg("HttpDownloader", "Error downloading '%s'...", NetUrl.m_aFile);
                        net_tcp_close(sock);
                        return false;
                    }

                    dstFile = io_open(dest, IOFLAG_WRITE);
                    if(!dstFile)
                    {
                        dbg_msg("HttpDownloader", "Error creating '%s'...", dest);
                        net_tcp_close(sock);
                        return false;
                    }

                    ++nlCount;
                }

				io_write(dstFile, &aNetBuff[i], 1);

				TotalRecv++;
				if (TotalRecv == TotalBytes)
					break;
			}
		}
	} while (CurrentRecv > 0);

	net_tcp_close(sock);
	if (TotalRecv > 0)
	{
		io_close(dstFile);
		return true;
	}

	return false;
}

char* CHttpDownloader::GetToMemory(const char *url, unsigned *size, unsigned timeOut, unsigned downloadSpeed)
{
	char *pData = 0x0;
	int64 downloadTime = time_get();
	unsigned chunkBytes = 0;
    NETSOCKET sock;
    NETADDR nadd, bindAddr;
    mem_zero(&nadd, sizeof(nadd));
    mem_zero(&bindAddr, sizeof(bindAddr));
    bindAddr.type = NETTYPE_IPV4;

    NETURL NetUrl = CreateUrl(url);

    if (net_host_lookup(NetUrl.m_aHost, &nadd, NETTYPE_IPV4) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't found '%s'...", NetUrl.m_aHost);
        return 0x0;
    }
    nadd.port = 80;

    sock = net_tcp_create(bindAddr);
    if (net_tcp_connect(sock, &nadd) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't connect with '%s'...", NetUrl.m_aHost);
        net_tcp_close(sock);
        return 0x0;
    }

    net_socket_rcv_timeout(sock, timeOut);

    char aBuff[512] = {0};
    str_format(aBuff, sizeof(aBuff), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", NetUrl.m_aSlug, NetUrl.m_aHost);
	net_tcp_send(sock, aBuff, str_length(aBuff));

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

		CurrentRecv = net_tcp_recv(sock, aNetBuff, sizeof(aNetBuff));
		chunkBytes += CurrentRecv;
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
                            return 0x0;
                        }
                        else if (NetData.find("content-length:") != std::string::npos)
                        {
                        	char aFileSize[64];
                        	str_copy(aFileSize, NetData.substr(15).c_str(), sizeof(aFileSize));
                        	str_trim(aFileSize);
                            TotalBytes = atoi(aFileSize);
                            pData = new char[TotalBytes];
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
			    if (nlCount == 2)
                {
                    if (TotalBytes <= 0)
                    {
                        dbg_msg("HttpDownloader", "Error downloading '%s'...", NetUrl.m_aFile);
                        net_tcp_close(sock);
                        return 0x0;
                    }

                    ++nlCount;
                }

			    pData[destCursor++] = aNetBuff[i];
				if (destCursor >= TotalBytes)
					break;
			}
		}
	} while (CurrentRecv > 0);
	net_tcp_close(sock);

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
