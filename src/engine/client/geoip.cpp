#include <base/math.h>
#include <base/system.h>
#include <game/version.h>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <engine/shared/config.h>
#include <engine/external/json-parser/json.h>
#include "geoip.h"

static NETSOCKET invalid_socket = {NETTYPE_INVALID, -1, -1};

static LOCK m_GeoIPLock = 0;

CGeoIP::CGeoIP()
{
    m_GeoIPLock = lock_create();
}

void CGeoIP::GetInfo(std::string ip, IGeoIP::GeoInfo *geoInfo)
{
    dbg_msg("GeoIP", "Searching geolocation of '%s'...", ip.c_str());

    NETSOCKET Socket = invalid_socket;
    NETADDR HostAddress, bindAddr;
    mem_zero(&HostAddress, sizeof(HostAddress));
    mem_zero(&bindAddr, sizeof(bindAddr));
    char aNetBuff[1024];
    std::string jsonData;

    //Lookup
    if(net_host_lookup("www.telize.com", &HostAddress, NETTYPE_IPV4) != 0)
    {
        dbg_msg("GeoIP","ERROR: Can't run host lookup.");
        geoInfo->m_CountryCode = "NULL";
        return;
    }
    HostAddress.port = 80;

    //Connect
    bindAddr.type = NETTYPE_IPV4;
    Socket = net_tcp_create(bindAddr);
    if(net_tcp_connect(Socket, &HostAddress) != 0)
    {
        dbg_msg("GeoIP","ERROR: Can't connect.");
        net_tcp_close(Socket);
        geoInfo->m_CountryCode = "NULL";
        return;
    }

    //Send request
    str_format(aNetBuff, sizeof(aNetBuff), "GET /geoip/%s HTTP/1.0\r\nHost: www.telize.com\r\n\r\n", ip.c_str());
    net_tcp_send(Socket, aNetBuff, strlen(aNetBuff));

    //read data
    std::string NetData;
    int TotalRecv = 0;
    int TotalBytes = 0;
    int CurrentRecv = 0;
    bool isHead = true;
    int enterCtrl = 0;
    while ((CurrentRecv = net_tcp_recv(Socket, aNetBuff, sizeof(aNetBuff))) > 0)
    {
        for (int i=0; i<CurrentRecv ; i++)
        {
            if (isHead)
            {
                if (aNetBuff[i]=='\n')
                {
                    enterCtrl++;
                    if (enterCtrl == 2)
                    {
                        isHead = false;
                        NetData.clear();
                        continue;
                    }

                    std::transform(NetData.begin(), NetData.end(), NetData.begin(), ::tolower);
					if (NetData.find("content-length:") != std::string::npos)
                    {
                        sscanf(NetData.c_str(), "content-length:%d", &TotalBytes);
                        if (TotalBytes == 0)
                            sscanf(NetData.c_str(), "content-length: %d", &TotalBytes);
                    }

                    NetData.clear();
                    continue;
                }
                else if (aNetBuff[i]!='\r')
                    enterCtrl=0;

                NetData+=aNetBuff[i];
            }
            else
            {
                if (TotalBytes == 0)
                {
                    net_tcp_close(Socket);
                    dbg_msg("GeoIP","ERROR: Error with size received data.");
                    geoInfo->m_CountryCode = "NULL";
                    return;
                }

                jsonData += aNetBuff[i];

                TotalRecv++;
                if (TotalRecv == TotalBytes)
                    break;
            }
        }
    }

    //Finish
    net_tcp_close(Socket);

    // parse json data
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	json_value *pJsonData = json_parse_ex(&JsonSettings, jsonData.c_str(), jsonData.length(), aError);
	if (pJsonData == 0)
	{
		dbg_msg("GeoIP", "Error: %s", aError);
		return;
	}

	// generate configurations
	const json_value &countryCode = (*pJsonData)["country_code"];
	if (countryCode.type == json_string) geoInfo->m_CountryCode = (const char *)countryCode;
	const json_value &countryName = (*pJsonData)["country"];
	if (countryName.type == json_string) geoInfo->m_CountryName = (const char *)countryName;
	const json_value &isp = (*pJsonData)["isp"];
	if (isp.type == json_string) geoInfo->m_Isp = (const char *)isp;
}

void ThreadGeoIP(void *params)
{
    InfoGeoIPThread *pInfoThread = static_cast<InfoGeoIPThread*>(params);

    lock_wait(m_GeoIPLock);
    pInfoThread->m_pGeoIP->GetInfo(pInfoThread->ip, pInfoThread->m_pGeoInfo);
    lock_release(m_GeoIPLock);
}

