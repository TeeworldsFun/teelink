/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <engine/storage.h> //H-Client
#include <game/generated/protocol.h>
#include <base/vmath.h>
#include <game/client/render.h>
#include "voting.h"

void CVoting::ConCallvote(IConsole::IResult *pResult, void *pUserData)
{
	CVoting *pSelf = (CVoting*)pUserData;
	pSelf->Callvote(pResult->GetString(0), pResult->GetString(1), pResult->NumArguments() > 2 ? pResult->GetString(2) : "");
}

void CVoting::ConVote(IConsole::IResult *pResult, void *pUserData)
{
	CVoting *pSelf = (CVoting *)pUserData;
	if (pSelf->m_LastVote != 0)
        return;

	if(str_comp_nocase(pResult->GetString(0), "yes") == 0)
		pSelf->m_LastVote = 1;

	else if(str_comp_nocase(pResult->GetString(0), "no") == 0)
		pSelf->m_LastVote = -1;

	pSelf->Vote(pSelf->m_LastVote);
}

void CVoting::Callvote(const char *pType, const char *pValue, const char *pReason)
{
	CNetMsg_Cl_CallVote Msg = {0};
	Msg.m_Type = pType;
	Msg.m_Value = pValue;
	Msg.m_Reason = pReason;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	m_LastVote = 1;
}

void CVoting::CallvoteSpectate(int ClientID, const char *pReason, bool ForceVote)
{
	if(ForceVote)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "set_team %d -1", ClientID);
		Client()->Rcon(aBuf);
	}
	else
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "%d", ClientID);
		Callvote("spectate", aBuf, pReason);
	}
}

void CVoting::CallvoteKick(int ClientID, const char *pReason, bool ForceVote)
{
	if(ForceVote)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "force_vote kick %d %s", ClientID, pReason);
		Client()->Rcon(aBuf);
	}
	else
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "%d", ClientID);
		Callvote("kick", aBuf, pReason);
	}
}

void CVoting::CallvoteOption(int OptionID, const char *pReason, bool ForceVote)
{
	CVoteOptionClient *pOption = m_pFirst;
	while(pOption && OptionID >= 0)
	{
		if(OptionID == 0)
		{
			if(ForceVote)
			{
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "force_vote option \"%s\" %s", pOption->m_aDescription, pReason);
				Client()->Rcon(aBuf);
			}
			else
				Callvote("option", pOption->m_aDescription, pReason);
			break;
		}

		OptionID--;
		pOption = pOption->m_pNext;
	}
}

void CVoting::RemovevoteOption(int OptionID)
{
	CVoteOptionClient *pOption = m_pFirst;
	while(pOption && OptionID >= 0)
	{
		if(OptionID == 0)
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "remove_vote \"%s\"", pOption->m_aDescription);
			Client()->Rcon(aBuf);
			break;
		}

		OptionID--;
		pOption = pOption->m_pNext;
	}
}

void CVoting::AddvoteOption(const char *pDescription, const char *pCommand)
{
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "add_vote \"%s\" %s", pDescription, pCommand);
	Client()->Rcon(aBuf);
}

void CVoting::Vote(int v)
{
	CNetMsg_Cl_Vote Msg = {v};
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

CVoting::CVoting()
{
	ClearOptions();
	OnReset();
}

void CVoting::AddOption(const char *pDescription)
{
	CVoteOptionClient *pOption;
	if(m_pRecycleFirst)
	{
		pOption = m_pRecycleFirst;
		m_pRecycleFirst = m_pRecycleFirst->m_pNext;
		if(m_pRecycleFirst)
			m_pRecycleFirst->m_pPrev = 0;
		else
			m_pRecycleLast = 0;
	}
	else
		pOption = (CVoteOptionClient *)m_Heap.Allocate(sizeof(CVoteOptionClient));

	pOption->m_pNext = 0;
	pOption->m_pPrev = m_pLast;
	if(pOption->m_pPrev)
		pOption->m_pPrev->m_pNext = pOption;
	m_pLast = pOption;
	if(!m_pFirst)
		m_pFirst = pOption;

	str_copy(pOption->m_aDescription, pDescription, sizeof(pOption->m_aDescription));
	++m_NumVoteOptions;
}

void CVoting::ClearOptions()
{
	m_Heap.Reset();

	m_NumVoteOptions = 0;
	m_pFirst = 0;
	m_pLast = 0;

	m_pRecycleFirst = 0;
	m_pRecycleLast = 0;
}

void CVoting::OnReset()
{
	m_Closetime = 0;
	m_aDescription[0] = 0;
	m_aReason[0] = 0;
	m_Yes = m_No = m_Pass = m_Total = 0;
	m_Voted = 0;
	//H-Client
	m_LastVote = 0;
	m_State = STATE_NORMAL;
	m_offSetX = 0.0f;
	m_VoteTargetClientID = -1;
	m_VoteType = TYPE_NONE;
	mem_zero(m_aVoteTargetMapName, sizeof(m_aVoteTargetMapName));
	//
}

void CVoting::OnConsoleInit()
{
	Console()->Register("callvote", "ss?r", CFGFLAG_CLIENT, ConCallvote, this, "Call vote");
	Console()->Register("vote", "r", CFGFLAG_CLIENT, ConVote, this, "Vote yes/no");
}

void CVoting::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_VOTESET)
	{
		OnReset();

		CNetMsg_Sv_VoteSet *pMsg = (CNetMsg_Sv_VoteSet *)pRawMsg;
		if(pMsg->m_Timeout)
		{
			str_copy(m_aDescription, pMsg->m_pDescription, sizeof(m_aDescription));
			str_copy(m_aReason, pMsg->m_pReason, sizeof(m_aReason));
			m_Closetime = time_get() + time_freq() * pMsg->m_Timeout;

			// H-Client
			AnalizeVote();

			if (g_Config.m_hcAutoVoteNoAction && m_VoteTargetClientID == m_pClient->m_Snap.m_LocalClientID)
			{
				m_LastVote = -1;
				Vote(m_LastVote);
			}
		}
	}
	else if(MsgType == NETMSGTYPE_SV_VOTESTATUS)
	{
		CNetMsg_Sv_VoteStatus *pMsg = (CNetMsg_Sv_VoteStatus *)pRawMsg;
		m_Yes = pMsg->m_Yes;
		m_No = pMsg->m_No;
		m_Pass = pMsg->m_Pass;
		m_Total = pMsg->m_Total;
	}
	else if(MsgType == NETMSGTYPE_SV_VOTECLEAROPTIONS)
	{
		ClearOptions();
	}
	else if(MsgType == NETMSGTYPE_SV_VOTEOPTIONLISTADD)
	{
		CNetMsg_Sv_VoteOptionListAdd *pMsg = (CNetMsg_Sv_VoteOptionListAdd *)pRawMsg;
		int NumOptions = pMsg->m_NumOptions;
		for(int i = 0; i < NumOptions; ++i)
		{
			switch(i)
			{
			case 0: AddOption(pMsg->m_pDescription0); break;
			case 1: AddOption(pMsg->m_pDescription1); break;
			case 2: AddOption(pMsg->m_pDescription2); break;
			case 3: AddOption(pMsg->m_pDescription3); break;
			case 4: AddOption(pMsg->m_pDescription4); break;
			case 5: AddOption(pMsg->m_pDescription5); break;
			case 6: AddOption(pMsg->m_pDescription6); break;
			case 7: AddOption(pMsg->m_pDescription7); break;
			case 8: AddOption(pMsg->m_pDescription8); break;
			case 9: AddOption(pMsg->m_pDescription9); break;
			case 10: AddOption(pMsg->m_pDescription10); break;
			case 11: AddOption(pMsg->m_pDescription11); break;
			case 12: AddOption(pMsg->m_pDescription12); break;
			case 13: AddOption(pMsg->m_pDescription13); break;
			case 14: AddOption(pMsg->m_pDescription14);
			}
		}
	}
	else if(MsgType == NETMSGTYPE_SV_VOTEOPTIONADD)
	{
		CNetMsg_Sv_VoteOptionAdd *pMsg = (CNetMsg_Sv_VoteOptionAdd *)pRawMsg;
		AddOption(pMsg->m_pDescription);
	}
	else if(MsgType == NETMSGTYPE_SV_VOTEOPTIONREMOVE)
	{
		CNetMsg_Sv_VoteOptionRemove *pMsg = (CNetMsg_Sv_VoteOptionRemove *)pRawMsg;

		for(CVoteOptionClient *pOption = m_pFirst; pOption; pOption = pOption->m_pNext)
		{
			if(str_comp(pOption->m_aDescription, pMsg->m_pDescription) == 0)
			{
				// remove it from the list
				if(m_pFirst == pOption)
					m_pFirst = m_pFirst->m_pNext;
				if(m_pLast == pOption)
					m_pLast = m_pLast->m_pPrev;
				if(pOption->m_pPrev)
					pOption->m_pPrev->m_pNext = pOption->m_pNext;
				if(pOption->m_pNext)
					pOption->m_pNext->m_pPrev = pOption->m_pPrev;
				--m_NumVoteOptions;

				// add it to recycle list
				pOption->m_pNext = 0;
				pOption->m_pPrev = m_pRecycleLast;
				if(pOption->m_pPrev)
					pOption->m_pPrev->m_pNext = pOption;
				m_pRecycleLast = pOption;
				if(!m_pRecycleFirst)
					m_pRecycleLast = pOption;

				break;
			}
		}
	}
}

// H-Client
void CVoting::AnalizeVote()
{
	const char aKickTypes[][6] = { "kick\0", "ban\0", "move\0" };
	const int numKickTypes = sizeof(aKickTypes)/sizeof(*aKickTypes);
	const char aStrMaps[][32] = {
		"Map:%s\0",
		"Map: %s\0",
		"map %s\0",
		"sv_map %s\0",
		"%s by \0"
	};
	const int numStrMaps = sizeof(aStrMaps)/sizeof(*aStrMaps);

	for (int i=0; i<MAX_CLIENTS; i++)
	{
		if (!m_pClient->m_aClients[i].m_Active)
			continue;

		// Has a player target?
		const char *pHL = str_find_nocase(m_aDescription, m_pClient->m_aClients[i].m_aName);
		if (pHL)
		{ // Ok, is for a player...
			m_VoteTargetClientID = i;

			// But... what type is? Kick, Ban or Spec?
			for (int e=0; e<numKickTypes; e++)
			{
				pHL = str_find_nocase(m_aDescription, aKickTypes[e]);
				if (pHL)
				{
					m_VoteType = e + 1; // Array of words coincide with enum order
					break;
				}
			}
			if (m_VoteType == TYPE_NONE)
				m_VoteType = TYPE_OTHER;

			break;
		}
	}

	if (m_VoteTargetClientID == -1)
	{ // Umm... perhaps is a vote for change the map?
		const char *pHL = str_find_nocase(m_aDescription, "map");
		if (pHL)
		{ // Yeah! it's a map vote... but... what map?
			char aMap[128] = {0};
			for (int i=0; i<numStrMaps; i++)
			{
				sscanf(m_aDescription, aStrMaps[i], aMap);
				if (aMap[0] != 0)
					break;
			}
			if (aMap[0] == 0) // try desperate search
				str_copy(aMap, m_aDescription, sizeof(aMap));

			str_append(aMap, ".png", sizeof(aMap));

			//Search a Preview
			bool found = false;
			char aBuf[512];
			if(Storage()->FindFile(aMap, "mappreviews", IStorage::TYPE_ALL, aBuf, sizeof(aBuf)))
				found = true;

			if(!found) //Try other way...
			{
				//Normalize map name
				for (int i=0; i<str_length(aMap); i++)
				{
					if(aMap[i] == 32)
						aMap[i] = '_';
				}

				found = Storage()->FindFile(aMap, "mappreviews", IStorage::TYPE_ALL, aBuf, sizeof(aBuf));
			}

			if(found)
				str_copy(m_aVoteTargetMapName, aMap, str_length(aMap)-4); //Clean ".png" extension

			m_VoteType = TYPE_MAP;
		}

		if (m_aVoteTargetMapName[0] == 0)
			m_VoteType = TYPE_OTHER;
	}
}
//

void CVoting::OnRender()
{ }


void CVoting::RenderBars(CUIRect Bars, bool Text)
{
	RenderTools()->DrawUIRect(&Bars, vec4(0.8f,0.8f,0.8f,0.5f), CUI::CORNER_ALL, Bars.h/3);

	CUIRect Splitter = Bars;
	Splitter.x = Splitter.x+Splitter.w/2;
	Splitter.w = Splitter.h/2.0f;
	Splitter.x -= Splitter.w/2;
	RenderTools()->DrawUIRect(&Splitter, vec4(0.4f,0.4f,0.4f,0.5f), CUI::CORNER_ALL, Splitter.h/4);

	if(m_Total)
	{
		CUIRect PassArea = Bars;
		if(m_Yes)
		{
			CUIRect YesArea = Bars;
			YesArea.w *= m_Yes/(float)m_Total;
			RenderTools()->DrawUIRect(&YesArea, vec4(0.2f,0.9f,0.2f,0.85f), CUI::CORNER_ALL, Bars.h/3);

			if(Text)
			{
				char Buf[256];
				str_format(Buf, sizeof(Buf), "%d", m_Yes);
				UI()->DoLabel(&YesArea, Buf, Bars.h*0.75f, 0);
			}

			PassArea.x += YesArea.w;
			PassArea.w -= YesArea.w;
		}

		if(m_No)
		{
			CUIRect NoArea = Bars;
			NoArea.w *= m_No/(float)m_Total;
			NoArea.x = (Bars.x + Bars.w)-NoArea.w;
			RenderTools()->DrawUIRect(&NoArea, vec4(0.9f,0.2f,0.2f,0.85f), CUI::CORNER_ALL, Bars.h/3);

			if(Text)
			{
				char Buf[256];
				str_format(Buf, sizeof(Buf), "%d", m_No);
				UI()->DoLabel(&NoArea, Buf, Bars.h*0.75f, 0);
			}

			PassArea.w -= NoArea.w;
		}

		if(Text && m_Pass)
		{
			char Buf[256];
			str_format(Buf, sizeof(Buf), "%d", m_Pass);
			UI()->DoLabel(&PassArea, Buf, Bars.h*0.75f, 0);
		}
	}
}

//H-Client
void CVoting::CallvoteBan(int ClientID, const char *pReason, bool ForceVote)
{
	if(ForceVote)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "force_vote ban %d %s", ClientID, pReason);
		Client()->Rcon(aBuf);
	}
}
