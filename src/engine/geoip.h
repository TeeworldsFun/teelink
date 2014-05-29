/*
    unsigned char*
*/
#ifndef ENGINE_GEOIP_H
#define ENGINE_GEOIP_H

#include "kernel.h"
#include <string>

class IGeoIP : public IInterface
{
	MACRO_INTERFACE("geoip", 0)
public:
    struct GeoInfo {
        GeoInfo() {
            m_CountryCode = "NULL";
        }

        std::string m_CountryCode;
        std::string m_CountryName;
        int m_RegionCode;
        std::string m_RegionName;
        std::string m_City;
        std::string m_ZipCode;
    };

    virtual void GetInfo(std::string ip, IGeoIP::GeoInfo *geoInfo) = 0;
};

struct InfoGeoIPThread {
    IGeoIP *m_pGeoIP;
    IGeoIP::GeoInfo *m_pGeoInfo;
    char ip[64];
};

void ThreadGeoIP(void *params);
#endif
