/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <stdio.h> // sscanf
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/serverbrowser.h> //H-Client
#include <engine/storage.h> //H-Client
#include <engine/shared/config.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/client/render.h>
#include <game/client/components/scoreboard.h>

#include "controls.h"
#include "camera.h"
#include "hud.h"
#include "voting.h"
#include "binds.h"
#include "mapimages.h" //H-Client
#include "skins.h" //H-Client
#include "menus.h" //H-Client
#include "chat.h" //H-Client
#include "players.h" //H-Client

#include <vector> // H-Client
#include <string> // H-Client

CHud::CHud()
{
	// won't work if zero
	m_AverageFPS = 1.0f;
}

void CHud::OnReset()
{
    //H-Client: DDRace
	m_CheckpointDiff = 0.0f;
	m_DDRaceTime = 0;
	m_LastReceivedTimeTick = 0;
	m_CheckpointTick = 0;
	m_DDRaceTick = 0;
	m_FinishTime = false;
	m_ServerRecord = -1.0f;
	m_PlayerRecord = -1.0f;
	m_DDRaceTimeReceived = false;
}

void CHud::RenderGameTimer()
{
    CServerInfo Info;
    Client()->GetServerInfo(&Info);

    static int LastChangeTick = 0;
    if(LastChangeTick != Client()->PredGameTick())
    {
        m_DDRaceTick += 100/Client()->GameTickSpeed();
        m_DDRaceTick += 100/Client()->GameTickSpeed();
        LastChangeTick = Client()->PredGameTick();
    }

    if(m_DDRaceTick >= 100)
        m_DDRaceTick = 0;


	float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;

	if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH))
	{
		char Buf[32];
		int Time = 0;
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			Time = m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit*60 - ((Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed());

			if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
				Time = 0;
		}
		else
			Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed();

		if(Time <= 0)
			str_format(Buf, sizeof(Buf), "00:00.0");
		else
			str_format(Buf, sizeof(Buf), "%02d:%02d.%d", Time/60, Time%60, m_DDRaceTick/10);
		float FontSize = 10.0f;
		float w = TextRender()->TextWidth(0, 12,"00:00.0",-1);
		// last 60 sec red, last 10 sec blink
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && Time <= 60 && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			float Alpha = Time <= 10 && (2*time_get()/time_freq()) % 2 ? 0.5f : 1.0f;
			TextRender()->TextColor(1.0f, 0.25f, 0.25f, Alpha);
		}
		TextRender()->Text(0, Half-w/2, 2, FontSize, Buf, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void CHud::RenderSuddenDeath()
{
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH)
	{
		float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;
		const char *pText = Localize("Sudden Death");
		float FontSize = 12.0f;
		float w = TextRender()->TextWidth(0, FontSize, pText, -1);
		TextRender()->Text(0, Half-w/2, 2, FontSize, pText, -1);
	}
}

void CHud::RenderScoreHud()
{
    CServerInfo Info;
    Client()->GetServerInfo(&Info);
    if (g_Config.m_hcUseHUD && str_find_nocase(Info.m_aGameType, "race"))
        return;

	// render small score hud
	if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
	{
		int GameFlags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
		float Whole = 300*Graphics()->ScreenAspect();

		if(GameFlags&GAMEFLAG_TEAMS && m_pClient->m_Snap.m_pGameDataObj)
		{
			char aScoreTeam[2][32];
			str_format(aScoreTeam[TEAM_RED], sizeof(aScoreTeam)/2, "%d", m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed);
			str_format(aScoreTeam[TEAM_BLUE], sizeof(aScoreTeam)/2, "%d", m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue);
			float aScoreTeamWidth[2] = { TextRender()->TextWidth(0, 14.0f, aScoreTeam[TEAM_RED], -1), TextRender()->TextWidth(0, 14.0f, aScoreTeam[TEAM_BLUE], -1) };
			int FlagCarrier[2] = { m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierRed, m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue };
			float ScoreWidthMax = max(max(aScoreTeamWidth[TEAM_RED], aScoreTeamWidth[TEAM_BLUE]), TextRender()->TextWidth(0, 14.0f, "100", -1));
			float Split = 3.0f;
			float ImageSize = GameFlags&GAMEFLAG_FLAGS ? 16.0f : Split;

			for(int t = 0; t < 2; t++)
			{
				// draw box
				Graphics()->BlendNormal();
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				if(t == 0)
					Graphics()->SetColor(1.0f, 0.0f, 0.0f, 0.25f);
				else
					Graphics()->SetColor(0.0f, 0.0f, 1.0f, 0.25f);
				RenderTools()->DrawRoundRectExt(Whole-ScoreWidthMax-ImageSize-2*Split, 245.0f+t*20, ScoreWidthMax+ImageSize+2*Split, 18.0f, 5.0f, CUI::CORNER_L);
				Graphics()->QuadsEnd();

				// draw score
				TextRender()->Text(0, Whole-ScoreWidthMax+(ScoreWidthMax-aScoreTeamWidth[t])/2-Split, 245.0f+t*20, 14.0f, aScoreTeam[t], -1);

				if(GameFlags&GAMEFLAG_FLAGS)
				{
					int BlinkTimer = (m_pClient->m_FlagDropTick[t] != 0 &&
										(Client()->GameTick()-m_pClient->m_FlagDropTick[t])/Client()->GameTickSpeed() >= 25) ? 10 : 20;
					if(FlagCarrier[t] == FLAG_ATSTAND || (FlagCarrier[t] == FLAG_TAKEN && ((Client()->GameTick()/BlinkTimer)&1)))
					{
						// draw flag
						Graphics()->BlendNormal();
						Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
						Graphics()->QuadsBegin();
						RenderTools()->SelectSprite(t==0?SPRITE_FLAG_RED:SPRITE_FLAG_BLUE);
						IGraphics::CQuadItem QuadItem(Whole-ScoreWidthMax-ImageSize, 246.0f+t*20, ImageSize/2, ImageSize);
						Graphics()->QuadsDrawTL(&QuadItem, 1);
						Graphics()->QuadsEnd();
					}
					else if(FlagCarrier[t] >= 0)
					{
						// draw name of the flag holder
						int ID = FlagCarrier[t]%MAX_CLIENTS;
						const char *pName = m_pClient->m_aClients[ID].m_aName;
						float w = TextRender()->TextWidth(0, 10.0f, pName, -1);
						TextRender()->Text(0, Whole-ScoreWidthMax-ImageSize-3*Split-w, 247.0f+t*20, 10.0f, pName, -1);

						// draw tee of the flag holder
						CTeeRenderInfo Info = m_pClient->m_aClients[ID].m_RenderInfo;
						Info.m_Size = 18.0f;
						RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0),
							vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, 246.0f+Info.m_Size/2+t*20));
					}
				}
			}
		}
		else
		{
			int Local = -1;
			int aPos[2] = { 1, 2 };
			const CNetObj_PlayerInfo *apPlayerInfo[2] = { 0, 0 };
			int i = 0;
			for(int t = 0; t < 2 && i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
			{
				if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
				{
					apPlayerInfo[t] = m_pClient->m_Snap.m_paInfoByScore[i];
					if(apPlayerInfo[t]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
						Local = t;
					++t;
				}
			}
			// search local player info if not a spectator, nor within top2 scores
			if(Local == -1 && m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team != TEAM_SPECTATORS)
			{
				for(; i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
				{
					if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
						++aPos[1];
					if(m_pClient->m_Snap.m_paInfoByScore[i]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
					{
						apPlayerInfo[1] = m_pClient->m_Snap.m_paInfoByScore[i];
						Local = 1;
						break;
					}
				}
			}
			char aScore[2][32];
			for(int t = 0; t < 2; ++t)
			{
				if(apPlayerInfo[t])
					str_format(aScore[t], sizeof(aScore)/2, "%d", apPlayerInfo[t]->m_Score);
				else
					aScore[t][0] = 0;
			}
			float aScoreWidth[2] = {TextRender()->TextWidth(0, 14.0f, aScore[0], -1), TextRender()->TextWidth(0, 14.0f, aScore[1], -1)};
			float ScoreWidthMax = max(max(aScoreWidth[0], aScoreWidth[1]), TextRender()->TextWidth(0, 14.0f, "10", -1));
			float Split = 3.0f, ImageSize = 16.0f, PosSize = 16.0f;

			for(int t = 0; t < 2; t++)
			{
				// draw box
				Graphics()->BlendNormal();
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				if(t == Local)
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
				else
					Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.25f);
				RenderTools()->DrawRoundRectExt(Whole-ScoreWidthMax-ImageSize-2*Split-PosSize, 245.0f+t*20, ScoreWidthMax+ImageSize+2*Split+PosSize, 18.0f, 5.0f, CUI::CORNER_L);
				Graphics()->QuadsEnd();

				// draw score
				TextRender()->Text(0, Whole-ScoreWidthMax+(ScoreWidthMax-aScoreWidth[t])/2-Split, 245.0f+t*20, 14.0f, aScore[t], -1);

				// draw tee
				if(apPlayerInfo[t])
 				{
					CTeeRenderInfo Info = m_pClient->m_aClients[apPlayerInfo[t]->m_ClientID].m_RenderInfo;
 					Info.m_Size = 18.0f;
 					RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0),
 						vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, 246.0f+Info.m_Size/2+t*20));
				}

				// draw position
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "%d.", aPos[t]);
				TextRender()->Text(0, Whole-ScoreWidthMax-ImageSize-Split-PosSize, 247.0f+t*20, 10.0f, aBuf, -1);
			}
		}
	}
}

void CHud::RenderWarmupTimer()
{
	// render warmup timer
	if(m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
	{
		char Buf[256];
		float FontSize = 20.0f;
		float w = TextRender()->TextWidth(0, FontSize, Localize("Warmup"), -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()+-w/2, 50, FontSize, Localize("Warmup"), -1);

		int Seconds = m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer/SERVER_TICK_SPEED;
		if(Seconds < 5)
			str_format(Buf, sizeof(Buf), "%d.%d", Seconds, (m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer*10/SERVER_TICK_SPEED)%10);
		else
			str_format(Buf, sizeof(Buf), "%d", Seconds);
		w = TextRender()->TextWidth(0, FontSize, Buf, -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()+-w/2, 75, FontSize, Buf, -1);
	}
}

void CHud::MapscreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup)
{
	float Points[4];
	RenderTools()->MapscreenToWorld(CenterX, CenterY, pGroup->m_ParallaxX/100.0f, pGroup->m_ParallaxY/100.0f,
		pGroup->m_OffsetX, pGroup->m_OffsetY, Graphics()->ScreenAspect(), 1.0f, Points);
	Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
}

void CHud::RenderFps()
{
	if(g_Config.m_ClShowfps)
	{
		// calculate avg. fps
		float FPS = 1.0f / Client()->RenderFrameTime();
		m_AverageFPS = (m_AverageFPS*(1.0f-(1.0f/m_AverageFPS))) + (FPS*(1.0f/m_AverageFPS));
		char Buf[512];
		str_format(Buf, sizeof(Buf), "%d", (int)m_AverageFPS);
		TextRender()->Text(0, m_Width-10-TextRender()->TextWidth(0,12,Buf,-1), 5, 12, Buf, -1);
	}
}

void CHud::RenderConnectionWarning()
{
	if(Client()->ConnectionProblems())
	{
		const char *pText = Localize("Connection Problems...");
		float w = TextRender()->TextWidth(0, 24, pText, -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()-w/2, 50, 24, pText, -1);
	}
}

void CHud::RenderTeambalanceWarning()
{
	// render prompt about team-balance
	bool Flash = time_get()/(time_freq()/2)%2 == 0;
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
	{
		int TeamDiff = m_pClient->m_Snap.m_aTeamSize[TEAM_RED]-m_pClient->m_Snap.m_aTeamSize[TEAM_BLUE];
		if (g_Config.m_ClWarningTeambalance && (TeamDiff >= 2 || TeamDiff <= -2))
		{
			const char *pText = Localize("Please balance teams!");
			if(Flash)
				TextRender()->TextColor(1,1,0.5f,1);
			else
				TextRender()->TextColor(0.7f,0.7f,0.2f,1.0f);
			TextRender()->Text(0x0, 5, 50, 6, pText, -1);
			TextRender()->TextColor(1,1,1,1);
		}
	}
}


void CHud::RenderVoting()
{
    static bool sFindFile = true;
	if(!m_pClient->m_pVoting->IsVoting() || Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
        sFindFile = true;
		return;
	}

    char aClientName[32] = {0}, aMap[64] = {0};
    int offSetPreview = 0;
    bool found = false;
    float offSetX = m_pClient->m_pVoting->m_offSetX;

    //Search Vote Kick or Move
    sscanf(m_pClient->m_pVoting->VoteDescription(), "Kick '%[^']s'", aClientName);
    if (aClientName[0] == 0)
        sscanf(m_pClient->m_pVoting->VoteDescription(), "move '%[^']s'", aClientName);
    if (aClientName[0] == 0)
        sscanf(m_pClient->m_pVoting->VoteDescription(), "Spec '%[^']s'", aClientName);
    if (aClientName[0] == 0)
        sscanf(m_pClient->m_pVoting->VoteDescription(), "spec '%[^']s'", aClientName);
    if (aClientName[0] == 0)
        sscanf(m_pClient->m_pVoting->VoteDescription(), "Pause '%[^']s'", aClientName);

    //Search Vote Map
    if (aClientName[0] == 0)
        sscanf(m_pClient->m_pVoting->VoteDescription(), "Map:%s", aMap);
    if (aMap[0] == 0)
        sscanf(m_pClient->m_pVoting->VoteDescription(), "Map: %s", aMap);
    if (aMap[0] == 0)
        sscanf(m_pClient->m_pVoting->VoteDescription(), "map %s", aMap);
    if (aMap[0] == 0)
        sscanf(m_pClient->m_pVoting->VoteDescription(), "sv_map %s", aMap);

    if (aClientName[0] != 0 && m_pClient->m_pVoting->GetState() == CVoting::STATE_NORMAL)
    {
        offSetPreview = 45;

        for (int i=0; i<MAX_CLIENTS; i++)
        {
            if (str_comp(m_pClient->m_aClients[i].m_aName, aClientName) == 0)
            {
                // draw tee of the flag holder
                CTeeRenderInfo Info = m_pClient->m_aClients[i].m_RenderInfo;
                Info.m_Size = 36.0f;
                RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(-1,0), vec2(130-offSetX, 83));

                break;
            }
        }
    }
    else if (sFindFile)
    {
        if (aMap[0] == 0)
            str_copy(aMap, m_pClient->m_pVoting->VoteDescription(), sizeof(aMap));

        str_append(aMap, ".png", sizeof(aMap));

        //Search a Preview
        found=false;
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

            if (Storage()->FindFile(aMap, "mappreviews", IStorage::TYPE_ALL, aBuf, sizeof(aBuf)))
                found = true;
        }
        else
        {
            //Clean ".png" extension
            for (int i=4; i>0; i--)
                aMap[str_length(aMap)-1]=0;

            offSetPreview = 45;
        }

        sFindFile = false;
    }

    float WidthBox = 100.0f+10.0f+4.0f+5.0f+offSetPreview;
    static long voteSmallTimer = time_get();
    static long voteNormalTimer = time_get();

    if (m_pClient->m_pVoting->GetState() == CVoting::STATE_NORMAL && m_pClient->m_pVoting->GetLastVote() != 0 && time_get() > voteNormalTimer+(long)(0.02f*time_freq()))
    {
        m_pClient->m_pVoting->m_offSetX = min(m_pClient->m_pVoting->m_offSetX+5.0f, WidthBox);

        if (m_pClient->m_pVoting->m_offSetX == WidthBox)
        {
            m_pClient->m_pVoting->m_offSetX = 30.0f;
            m_pClient->m_pVoting->SetState(CVoting::STATE_SMALL);
        }

        voteNormalTimer = time_get();
    }
    else if (m_pClient->m_pVoting->GetState() == CVoting::STATE_SMALL && time_get() > voteSmallTimer+(long)(0.02f*time_freq()))
    {
        m_pClient->m_pVoting->m_offSetX = max(-10.0f, m_pClient->m_pVoting->m_offSetX-0.5f);
        voteSmallTimer = time_get();
    }

    offSetX = m_pClient->m_pVoting->m_offSetX;


    if (m_pClient->m_pVoting->GetState() == CVoting::STATE_NORMAL)
    {
        Graphics()->TextureSet(-1);
        Graphics()->QuadsBegin();
        Graphics()->SetColor(0,0,0,0.40f);
        RenderTools()->DrawRoundRect(-10-offSetX, 60-2, WidthBox , 46, 5.0f);
        Graphics()->QuadsEnd();

        TextRender()->TextColor(1,1,1,1);

        CTextCursor Cursor;
        char aBuf[512];
        str_format(aBuf, sizeof(aBuf), Localize("%ds left"), m_pClient->m_pVoting->SecondsLeft());
        float tw = TextRender()->TextWidth(0x0, 6, aBuf, -1);
        TextRender()->SetCursor(&Cursor, 5.0f+100.0f-tw-offSetX, 60.0f, 6.0f, TEXTFLAG_RENDER);
        TextRender()->TextEx(&Cursor, aBuf, -1);

        TextRender()->SetCursor(&Cursor, 5.0f-offSetX, 60.0f, 6.0f, TEXTFLAG_RENDER);
        Cursor.m_LineWidth = 100.0f-tw;
        Cursor.m_MaxLines = 3;
        TextRender()->TextEx(&Cursor, m_pClient->m_pVoting->VoteDescription(), -1);

        // reason
        str_format(aBuf, sizeof(aBuf), "%s %s", Localize("Reason:"), m_pClient->m_pVoting->VoteReason());
        TextRender()->SetCursor(&Cursor, 5.0f-offSetX, 79.0f, 6.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
        Cursor.m_LineWidth = 100.0f;
        TextRender()->TextEx(&Cursor, aBuf, -1);

        CUIRect Base = {5-offSetX, 88, 100, 4};
        m_pClient->m_pVoting->RenderBars(Base, false);

        char aYesKey[8], aNoKey[8];
        str_copy(aYesKey, m_pClient->m_pBinds->GetKey("vote yes"), sizeof(aYesKey));
        str_to_upper(aYesKey, str_length(aYesKey));
        str_copy(aNoKey, m_pClient->m_pBinds->GetKey("vote no"), sizeof(aNoKey));
        str_to_upper(aNoKey, str_length(aYesKey));

        if (m_pClient->m_pVoting->GetLastVote() == 1)
            TextRender()->TextColor(0.5f, 1.0f, 0.5f, 1.0f);
        else if (m_pClient->m_pVoting->GetLastVote() != 0)
            TextRender()->TextColor(1.0f, 1.0f, 1.0f, 0.5f);

        str_format(aBuf, sizeof(aBuf), "%s - %s", aYesKey, Localize("Vote yes"));
        Base.y += Base.h+1;
        UI()->DoLabel(&Base, aBuf, 6.0f, -1);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

        if (m_pClient->m_pVoting->GetLastVote() == -1)
            TextRender()->TextColor(0.5f, 1.0f, 0.5f, 1.0f);
        else if (m_pClient->m_pVoting->GetLastVote() != 0)
            TextRender()->TextColor(1.0f, 1.0f, 1.0f, 0.5f);

        str_format(aBuf, sizeof(aBuf), "%s - %s", Localize("Vote no"), aNoKey);
        UI()->DoLabel(&Base, aBuf, 6.0f, 1);

        if(found)
        {
            int preview = m_pClient->m_pMenus->GetImageMapPreview(aMap);
            IGraphics::CQuadItem QuadItem(108.f-offSetX, 61.0f, 42.0f, 40.0f);
            Graphics()->TextureSet(preview);
            Graphics()->QuadsBegin();
                Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
                Graphics()->QuadsDrawTL(&QuadItem, 1);
            Graphics()->QuadsEnd();
        }
    }
    else if (m_pClient->m_pVoting->GetState() == CVoting::STATE_SMALL)
    {
        Graphics()->TextureSet(-1);
        CUIRect VoteBox;
        VoteBox.x = -10.0f-offSetX; VoteBox.y = 58.0f;
        VoteBox.w = 37.0f; VoteBox.h = 22.0f;
        RenderTools()->DrawUIRect(&VoteBox, vec4(0,0,0,0.8f), CUI::CORNER_R, 2.5f);
        VoteBox.x = 15.0f-offSetX; VoteBox.y = 58.0f;
        VoteBox.w = 12.0f; VoteBox.h = 22.0f;
        RenderTools()->DrawUIRect(&VoteBox, vec4(1,1,1,0.15f), CUI::CORNER_R, 2.5f);

        CTextCursor Cursor;

        TextRender()->TextColor(0.0f, 0.5f, 0.0f, 1.0f);
        if (m_pClient->m_pVoting->GetNumVotes(1) > m_pClient->m_pVoting->GetNumVotes(2))
            TextRender()->TextColor(0.5f, 1.0f, 0.0f, 1.0f);
        char aBuf[512];
        str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("YES"), m_pClient->m_pVoting->GetNumVotes(1));
        TextRender()->SetCursor(&Cursor, -7.0f-offSetX, 60.0f, 6.0f, TEXTFLAG_RENDER);
        TextRender()->TextEx(&Cursor, aBuf, -1);

        TextRender()->TextColor(0.5f, 0.0f, 0.0f, 1.0f);
        if (m_pClient->m_pVoting->GetNumVotes(2) > m_pClient->m_pVoting->GetNumVotes(1))
            TextRender()->TextColor(1.0f, 0.5f, 0.5f, 1.0f);
        str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("NO"), m_pClient->m_pVoting->GetNumVotes(2));
        TextRender()->SetCursor(&Cursor, -7.0f-offSetX, 70.0f, 6.0f, TEXTFLAG_RENDER);
        TextRender()->TextEx(&Cursor, aBuf, -1);

        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
        str_format(aBuf, sizeof(aBuf), "%d", m_pClient->m_pVoting->SecondsLeft());
        TextRender()->SetCursor(&Cursor, 17.0f-offSetX, 65.0f, 6.0f, TEXTFLAG_RENDER);
        TextRender()->TextEx(&Cursor, aBuf, -1);


    }

    TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CHud::RenderCursor()
{
    //H-Client: IF Change
	if(!m_pClient->m_Snap.m_pLocalCharacter || Client()->State() == IClient::STATE_DEMOPLAYBACK || Graphics()->Tumbtail())
		return;

	MapscreenToGroup(m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y, Layers()->GameGroup());
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();

	// render cursor
	if (m_pClient->m_Snap.m_pLocalCharacter->m_Weapon >= NUM_WEAPONS)
        RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_pSpriteCursor);
	else
        RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[m_pClient->m_Snap.m_pLocalCharacter->m_Weapon%NUM_WEAPONS].m_pSpriteCursor);
	float CursorSize = 64;
	RenderTools()->DrawSprite(m_pClient->m_pControls->m_TargetPos.x, m_pClient->m_pControls->m_TargetPos.y, CursorSize);
	Graphics()->QuadsEnd();
}

void CHud::RenderHealthAndAmmo(const CNetObj_Character *pCharacter, int localID)
{
	if(!pCharacter)
		return;

	//mapscreen_to_group(gacenter_x, center_y, layers_game_group());

	float x = 5;
	float y = 5;

	// render ammo count
	// render gui stuff

    CServerInfo Info;
    Client()->GetServerInfo(&Info);

    if (g_Config.m_hcUseHUD && !str_find_nocase(Info.m_aGameType, "race"))
    {
        Graphics()->TextureSet(-1);
        Graphics()->QuadsBegin();
            Graphics()->SetColor(1.0f,1.0f,1.0f,0.5f);
            RenderTools()->DrawRoundRect(x-25.0f, y-2.0f, 51.0f, 14.0f, 5.0f);
            RenderTools()->DrawRoundRect(x-25.0f, y-1.0f, 37.5f, 12.0f, 5.0f);

            RenderTools()->DrawRoundRect(x-25.0f, y+16.0f-2.0f, 51.0f, 14.0f, 5.0f);
            RenderTools()->DrawRoundRect(x-25.0f, y+16.0f-1.0f, 37.5f, 12.0f, 5.0f);
        Graphics()->QuadsEnd();
        char aBuf[15];
        str_format(aBuf, sizeof(aBuf), "%d", min(pCharacter->m_Health, 10));
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
        TextRender()->Text(0, x+13.0f, y-1.0f, 8.0f, aBuf, -1);

        str_format(aBuf, sizeof(aBuf), "%d", min(pCharacter->m_Armor, 10));
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
        TextRender()->Text(0, x+13.0f, y+16.0f-1.0f, 8.0f, aBuf, -1);

        if (!str_find_nocase(Info.m_aGameType, "ictf") && !str_find_nocase(Info.m_aGameType, "idm") && !str_find_nocase(Info.m_aGameType, "itdm") && pCharacter->m_Weapon != WEAPON_HAMMER)
        {
            Graphics()->TextureSet(-1);
            Graphics()->QuadsBegin();
                Graphics()->SetColor(1.0f,1.0f,1.0f,0.5f);
                RenderTools()->DrawRoundRect(x-25.0f, y+32.0f-2.0f, 51.0f, 14.0f, 5.0f);
                RenderTools()->DrawRoundRect(x-25.0f, y+32.0f-1.0f, 37.5f, 12.0f, 5.0f);
            Graphics()->QuadsEnd();

            char aBuf[15];
            str_format(aBuf, sizeof(aBuf), "%d", pCharacter->m_AmmoCount);
            TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
            TextRender()->Text(0, x+13.0f, y+32.0f-1.0f, 8.0f, aBuf, -1);
        }

        int h = 0;
        if (!str_find_nocase(Info.m_aGameType, "ictf") && !str_find_nocase(Info.m_aGameType, "idm") && !str_find_nocase(Info.m_aGameType, "itdm") && pCharacter->m_Weapon != WEAPON_HAMMER)
        {
            Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
            IGraphics::CQuadItem QuadItem(x,y+32,10,10);

            h = min(pCharacter->m_AmmoCount, 10);
            // if weaponstage is active, put a "glow" around the stage ammo
            Graphics()->QuadsBegin();
                RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[pCharacter->m_Weapon%NUM_WEAPONS].m_pSpriteProj);
                Graphics()->SetColor(1.0f, 1.0f, 1.0f, h/10.0f);
                Graphics()->QuadsDrawTL(&QuadItem, 1);
            Graphics()->QuadsEnd();
        }

        Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
        IGraphics::CQuadItem QuadItem(x,y+32,10,10);

        QuadItem = IGraphics::CQuadItem(x,y,10,10);
        h = min(pCharacter->m_Health, 10);
        // render health meter
        Graphics()->QuadsBegin();
            RenderTools()->SelectSprite(SPRITE_HEALTH_FULL);
            Graphics()->SetColor(1.0f, 1.0f, 1.0f, h/10.0f);
            Graphics()->QuadsDrawTL(&QuadItem, 1);
        Graphics()->QuadsEnd();

        Graphics()->QuadsBegin();
            RenderTools()->SelectSprite(SPRITE_HEALTH_EMPTY);
            Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f - h/10.0f);
            Graphics()->QuadsDrawTL(&QuadItem, 1);
        Graphics()->QuadsEnd();

        QuadItem = IGraphics::CQuadItem(x,y+16.0f,10,10);
        h = min(pCharacter->m_Armor, 10);
        // render armor meter
        Graphics()->QuadsBegin();
            RenderTools()->SelectSprite(SPRITE_ARMOR_FULL);
            Graphics()->SetColor(1.0f, 1.0f, 1.0f, h/10.0f);
            Graphics()->QuadsDrawTL(&QuadItem, 1);
        Graphics()->QuadsEnd();

        Graphics()->QuadsBegin();
            RenderTools()->SelectSprite(SPRITE_ARMOR_EMPTY);
            Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f - h/10.0f);
            Graphics()->QuadsDrawTL(&QuadItem, 1);
        Graphics()->QuadsEnd();
    }
    else if (!g_Config.m_hcUseHUD)
    {
        Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);

        Graphics()->QuadsBegin();

        // if weaponstage is active, put a "glow" around the stage ammo
        RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[pCharacter->m_Weapon%NUM_WEAPONS].m_pSpriteProj);
        IGraphics::CQuadItem Array[10];
        int i;
        for (i = 0; i < min(pCharacter->m_AmmoCount, 10); i++)
            Array[i] = IGraphics::CQuadItem(x+i*12,y+24,10,10);
        Graphics()->QuadsDrawTL(Array, i);
        Graphics()->QuadsEnd();

        Graphics()->QuadsBegin();
        int h = 0;

        // render health
        RenderTools()->SelectSprite(SPRITE_HEALTH_FULL);
        for(; h < min(pCharacter->m_Health, 10); h++)
            Array[h] = IGraphics::CQuadItem(x+h*12,y,10,10);
        Graphics()->QuadsDrawTL(Array, h);

        i = 0;
        RenderTools()->SelectSprite(SPRITE_HEALTH_EMPTY);
        for(; h < 10; h++)
            Array[i++] = IGraphics::CQuadItem(x+h*12,y,10,10);
        Graphics()->QuadsDrawTL(Array, i);

        // render armor meter
        h = 0;
        RenderTools()->SelectSprite(SPRITE_ARMOR_FULL);
        for(; h < min(pCharacter->m_Armor, 10); h++)
            Array[h] = IGraphics::CQuadItem(x+h*12,y+12,10,10);
        Graphics()->QuadsDrawTL(Array, h);

        i = 0;
        RenderTools()->SelectSprite(SPRITE_ARMOR_EMPTY);
        for(; h < 10; h++)
            Array[i++] = IGraphics::CQuadItem(x+h*12,y+12,10,10);
        Graphics()->QuadsDrawTL(Array, i);
        Graphics()->QuadsEnd();
    }
}

void CHud::RenderSpectatorHud()
{
	// draw the box
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.4f);
	RenderTools()->DrawRoundRectExt(m_Width-180.0f, m_Height-15.0f, 180.0f, 15.0f, 5.0f, CUI::CORNER_TL);
	Graphics()->QuadsEnd();

	// draw the text
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Spectate"), m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW ?
		m_pClient->m_aClients[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_aName : Localize("Free-View"));
	TextRender()->Text(0, m_Width-174.0f, m_Height-13.0f, 8.0f, aBuf, -1);
}

void CHud::OnRender()
{
	if(!m_pClient->m_Snap.m_pGameInfoObj)
		return;

	m_Width = 300.0f*Graphics()->ScreenAspect();
	m_Height = 300.0f;
	Graphics()->MapScreen(0.0f, 0.0f, m_Width, m_Height);

	if(g_Config.m_ClShowhud && !m_pClient->m_pScoreboard->Active())
	{
		if(m_pClient->m_Snap.m_pLocalCharacter && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
			RenderHealthAndAmmo(m_pClient->m_Snap.m_pLocalCharacter, m_pClient->m_Snap.m_LocalClientID);
		else if(m_pClient->m_Snap.m_SpecInfo.m_Active)
		{
			if(m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW)
			{
				RenderHealthAndAmmo(&m_pClient->m_Snap.m_aCharacters[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_Cur, m_pClient->m_Snap.m_LocalClientID);
                RenderSelectorSpectatorHud(); //H-Client
			}
			RenderSpectatorHud();
		}

		RenderGameTimer();
		RenderSuddenDeath();
		RenderPlayerInfo();

        RenderScoreHud();
		RenderWarmupTimer();
		RenderFps();
		if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
			RenderConnectionWarning();
		RenderTeambalanceWarning();
		RenderVoting();

		if (!m_pClient->m_pMenus->IsActive())
            RenderRecord(); //H-Client: DDRace
	}
	RenderCursor();
}

//H-Client
void CHud::RenderSelectorSpectatorHud()
{
    static const float boxSize = 60.0f;
    static float offSet = 0.0f;
    static float mouseX = m_pClient->m_pControls->m_MousePos.x;
    static float mouseY = m_pClient->m_pControls->m_MousePos.y;
    static float timerMove = Client()->GameTick();
    static float lastSpecID = m_pClient->m_Snap.m_SpecInfo.m_SpectatorID;
    static int mouseMove = 0;

    //Control Mouse Movement
    if ((m_pClient->m_pControls->m_MousePos.x != mouseX && m_pClient->m_pControls->m_MousePos.y != mouseY) || lastSpecID != m_pClient->m_Snap.m_SpecInfo.m_SpectatorID)
    {
        mouseMove = 1;
        timerMove = Client()->GameTick();

        mouseX = m_pClient->m_pControls->m_MousePos.x;
        mouseY = m_pClient->m_pControls->m_MousePos.y;
        lastSpecID = m_pClient->m_Snap.m_SpecInfo.m_SpectatorID;
    }

    if (mouseMove && Client()->GameTick() > timerMove + 12.0f*Client()->GameTickSpeed())
        mouseMove = 0;

    //Calculate Offset
    if (mouseMove && offSet < boxSize)
        offSet = clamp(offSet+0.5f, 0.0f, boxSize);
    else if (!mouseMove && offSet > 0.0f)
        offSet = clamp(offSet-0.5f, 0.0f, boxSize);

    bool GotNewSpectatorID = false;
    int NewSpectatorID = 0;
    float IntraTick = Client()->IntraGameTick();

    for(int i = m_pClient->m_Snap.m_SpecInfo.m_SpectatorID - 1; i > -1; i--)
    {
        if(!m_pClient->m_Snap.m_paPlayerInfos[i] || m_pClient->m_Snap.m_paPlayerInfos[i]->m_Team == TEAM_SPECTATORS)
            continue;

        NewSpectatorID = i;
        GotNewSpectatorID = true;
        break;
    }
	if (GotNewSpectatorID && offSet > 0.0f)
	{
        // draw the left box
        Graphics()->TextureSet(-1);
        Graphics()->QuadsBegin();
        Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.6f);
        RenderTools()->DrawRoundRectExt(0.0f, m_Height/2-30.f, offSet, 50.0f, 5.0f, CUI::CORNER_R);
        Graphics()->QuadsEnd();

        //Title
        TextRender()->TextColor(1.0f, 0.39f, 0.0f, 0.95f);
        TextRender()->Text(0x0, 2.0f - (60.0f - offSet), m_Height/2-30.f+2.0f, 8.0f, "PREV", -1);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

        //Tee
        CNetObj_Character CurChar = m_pClient->m_Snap.m_aCharacters[NewSpectatorID].m_Cur;
        CNetObj_Character PrevChar = m_pClient->m_Snap.m_aCharacters[NewSpectatorID].m_Prev;
        float Angle = mix((float)PrevChar.m_Angle, (float)CurChar.m_Angle, IntraTick)/256.0f;
        vec2 Direction = GetDirection((int)(Angle*256.0f));
        vec2 TeePos = vec2(30.0f - (60.0f - offSet), m_Height/2-30.f+25.0f);
	    CTeeRenderInfo TeeInfo = m_pClient->m_aClients[NewSpectatorID].m_RenderInfo;
	    TeeInfo.m_Size = 22.0f;
        //Freeze/Ninja
        if (CurChar.m_Weapon == WEAPON_NINJA)
        {
            // change the skin for the player to the ninja
            int Skin = m_pClient->m_pSkins->Find("x_ninja");
            if(Skin != -1)
            {
                TeeInfo.m_Texture = m_pClient->m_pSkins->Get(Skin)->m_OrgTexture;
                TeeInfo.m_ColorBody = vec4(1,1,1,1);
                TeeInfo.m_ColorFeet = vec4(1,1,1,1);
            }
        }
        RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, CurChar.m_Emote, Direction, TeePos);

        //Name Tee
        char aBuf[MAX_NAME_LENGTH];
        str_copy(aBuf, m_pClient->m_aClients[NewSpectatorID].m_aName, sizeof(aBuf));
        float TWidth = TextRender()->TextWidth(0, 6.0f, aBuf, -1);
        TextRender()->Text(0x0, TeePos.x-TWidth/2 - (60.0f - offSet), TeePos.y+8.0f, 6.0f, aBuf, -1);
	}

    GotNewSpectatorID = false;
    for(int i = m_pClient->m_Snap.m_SpecInfo.m_SpectatorID + 1; i < MAX_CLIENTS; i++)
    {
        if(!m_pClient->m_Snap.m_paPlayerInfos[i] || m_pClient->m_Snap.m_paPlayerInfos[i]->m_Team == TEAM_SPECTATORS)
            continue;

        NewSpectatorID = i;
        GotNewSpectatorID = true;
        break;
    }
	if (GotNewSpectatorID && offSet > 0.0f)
	{
        // draw the left box
        Graphics()->TextureSet(-1);
        Graphics()->QuadsBegin();
        Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.6f);
        RenderTools()->DrawRoundRectExt(m_Width-offSet, m_Height/2-30.f, 60.0f, 50.0f, 5.0f, CUI::CORNER_L);
        Graphics()->QuadsEnd();

        //Title
        float TWidth = TextRender()->TextWidth(0, 8.0f, "NEXT", -1);
        TextRender()->TextColor(1.0f, 0.39f, 0.0f, 0.95f);
        TextRender()->Text(0x0, ((m_Width-4.0f)-TWidth)+(60.0f - offSet), m_Height/2-30.f+2.0f, 8.0f, "NEXT", -1);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

        //Tee
        CNetObj_Character CurChar = m_pClient->m_Snap.m_aCharacters[NewSpectatorID].m_Cur;
        CNetObj_Character PrevChar = m_pClient->m_Snap.m_aCharacters[NewSpectatorID].m_Prev;
        float Angle = mix((float)PrevChar.m_Angle, (float)CurChar.m_Angle, IntraTick)/256.0f;
        vec2 Direction = GetDirection((int)(Angle*256.0f));
        vec2 TeePos = vec2((m_Width-offSet)+30.0f, m_Height/2-30.f+25.0f);
	    CTeeRenderInfo TeeInfo = m_pClient->m_aClients[NewSpectatorID].m_RenderInfo;
	    TeeInfo.m_Size =22.0f;
        //Freeze/Ninja
        if (CurChar.m_Weapon == WEAPON_NINJA)
        {
            // change the skin for the player to the ninja
            int Skin = m_pClient->m_pSkins->Find("x_ninja");
            if(Skin != -1)
            {
                TeeInfo.m_Texture = m_pClient->m_pSkins->Get(Skin)->m_OrgTexture;
                TeeInfo.m_ColorBody = vec4(1,1,1,1);
                TeeInfo.m_ColorFeet = vec4(1,1,1,1);
            }
        }

        RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, CurChar.m_Emote, Direction, TeePos);

        //Name Tee
        char aBuf[MAX_NAME_LENGTH];
        str_copy(aBuf, m_pClient->m_aClients[NewSpectatorID ].m_aName, sizeof(aBuf));
        TWidth = TextRender()->TextWidth(0, 6.0f, aBuf, -1);
        TextRender()->Text(0x0, TeePos.x-TWidth/2 + (60.0f - offSet), TeePos.y+8.0f, 6.0f, aBuf, -1);
	}
}


void CHud::RenderRecord()
{
    if (m_ServerRecord <= 0 && m_PlayerRecord <= 0)
        return;

    int h = (g_Config.m_hcUseHUD)?5:40;

    CUIRect PRec;
    PRec.x = 0.0f; PRec.y=h;
    PRec.w = 117.0f; PRec.h =21.0f;
    RenderTools()->DrawUIRect(&PRec, vec4(0.0f, 0.0f, 0.0f, 0.15f), CUI::CORNER_R, 5.0f);

    char aBuf[64];
    str_format(aBuf, sizeof(aBuf), Localize("Server best:"));
    TextRender()->Text(0, 5, h+3, 6, aBuf, -1);
    if(m_ServerRecord > 0)
        str_format(aBuf, sizeof(aBuf), "%02d:%05.2f", (int)m_ServerRecord/60, m_ServerRecord-((int)m_ServerRecord/60*60));
    else
        str_format(aBuf, sizeof(aBuf), "Undefined           ", (int)m_ServerRecord/60, m_ServerRecord-((int)m_ServerRecord/60*60));
    TextRender()->Text(0, 53, h+3, 6, aBuf, -1);


    str_format(aBuf, sizeof(aBuf), Localize("Personal best:"));
    TextRender()->Text(0, 5, h+7+3, 6, aBuf, -1);
	if(m_PlayerRecord > 0)
	{
		str_format(aBuf, sizeof(aBuf), "%02d:%05.2f", (int)m_PlayerRecord/60, m_PlayerRecord-((int)m_PlayerRecord/60*60));
		TextRender()->Text(0, 53, h+7+3, 6, aBuf, -1);
		str_format(aBuf, sizeof(aBuf), "+%02d:%05.2f", (int)(m_PlayerRecord-m_ServerRecord)/60, (m_PlayerRecord-m_ServerRecord)-((int)(m_PlayerRecord-m_ServerRecord)/60*60));
		if (m_PlayerRecord-m_ServerRecord > 0)
            TextRender()->TextColor(1.0f,0.0f,0.0f,1.0f);
        else
            TextRender()->TextColor(0.0f,1.0f,0.0f,1.0f);
		TextRender()->Text(0, 85, h+8+3, 5, aBuf, -1);
	}
	else
	{
        str_format(aBuf, sizeof(aBuf), "Undefined", (int)m_PlayerRecord/60, m_PlayerRecord-((int)m_PlayerRecord/60*60));
		TextRender()->Text(0, 53, h+7+3, 6, aBuf, -1);
	}

	 TextRender()->TextColor(1.0f,1.0f,1.0f,1.0f);
}

void CHud::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_DDRACETIME)
	{
		m_DDRaceTimeReceived = true;

		CNetMsg_Sv_DDRaceTime *pMsg = (CNetMsg_Sv_DDRaceTime *)pRawMsg;

		m_DDRaceTime = pMsg->m_Time;
		m_DDRaceTick = 0;

		m_LastReceivedTimeTick = Client()->GameTick();

		m_FinishTime = pMsg->m_Finish ? true : false;

		if(pMsg->m_Check)
		{
			m_CheckpointDiff = (float)pMsg->m_Check/100;
			m_CheckpointTick = Client()->GameTick();
		}
	}
	else if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;
		if(pMsg->m_Victim == m_pClient->m_Snap.m_LocalClientID)
		{
			m_CheckpointTick = 0;
			m_DDRaceTime = 0;
		}
	}
	else if(MsgType == NETMSGTYPE_SV_RECORD)
	{
		CNetMsg_Sv_Record *pMsg = (CNetMsg_Sv_Record *)pRawMsg;
		m_ServerRecord = (float)pMsg->m_ServerTimeBest/100;
		m_PlayerRecord = (float)pMsg->m_PlayerTimeBest/100;
	}
}

void CHud::RenderPlayerInfo()
{
    if (!g_Config.m_hcPlayerInfo)
        return;

    std::vector<CPlayerInfoLine> infomsg;
    const float top = 30.0f;

    if (m_pClient->m_LocalInfo.m_Jetpack)
        infomsg.push_back(CPlayerInfoLine("Have jetpack",1));
    if (m_pClient->m_LocalInfo.m_EndlessHook)
        infomsg.push_back(CPlayerInfoLine("Have endless hook",1));
    if (m_pClient->m_LocalInfo.m_SoloPart)
        infomsg.push_back(CPlayerInfoLine("In solo part",2));
    if (m_pClient->m_LocalInfo.m_Jumps != 2 || m_pClient->m_LocalInfo.m_InfiniteJumps)
    {
        if (m_pClient->m_LocalInfo.m_Jumps == 0)
            infomsg.push_back(CPlayerInfoLine("Can't Jump",3));

        else
        {
            if (!m_pClient->m_LocalInfo.m_InfiniteJumps)
            {
                char aBuf[128];
                str_format(aBuf, sizeof(aBuf), "Jumps: %d", m_pClient->m_LocalInfo.m_Jumps);
                infomsg.push_back(CPlayerInfoLine(aBuf,2));
            }
            else
                infomsg.push_back(CPlayerInfoLine("Jumps: Infinite",2));
        }
    }
    if (!m_pClient->m_LocalInfo.m_CanHook)
        infomsg.push_back(CPlayerInfoLine("Can't hook other players",3));
    if (!m_pClient->m_LocalInfo.m_CollidePlayers)
        infomsg.push_back(CPlayerInfoLine("Can't collide with other players",3));
    if (!m_pClient->m_LocalInfo.m_CanHit)
        infomsg.push_back(CPlayerInfoLine("Can't hit other players",3));

    if (!infomsg.empty())
    {
        std::vector<CPlayerInfoLine>::iterator it = infomsg.begin();
        int i=0;
        for (it=infomsg.begin(); it!=infomsg.end(); ++it,i++)
        {
            char aBuf[128];
            str_format(aBuf, sizeof(aBuf), "- %s", it->m_Text.c_str());

            if (it->m_Type == 1)
                TextRender()->TextColor(0.4f, 1.0f, 0.4f, 0.8f);
            else if (it->m_Type == 2)
                TextRender()->TextColor(1.0f, 1.0f, 0.4f, 0.8f);
            else if (it->m_Type == 3)
                TextRender()->TextColor(1.0f, 0.4f, 0.4f, 0.8f);

            TextRender()->Text(0, 5.0f, top+9.0f*i, 6.0f, aBuf, -1);
        }
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
}
