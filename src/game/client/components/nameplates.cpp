/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>
#include <engine/graphics.h> // H-Client
#include <engine/shared/config.h>
#include <engine/serverbrowser.h> //H-Client
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/client/components/voting.h>
#include "nameplates.h"
#include "controls.h"

// H-Client
void CNamePlates::RenderWarnings(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo
	)
{
	if (g_Config.m_hcMarkVoteTarget && m_pClient->m_pVoting->GetTargetClientID() == pPlayerInfo->m_ClientID)
	{
		vec2 Position;

		if (pPlayerInfo->m_Local)
		{
			float IntraTick = Client()->IntraGameTick();
			// H-Client
	        CNetObj_Character Prev = *pPrevChar;
	        CNetObj_Character Player = *pPlayerChar;

	        // use preditect players if needed
	        if((pPlayerInfo->m_Local && g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK) && !(m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)))
	        {
	            // apply predicted results
	            m_pClient->m_PredictedChar.Write(&Player);
	            m_pClient->m_PredictedPrevChar.Write(&Prev);
	            IntraTick = Client()->PredIntraGameTick();
	        }

	        Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Player.m_X, Player.m_Y), IntraTick);
		} else
		{
			Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), Client()->IntraGameTick());
		}

		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.5f);
		TextRender()->TextColor(1.0f, 0.5f, 0.5f, 1.0f); // Red

		const float FontSize = 18.0f + 20.0f * 0.7f;
		if (m_pClient->m_pVoting->GetVoteType() == CVoting::TYPE_KICK)
		{
			const float tw = TextRender()->TextWidth(0, FontSize, "VOTED FOR KICK", -1);
			TextRender()->Text(0, Position.x-tw/2.0f, Position.y-FontSize-60.0f, FontSize, "VOTED FOR KICK", -1);
		} else if (m_pClient->m_pVoting->GetVoteType() == CVoting::TYPE_BAN)
		{
			const float tw = TextRender()->TextWidth(0, FontSize, "VOTED FOR BAN", -1);
			TextRender()->Text(0, Position.x-tw/2.0f, Position.y-FontSize-60.0f, FontSize, "VOTED FOR BAN", -1);
		} else if (m_pClient->m_pVoting->GetVoteType() == CVoting::TYPE_SPEC)
		{
			const float tw = TextRender()->TextWidth(0, FontSize, "VOTED FOR SPECTATE", -1);
			TextRender()->Text(0, Position.x-tw/2.0f, Position.y-FontSize-60.0f, FontSize, "VOTED FOR SPECTATE", -1);
		} else
		{
			const float tw = TextRender()->TextWidth(0, FontSize, "VOTED FOR SOMETHING", -1);
			TextRender()->Text(0, Position.x-tw/2.0f, Position.y-FontSize-60.0f, FontSize, "VOTED FOR SOMETHING", -1);
		}
	}
}

void CNamePlates::RenderPlayerDirections(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo
	)
{
	// Tee Direction Info
	if (g_Config.m_ddrShowTeeDirection && !pPlayerInfo->m_Local)
	{
		const float IntraTick = Client()->IntraGameTick();
		const vec2 Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), IntraTick);

		if (pPlayerChar->m_Direction != 0)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->QuadsBegin();
			IGraphics::CQuadItem QuadItem(Position.x-15, Position.y - 70, 22, 22);
			if (pPlayerChar->m_Direction == -1)
				Graphics()->QuadsSetRotation(GetAngle(vec2(1,0))+PI);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
		if (pPlayerChar->m_Jumped&1)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->QuadsBegin();
			IGraphics::CQuadItem QuadItem(Position.x+15, Position.y - 70, 22, 22);
			Graphics()->QuadsSetRotation(GetAngle(vec2(0,1))+PI);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
	}
}
//

void CNamePlates::RenderNameplate(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo
	)
{
	float IntraTick = Client()->IntraGameTick();
	vec2 Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), IntraTick);
	const float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;

	// render name plate
	if(!pPlayerInfo->m_Local)
	{
		float a = 1.0f;
		if(g_Config.m_ClNameplatesAlways == 0)
			a = clamp(1-powf(distance(m_pClient->m_pControls->m_TargetPos, Position)/200.0f,16.0f), 0.0f, 1.0f);

		const char *pName = m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_aName;
		const float tw = TextRender()->TextWidth(0, FontSize, pName, -1);

		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.5f*a);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, a);

		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS))
		{
			if(pPlayerInfo->m_Team == TEAM_RED)
				TextRender()->TextColor(1.0f, 0.5f, 0.5f, a);
			else if(pPlayerInfo->m_Team == TEAM_BLUE)
				TextRender()->TextColor(0.7f, 0.7f, 1.0f, a);
		}

        //H-Client
        if (g_Config.m_hcColorClan && m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID].m_aClan[0] != 0 && str_comp_nocase(m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_aClan, m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID].m_aClan) == 0)
            TextRender()->TextColor(0.7f, 0.7f, 0.2f, a);
        //

		TextRender()->Text(0, Position.x-tw/2.0f, Position.y-FontSize-38.0f, FontSize, pName, -1);

		if(g_Config.m_Debug) // render client id when in debug aswell
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf),"%d", pPlayerInfo->m_ClientID);
			TextRender()->Text(0, Position.x, Position.y-90, 28.0f, aBuf, -1);
		}

        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
	}
	else if (g_Config.m_hcUseHUD && Client()->IsServerType(SERVER_GAMETYPE_DDRACE) && m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_FreezedState.m_Freezed)
    {
		// H-Client
        CNetObj_Character Prev = *pPrevChar;
        CNetObj_Character Player = *pPlayerChar;

        // use preditect players if needed
        if((pPlayerInfo->m_Local && g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK) && !(m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)))
        {
            // apply predicted results
            m_pClient->m_PredictedChar.Write(&Player);
            m_pClient->m_PredictedPrevChar.Write(&Prev);
            IntraTick = Client()->PredIntraGameTick();
        }

        Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Player.m_X, Player.m_Y), IntraTick);
    }
}

void CNamePlates::OnRender()
{
	if (!g_Config.m_ClNameplates)
		return;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// only render active characters
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
			continue;

		const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);

		if(pInfo)
		{
			RenderNameplate(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				(const CNetObj_PlayerInfo *)pInfo);

			RenderPlayerDirections(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				(const CNetObj_PlayerInfo *)pInfo);

			RenderWarnings(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				(const CNetObj_PlayerInfo *)pInfo);
		}
	}
}
