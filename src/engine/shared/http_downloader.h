#ifndef ENGINE_SHARED_HTTP_DOWNLOADER_H
#define ENGINE_SHARED_HTTP_DOWNLOADER_H

#include <string>

class CHttpDownloader
{
public:
	static bool GetToFile(std::string url, const char *dest, unsigned timeOut = 3, unsigned downloadSpeed = 0);
	static bool GetToMemory(std::string url, char *dest, unsigned destSize, unsigned timeOut = 3, unsigned downloadSpeed = 0);
	static unsigned GetFileSize(std::string url, unsigned timeOut = 3);
};

#endif
