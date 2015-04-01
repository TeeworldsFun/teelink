/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <engine/demo.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <engine/serverbrowser.h> //H-Client

#include <game/generated/client_data.h>
#include <game/generated/protocol.h>

#include <game/localization.h>
#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/components/countryflags.h>
#include <game/client/components/motd.h>

#include "scoreboard.h"

#include <ctime> //H-Client


CScoreboard::CScoreboard()
{
	OnReset();
}

void CScoreboard::ConKeyScoreboard(IConsole::IResult *pResult, void *pUserData)
{
	((CScoreboard *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CScoreboard::OnReset()
{
	m_Active = false;
}

void CScoreboard::OnRelease()
{
	m_Active = false;
}

void CScoreboard::OnConsoleInit()
{
	Console()->Register("+scoreboard", "", CFGFLAG_CLIENT, ConKeyScoreboard, this, "Show scoreboard");
}

void CScoreboard::RenderGoals(float x, float y, float w)
{
    //TODO: Tutututututuuuuu
    w += 350.0f;
    x -= 175.0f;

	float h = 140.0f;
    char aBuf[128];
	CUIRect area, rTitle, rExtraInfo;
	CServerInfo CurrentServerInfo;

	area.x = x; area.y = y;
	area.w = w; area.h = h;
	area.HSplitTop(60.0f, &rTitle, &rExtraInfo);

	Client()->GetServerInfo(&CurrentServerInfo);

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);

	//Graphics()->QuadsBegin();
	//Graphics()->SetColor(0,0,0,0.5f);
	//Graphics()->QuadsEnd();
	/**H-Client**/
	//Title
	RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcContainerHeaderBackgroundColor), 0, 0.0f);
	rTitle.Margin(10.0f, &rTitle);
	float textWidth = TextRender()->TextWidth(0, 32.0f, CurrentServerInfo.m_aName, str_length(CurrentServerInfo.m_aName));
	TextRender()->Text(0, rTitle.x + rTitle.w/2 - textWidth/2, rTitle.y, 32.0f, CurrentServerInfo.m_aName, -1);

	//Extra Info
	RenderTools()->DrawUIRect(&rExtraInfo, HexToVec4(g_Config.m_hcContainerBackgroundColor), CUI::CORNER_B, 10.0f);
	rExtraInfo.Margin(10.0f, &rExtraInfo);
	/**/

	// render goals
	//y += 10.0f;
	//if(m_pClient->m_Snap.m_pGameInfoObj)
	//{
        CUIRect rRow;
        rExtraInfo.HSplitTop(27.0f, &rRow, &rExtraInfo);
        float colSize = rRow.w/4;

        //ScoreLimit
		{
			if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_ScoreLimit)
                str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Score limit"), m_pClient->m_Snap.m_pGameInfoObj->m_ScoreLimit);
            else
                str_format(aBuf, sizeof(aBuf), "%s: ∞", Localize("Score limit"));
			TextRender()->Text(0, rRow.x, rRow.y, 20.0f, aBuf, -1);
		}

        //TimeLimit
		{
            rRow.x += colSize+30.0f;
			if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit)
                str_format(aBuf, sizeof(aBuf), Localize("Time limit: %d min"), m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit);
            else
                 str_format(aBuf, sizeof(aBuf), "%s: ∞", Localize("Time limit"));
			TextRender()->Text(0, rRow.x, rRow.y, 20.0f, aBuf, -1);
		}

		//Round
		{
		    rRow.x += colSize+15.0f;
			if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_RoundNum && m_pClient->m_Snap.m_pGameInfoObj->m_RoundCurrent)
                str_format(aBuf, sizeof(aBuf), "%s: %d/%d", Localize("Round"), m_pClient->m_Snap.m_pGameInfoObj->m_RoundCurrent, m_pClient->m_Snap.m_pGameInfoObj->m_RoundNum);
            else
                str_format(aBuf, sizeof(aBuf), "%s: ∞", Localize("Round"));
			TextRender()->Text(0, rRow.x, rRow.y, 20.0f, aBuf, -1);
		}

		//Current
		{
		    rRow.x += colSize;
		    int Time = 0;
		    if (m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)
		    {
                Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed();
                str_format(aBuf, sizeof(aBuf), "%s: %d:%02d", Localize("Current Time"), Time/60, Time%60);
		    }
            else
                str_format(aBuf, sizeof(aBuf), "%s: ∞", Localize("Current Time"));
			TextRender()->Text(0, rRow.x, rRow.y, 20.0f, aBuf, -1);
		}

        rExtraInfo.HSplitTop(27.0f, &rRow, &rExtraInfo);
        colSize = rRow.w/4;
		//GameType
		{
			str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("GameType"), CurrentServerInfo.m_aGameType);
			TextRender()->Text(0, rRow.x, rRow.y, 20.0f, aBuf, -1);
		}

		//Players
		{
		    rRow.x += colSize+30.f;
            str_format(aBuf, sizeof(aBuf), "%s: %d/%d", Localize("Players"), CurrentServerInfo.m_NumPlayers, CurrentServerInfo.m_MaxPlayers);
			TextRender()->Text(0, rRow.x, rRow.y, 20.0f, aBuf, -1);
		}

		//Map
		{
		    rRow.x += colSize+15.0f;
            str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Map"), CurrentServerInfo.m_aMap);
			TextRender()->Text(0, rRow.x, rRow.y, 20.0f, aBuf, -1);
		}

		//Remaining Time
		{
		    rRow.x += colSize;
            int Time = 0;
            if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
            {
                Time = m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit*60 - ((Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed());
                if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
                    Time = 0;

                str_format(aBuf, sizeof(aBuf), "%s: %d:%02d", Localize("Remaining Time"), Time/60, Time%60);
            }
            else
                str_format(aBuf, sizeof(aBuf), "%s: ∞", Localize("Remaining Time"));
			TextRender()->Text(0, rRow.x, rRow.y, 20.0f, aBuf, -1);
		}
	//}
}

void CScoreboard::RenderSpectators(float x, float y, float w)
{
	float h = 140.0f;

	CUIRect area, rTitle, rExtraInfo;

	area.x = x; area.y = y;
	area.w = w; area.h = h;


	// background
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);

    area.HSplitTop(40.0f, &rTitle, &rExtraInfo);
	RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcContainerHeaderBackgroundColor), CUI::CORNER_T, 10.0f);
	rTitle.Margin(5.0f, &rTitle);

	// spectator names
	RenderTools()->DrawUIRect(&rExtraInfo, HexToVec4(g_Config.m_hcContainerBackgroundColor), 0, 0.0f);
	y += 45.0f;
	char aBuffer[1024*4];
	aBuffer[0] = 0;
	bool Multiple = false;
	int NumPlayers = 0;
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paPlayerInfos[i];
		if(!pInfo || pInfo->m_Team != TEAM_SPECTATORS)
			continue;

        NumPlayers++;

		if(Multiple)
			str_append(aBuffer, ", ", sizeof(aBuffer));
		str_append(aBuffer, m_pClient->m_aClients[pInfo->m_ClientID].m_aName, sizeof(aBuffer));
		Multiple = true;
	}
	CTextCursor Cursor;
	TextRender()->SetCursor(&Cursor, x+10.0f, y, 22.0f, TEXTFLAG_RENDER);
	Cursor.m_LineWidth = w-20.0f;
	Cursor.m_MaxLines = 4;
	TextRender()->TextEx(&Cursor, aBuffer, -1);

	//Title
	str_format(aBuffer, sizeof(aBuffer), "%s [%d]", Localize("Spectators"), NumPlayers);
	TextRender()->Text(0, rTitle.x, rTitle.y-3.0f, 28.0f, aBuffer, -1);
}

void CScoreboard::RenderScoreboard64(float x, float y, float w, int Team, const char *pTitle)
{
	if(Team == TEAM_SPECTATORS)
		return;

    float h = 760.0f;
	CUIRect area;
	area.x = x; area.y = y;
	area.w = w; area.h = h;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);

	// render title
	CUIRect rTitle;
	area.HSplitTop(60.0f, &rTitle, &area);
	float TitleFontsize = 40.0f;

    if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
        pTitle = Localize("Game over");
    else
        pTitle = Localize("Score board");

    RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcContainerHeaderBackgroundColor), CUI::CORNER_T, 10.0f);


	rTitle.Margin(10.0f, &rTitle);

	char aBuf[128] = {0};
    if(m_pClient->m_Snap.m_pGameDataObj)
    {
        int Score = Team == TEAM_RED ? m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed : m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue;
        str_format(aBuf, sizeof(aBuf), "%d", Score);
    }

	float tw = TextRender()->TextWidth(0, TitleFontsize, aBuf, -1);
	CUIRect rTitlePoints;
	rTitle.VSplitRight(tw+5.0f, 0x0, &rTitlePoints);
	TextRender()->Text(0, rTitlePoints.x, rTitlePoints.y-5.0f, TitleFontsize, aBuf, -1);

    float LineHeight = 40.0f;
    float TeeSizeMod = 0.4f;
    float Spacing = 0.0f;
    float HeadlineFontsize = 12.0f;
    float FontSize = 14.0f;
    int part = 0;
    CTextCursor Cursor;

    CUIRect parts[4];
    area.VSplitMid(&parts[0], &parts[2]);
    parts[0].VSplitMid(&parts[0], &parts[1]);
    parts[2].VSplitMid(&parts[2], &parts[3]);

    float ScoreOffset, ScoreLength;
    float TeeOffset, TeeLength;
    float NameOffset, NameLength;
    float PingOffset, PingLength;
    float CountryOffset, CountryLength;
    float ClanOffset, ClanLength;

    int nPlayers = 0;
    bool fi = false;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
	    if (!fi || (i%16) == 0)
	    {
	        if (fi)
                part++;

            x = parts[part].x;
            y = parts[part].y;

	        ScoreOffset = x+10.0f; ScoreLength = 60.0f;
            TeeOffset = parts[part].x+90.0f; TeeLength = 60*TeeSizeMod;
            NameOffset = TeeOffset+TeeLength; NameLength = 200.0f-TeeLength;
            PingOffset = x+390.0f; PingLength = 65.0f;
            CountryOffset = PingOffset-(LineHeight-Spacing-TeeSizeMod); CountryLength = (LineHeight-Spacing-TeeSizeMod*3.0f)*2.0f;
            ClanOffset = x+230.0f; ClanLength = 230.0f-CountryLength;

            // render headlines
            CUIRect rHeadLines, rLabel;
            parts[part].HSplitTop(40.0f, &rHeadLines, &parts[part]);
            RenderTools()->DrawUIRect(&rHeadLines, HexToVec4(g_Config.m_hcListHeaderBackgroundColor), 0, 0.0f);
            rHeadLines.Margin(5.0f,&rHeadLines);

            y += 50.0f;

            rHeadLines.VSplitLeft(80.0f, &rLabel, &rHeadLines);
            TextRender()->Text(0, rLabel.x, rLabel.y, HeadlineFontsize, Localize("Score"), -1);

            rHeadLines.VSplitLeft(200.0f, &rLabel, &rHeadLines);
            TextRender()->Text(0, rLabel.x, rLabel.y, HeadlineFontsize, Localize("Name"), -1);

            rHeadLines.VSplitLeft(150.0f, &rLabel, &rHeadLines);
            TextRender()->Text(0, rLabel.x, rLabel.y, HeadlineFontsize, Localize("Clan"), -1);

            rHeadLines.VSplitLeft(80.0f, &rLabel, &rHeadLines);
            TextRender()->Text(0, rLabel.x, rLabel.y, HeadlineFontsize, Localize("Ping"), -1);

            int corner = 0;
            if (part==0) corner = CUI::CORNER_BL;
            else if (part==3) corner = CUI::CORNER_BR;
            vec4 baColor = HexToVec4(g_Config.m_hcContainerBackgroundColor);
            RenderTools()->DrawUIRect(&parts[part], vec4(baColor.r, baColor.g, baColor.b, part%2==0?0.5f:0.65f), corner, 10.0f);

            fi = true;
	    }

		// make sure that we render the correct team
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByDDTeam[i];
		if(!pInfo)
			continue;

        nPlayers++;

		// background so it's easy to find the local player or the followed one in spectator mode
		if(pInfo->m_Local || (m_pClient->m_Snap.m_SpecInfo.m_Active && pInfo->m_ClientID == m_pClient->m_Snap.m_SpecInfo.m_SpectatorID))
		{
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
			RenderTools()->DrawRoundRectExt(x, y, parts[part].w, LineHeight, 0.0f, 0);
			Graphics()->QuadsEnd();
		}

		// score
		str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Score, -999, 999));
		tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
		TextRender()->SetCursor(&Cursor, ScoreOffset+ScoreLength-tw, y+Spacing+3.0f, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = ScoreLength;
		TextRender()->TextEx(&Cursor, aBuf, -1);

		// avatar
		CTeeRenderInfo TeeInfo = m_pClient->m_aClients[pInfo->m_ClientID].m_RenderInfo;
		TeeInfo.m_Size *= TeeSizeMod;
		RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, EMOTE_NORMAL, vec2(1.0f, 0.0f), vec2(TeeOffset+TeeLength/2, y+LineHeight/2-3.0f));

		// name
        if (g_Config.m_hcColorClan && m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID].m_aClan[0] != 0 && str_comp_nocase(m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID].m_aClan) == 0) //H-Client
            TextRender()->TextColor(0.7f, 0.7f, 0.2f, 1.0f);
		TextRender()->SetCursor(&Cursor, NameOffset, y+Spacing+3.0f, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = NameLength;
		TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aName, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

		// clan
		tw = TextRender()->TextWidth(0, FontSize, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);
		TextRender()->SetCursor(&Cursor, ClanOffset+ClanLength/2-tw/2-5.0f, y+Spacing+3.0f, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = ClanLength;
		TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);

		// country flag
		Graphics()->TextureSet(m_pClient->m_pCountryFlags->GetByCountryCode(m_pClient->m_aClients[pInfo->m_ClientID].m_Country)->m_Texture);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.5f);
		IGraphics::CQuadItem QuadItem(CountryOffset, y+(Spacing+TeeSizeMod*5.0f)/2.0f, CountryLength, LineHeight-Spacing-TeeSizeMod*5.0f);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
		Graphics()->QuadsEnd();

		// ping
        //H-Client
        const int maxLatency = 200;
        vec3 Rgb =  HslToRgb(vec3(1.0f, 1.0f, 0.5f));
        if (pInfo->m_Latency <= maxLatency)
            Rgb = HslToRgb(vec3((((maxLatency - pInfo->m_Latency) * 0.35f) / maxLatency), 1.0f, 0.5f));
        TextRender()->TextColor(Rgb.r, Rgb.g, Rgb.b, 1.0f);
        //

		str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Latency, 0, 1000));
		tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
		TextRender()->SetCursor(&Cursor, PingOffset+PingLength-tw, y+Spacing+3.0f, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = PingLength;
		TextRender()->TextEx(&Cursor, aBuf, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

		y += LineHeight+Spacing;
	}

	//Title
    char aTitle[128]={0};
    str_format(aTitle, sizeof(aTitle), "%s [%d]", pTitle, nPlayers);
	TextRender()->Text(0, rTitle.x, rTitle.y-5.0f, TitleFontsize, aTitle, -1);
}

void CScoreboard::RenderScoreboard(float x, float y, float w, int Team, const char *pTitle)
{
	if(Team == TEAM_SPECTATORS)
		return;

    float h = 430.0f;

    if(!pTitle)
        h = 760.0f;
	CUIRect area;
	area.x = x; area.y = y;
	area.w = w; area.h = h;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);

	// render title
	CUIRect rTitle;
	area.HSplitTop(60.0f, &rTitle, &area);
	float TitleFontsize = 40.0f;
	if(!pTitle)
	{
		if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
			pTitle = Localize("Game over");
		else
			pTitle = Localize("Score board");

        RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcContainerHeaderBackgroundColor), CUI::CORNER_T, 10.0f);
	}
	else
	{
	    if (Team == TEAM_RED)
            RenderTools()->DrawUIRect(&rTitle, vec4(1.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_T, 10.0f);
	    if (Team == TEAM_BLUE)
            RenderTools()->DrawUIRect(&rTitle, vec4(0.0f, 0.0f, 1.0f, 0.5f), CUI::CORNER_T, 10.0f);
	}

	rTitle.Margin(10.0f, &rTitle);

	char aBuf[128] = {0};
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
	{
		if(m_pClient->m_Snap.m_pGameDataObj)
		{
			int Score = Team == TEAM_RED ? m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed : m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue;
			str_format(aBuf, sizeof(aBuf), "%d", Score);
		}
	}
	else
	{
		if(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW &&
			m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID])
		{
			int Score = m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID]->m_Score;
			str_format(aBuf, sizeof(aBuf), "%d", Score);
		}
		else if(m_pClient->m_Snap.m_pLocalInfo)
		{
			int Score = m_pClient->m_Snap.m_pLocalInfo->m_Score;
			str_format(aBuf, sizeof(aBuf), "%d", Score);
		}
	}
	float tw = TextRender()->TextWidth(0, TitleFontsize, aBuf, -1);
	CUIRect rTitlePoints;
	rTitle.VSplitRight(tw+5.0f, 0x0, &rTitlePoints);
	TextRender()->Text(0, rTitlePoints.x, rTitlePoints.y-5.0f, TitleFontsize, aBuf, -1);

	// calculate measurements
	x += 10.0f;

    float LineHeight = 40.0f;
    float TeeSizeMod = 0.8f;
    float Spacing = 0.0f;

	float ScoreOffset = x+10.0f, ScoreLength = 60.0f;
	float TeeOffset = area.x+90.0f, TeeLength = 60*TeeSizeMod;
	float NameOffset = TeeOffset+TeeLength, NameLength = 300.0f-TeeLength;
	float PingOffset = x+610.0f, PingLength = 65.0f;
	float CountryOffset = PingOffset-(LineHeight-Spacing-TeeSizeMod*5.0f)*2.0f, CountryLength = (LineHeight-Spacing-TeeSizeMod*5.0f)*2.0f;
	float ClanOffset = x+370.0f, ClanLength = 230.0f-CountryLength;

	// render headlines
	CUIRect rHeadLines, rLabel;
	area.HSplitTop(40.0f, &rHeadLines, &area);
	RenderTools()->DrawUIRect(&rHeadLines, HexToVec4(g_Config.m_hcListHeaderBackgroundColor), 0, 0.0f);
	rHeadLines.Margin(5.0f,&rHeadLines);

	y += 50.0f;
	float HeadlineFontsize = 22.0f;

	rHeadLines.VSplitLeft(90.0f, &rLabel, &rHeadLines);
	TextRender()->Text(0, rLabel.x, rLabel.y, HeadlineFontsize, Localize("Score"), -1);

    rHeadLines.VSplitLeft(335.0f, &rLabel, &rHeadLines);
	TextRender()->Text(0, rLabel.x, rLabel.y, HeadlineFontsize, Localize("Name"), -1);

    rHeadLines.VSplitLeft(210.0f, &rLabel, &rHeadLines);
	TextRender()->Text(0, rLabel.x, rLabel.y, HeadlineFontsize, Localize("Clan"), -1);

    rHeadLines.VSplitLeft(80.0f, &rLabel, &rHeadLines);
	TextRender()->Text(0, rLabel.x, rLabel.y, HeadlineFontsize, Localize("Ping"), -1);

    RenderTools()->DrawUIRect(&area, HexToVec4(g_Config.m_hcContainerBackgroundColor), CUI::CORNER_B, 10.0f);

	// render player entries
	float FontSize = 24.0f;
	CTextCursor Cursor;

	x = area.x;
	y = area.y;

    int nPlayers = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// make sure that we render the correct team
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];
		if(!pInfo || pInfo->m_Team != Team)
			continue;

        nPlayers++;

		// background so it's easy to find the local player or the followed one in spectator mode
		if(pInfo->m_Local || (m_pClient->m_Snap.m_SpecInfo.m_Active && pInfo->m_ClientID == m_pClient->m_Snap.m_SpecInfo.m_SpectatorID))
		{
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
			RenderTools()->DrawRoundRectExt(x, y, w, LineHeight, 0.0f, 0);
			Graphics()->QuadsEnd();
		}

		// score
		str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Score, -999, 999));
		tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
		TextRender()->SetCursor(&Cursor, ScoreOffset+ScoreLength-tw, y+Spacing+3.0f, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = ScoreLength;
		TextRender()->TextEx(&Cursor, aBuf, -1);

		// flag
		if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS &&
			m_pClient->m_Snap.m_pGameDataObj && (m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierRed == pInfo->m_ClientID ||
			m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue == pInfo->m_ClientID))
		{
			Graphics()->BlendNormal();
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
			Graphics()->QuadsBegin();

			RenderTools()->SelectSprite(pInfo->m_Team==TEAM_RED ? SPRITE_FLAG_BLUE : SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);

			float Size = LineHeight;
			IGraphics::CQuadItem QuadItem(TeeOffset+0.0f, y-5.0f-Spacing/2.0f+1.0f, Size/2.0f, Size);
			Graphics()->QuadsDrawTL(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}

		// avatar
		CTeeRenderInfo TeeInfo = m_pClient->m_aClients[pInfo->m_ClientID].m_RenderInfo;
		TeeInfo.m_Size *= TeeSizeMod;
		RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, EMOTE_NORMAL, vec2(1.0f, 0.0f), vec2(TeeOffset+TeeLength/2, y+LineHeight/2+1.0f));

		// name
        if (g_Config.m_hcColorClan && m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID].m_aClan[0] != 0 && str_comp_nocase(m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID].m_aClan) == 0) //H-Client
            TextRender()->TextColor(0.7f, 0.7f, 0.2f, 1.0f);
		TextRender()->SetCursor(&Cursor, NameOffset, y+Spacing+3.0f, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = NameLength;
		TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aName, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

		// clan
		tw = TextRender()->TextWidth(0, FontSize, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);
		TextRender()->SetCursor(&Cursor, ClanOffset+ClanLength/2-tw/2-5.0f, y+Spacing+3.0f, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = ClanLength;
		TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);

		// country flag
		Graphics()->TextureSet(m_pClient->m_pCountryFlags->GetByCountryCode(m_pClient->m_aClients[pInfo->m_ClientID].m_Country)->m_Texture);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.5f);
		IGraphics::CQuadItem QuadItem(CountryOffset, y+(Spacing+TeeSizeMod*5.0f)/2.0f, CountryLength, LineHeight-Spacing-TeeSizeMod*5.0f);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
		Graphics()->QuadsEnd();

		// ping
        //H-Client
        const int maxLatency = 200;
        vec3 Rgb =  HslToRgb(vec3(1.0f, 1.0f, 0.5f));
        if (pInfo->m_Latency <= maxLatency)
            Rgb = HslToRgb(vec3((((maxLatency - pInfo->m_Latency) * 0.35f) / maxLatency), 1.0f, 0.5f));
        TextRender()->TextColor(Rgb.r, Rgb.g, Rgb.b, 1.0f);
        //

		str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Latency, 0, 1000));
		tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
		TextRender()->SetCursor(&Cursor, PingOffset+PingLength-tw, y+Spacing+3.0f, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
		Cursor.m_LineWidth = PingLength;
		TextRender()->TextEx(&Cursor, aBuf, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

		y += LineHeight+Spacing;
	}

	//Title
    char aTitle[128]={0};
    str_format(aTitle, sizeof(aTitle), "%s [%d]", pTitle, nPlayers);
	TextRender()->Text(0, rTitle.x, rTitle.y-5.0f, TitleFontsize, aTitle, -1);
}

void CScoreboard::RenderLocalTime()
{
	//draw the box
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(HexToVec4(g_Config.m_hcContainerBackgroundColor));
	RenderTools()->DrawRoundRectExt(0.0f, 180.0f, 190.0f, 75.0f, 15.0f, CUI::CORNER_R);
	Graphics()->QuadsEnd();

	//draw the text
    char aBuf[12];
    time_t rawtime = time(NULL);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(aBuf, sizeof(aBuf), "%I:%M:%S %p", timeinfo);
    TextRender()->Text(0, 10.0f, 190.0f, 20.0f, Localize("LOCAL TIME"), -1);
	TextRender()->Text(0, 40.0f, 215.0f, 20.0f, aBuf, -1);
}

void CScoreboard::RenderRecordingNotification()
{
	if(!m_pClient->DemoRecorder()->IsRecording())
		return;

	//draw the box
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(HexToVec4(g_Config.m_hcContainerBackgroundColor));
	RenderTools()->DrawRoundRectExt(0.0f, 150.0f, 180.0f, 50.0f, 15.0f, CUI::CORNER_R);
	Graphics()->QuadsEnd();

	//draw the red dot
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	RenderTools()->DrawRoundRect(20, 165.0f, 20.0f, 20.0f, 10.0f);
	Graphics()->QuadsEnd();

	//draw the text
	char aBuf[64];
	int Seconds = m_pClient->DemoRecorder()->Length();
	str_format(aBuf, sizeof(aBuf), Localize("REC %3d:%02d"), Seconds/60, Seconds%60);
	TextRender()->Text(0, 50.0f, 160.0f, 20.0f, aBuf, -1);
}

void CScoreboard::OnRender()
{
    static bool s_NeedUpdate = true;
	if(!Active())
		return;

	// if the score board is active, then we should clear the motd message aswell
	if(m_pClient->m_pMotd->IsActive())
		m_pClient->m_pMotd->Clear();


	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width, Height);

	float w = 700.0f;

	if(m_pClient->m_Snap.m_pGameInfoObj)
	{
		if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS))
		{
            // H-Client: DDNet
            CServerInfo SInfo;
            Client()->GetServerInfo(&SInfo);
		    if (SInfo.m_MaxPlayers > 16)
                RenderScoreboard64(10.0f, 270.0f, Width-20.0f, 0, 0);
            else
                RenderScoreboard(Width/2-w/2, 270.0f, w, 0, 0);

		    if (Client()->State() != IClient::STATE_DEMOPLAYBACK && s_NeedUpdate)
		    {
                CServerInfoReg *ServerInfo = ServerBrowser()->GetServerInfoReg(g_Config.m_UiServerAddress); //H-Client
                if (ServerInfo)
                {
                    if (m_pClient->m_Snap.m_paInfoByScore[0]->m_ClientID != m_pClient->m_Snap.m_LocalClientID)
                        ServerInfo->m_Losts++;
                    else
                        ServerInfo->m_Wins++;
                }

                s_NeedUpdate = false;
		    }
		}
		else
		{
			const char *pRedClanName = GetClanName(TEAM_RED);
			const char *pBlueClanName = GetClanName(TEAM_BLUE);

			if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER && m_pClient->m_Snap.m_pGameDataObj)
			{
				char aText[256];
				str_copy(aText, Localize("Draw!"), sizeof(aText));

				if(m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed > m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue)
				{
					if(pRedClanName)
						str_format(aText, sizeof(aText), Localize("%s wins!"), pRedClanName);
					else
						str_copy(aText, Localize("Red team wins!"), sizeof(aText));

                    if (s_NeedUpdate)
                    {
                        CServerInfoReg *ServerInfo = ServerBrowser()->GetServerInfoReg(g_Config.m_UiServerAddress); //H-Client
                        if (ServerInfo)
                        {
                            if (m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_RED)
                                ServerInfo->m_Wins++;
                            else
                                ServerInfo->m_Losts++;
                        }

                        s_NeedUpdate = false;
                    }
				}
				else if(m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue > m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed)
				{
					if(pBlueClanName)
						str_format(aText, sizeof(aText), Localize("%s wins!"), pBlueClanName);
					else
						str_copy(aText, Localize("Blue team wins!"), sizeof(aText));

                    if (s_NeedUpdate)
                    {
                        CServerInfoReg *ServerInfo = ServerBrowser()->GetServerInfoReg(g_Config.m_UiServerAddress); //H-Client
                        if (ServerInfo)
                        {
                            if (m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_BLUE)
                                ServerInfo->m_Wins++;
                            else
                                ServerInfo->m_Losts++;
                        }

                        s_NeedUpdate = false;
                    }
				}

				float w = TextRender()->TextWidth(0, 86.0f, aText, -1);
				TextRender()->Text(0, Width/2-w/2, 150.0f, 86.0f, aText, -1);
			}
			else if (!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER) && !s_NeedUpdate)
                s_NeedUpdate = true;

			RenderScoreboard(Width/2-w-5.0f, 270.0f, w, TEAM_RED, pRedClanName ? pRedClanName : Localize("Red team"));
			RenderScoreboard(Width/2+5.0f, 270.0f, w, TEAM_BLUE, pBlueClanName ? pBlueClanName : Localize("Blue team"));
		}
	}

	RenderGoals(Width/2-w/2, 0, w);
	RenderSpectators(Width/2-w/2, 230+760+10+50+10, w);
	RenderRecordingNotification();
	RenderLocalTime();
}

bool CScoreboard::Active()
{
	// if we activly wanna look on the scoreboard
	if(m_Active)
		return true;

	if(m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team != TEAM_SPECTATORS)
	{
		// we are not a spectator, check if we are dead
		if(!m_pClient->m_Snap.m_pLocalCharacter)
			return true;
	}

	// if the game is over
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
		return true;

	return false;
}

const char *CScoreboard::GetClanName(int Team)
{
	int ClanPlayers = 0;
	const char *pClanName = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];
		if(!pInfo || pInfo->m_Team != Team)
			continue;

		if(!pClanName)
		{
			pClanName = m_pClient->m_aClients[pInfo->m_ClientID].m_aClan;
			ClanPlayers++;
		}
		else
		{
			if(str_comp(m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, pClanName) == 0)
				ClanPlayers++;
			else
				return 0;
		}
	}

	if(ClanPlayers > 1 && pClanName[0])
		return pClanName;
	else
		return 0;
}
