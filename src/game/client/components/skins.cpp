/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>

#include <engine/engine.h> // H-Client
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>
#include <engine/shared/http_downloader.h> // H-Client

#include "skins.h"
#include <string> // H-Client
#include <algorithm> // H-Client
#include <cstdio> // H-Client

// H-Client
int ThreadDownloadSkin(void *params)
{
    CSkins::InfoDownloadSkinThread *pInfoThread = static_cast<CSkins::InfoDownloadSkinThread*>(params);
    if (!pInfoThread || !pInfoThread->m_pSkins || pInfoThread->m_SkinName[0] == 0)
    	return -1;

    lock_wait(pInfoThread->m_pSkins->m_DownloadLock);
    pInfoThread->m_pSkins->DownloadSkin(pInfoThread->m_SkinName);
    lock_unlock(pInfoThread->m_pSkins->m_DownloadLock);

    return 0;
}
//

int CSkins::SkinScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CSkins *pSelf = (CSkins *)pUser;
	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".png") != 0)
		return 0;

	// Check image status
	char aSinName[128]={0};
	str_copy(aSinName, pName, l-4);
	if (pSelf->IsDownloding(aSinName))
		return 0;

    pSelf->LoadSkinFromFile("skins", pName, DirType);

	return 0;
}

int CSkins::LoadSkinFromFile(const char *pPath, const char *pName, int DirType)
{
    char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "%s/%s", pPath, pName);

	CImageInfo Info;
	if(!Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load skin from %s", pPath);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return -1;
	}

	CSkin Skin;
	Skin.m_OrgTexture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);

	int BodySize = 96; // body size
	unsigned char *d = (unsigned char *)Info.m_pData;
	int Pitch = Info.m_Width*4;

	// dig out blood color
	{
		int aColors[3] = {0};
		for(int y = 0; y < BodySize; y++)
			for(int x = 0; x < BodySize; x++)
			{
				if(d[y*Pitch+x*4+3] > 128)
				{
					aColors[0] += d[y*Pitch+x*4+0];
					aColors[1] += d[y*Pitch+x*4+1];
					aColors[2] += d[y*Pitch+x*4+2];
				}
			}

		Skin.m_BloodColor = normalize(vec3(aColors[0], aColors[1], aColors[2]));
	}

	// create colorless version
	int Step = Info.m_Format == CImageInfo::FORMAT_RGBA ? 4 : 3;

	// make the texture gray scale
	for(int i = 0; i < Info.m_Width*Info.m_Height; i++)
	{
		int v = (d[i*Step]+d[i*Step+1]+d[i*Step+2])/3;
		d[i*Step] = v;
		d[i*Step+1] = v;
		d[i*Step+2] = v;
	}


	int Freq[256] = {0};
	int OrgWeight = 0;
	int NewWeight = 192;

	// find most common frequence
	for(int y = 0; y < BodySize; y++)
		for(int x = 0; x < BodySize; x++)
		{
			if(d[y*Pitch+x*4+3] > 128)
				Freq[d[y*Pitch+x*4]]++;
		}

	for(int i = 1; i < 256; i++)
	{
		if(Freq[OrgWeight] < Freq[i])
			OrgWeight = i;
	}

	// reorder
	int InvOrgWeight = 255-OrgWeight;
	int InvNewWeight = 255-NewWeight;
	for(int y = 0; y < BodySize; y++)
		for(int x = 0; x < BodySize; x++)
		{
			int v = d[y*Pitch+x*4];
			if(v <= OrgWeight)
				v = (int)(((v/(float)OrgWeight) * NewWeight));
			else
				v = (int)(((v-OrgWeight)/(float)InvOrgWeight)*InvNewWeight + NewWeight);
			d[y*Pitch+x*4] = v;
			d[y*Pitch+x*4+1] = v;
			d[y*Pitch+x*4+2] = v;
		}

	Skin.m_ColorTexture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);

	// set skin data
	str_copy(Skin.m_aName, pName, min((int)sizeof(Skin.m_aName), str_length(pName)-3));
	if(g_Config.m_Debug)
	{
		str_format(aBuf, sizeof(aBuf), "load skin %s", Skin.m_aName);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	}

	return m_aSkins.add(Skin);
}

void CSkins::OnInit()
{
	m_DownloadLock = lock_create();

	// load skins
	m_aSkins.clear();
	Storage()->ListDirectory(IStorage::TYPE_ALL, "skins", SkinScan, this);
	if(!m_aSkins.size())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "failed to load skins. folder='skins/'");
		CSkin DummySkin;
		DummySkin.m_OrgTexture = -1;
		DummySkin.m_ColorTexture = -1;
		str_copy(DummySkin.m_aName, "dummy", sizeof(DummySkin.m_aName));
		DummySkin.m_BloodColor = vec3(1.0f, 1.0f, 1.0f);
		m_aSkins.add(DummySkin);
	}
}

int CSkins::Num()
{
	return m_aSkins.size();
}

const CSkins::CSkin *CSkins::Get(int Index)
{
	return &m_aSkins[max(0, Index%m_aSkins.size())];
}

int CSkins::Find(const char *pName, bool tryDownload)
{
    for (std::map<std::string,bool>::iterator it = m_DownloadedSkinsSet.begin(); it != m_DownloadedSkinsSet.end();) // H-Client: Check if Need Load
    {
        if (!it->second)
        {
            ++it;
            continue;
        }

        char fullName[128];
        str_format(fullName, sizeof(fullName), "%s.png", it->first.c_str());
        LoadSkinFromFile("skins", fullName, IStorage::TYPE_ALL);
        m_DownloadedSkinsSet.erase(it++);
    }

	for(int i = 0; i < m_aSkins.size(); i++)
	{
		if(str_comp(m_aSkins[i].m_aName, pName) == 0)
			return i;
	}

    if (g_Config.m_hcAutoDownloadSkins && Client()->State() != IClient::STATE_DEMOPLAYBACK && tryDownload) // H-Client
    {
		char aSanitizeSkinName[64];
		str_copy(aSanitizeSkinName, pName, sizeof(aSanitizeSkinName));
		str_sanitize_cc(aSanitizeSkinName);
		AddDownloadJob(aSanitizeSkinName);
    }

    return -1;
}

vec3 CSkins::GetColorV3(int v)
{
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
}

vec4 CSkins::GetColorV4(int v)
{
	vec3 r = GetColorV3(v);
	return vec4(r.r, r.g, r.b, 1.0f);
}

// H-Client
void CSkins::DownloadSkin(const char *pName)
{
	// Check if skins are in processing state
    std::string sName(pName);
    if (m_DownloadedSkinsSet.size() > 0 && m_DownloadedSkinsSet.find(sName) != m_DownloadedSkinsSet.end())
        return;

    m_DownloadedSkinsSet.insert(std::pair<std::string,bool>(sName, false));

    const unsigned downloadSpeed = clamp(atoi(g_Config.m_hcAutoDownloadSkinsSpeed), 0, 2048) * 1024;
    char aUrl[255];
    str_format(aUrl, sizeof(aUrl), "https://ddnet.org/skins/skin/%s.png", pName);
    char aDest[255], aCompleteFilename[512];
    str_format(aDest, sizeof(aDest), "skins/%s.png", pName);
    Storage()->GetPath(IStorage::TYPE_SAVE+1, aDest, aCompleteFilename, sizeof(aCompleteFilename));

    CHttpDownloader::NETDOWNLOADINFO DownloadStatus;
    if (CHttpDownloader::GetToFile(aUrl, aCompleteFilename, &DownloadStatus, 3, downloadSpeed))
    {
    	m_DownloadedSkinsSet.find(sName)->second = true;
    	dbg_msg("skins", "'%s' downloaded successfully :)", pName);
    }
    else
    {
    	m_DownloadedSkinsSet.erase(m_DownloadedSkinsSet.find(sName));
    	dbg_msg("skins", "Can't download '%s' from DDNet DataBase :(", pName);
    }
}

void CSkins::AddDownloadJob(const char *name)
{
	int UsableIndex = -1;

	for (unsigned i=0; i<MAX_DOWNLOADS; i++)
	{
		if (m_Jobs[i].CurrentStatus() == CJob::STATE_DONE)
		{
			UsableIndex = i;
			return;
		}
		else if (str_comp(m_InfoThreads[i].m_SkinName, name) == 0)
			return;
	}

	if (UsableIndex >= 0)
	{
		m_InfoThreads[UsableIndex].m_pSkins = this;
		str_copy(m_InfoThreads[UsableIndex].m_SkinName, name, sizeof(m_InfoThreads[UsableIndex].m_SkinName));
		m_pClient->Engine()->AddJob(&m_Jobs[UsableIndex], ThreadDownloadSkin, &m_InfoThreads[UsableIndex]);
	}
}
