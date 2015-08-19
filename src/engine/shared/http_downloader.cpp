#include "http_downloader.h"
#include <base/math.h>
#include <base/system.h>
#include <algorithm>

// TODO: Clean duplicate code and HTTP1.1 support
// TODO 2: Add regex or ssl support? ummm if yes best use libcurl

bool CHttpDownloader::GetToFile(std::string url, const char *dest, unsigned timeOut, unsigned downloadSpeed)
{
	if (!dest || dest[0] == 0)
		return false;

	int64 downloadTime = time_get();
	unsigned chunkBytes = 0;
    NETSOCKET sockDDNet;
    NETADDR naddDDNet, bindAddr;
    mem_zero(&naddDDNet, sizeof(naddDDNet));
    mem_zero(&bindAddr, sizeof(bindAddr));
    bindAddr.type = NETTYPE_IPV4;

    // Remove http/https
    if (url.find("https://") != std::string::npos)
    	url = url.substr(8);
    else if (url.find("http://") != std::string::npos)
    	url = url.substr(7);

    // Get Host and Slug
    std::string host, slug, file;
    size_t charpos = url.find_first_of("/\\");
    host = charpos == std::string::npos?url:url.substr(0, charpos);
    slug = charpos == std::string::npos?"":url.substr(charpos);
    charpos = slug.find_last_of("/\\");
    file = charpos == std::string::npos?"":slug.substr(charpos+1);


    if (net_host_lookup(host.c_str(), &naddDDNet, NETTYPE_IPV4) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't found '%s'...", host.c_str());
        return false;
    }
    naddDDNet.port = 80;

    sockDDNet = net_tcp_create(bindAddr);
    net_socket_rcv_timeout(sockDDNet, timeOut);
    if (net_tcp_connect(sockDDNet, &naddDDNet) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't connect with '%s'...", host.c_str());
        net_tcp_close(sockDDNet);
        return false;
    }

    char aBuff[512] = {0};
    str_format(aBuff, sizeof(aBuff), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", slug.c_str(), host.c_str());
	net_tcp_send(sockDDNet, aBuff, str_length(aBuff));

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

		CurrentRecv = net_tcp_recv(sockDDNet, aNetBuff, sizeof(aNetBuff));
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
                            dbg_msg("HttpDownloader", "ERROR 404: '%s' not found...", file.c_str());
                            net_tcp_close(sockDDNet);
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
                        dbg_msg("HttpDownloader", "Error downloading '%s'...", file.c_str());
                        net_tcp_close(sockDDNet);
                        return false;
                    }

                    dstFile = io_open(dest, IOFLAG_WRITE);
                    if(!dstFile)
                    {
                        dbg_msg("HttpDownloader", "Error creating '%s'...", dest);
                        net_tcp_close(sockDDNet);
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

	net_tcp_close(sockDDNet);
	io_close(dstFile);
    return true;
}

bool CHttpDownloader::GetToMemory(std::string url, char *dest, unsigned destSize, unsigned timeOut, unsigned downloadSpeed)
{
	if (!dest || destSize == 0)
		return false;

	int64 downloadTime = time_get();
	unsigned chunkBytes = 0;
    NETSOCKET sockDDNet;
    NETADDR naddDDNet, bindAddr;
    mem_zero(&naddDDNet, sizeof(naddDDNet));
    mem_zero(&bindAddr, sizeof(bindAddr));
    bindAddr.type = NETTYPE_IPV4;

    // Remove http/https
    if (url.find("https://") != std::string::npos)
    	url = url.substr(8);
    else if (url.find("http://") != std::string::npos)
    	url = url.substr(7);

    // Get Host and Slug
    std::string host, slug, file;
    size_t charpos = url.find_first_of("/\\");
    host = charpos == std::string::npos?url:url.substr(0, charpos);
    slug = charpos == std::string::npos?"":url.substr(charpos);
    charpos = slug.find_last_of("/\\");
    file = charpos == std::string::npos?"":slug.substr(charpos+1);


    if (net_host_lookup(host.c_str(), &naddDDNet, NETTYPE_IPV4) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't found '%s'...", host.c_str());
        return false;
    }
    naddDDNet.port = 80;

    sockDDNet = net_tcp_create(bindAddr);
    net_socket_rcv_timeout(sockDDNet, timeOut);
    if (net_tcp_connect(sockDDNet, &naddDDNet) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't connect with '%s'...", host.c_str());
        net_tcp_close(sockDDNet);
        return false;
    }

    char aBuff[512] = {0};
    str_format(aBuff, sizeof(aBuff), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", slug.c_str(), host.c_str());
	net_tcp_send(sockDDNet, aBuff, str_length(aBuff));

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

		CurrentRecv = net_tcp_recv(sockDDNet, aNetBuff, sizeof(aNetBuff));
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
                            dbg_msg("HttpDownloader", "ERROR 404: '%s' not found...", file.c_str());
                            net_tcp_close(sockDDNet);
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
                        dbg_msg("HttpDownloader", "Error downloading '%s'...", file.c_str());
                        net_tcp_close(sockDDNet);
                        return false;
                    }

                    ++nlCount;
                }

			    dest[destCursor++] = aNetBuff[i];
				if (destCursor >= TotalBytes || destCursor >= destSize-1)
					break;
			}
		}
	} while (CurrentRecv > 0);

	net_tcp_close(sockDDNet);
    return true;
}

unsigned CHttpDownloader::GetFileSize(std::string url, unsigned timeOut)
{
	unsigned chunkBytes = 0;
    NETSOCKET sockDDNet;
    NETADDR naddDDNet, bindAddr;
    mem_zero(&naddDDNet, sizeof(naddDDNet));
    mem_zero(&bindAddr, sizeof(bindAddr));
    bindAddr.type = NETTYPE_IPV4;

    // Remove http/https
    if (url.find("https://") != std::string::npos)
    	url = url.substr(8);
    else if (url.find("http://") != std::string::npos)
    	url = url.substr(7);

    // Get Host and Slug
    std::string host, slug, file;
    size_t charpos = url.find_first_of("/\\");
    host = charpos == std::string::npos?url:url.substr(0, charpos);
    slug = charpos == std::string::npos?"":url.substr(charpos);
    charpos = slug.find_last_of("/\\");
    file = charpos == std::string::npos?"":slug.substr(charpos+1);


    if (net_host_lookup(host.c_str(), &naddDDNet, NETTYPE_IPV4) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't found '%s'...", host.c_str());
        return false;
    }
    naddDDNet.port = 80;

    sockDDNet = net_tcp_create(bindAddr);
    net_socket_rcv_timeout(sockDDNet, timeOut);
    if (net_tcp_connect(sockDDNet, &naddDDNet) != 0)
    {
        dbg_msg("HttpDownloader", "Error can't connect with '%s'...", host.c_str());
        net_tcp_close(sockDDNet);
        return false;
    }

    char aBuff[512] = {0};
    str_format(aBuff, sizeof(aBuff), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", slug.c_str(), host.c_str());
	net_tcp_send(sockDDNet, aBuff, str_length(aBuff));

	std::string NetData;
	int TotalBytes = 0;
	int CurrentRecv = 0;
	int nlCount = 0;
	char aNetBuff[1024] = {0};
	do
	{
		CurrentRecv = net_tcp_recv(sockDDNet, aNetBuff, sizeof(aNetBuff));
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
                            dbg_msg("HttpDownloader", "ERROR 404: '%s' not found...", file.c_str());
                            net_tcp_close(sockDDNet);
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
                        dbg_msg("HttpDownloader", "Error getting size of '%s'...", file.c_str());
                        net_tcp_close(sockDDNet);
                        return 0;
                    }
                    else
                    {
                    	return TotalBytes;
                    }
                }
			}
		}
	} while (CurrentRecv > 0);

    return 0;
}
