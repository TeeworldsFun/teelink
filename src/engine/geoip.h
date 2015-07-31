/* (c) unsigned char*. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at https://github.com/CytraL/HClient */
#ifndef ENGINE_GEOIP_H
#define ENGINE_GEOIP_H

#include "kernel.h"
#include <string>
#include <csignal>
#include "serverbrowser.h"

class IGeoIP : public IInterface
{
	MACRO_INTERFACE("geoip", 0)
public:
    struct GeoInfo
    {
        GeoInfo()
        {
        	str_copy(m_aCountryCode, "NULL", sizeof(m_aCountryCode));
        	str_copy(m_aCountryName, "NULL", sizeof(m_aCountryName));
        }

        char m_aCountryCode[8];
        char m_aCountryName[32];
    };

    struct InfoGeoIPThread
    {
        IGeoIP *m_pGeoIP;
        IGeoIP::GeoInfo *m_pGeoInfo;
        CServerInfoRegv2 *m_pServerInfoReg;

        char m_aIpAddress[256];
    };

    virtual void Search(InfoGeoIPThread *pGeoInfo) = 0;
    virtual bool IsActive() const = 0;
    virtual void Init() = 0;
};
#endif
