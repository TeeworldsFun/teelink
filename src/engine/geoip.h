/*
    unsigned char*
*/
#ifndef ENGINE_GEOIP_H
#define ENGINE_GEOIP_H

#include "kernel.h"
#include <string>
#include <csignal>

class IGeoIP : public IInterface
{
	MACRO_INTERFACE("geoip", 0)
public:
    struct GeoInfo
    {
        GeoInfo()
        {
            m_CountryCode = "NULL";
        }

        std::string m_CountryCode;
        std::string m_CountryName;
        std::string m_Isp;
    };

    struct InfoGeoIPThread
    {
        IGeoIP *m_pGeoIP;
        IGeoIP::GeoInfo *m_pGeoInfo;
        char ip[64];
    };

    virtual void Search(InfoGeoIPThread *pGeoInfo) = 0;
    virtual bool IsActive() const = 0;
    virtual void Init() = 0;
};
#endif
