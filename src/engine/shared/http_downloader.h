#ifndef ENGINE_SHARED_HTTP_DOWNLOADER_H
#define ENGINE_SHARED_HTTP_DOWNLOADER_H

#include <string>

class CHttpDownloader
{
public:
	struct NETURL
	{
		char m_aProtocol[8];
		char m_aHost[128];
		char m_aFile[255];
		char m_aSlug[2048];
	};

	static bool GetToFile(const char *url, const char *dest, unsigned timeOut = 3, unsigned downloadSpeed = 0);
	static bool GetToMemory(const char *url, char *dest, unsigned destSize, unsigned timeOut = 3, unsigned downloadSpeed = 0);
	static unsigned GetFileSize(const char *url, unsigned timeOut = 3);

	static NETURL CreateUrl(std::string url);
};

#endif
