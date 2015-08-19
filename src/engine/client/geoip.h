/* (c) unsigned char*. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at https://github.com/CytraL/HClient */
#ifndef ENGINE_CLIENT_GEOIP_H
#define ENGINE_CLIENT_GEOIP_H

#include <base/system.h>
#include <engine/geoip.h>
#include <engine/shared/jobs.h>
#include <string>
#include <list>

class CGeoIP : public IGeoIP
{
public:
    void Search(CServerInfo *pServerInfo, CServerInfoRegv2 *pServer);
    bool IsActive() const {
    	for (unsigned i = 0; i<3; i++)
    	{
    	 if (m_aGeoJobs[i].Status() != CJob::STATE_DONE)
    		 return true;
    	}

    	return false;
    }
    void Init();

protected:
    InfoGeoIPThread m_aInfoThreads[3];
    class CJob m_aGeoJobs[3];
    class CJobPool m_JobPool;

private:
    static GeoInfo GetInfo(std::string ip);
    static int ThreadGeoIP(void *params);
};

#endif
