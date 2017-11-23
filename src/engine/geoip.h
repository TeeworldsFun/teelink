/* (c) unsigned char*. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at https://github.com/CytraL/HClient */
#ifndef ENGINE_GEOIP_H
#define ENGINE_GEOIP_H

#include "kernel.h"
#include <string>
#include <csignal>
#include "serverbrowser.h"

struct GeoInfo
{
	GeoInfo()
	{
		mem_zero(m_aCountryCode, sizeof(m_aCountryCode));
		mem_zero(m_aCountryName,  sizeof(m_aCountryName));
		mem_zero(m_aTimeZone, sizeof(m_aTimeZone));
	}

	char m_aCountryCode[8];
	char m_aCountryName[32];
	char m_aTimeZone[128];
};

class IGeoIP : public IInterface
{
	MACRO_INTERFACE("geoip", 0)
public:
    struct InfoGeoIPThread
    {
        IGeoIP *m_pGeoIP;
        class CServerInfo *m_pServerInfo;
        class CServerInfoRegv2 *m_pServerInfoReg;

        char m_aIpAddress[256];
    };

    virtual void Search(class CServerInfo *pServerInfo, class CServerInfoRegv2 *pServer) = 0;
    virtual bool IsActive() const = 0;
    virtual void Init() = 0;
};
#endif
