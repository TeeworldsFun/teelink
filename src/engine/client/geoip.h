/* (c) unsigned char*. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at https://github.com/CytraL/HClient */
#ifndef ENGINE_CLIENT_GEOIP_H
#define ENGINE_CLIENT_GEOIP_H

#include <base/system.h>
#include <engine/geoip.h>
#include <string>
#include <list>

class CGeoIP : public IGeoIP
{
public:
    CGeoIP();

    void Search(InfoGeoIPThread *pGeoInfo);
    bool IsActive() const { return m_Active; }
    void Init();

protected:
    void *m_pGeoIPThread;
    static NETADDR m_HostAddress;
    static NETSOCKET m_Socket;
    bool m_Active;

private:
    static IGeoIP::GeoInfo GetInfo(std::string ip);
    static void ThreadGeoIP(void *params);
};

#endif
