/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>

#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include "skins.h"
#include <string> // H-Client
#include <algorithm> // H-Client
#include <cstdio> // H-Client

int CSkins::SkinScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CSkins *pSelf = (CSkins *)pUser;
	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".png") != 0)
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
	for(int i = 0; i < m_aSkins.size(); i++)
	{
		if(str_comp(m_aSkins[i].m_aName, pName) == 0)
			return i;
	}

    if (tryDownload)
        return DownloadSkin(pName); // H-Client

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
int CSkins::DownloadSkin(const char *pName)
{
    dbg_msg("skins", "Try download '%s'...", pName);

    NETSOCKET sockDDNet;
    NETADDR naddDDNet, bindAddr;
    mem_zero(&naddDDNet, sizeof(naddDDNet));
    mem_zero(&bindAddr, sizeof(bindAddr));
    bindAddr.type = NETTYPE_IPV4;

    if (net_host_lookup("ddnet.tw", &naddDDNet, NETTYPE_IPV4) != 0)
    {
        dbg_msg("skins", "Error can't found DDNet DataBase :(", pName);
        return -1;
    }
    naddDDNet.port = 80;

    sockDDNet = net_tcp_create(bindAddr);
    if (net_tcp_connect(sockDDNet, &naddDDNet) != 0)
    {
        dbg_msg("skins", "Error can't connect with DDNet DataBase :(", pName);
        net_tcp_close(sockDDNet);
        return -1;
    }

    IOHANDLE dstFile = NULL;
    char fullName[255] = {0};
    str_format(fullName, sizeof(fullName), "skins/%s.png", pName);

    char aNetBuff[255] = {0};
    str_format(aNetBuff, sizeof(aNetBuff), "GET /skins/skin/%s.png HTTP/1.0\r\nHost: ddnet.tw\r\n\r\n", pName);
	net_tcp_send(sockDDNet, aNetBuff, str_length(aNetBuff));

	std::string NetData;
	int TotalRecv = 0;
	int TotalBytes = 0;
	int CurrentRecv = 0;
	int nlCount = 0;
	while ((CurrentRecv = net_tcp_recv(sockDDNet, aNetBuff, sizeof(aNetBuff))) > 0)
	{
		for (int i=0; i<CurrentRecv ; i++)
		{
			if (nlCount < 2)
			{
				if (aNetBuff[i] == '\r' || aNetBuff[i] == '\n')
				{
				    ++nlCount;
					if (NetData.size() > 0)
					{
                        std::transform(NetData.begin(), NetData.end(), NetData.begin(), ::tolower);
                        if (NetData.find("404 not found") != std::string::npos)
                        {
                            dbg_msg("skins", "Can't found '%s' on DDNet DataBase...", pName);
                            net_tcp_close(sockDDNet);
                            return -1;
                        }
                        else if (NetData.find("content-length:") != std::string::npos)
                        {
                            sscanf(NetData.c_str(), "content-length:%d", &TotalBytes);
                            if (TotalBytes == 0)
                                sscanf(NetData.c_str(), "content-length: %d", &TotalBytes);
                        }

                        NetData.clear();
					}

					if (aNetBuff[i] == '\r') ++i;
					continue;
				}

                nlCount = 0;
                NetData += aNetBuff[i];
			}
			else
			{
			    if (nlCount == 2) // FIXE: Ugly check :/
                {
                    if (TotalBytes <= 0)
                    {
                        dbg_msg("skins", "Error downloading '%s'...", pName);
                        break;
                    }

                    char aCompleteFilename[512];
                    dstFile = Storage()->OpenFile(fullName, IOFLAG_WRITE, IStorage::TYPE_ALL, aCompleteFilename, sizeof(aCompleteFilename));
                    if(!dstFile)
                    {
                        dbg_msg("skins", "Error creating '%s'...", fullName);
                        net_tcp_close(sockDDNet);
                        return -1;
                    }

                    ++nlCount;
                }

				io_write(dstFile, &aNetBuff[i], 1);

				TotalRecv++;
				if (TotalRecv == TotalBytes)
					break;
			}
		}
	}

	net_tcp_close(sockDDNet);

    if (dstFile)
    {
        io_close(dstFile);

        dbg_msg("skins", "'%s' downloaded successfull :)", pName);
        str_format(fullName, sizeof(fullName), "%s.png", pName);
        return LoadSkinFromFile("skins", fullName, IStorage::TYPE_ALL);
    }

    return -1;
}
