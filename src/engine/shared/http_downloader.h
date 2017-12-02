#ifndef ENGINE_SHARED_HTTP_DOWNLOADER_H
#define ENGINE_SHARED_HTTP_DOWNLOADER_H

#include <string>
#include <base/system.h>


class CHttpDownloader
{
public:
	enum
	{
		UNNUSED=0,
		DOWNLOADED,
		CONNECTING,
		DOWNLOADING,
		ERROR
	};

	struct NETURL
	{
		char m_aProtocol[8];
		char m_aHost[128];
		char m_aFile[255];
		char m_aFullPath[2048];
		unsigned int m_Port;
	};

	struct NETDOWNLOADINFO
	{
		NETDOWNLOADINFO()
		{
			Reset();
		}

		void Reset()
		{
			m_ForceStop = false;
			m_Received = 0;
			m_FileSize = 0;
			mem_zero(&m_NAddr, sizeof(m_NAddr));
			mem_zero(&m_BindAddr, sizeof(m_BindAddr));
			m_BindAddr.type = NETTYPE_IPV4; // Forced
			m_Status = UNNUSED;
		}

		bool m_ForceStop;
		unsigned int m_Received;
		unsigned int m_FileSize;
		NETSOCKET m_Socket;
		NETADDR m_NAddr;
		NETADDR m_BindAddr;
		NETURL m_NetURL;
		int m_Status;
	};

	static bool GetToFile(const char *url, const char *dest, NETDOWNLOADINFO *pNetDownload, unsigned timeOut = 3, unsigned downloadSpeed = 0);
	static unsigned char* GetToMemory(const char *url, NETDOWNLOADINFO *pNetDownload, unsigned timeOut = 3, unsigned downloadSpeed = 0, bool onlyInfo = false);

private:
	static NETURL CreateUrl(std::string url);
};

#endif
