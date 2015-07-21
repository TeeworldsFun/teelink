/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/client/stats.h> // H-Client
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include "killmessages.h"

void CKillMessages::OnReset()
{
	m_KillmsgCurrent = 0;
	for(int i = 0; i < MAX_KILLMSGS; i++)
		m_aKillmsgs[i].m_Tick = -100000;
}

void CKillMessages::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;

		// unpack messages
		CKillMsg Kill;
		Kill.m_VictimID = pMsg->m_Victim;
		Kill.m_VictimTeam = m_pClient->m_aClients[Kill.m_VictimID].m_Team;
		str_copy(Kill.m_aVictimName, m_pClient->m_aClients[Kill.m_VictimID].m_aName, sizeof(Kill.m_aVictimName));
		Kill.m_VictimRenderInfo = m_pClient->m_aClients[Kill.m_VictimID].m_RenderInfo;
		Kill.m_KillerID = pMsg->m_Killer;
		Kill.m_KillerTeam = m_pClient->m_aClients[Kill.m_KillerID].m_Team;
		str_copy(Kill.m_aKillerName, m_pClient->m_aClients[Kill.m_KillerID].m_aName, sizeof(Kill.m_aKillerName));
		Kill.m_KillerRenderInfo = m_pClient->m_aClients[Kill.m_KillerID].m_RenderInfo;
		Kill.m_Weapon = pMsg->m_Weapon;
		Kill.m_ModeSpecial = pMsg->m_ModeSpecial;
		Kill.m_Tick = Client()->GameTick();

		//H-Client
        Kill.m_Show = false;
		Kill.m_ID = 1;

		if (Kill.m_VictimID == m_pClient->m_Snap.m_LocalClientID)
		{
            m_pClient->m_LocalInfo.Reset();
            g_Stats.m_Deaths++;
		}
		else if (Kill.m_KillerID == m_pClient->m_Snap.m_LocalClientID)
			g_Stats.m_Kills++;
		//

		// add the message
		m_KillmsgCurrent = (m_KillmsgCurrent+1)%MAX_KILLMSGS;
		m_aKillmsgs[m_KillmsgCurrent] = Kill;

        //H-Client: Reset Freeze State
        m_pClient->m_aClients[pMsg->m_Victim].m_FreezedState.Reset();
	}
}

// H-Client
float *CKillMessages::TeeSize(const void *pID, float Seconds, int Checked)
{
	float *pFade = (float*)pID;
	if(!Checked)
		*pFade = Seconds;
	if((*pFade) > 0.0f)
	{
		*pFade -= Client()->RenderFrameTime();
		if(*pFade < 0.0f)
			*pFade = 0.0f;
	}
	return pFade;
}
//

void CKillMessages::OnRender()
{
	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width*1.5f, Height*1.5f);
	float StartX = Width*1.5f-10.0f;
	float y = 20.0f;

	for(int i = 1; i <= MAX_KILLMSGS; i++)
	{
		int r = (m_KillmsgCurrent+i)%MAX_KILLMSGS;
		if(Client()->GameTick() > m_aKillmsgs[r].m_Tick+50*10)
			continue;

		float FontSize = 36.0f;
		float KillerNameW = TextRender()->TextWidth(0, FontSize, m_aKillmsgs[r].m_aKillerName, -1);
		float VictimNameW = TextRender()->TextWidth(0, FontSize, m_aKillmsgs[r].m_aVictimName, -1);

		float x = StartX;

		// render victim name
		x -= VictimNameW;
		TextRender()->Text(0, x, y, FontSize, m_aKillmsgs[r].m_aVictimName, -1);

		// render victim tee
		x -= 24.0f;

		if(m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS))
		{
			if(m_aKillmsgs[r].m_ModeSpecial&1)
			{
				Graphics()->BlendNormal();
				Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FLAGS].m_Id);
				Graphics()->QuadsBegin();

				if(m_aKillmsgs[r].m_VictimTeam == TEAM_RED)
					RenderTools()->SelectSprite(SPRITE_FLAG_BLUE03);
				else
					RenderTools()->SelectSprite(SPRITE_FLAG_RED03);

				float Size = 56.0f;
				IGraphics::CQuadItem QuadItem(x, y-16, Size/2, Size);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}

        // H-Client
        float Seconds = 0.6f; //  0.6 seconds for fade
        float *pFade = TeeSize(&m_aKillmsgs[r].m_ID, Seconds, m_aKillmsgs[r].m_Show);
        float FadeVal = *pFade/Seconds;
        if (FadeVal > 0.0f)
        {
            float orgSize = m_aKillmsgs[r].m_VictimRenderInfo.m_Size;
            m_aKillmsgs[r].m_VictimRenderInfo.m_Size = orgSize + (1.0-FadeVal)*45.0f;
            m_aKillmsgs[r].m_VictimRenderInfo.m_ColorBody.a = FadeVal;
            m_aKillmsgs[r].m_VictimRenderInfo.m_ColorFeet.a = FadeVal;
            RenderTools()->RenderTee(CAnimState::GetIdle(), &m_aKillmsgs[r].m_VictimRenderInfo, EMOTE_PAIN, vec2(-1,0), vec2(x, y+28));

            //dbg_msg("----", "f: %.2f -- a: %.2f -- s: %.2f", *pFade, FadeVal, m_aKillmsgs[r].m_VictimRenderInfo.m_Size);

            m_aKillmsgs[r].m_Show = true;
            m_aKillmsgs[r].m_VictimRenderInfo.m_Size = orgSize;
            m_aKillmsgs[r].m_VictimRenderInfo.m_ColorBody.a = 1.0f;
            m_aKillmsgs[r].m_VictimRenderInfo.m_ColorFeet.a = 1.0f;
        }

		RenderTools()->RenderTee(CAnimState::GetIdle(), &m_aKillmsgs[r].m_VictimRenderInfo, EMOTE_PAIN, vec2(-1,0), vec2(x, y+28));
		x -= 32.0f;

		// render weapon
		x -= 44.0f;
		if (m_aKillmsgs[r].m_Weapon >= 0)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
			Graphics()->QuadsBegin();
			RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[m_aKillmsgs[r].m_Weapon].m_pSpriteBody);
			RenderTools()->DrawSprite(x, y+28, 96);
			Graphics()->QuadsEnd();
		}
		x -= 52.0f;

		if(m_aKillmsgs[r].m_VictimID != m_aKillmsgs[r].m_KillerID)
		{
			if(m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS))
			{
				if(m_aKillmsgs[r].m_ModeSpecial&2)
				{
					Graphics()->BlendNormal();
					Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FLAGS].m_Id);
					Graphics()->QuadsBegin();

					if(m_aKillmsgs[r].m_KillerTeam == TEAM_RED)
						RenderTools()->SelectSprite(SPRITE_FLAG_BLUE03, SPRITE_FLAG_FLIP_X);
					else
						RenderTools()->SelectSprite(SPRITE_FLAG_RED03, SPRITE_FLAG_FLIP_X);

					float Size = 56.0f;
					IGraphics::CQuadItem QuadItem(x-56, y-16, Size/2, Size);
					Graphics()->QuadsDrawTL(&QuadItem, 1);
					Graphics()->QuadsEnd();
				}
			}

			// render killer tee
			x -= 24.0f;
			RenderTools()->RenderTee(CAnimState::GetIdle(), &m_aKillmsgs[r].m_KillerRenderInfo, EMOTE_ANGRY, vec2(1,0), vec2(x, y+28));
			x -= 32.0f;

			// render killer name
			x -= KillerNameW;
			TextRender()->Text(0, x, y, FontSize, m_aKillmsgs[r].m_aKillerName, -1);
		}

		y += 46.0f;
	}
}
