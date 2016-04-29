/* (c) unsigned char*. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at https://github.com/CytraL/HClient */
#include <base/math.h>
#include <base/system.h>
#include <game/version.h>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <engine/shared/config.h>
#include <engine/shared/http_downloader.h>
#include <engine/external/json-parser/json.h>
#include "geoip.h"


void CGeoIP::Init()
{
	m_JobPool.Init(1);
}

void CGeoIP::Search(CServerInfo *pServerInfo, CServerInfoRegv2 *pServerReg)
{
	for (unsigned i = 0; i<3; i++)
	{
		if (m_aGeoJobs[i].CurrentStatus() != CJob::STATE_RUNNING)
		{
			InfoGeoIPThread *pGeoThread = &m_aInfoThreads[i];
			str_copy(pGeoThread->m_aIpAddress, pServerInfo->m_aAddress, sizeof(pGeoThread->m_aIpAddress));
			pGeoThread->m_pGeoIP = this;
			pGeoThread->m_pServerInfo = pServerInfo;
			pGeoThread->m_pServerInfoReg = pServerReg;

			m_JobPool.Remove(&m_aGeoJobs[i]);
			m_JobPool.Add(&m_aGeoJobs[i], ThreadGeoIP, pGeoThread);
			break;
		}

	}
}

GeoInfo CGeoIP::GetInfo(std::string ip)
{
    GeoInfo rInfo;
    char aUrl[128];

    dbg_msg("GeoIP", "Searching geolocation of '%s'...", ip.c_str());

    //Format URL
    str_format(aUrl, sizeof(aUrl), "http://ip-api.com/json/%s", ip.c_str());

    //read data
    unsigned FileSize = 0;
    char *pHttpData = CHttpDownloader::GetToMemory(aUrl, &FileSize, 1);

    if (pHttpData && FileSize > 0)
    {
		// parse json data
		json_settings JsonSettings;
		mem_zero(&JsonSettings, sizeof(JsonSettings));
		char aError[256];
		json_value *pJsonData = json_parse_ex(&JsonSettings, pHttpData, FileSize, aError);
		if (pJsonData == 0)
		{
			dbg_msg("GeoIP", "Error: %s", aError);
			return rInfo;
		}

		// generate configurations
		const json_value &countryCode = (*pJsonData)["countryCode"];
		if (countryCode.type == json_string) str_copy(rInfo.m_aCountryCode, (const char *)countryCode, sizeof(rInfo.m_aCountryCode));
		const json_value &countryName = (*pJsonData)["country"];
		if (countryName.type == json_string) str_copy(rInfo.m_aCountryName, (const char *)countryName, sizeof(rInfo.m_aCountryName));
		//const json_value &isp = (*pJsonData)["isp"];
		//if (isp.type == json_string) geoInfo->m_Isp = (const char *)isp;
		json_value_free(pJsonData);
    }

    delete pHttpData;
	return rInfo;
}

int CGeoIP::ThreadGeoIP(void *params)
{
    InfoGeoIPThread *pInfoThread = static_cast<InfoGeoIPThread*>(params);
    std::string host = pInfoThread->m_aIpAddress;
    GeoInfo info = GetInfo(host.substr(0, host.find_first_of(":")).c_str());

    if (pInfoThread->m_pServerInfoReg)
    {
    	str_copy(pInfoThread->m_pServerInfoReg->m_aCountryCode, info.m_aCountryCode, sizeof(pInfoThread->m_pServerInfoReg->m_aCountryCode));
    	str_copy(pInfoThread->m_pServerInfoReg->m_aCountryName, info.m_aCountryName, sizeof(pInfoThread->m_pServerInfoReg->m_aCountryName));
    }

	str_copy(pInfoThread->m_pServerInfo->m_aCountryCode, info.m_aCountryCode, sizeof(pInfoThread->m_pServerInfo->m_aCountryCode));
	str_copy(pInfoThread->m_pServerInfo->m_aCountryName, info.m_aCountryName[0]!=0?info.m_aCountryName:"NULL", sizeof(pInfoThread->m_pServerInfo->m_aCountryName));

    return 0;
}

