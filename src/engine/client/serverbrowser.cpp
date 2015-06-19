/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <algorithm> // sort  TODO: remove this

#include <base/math.h>
#include <base/system.h>

#include <engine/shared/config.h>
#include <engine/shared/memheap.h>
#include <engine/shared/network.h>
#include <engine/shared/protocol.h>

#include <engine/storage.h> // H-Client
#include <engine/config.h>
#include <engine/console.h>
#include <engine/friends.h>
#include <engine/masterserver.h>

#include <mastersrv/mastersrv.h>
#include <zlib.h> // H-Client
#include <string> // H-Client
#include <cstdio> // H-Client

#include "serverbrowser.h"
class SortWrap
{
	typedef bool (CServerBrowser::*SortFunc)(int, int) const;
	SortFunc m_pfnSort;
	CServerBrowser *m_pThis;
public:
	SortWrap(CServerBrowser *t, SortFunc f) : m_pfnSort(f), m_pThis(t) {}
	bool operator()(int a, int b) { return (g_Config.m_BrSortOrder ? (m_pThis->*m_pfnSort)(b, a) : (m_pThis->*m_pfnSort)(a, b)); }
};

CServerBrowser::CServerBrowser()
{
	m_pMasterServer = 0;
	m_ppServerlist = 0;
	m_pSortedServerlist = 0;

	m_NumFavoriteServers = 0;

	mem_zero(m_aServerlistIp, sizeof(m_aServerlistIp));

	m_pFirstReqServer = 0; // request list
	m_pLastReqServer = 0;
	m_NumRequests = 0;

	m_NeedRefresh = 0;

	m_NumSortedServers = 0;
	m_NumSortedServersCapacity = 0;
	m_NumServers = 0;
	m_NumServerCapacity = 0;

	m_Sorthash = 0;
	m_aFilterString[0] = 0;
	m_aFilterGametypeString[0] = 0;

	// the token is to keep server refresh separated from each other
	m_CurrentToken = 1;

	m_ServerlistType = 0;
	m_BroadcastTime = 0;
}

void CServerBrowser::SetBaseInfo(class CNetClient *pClient, const char *pNetVersion)
{
	m_pNetClient = pClient;
	str_copy(m_aNetVersion, pNetVersion, sizeof(m_aNetVersion));
	m_pMasterServer = Kernel()->RequestInterface<IMasterServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_pFriends = Kernel()->RequestInterface<IFriends>();
	IConfig *pConfig = Kernel()->RequestInterface<IConfig>();
	if(pConfig)
		pConfig->RegisterCallback(ConfigSaveCallback, this);

    m_pStorage = Kernel()->RequestInterface<IStorage>(); // H-Client
	LoadServerInfo(); //H-Client
}

const CServerInfo *CServerBrowser::SortedGet(int Index) const
{
	if(Index < 0 || Index >= m_NumSortedServers)
		return 0;
	return &m_ppServerlist[m_pSortedServerlist[Index]]->m_Info;
}


bool CServerBrowser::SortCompareName(int Index1, int Index2) const
{
	CServerEntry *a = m_ppServerlist[Index1];
	CServerEntry *b = m_ppServerlist[Index2];
	//	make sure empty entries are listed last
	return (a->m_GotInfo && b->m_GotInfo) || (!a->m_GotInfo && !b->m_GotInfo) ? str_comp_nocase(a->m_Info.m_aName, b->m_Info.m_aName) < 0 :
			a->m_GotInfo ? true : false;
}

bool CServerBrowser::SortCompareMap(int Index1, int Index2) const
{
	CServerEntry *a = m_ppServerlist[Index1];
	CServerEntry *b = m_ppServerlist[Index2];
	return str_comp_nocase(a->m_Info.m_aMap, b->m_Info.m_aMap) < 0;
}

bool CServerBrowser::SortComparePing(int Index1, int Index2) const
{
	CServerEntry *a = m_ppServerlist[Index1];
	CServerEntry *b = m_ppServerlist[Index2];
	return a->m_Info.m_Latency < b->m_Info.m_Latency;
}

bool CServerBrowser::SortCompareGametype(int Index1, int Index2) const
{
	CServerEntry *a = m_ppServerlist[Index1];
	CServerEntry *b = m_ppServerlist[Index2];
	return str_comp_nocase(a->m_Info.m_aGameType, b->m_Info.m_aGameType) < 0;
}

bool CServerBrowser::SortCompareNumPlayers(int Index1, int Index2) const
{
	CServerEntry *a = m_ppServerlist[Index1];
	CServerEntry *b = m_ppServerlist[Index2];
	return a->m_Info.m_NumPlayers < b->m_Info.m_NumPlayers;
}

bool CServerBrowser::SortCompareNumClients(int Index1, int Index2) const
{
	CServerEntry *a = m_ppServerlist[Index1];
	CServerEntry *b = m_ppServerlist[Index2];
	return a->m_Info.m_NumClients < b->m_Info.m_NumClients;
}

void CServerBrowser::Filter()
{
	int i = 0, p = 0;
	m_NumSortedServers = 0;

	// allocate the sorted list
	if(m_NumSortedServersCapacity < m_NumServers)
	{
		if(m_pSortedServerlist)
			mem_free(m_pSortedServerlist);
		m_NumSortedServersCapacity = m_NumServers;
		m_pSortedServerlist = (int *)mem_alloc(m_NumSortedServersCapacity*sizeof(int), 1);
	}

	// filter the servers
	for(i = 0; i < m_NumServers; i++)
	{
		int Filtered = 0;

		if(g_Config.m_BrFilterEmpty && ((g_Config.m_BrFilterSpectators && m_ppServerlist[i]->m_Info.m_NumPlayers == 0) || m_ppServerlist[i]->m_Info.m_NumClients == 0))
			Filtered = 1;
		else if(g_Config.m_BrFilterFull && ((g_Config.m_BrFilterSpectators && m_ppServerlist[i]->m_Info.m_NumPlayers == m_ppServerlist[i]->m_Info.m_MaxPlayers) ||
				m_ppServerlist[i]->m_Info.m_NumClients == m_ppServerlist[i]->m_Info.m_MaxClients))
			Filtered = 1;
		else if(g_Config.m_BrFilterPw && (m_ppServerlist[i]->m_Info.m_Flags&SERVER_FLAG_PASSWORD))
			Filtered = 1;
		else if(g_Config.m_BrFilterPure &&
			(str_comp(m_ppServerlist[i]->m_Info.m_aGameType, "DM") != 0 &&
			str_comp(m_ppServerlist[i]->m_Info.m_aGameType, "TDM") != 0 &&
			str_comp(m_ppServerlist[i]->m_Info.m_aGameType, "CTF") != 0))
		{
			Filtered = 1;
		}
		else if(g_Config.m_BrFilterPureMap &&
			!(str_comp(m_ppServerlist[i]->m_Info.m_aMap, "dm1") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "dm2") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "dm6") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "dm7") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "dm8") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "dm9") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "ctf1") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "ctf2") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "ctf3") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "ctf4") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "ctf5") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "ctf6") == 0 ||
			str_comp(m_ppServerlist[i]->m_Info.m_aMap, "ctf7") == 0)
		)
		{
			Filtered = 1;
		}
		else if(g_Config.m_BrFilterPing < m_ppServerlist[i]->m_Info.m_Latency)
			Filtered = 1;
		else if(g_Config.m_BrFilterCompatversion && str_comp_num(m_ppServerlist[i]->m_Info.m_aVersion, m_aNetVersion, 3) != 0)
			Filtered = 1;
		else if(g_Config.m_BrFilterServerAddress[0] && !str_find_nocase(m_ppServerlist[i]->m_Info.m_aAddress, g_Config.m_BrFilterServerAddress))
			Filtered = 1;
		else if(g_Config.m_BrFilterGametypeStrict && g_Config.m_BrFilterGametype[0] && str_comp_nocase(m_ppServerlist[i]->m_Info.m_aGameType, g_Config.m_BrFilterGametype))
			Filtered = 1;
		else if(!g_Config.m_BrFilterGametypeStrict && g_Config.m_BrFilterGametype[0] && !str_find_nocase(m_ppServerlist[i]->m_Info.m_aGameType, g_Config.m_BrFilterGametype))
			Filtered = 1;
		else
		{
			if(g_Config.m_BrFilterCountry)
			{
				Filtered = 1;
				// match against player country
				for(p = 0; p < m_ppServerlist[i]->m_Info.m_NumClients; p++)
				{
					if(m_ppServerlist[i]->m_Info.m_aClients[p].m_Country == g_Config.m_BrFilterCountryIndex)
					{
						Filtered = 0;
						break;
					}
				}
			}

			if(!Filtered && g_Config.m_BrFilterString[0] != 0)
			{
				int MatchFound = 0;

				m_ppServerlist[i]->m_Info.m_QuickSearchHit = 0;

				// match against server name
				if(str_find_nocase(m_ppServerlist[i]->m_Info.m_aName, g_Config.m_BrFilterString))
				{
					MatchFound = 1;
					m_ppServerlist[i]->m_Info.m_QuickSearchHit |= IServerBrowser::QUICK_SERVERNAME;
				}

				// match against players
				for(p = 0; p < m_ppServerlist[i]->m_Info.m_NumClients; p++)
				{
					if(str_find_nocase(m_ppServerlist[i]->m_Info.m_aClients[p].m_aName, g_Config.m_BrFilterString) ||
						str_find_nocase(m_ppServerlist[i]->m_Info.m_aClients[p].m_aClan, g_Config.m_BrFilterString))
					{
						MatchFound = 1;
						m_ppServerlist[i]->m_Info.m_QuickSearchHit |= IServerBrowser::QUICK_PLAYER;
						break;
					}
				}

				// match against map
				if(str_find_nocase(m_ppServerlist[i]->m_Info.m_aMap, g_Config.m_BrFilterString))
				{
					MatchFound = 1;
					m_ppServerlist[i]->m_Info.m_QuickSearchHit |= IServerBrowser::QUICK_MAPNAME;
				}

				if(!MatchFound)
					Filtered = 1;
			}
		}

		if(Filtered == 0)
		{
			// check for friend
			m_ppServerlist[i]->m_Info.m_FriendState = IFriends::FRIEND_NO;
			for(p = 0; p < m_ppServerlist[i]->m_Info.m_NumClients; p++)
			{
				m_ppServerlist[i]->m_Info.m_aClients[p].m_FriendState = m_pFriends->GetFriendState(m_ppServerlist[i]->m_Info.m_aClients[p].m_aName,
					m_ppServerlist[i]->m_Info.m_aClients[p].m_aClan);
				m_ppServerlist[i]->m_Info.m_FriendState = max(m_ppServerlist[i]->m_Info.m_FriendState, m_ppServerlist[i]->m_Info.m_aClients[p].m_FriendState);
			}

			if(!g_Config.m_BrFilterFriends || m_ppServerlist[i]->m_Info.m_FriendState != IFriends::FRIEND_NO)
				m_pSortedServerlist[m_NumSortedServers++] = i;
		}
	}
}

int CServerBrowser::SortHash() const
{
	int i = g_Config.m_BrSort&0xf;
	i |= g_Config.m_BrFilterEmpty<<4;
	i |= g_Config.m_BrFilterFull<<5;
	i |= g_Config.m_BrFilterSpectators<<6;
	i |= g_Config.m_BrFilterFriends<<7;
	i |= g_Config.m_BrFilterPw<<8;
	i |= g_Config.m_BrSortOrder<<9;
	i |= g_Config.m_BrFilterCompatversion<<10;
	i |= g_Config.m_BrFilterPure<<11;
	i |= g_Config.m_BrFilterPureMap<<12;
	i |= g_Config.m_BrFilterGametypeStrict<<13;
	i |= g_Config.m_BrFilterCountry<<14;
	i |= g_Config.m_BrFilterPing<<15;
	return i;
}

void CServerBrowser::Sort()
{
	int i;

	// create filtered list
	Filter();

	// sort
	if(g_Config.m_BrSort == IServerBrowser::SORT_NAME)
		std::stable_sort(m_pSortedServerlist, m_pSortedServerlist+m_NumSortedServers, SortWrap(this, &CServerBrowser::SortCompareName));
	else if(g_Config.m_BrSort == IServerBrowser::SORT_PING)
		std::stable_sort(m_pSortedServerlist, m_pSortedServerlist+m_NumSortedServers, SortWrap(this, &CServerBrowser::SortComparePing));
	else if(g_Config.m_BrSort == IServerBrowser::SORT_MAP)
		std::stable_sort(m_pSortedServerlist, m_pSortedServerlist+m_NumSortedServers, SortWrap(this, &CServerBrowser::SortCompareMap));
	else if(g_Config.m_BrSort == IServerBrowser::SORT_NUMPLAYERS)
		std::stable_sort(m_pSortedServerlist, m_pSortedServerlist+m_NumSortedServers, SortWrap(this,
					g_Config.m_BrFilterSpectators ? &CServerBrowser::SortCompareNumPlayers : &CServerBrowser::SortCompareNumClients));
	//else if(g_Config.m_BrSort == IServerBrowser::SORT_GAMETYPE)

    // H-Client
    std::stable_sort(m_pSortedServerlist, m_pSortedServerlist+m_NumSortedServers, SortWrap(this, &CServerBrowser::SortCompareGametype));

	// set indexes
	for(i = 0; i < m_NumSortedServers; i++)
		m_ppServerlist[m_pSortedServerlist[i]]->m_Info.m_SortedIndex = i;

	str_copy(m_aFilterGametypeString, g_Config.m_BrFilterGametype, sizeof(m_aFilterGametypeString));
	str_copy(m_aFilterString, g_Config.m_BrFilterString, sizeof(m_aFilterString));
	m_Sorthash = SortHash();
}

void CServerBrowser::RemoveRequest(CServerEntry *pEntry)
{
	if(pEntry->m_pPrevReq || pEntry->m_pNextReq || m_pFirstReqServer == pEntry)
	{
		if(pEntry->m_pPrevReq)
			pEntry->m_pPrevReq->m_pNextReq = pEntry->m_pNextReq;
		else
			m_pFirstReqServer = pEntry->m_pNextReq;

		if(pEntry->m_pNextReq)
			pEntry->m_pNextReq->m_pPrevReq = pEntry->m_pPrevReq;
		else
			m_pLastReqServer = pEntry->m_pPrevReq;

		pEntry->m_pPrevReq = 0;
		pEntry->m_pNextReq = 0;
		m_NumRequests--;
	}
}

CServerBrowser::CServerEntry *CServerBrowser::Find(const NETADDR &Addr)
{
	CServerEntry *pEntry = m_aServerlistIp[Addr.ip[0]];

	for(; pEntry; pEntry = pEntry->m_pNextIp)
	{
		if(net_addr_comp(&pEntry->m_Addr, &Addr) == 0)
			return pEntry;
	}
	return (CServerEntry*)0;
}

void CServerBrowser::QueueRequest(CServerEntry *pEntry)
{
	// add it to the list of servers that we should request info from
	pEntry->m_pPrevReq = m_pLastReqServer;
	if(m_pLastReqServer)
		m_pLastReqServer->m_pNextReq = pEntry;
	else
		m_pFirstReqServer = pEntry;
	m_pLastReqServer = pEntry;
	pEntry->m_pNextReq = 0;
	m_NumRequests++;
}

void CServerBrowser::SetInfo(CServerEntry *pEntry, const CServerInfo &Info)
{
	int Fav = pEntry->m_Info.m_Favorite;
	pEntry->m_Info = Info;
	pEntry->m_Info.m_Favorite = Fav;
	pEntry->m_Info.m_NetAddr = pEntry->m_Addr;

	// all these are just for nice compability
	if(pEntry->m_Info.m_aGameType[0] == '0' && pEntry->m_Info.m_aGameType[1] == 0)
		str_copy(pEntry->m_Info.m_aGameType, "DM", sizeof(pEntry->m_Info.m_aGameType));
	else if(pEntry->m_Info.m_aGameType[0] == '1' && pEntry->m_Info.m_aGameType[1] == 0)
		str_copy(pEntry->m_Info.m_aGameType, "TDM", sizeof(pEntry->m_Info.m_aGameType));
	else if(pEntry->m_Info.m_aGameType[0] == '2' && pEntry->m_Info.m_aGameType[1] == 0)
		str_copy(pEntry->m_Info.m_aGameType, "CTF", sizeof(pEntry->m_Info.m_aGameType));

	/*if(!request)
	{
		pEntry->m_Info.latency = (time_get()-pEntry->request_time)*1000/time_freq();
		RemoveRequest(pEntry);
	}*/

	pEntry->m_GotInfo = 1;
}

CServerBrowser::CServerEntry *CServerBrowser::Add(const NETADDR &Addr)
{
	int Hash = Addr.ip[0];
	CServerEntry *pEntry = 0;
	int i;

	// create new pEntry
	pEntry = (CServerEntry *)m_ServerlistHeap.Allocate(sizeof(CServerEntry));
	mem_zero(pEntry, sizeof(CServerEntry));

	// set the info
	pEntry->m_Addr = Addr;
	pEntry->m_Info.m_NetAddr = Addr;

	pEntry->m_Info.m_Latency = 999;
	net_addr_str(&Addr, pEntry->m_Info.m_aAddress, sizeof(pEntry->m_Info.m_aAddress), true);
	str_copy(pEntry->m_Info.m_aName, pEntry->m_Info.m_aAddress, sizeof(pEntry->m_Info.m_aName));

	// check if it's a favorite
	for(i = 0; i < m_NumFavoriteServers; i++)
	{
		if(net_addr_comp(&Addr, &m_aFavoriteServers[i]) == 0)
			pEntry->m_Info.m_Favorite = 1;
	}

	// add to the hash list
	pEntry->m_pNextIp = m_aServerlistIp[Hash];
	m_aServerlistIp[Hash] = pEntry;

	if(m_NumServers == m_NumServerCapacity)
	{
		CServerEntry **ppNewlist;
		m_NumServerCapacity += 100;
		ppNewlist = (CServerEntry **)mem_alloc(m_NumServerCapacity*sizeof(CServerEntry*), 1);
		mem_copy(ppNewlist, m_ppServerlist, m_NumServers*sizeof(CServerEntry*));
		mem_free(m_ppServerlist);
		m_ppServerlist = ppNewlist;
	}

	// add to list
	m_ppServerlist[m_NumServers] = pEntry;
	pEntry->m_Info.m_ServerIndex = m_NumServers;
	m_NumServers++;

	return pEntry;
}

void CServerBrowser::Set(const NETADDR &Addr, int Type, int Token, const CServerInfo *pInfo)
{
	static int temp = 0;
	CServerEntry *pEntry = 0;
	if(Type == IServerBrowser::SET_MASTER_ADD)
	{
		if(m_ServerlistType != IServerBrowser::TYPE_INTERNET)
			return;
		m_LastPacketTick = 0;
		++temp;
		if(!Find(Addr))
		{
			pEntry = Add(Addr);
			QueueRequest(pEntry);
		}
	}
	else if(Type == IServerBrowser::SET_FAV_ADD)
	{
		if(m_ServerlistType != IServerBrowser::TYPE_FAVORITES)
			return;

		if(!Find(Addr))
		{
			pEntry = Add(Addr);
			QueueRequest(pEntry);
		}
	}
	else if(Type == IServerBrowser::SET_TOKEN)
	{
		if(Token != m_CurrentToken)
			return;

		pEntry = Find(Addr);
		if(!pEntry)
			pEntry = Add(Addr);
		if(pEntry)
		{
			SetInfo(pEntry, *pInfo);
			if(m_ServerlistType == IServerBrowser::TYPE_LAN)
				pEntry->m_Info.m_Latency = min(static_cast<int>((time_get()-m_BroadcastTime)*1000/time_freq()), 999);
			else if (pEntry->m_RequestTime > 0)
			{
				pEntry->m_Info.m_Latency = min(static_cast<int>((time_get()-pEntry->m_RequestTime)*1000/time_freq()), 999);
				pEntry->m_RequestTime = -1; // Request has been answered
			}
			RemoveRequest(pEntry);
		}
	}
	else if(Type == IServerBrowser::SET_HISTORY_ADD)
	{
		if(m_ServerlistType != IServerBrowser::TYPE_HISTORY)
			return;
		m_LastPacketTick = 0;
		++temp;
		if(!Find(Addr))
		{
			pEntry = Add(Addr);
			QueueRequest(pEntry);
		}
	}

	Sort();
}

void CServerBrowser::Refresh(int Type)
{
	// clear out everything
	m_ServerlistHeap.Reset();
	m_NumServers = 0;
	m_NumSortedServers = 0;
	mem_zero(m_aServerlistIp, sizeof(m_aServerlistIp));
	m_pFirstReqServer = 0;
	m_pLastReqServer = 0;
	m_NumRequests = 0;
	m_CurrentMaxRequests = g_Config.m_BrMaxRequests;
	// next token
	m_CurrentToken = (m_CurrentToken+1)&0xff;

	//
	m_ServerlistType = Type;

	if(Type == IServerBrowser::TYPE_LAN)
	{
		unsigned char Buffer[sizeof(SERVERBROWSE_GETINFO)+1];
		CNetChunk Packet;
		int i;

		mem_copy(Buffer, SERVERBROWSE_GETINFO, sizeof(SERVERBROWSE_GETINFO));
		Buffer[sizeof(SERVERBROWSE_GETINFO)] = m_CurrentToken;

		/* do the broadcast version */
		Packet.m_ClientID = -1;
		mem_zero(&Packet, sizeof(Packet));
		Packet.m_Address.type = m_pNetClient->NetType()|NETTYPE_LINK_BROADCAST;
		Packet.m_Flags = NETSENDFLAG_CONNLESS;
		Packet.m_DataSize = sizeof(Buffer);
		Packet.m_pData = Buffer;
		m_BroadcastTime = time_get();

		for(i = 8303; i <= 8310; i++)
		{
			Packet.m_Address.port = i;
			m_pNetClient->Send(&Packet);
		}

		if(g_Config.m_Debug)
			m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvbrowse", "broadcasting for servers");
	}
	else if(Type == IServerBrowser::TYPE_INTERNET)
		m_NeedRefresh = 1;
	else if(Type == IServerBrowser::TYPE_FAVORITES)
	{
		for(int i = 0; i < m_NumFavoriteServers; i++)
			Set(m_aFavoriteServers[i], IServerBrowser::SET_FAV_ADD, -1, 0);
	}
	else if(Type == IServerBrowser::TYPE_HISTORY) // H-Client
	{
		for(int i = 0; i < m_lServInfo.size(); i++)
		{
			NETADDR netAddr;
			mem_zero(&netAddr, sizeof(netAddr));

			netAddr.type = NETTYPE_IPV4;
			std::string host = m_lServInfo[i].m_Address;
			std::size_t npos = host.find_first_of(":");

			sscanf(host.substr(0, npos).c_str(), "%d.%d.%d.%d", &netAddr.ip[0], &netAddr.ip[1], &netAddr.ip[2], &netAddr.ip[3]); // FIXME: warnings
			sscanf(host.substr(npos+1).c_str(), "%d", &netAddr.port); // FIXME: warnings

			Set(netAddr, IServerBrowser::SET_HISTORY_ADD, -1, 0);
		}
	}
}

void CServerBrowser::RequestImpl(const NETADDR &Addr, CServerEntry *pEntry) const
{
	unsigned char Buffer[sizeof(SERVERBROWSE_GETINFO)+1];
	CNetChunk Packet;

	if(g_Config.m_Debug)
	{
		char aAddrStr[NETADDR_MAXSTRSIZE];
		net_addr_str(&Addr, aAddrStr, sizeof(aAddrStr), true);
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf),"requesting server info from %s", aAddrStr);
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvbrowse", aBuf);
	}

	mem_copy(Buffer, SERVERBROWSE_GETINFO, sizeof(SERVERBROWSE_GETINFO));
	Buffer[sizeof(SERVERBROWSE_GETINFO)] = m_CurrentToken;

	Packet.m_ClientID = -1;
	Packet.m_Address = Addr;
	Packet.m_Flags = NETSENDFLAG_CONNLESS;
	Packet.m_DataSize = sizeof(Buffer);
	Packet.m_pData = Buffer;

	m_pNetClient->Send(&Packet);

	if(pEntry)
		pEntry->m_RequestTime = time_get();
}

void CServerBrowser::RequestImpl64(const NETADDR &Addr, CServerEntry *pEntry) const
{
	unsigned char Buffer[sizeof(SERVERBROWSE_GETINFO64)+1];
	CNetChunk Packet;

	if(g_Config.m_Debug)
	{
		char aAddrStr[NETADDR_MAXSTRSIZE];
		net_addr_str(&Addr, aAddrStr, sizeof(aAddrStr), true);
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf),"requesting server info 64 from %s", aAddrStr);
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvbrowse", aBuf);
	}

	mem_copy(Buffer, SERVERBROWSE_GETINFO64, sizeof(SERVERBROWSE_GETINFO64));
	Buffer[sizeof(SERVERBROWSE_GETINFO64)] = m_CurrentToken;

	Packet.m_ClientID = -1;
	Packet.m_Address = Addr;
	Packet.m_Flags = NETSENDFLAG_CONNLESS;
	Packet.m_DataSize = sizeof(Buffer);
	Packet.m_pData = Buffer;

	m_pNetClient->Send(&Packet);

	if(pEntry)
		pEntry->m_RequestTime = time_get();
}

void CServerBrowser::Request(const NETADDR &Addr) const
{
	// Call both because we can't know what kind the server is
	RequestImpl64(Addr, 0);
	RequestImpl(Addr, 0);
}


void CServerBrowser::Update(bool ForceResort)
{
	int64 Timeout = time_freq();
	int64 Now = time_get();
	int Count;
	CServerEntry *pEntry, *pNext;

	// do server list requests
	if(m_NeedRefresh && !m_pMasterServer->IsRefreshing())
	{
		NETADDR Addr;
		CNetChunk Packet;
		int i = 0;

		m_NeedRefresh = 0;
		m_MasterServerCount = -1;
		mem_zero(&Packet, sizeof(Packet));
		Packet.m_ClientID = -1;
		Packet.m_Flags = NETSENDFLAG_CONNLESS;
		Packet.m_DataSize = sizeof(SERVERBROWSE_GETCOUNT);
		Packet.m_pData = SERVERBROWSE_GETCOUNT;

		for(i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
		{
			if(!m_pMasterServer->IsValid(i))
				continue;

			Addr = m_pMasterServer->GetAddr(i);
			m_pMasterServer->SetCount(i, -1);
			Packet.m_Address = Addr;
			m_pNetClient->Send(&Packet);
			if(g_Config.m_Debug)
			{
				dbg_msg("client_srvbrowse", "Count-Request sent to %d", i);
			}
		}
	}

	//Check if all server counts arrived
	if(m_MasterServerCount == -1)
	{
		m_MasterServerCount = 0;
		for(int i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
			{
				if(!m_pMasterServer->IsValid(i))
					continue;
				int Count = m_pMasterServer->GetCount(i);
				if(Count == -1)
				{
				/* ignore Server
					m_MasterServerCount = -1;
					return;
					// we don't have the required server information
					*/
				}
				else
					m_MasterServerCount += Count;
			}
		//request Server-List
		NETADDR Addr;
		CNetChunk Packet;
		mem_zero(&Packet, sizeof(Packet));
		Packet.m_ClientID = -1;
		Packet.m_Flags = NETSENDFLAG_CONNLESS;
		Packet.m_DataSize = sizeof(SERVERBROWSE_GETLIST);
		Packet.m_pData = SERVERBROWSE_GETLIST;

		for(int i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
		{
			if(!m_pMasterServer->IsValid(i))
				continue;

			Addr = m_pMasterServer->GetAddr(i);
			Packet.m_Address = Addr;
			m_pNetClient->Send(&Packet);
		}
		if(g_Config.m_Debug)
		{
			dbg_msg("client_srvbrowse", "ServerCount: %d, requesting server list", m_MasterServerCount);
		}
		m_LastPacketTick = 0;
	}
	else if(m_MasterServerCount > -1)
	{
		m_MasterServerCount = 0;
		for(int i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
			{
				if(!m_pMasterServer->IsValid(i))
					continue;
				int Count = m_pMasterServer->GetCount(i);
				if(Count == -1)
				{
				/* ignore Server
					m_MasterServerCount = -1;
					return;
					// we don't have the required server information
					*/
				}
				else
					m_MasterServerCount += Count;
			}
			//if(g_Config.m_Debug)
			//{
			//	dbg_msg("client_srvbrowse", "ServerCount2: %d", m_MasterServerCount);
			//}
	}
	if(m_MasterServerCount > m_NumRequests  + m_LastPacketTick)
	{
		++m_LastPacketTick;
		return; //wait for more packets
	}
	pEntry = m_pFirstReqServer;
	Count = 0;
	while(1)
	{
		if(!pEntry) // no more entries
			break;
		if(pEntry->m_RequestTime && pEntry->m_RequestTime+Timeout < Now)
		{
			pEntry = pEntry->m_pNextReq;
			continue;
		}
		// no more then 10 concurrent requests
		if(Count == m_CurrentMaxRequests)
			break;

		if(pEntry->m_RequestTime == 0)
		{
			if (pEntry->m_Is64)
				RequestImpl64(pEntry->m_Addr, pEntry);
			else
				RequestImpl(pEntry->m_Addr, pEntry);
		}

		Count++;
		pEntry = pEntry->m_pNextReq;
	}

	if(m_pFirstReqServer && Count == 0 && m_CurrentMaxRequests > 1) //NO More current Server Requests
	{
		//reset old ones
		pEntry = m_pFirstReqServer;
		while(1)
		{
			if(!pEntry) // no more entries
				break;
			pEntry->m_RequestTime = 0;
			pEntry = pEntry->m_pNextReq;
		}

		//update max-requests
		m_CurrentMaxRequests = m_CurrentMaxRequests/2;
		if(m_CurrentMaxRequests < 1)
			m_CurrentMaxRequests = 1;
	}
	else if(Count == 0 && m_CurrentMaxRequests == 1) //we reached the limit, just release all left requests. IF a server sends us a packet, a new request will be added automatically, so we can delete all
	{
		pEntry = m_pFirstReqServer;
		while(1)
		{
			if(!pEntry) // no more entries
				break;
			pNext = pEntry->m_pNextReq;
			RemoveRequest(pEntry);	//release request
			pEntry = pNext;
		}
	}

	// check if we need to resort
	if(m_Sorthash != SortHash() || ForceResort)
		Sort();
}


bool CServerBrowser::IsFavorite(const NETADDR &Addr) const
{
	// search for the address
	int i;
	for(i = 0; i < m_NumFavoriteServers; i++)
	{
		if(net_addr_comp(&Addr, &m_aFavoriteServers[i]) == 0)
			return true;
	}
	return false;
}

void CServerBrowser::AddFavorite(const NETADDR &Addr)
{
	CServerEntry *pEntry;

	if(m_NumFavoriteServers == MAX_FAVORITES)
		return;

	// make sure that we don't already have the server in our list
	for(int i = 0; i < m_NumFavoriteServers; i++)
	{
		if(net_addr_comp(&Addr, &m_aFavoriteServers[i]) == 0)
			return;
	}

	// add the server to the list
	m_aFavoriteServers[m_NumFavoriteServers++] = Addr;
	pEntry = Find(Addr);
	if(pEntry)
		pEntry->m_Info.m_Favorite = 1;

	if(g_Config.m_Debug)
	{
		char aAddrStr[NETADDR_MAXSTRSIZE];
		net_addr_str(&Addr, aAddrStr, sizeof(aAddrStr), true);
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "added fav, %s", aAddrStr);
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client_srvbrowse", aBuf);
	}
}

void CServerBrowser::RemoveFavorite(const NETADDR &Addr)
{
	int i;
	CServerEntry *pEntry;

	for(i = 0; i < m_NumFavoriteServers; i++)
	{
		if(net_addr_comp(&Addr, &m_aFavoriteServers[i]) == 0)
		{
			mem_move(&m_aFavoriteServers[i], &m_aFavoriteServers[i+1], sizeof(NETADDR)*(m_NumFavoriteServers-(i+1)));
			m_NumFavoriteServers--;

			pEntry = Find(Addr);
			if(pEntry)
				pEntry->m_Info.m_Favorite = 0;

			return;
		}
	}
}

bool CServerBrowser::IsRefreshing() const
{
	return m_pFirstReqServer != 0;
}

bool CServerBrowser::IsRefreshingMasters() const
{
	return m_pMasterServer->IsRefreshing();
}


int CServerBrowser::LoadingProgression() const
{
	if(m_NumServers == 0)
		return 0;

	int Servers = m_NumServers;
	int Loaded = m_NumServers-m_NumRequests;
	return 100.0f * Loaded/Servers;
}


void CServerBrowser::ConfigSaveCallback(IConfig *pConfig, void *pUserData)
{
	CServerBrowser *pSelf = (CServerBrowser *)pUserData;

	char aAddrStr[128];
	char aBuffer[256];
	for(int i = 0; i < pSelf->m_NumFavoriteServers; i++)
	{
		net_addr_str(&pSelf->m_aFavoriteServers[i], aAddrStr, sizeof(aAddrStr), true);
		str_format(aBuffer, sizeof(aBuffer), "add_favorite %s", aAddrStr);
		pConfig->WriteLine(aBuffer);
	}
}

//H-Client
bool CServerBrowser::SaveServerInfo()
{
    Storage()->RemoveFile("serverinfo.tw", IStorage::TYPE_SAVE);
    IOHANDLE File = Storage()->OpenFile("serverinfo.tw", IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
		return false;

    //H-Client 1.0.4
    char version[] = { 'H', 'C', '3', '8', '0', 0 };
    io_write(File, version, sizeof(version));
    //

    unsigned long TotalSize = m_lServInfo.size() * sizeof(CServerInfoRegv2);
	unsigned long s = compressBound(TotalSize);
	void *pCompData = mem_alloc(s, 1); // temporary buffer that we use during compression

	int Result = compress((Bytef*)pCompData, &s, (Bytef*)m_lServInfo.base_ptr(), TotalSize); // ignore_convention
	if(Result != Z_OK)
	{
		dbg_msg("datafile", "compression error %d", Result);
		dbg_assert(0, "zlib error");
	}

	io_write(File, &TotalSize, sizeof(TotalSize));
	io_write(File, pCompData, (int)s);
	io_close(File);

	mem_free(pCompData);

	return true;
}

bool CServerBrowser::LoadServerInfo()
{
    IOHANDLE File = Storage()->OpenFile("serverinfo.tw", IOFLAG_READ, IStorage::TYPE_SAVE);
	if(!File)
		return false;

    //Verify Version
    char version[] = { 'H', 'C', '3', '8', '0', 0 };
    io_read(File, version, sizeof(version));
    if (str_comp(version, "HC380") == 0)
    {
    	unsigned long UncompressedSize;
    	io_read(File, &UncompressedSize, sizeof(UncompressedSize));
    	unsigned char *pUncompressedData = (unsigned char*)mem_alloc(UncompressedSize, 1);

    	int ctt = io_tell(File);
    	io_seek(File, 0, IOSEEK_END);
    	int TotalSize = io_tell(File) - ctt;

    	io_seek(File, ctt, IOSEEK_START);
    	unsigned char *pData = (unsigned char*)mem_alloc(TotalSize, 1);
    	io_read(File, pData, TotalSize);

    	uncompress((Bytef*)pUncompressedData, &UncompressedSize, (Bytef*)pData, TotalSize); // ignore_convention

        CServerInfoRegv2 ServReg;
        for (std::size_t cursor = 0; cursor<UncompressedSize-sizeof(CServerInfoRegv2); cursor+=sizeof(CServerInfoRegv2))
        {
        	mem_copy(&ServReg, pUncompressedData+cursor, sizeof(CServerInfoRegv2));
            m_lServInfo.add(ServReg);
        }

        mem_free(pUncompressedData);
        mem_free(pData);
    }
    else if (str_comp(version, "HC104") == 0)
    {
        //Load Num Of Regs
    	int numServInfoRegs = 0;
        io_read(File, &numServInfoRegs, sizeof(numServInfoRegs));

        CServerInfoRegv2 ServReg;
        for (int i=0;i<numServInfoRegs;i++)
        {
            io_read(File, &ServReg, sizeof(CServerInfoReg));
            mem_zero(ServReg.m_aCountryCode, sizeof(ServReg.m_aCountryCode));
            mem_zero(ServReg.m_aCountryName, sizeof(ServReg.m_aCountryName));
            m_lServInfo.add(ServReg);
        }
    }
    else
    {
        //Parse old data
        io_seek(File, 0, IOSEEK_START);

        //Load Num Of Regs
        int numServInfoRegs = 0;
        io_read(File, &numServInfoRegs, sizeof(numServInfoRegs));

        CServerInfoRegOld ServReg;
        for (int i=0;i<numServInfoRegs;i++)
        {
            io_read(File, &ServReg, sizeof(ServReg));

            CServerInfoRegv2 parseReg;
            str_copy(parseReg.m_Address, ServReg.m_Address, sizeof(parseReg.m_Address));
            str_copy(parseReg.m_LastEntry, ServReg.m_LastEntry, sizeof(parseReg.m_LastEntry));
            parseReg.m_NumEntry = ServReg.m_NumEntry;
            parseReg.m_Wins = 0;
            parseReg.m_Losts = 0;
            mem_zero(parseReg.m_aCountryCode, sizeof(parseReg.m_aCountryCode));
            mem_zero(parseReg.m_aCountryName, sizeof(parseReg.m_aCountryName));
            m_lServInfo.add(parseReg);
        }
    }

	io_close(File);
	return true;
}


void CServerBrowser::UpdateServerInfo(const char *address)
{
    int i = 0;

	for (i=0;i<m_lServInfo.size();i++)
	{
	    if (str_comp(m_lServInfo[i].m_Address, address) == 0)
	    {
	        m_lServInfo[i].m_NumEntry++;
            str_timestamp(m_lServInfo[i].m_LastEntry, sizeof(m_lServInfo[i].m_LastEntry));
            break;
	    }
	}

	if (i == m_lServInfo.size())
	{
	    CServerInfoRegv2 nServReg;
	    nServReg.m_NumEntry = 1;
	    str_timestamp(nServReg.m_LastEntry, sizeof(nServReg.m_LastEntry));
	    str_copy(nServReg.m_Address, address, sizeof(nServReg.m_Address));
	    nServReg.m_Wins = 0;
	    nServReg.m_Losts = 0;
        mem_zero(nServReg.m_aCountryCode, sizeof(nServReg.m_aCountryCode));
        mem_zero(nServReg.m_aCountryName, sizeof(nServReg.m_aCountryName));
	    m_lServInfo.add(nServReg);
	}

	return;
}

CServerInfoRegv2* CServerBrowser::GetServerInfoReg(const char *address)
{
    int i = 0;

	for (i=0;i<m_lServInfo.size();i++)
	{
	    if (str_comp(m_lServInfo[i].m_Address, address) == 0)
            return &m_lServInfo[i];
	}

	return 0x0;
}

//
