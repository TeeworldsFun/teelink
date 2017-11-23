/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>

#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/texturepack.h> // H-Client
#include <engine/updater.h> //H-Client
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>
#include <engine/client/stats.h> //H-Client

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/components/sounds.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/localization.h>

#include <game/client/components/effects.h> // H-Client

#include "binds.h"
#include "countryflags.h"
#include "menus.h"
#include "skins.h"

#include <list> // H-Client
#include <cstdio> // H-Client
#include <ctime> // H-Client

CMenusKeyBinder CMenus::m_Binder;

CMenusKeyBinder::CMenusKeyBinder()
{
	m_TakeKey = false;
	m_GotKey = false;
}

bool CMenusKeyBinder::OnInput(IInput::CEvent Event)
{
	if(m_TakeKey)
	{
		if(Event.m_Flags&IInput::FLAG_PRESS)
		{
			m_Key = Event;
			m_GotKey = true;
			m_TakeKey = false;
		}
		return true;
	}

	return false;
}

void CMenus::RenderSettingsGeneral(CUIRect MainView)
{
	char aBuf[128];
	CUIRect Label, Button, Left, Right, Game, Client;
	MainView.HSplitTop(150.0f, &Game, &Client);

	// game
	{
		// headline
		Game.HSplitTop(30.0f, &Label, &Game);
		UI()->DoLabelScaled(&Label, Localize("Game"), 20.0f, -1);
		Game.Margin(5.0f, &Game);
		Game.VSplitMid(&Left, &Right);
		Left.VSplitRight(5.0f, &Left, 0);
		Right.VMargin(5.0f, &Right);

		// dynamic camera
		Left.HSplitTop(20.0f, &Button, &Left);
		static int s_DynamicCameraButton = 0;
		if(DoButton_CheckBox(&s_DynamicCameraButton, Localize("Dynamic Camera"), g_Config.m_ClMouseDeadzone != 0, &Button))
		{
			if(g_Config.m_ClMouseDeadzone)
			{
				g_Config.m_ClMouseFollowfactor = 0;
				g_Config.m_ClMouseMaxDistance = 400;
				g_Config.m_ClMouseDeadzone = 0;
			}
			else
			{
				g_Config.m_ClMouseFollowfactor = 60;
				g_Config.m_ClMouseMaxDistance = 1000;
				g_Config.m_ClMouseDeadzone = 300;
			}
		}

		// weapon pickup
		Left.HSplitTop(5.0f, 0, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_ClAutoswitchWeapons, Localize("Switch weapon on pickup"), g_Config.m_ClAutoswitchWeapons, &Button))
			g_Config.m_ClAutoswitchWeapons ^= 1;

		// show hud
		Left.HSplitTop(5.0f, 0, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_ClShowhud, Localize("Show ingame HUD"), g_Config.m_ClShowhud, &Button))
			g_Config.m_ClShowhud ^= 1;

		// chat messages
		Left.HSplitTop(5.0f, 0, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_ClShowChatFriends, Localize("Show only chat messages from friends"), g_Config.m_ClShowChatFriends, &Button))
			g_Config.m_ClShowChatFriends ^= 1;

		// name plates
		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_ClNameplates, Localize("Show name plates"), g_Config.m_ClNameplates, &Button))
			g_Config.m_ClNameplates ^= 1;

		if(g_Config.m_ClNameplates)
		{
			Right.HSplitTop(2.5f, 0, &Right);
			Right.VSplitLeft(30.0f, 0, &Right);
			Right.HSplitTop(20.0f, &Button, &Right);
			if(DoButton_CheckBox(&g_Config.m_ClNameplatesAlways, Localize("Always show name plates"), g_Config.m_ClNameplatesAlways, &Button))
				g_Config.m_ClNameplatesAlways ^= 1;

			Right.HSplitTop(2.5f, 0, &Right);
			Right.HSplitTop(20.0f, &Label, &Right);
			Right.HSplitTop(20.0f, &Button, &Right);
			str_format(aBuf, sizeof(aBuf), "%s: %i", Localize("Name plates size"), g_Config.m_ClNameplatesSize);
			UI()->DoLabelScaled(&Label, aBuf, 13.0f, -1);
			Button.HMargin(2.0f, &Button);
			g_Config.m_ClNameplatesSize = (int)(DoScrollbarH(&g_Config.m_ClNameplatesSize, &Button, g_Config.m_ClNameplatesSize/100.0f)*100.0f+0.1f);

			Right.HSplitTop(20.0f, &Button, &Right);
			if(DoButton_CheckBox(&g_Config.m_ClNameplatesTeamcolors, Localize("Use team colors for name plates"), g_Config.m_ClNameplatesTeamcolors, &Button))
				g_Config.m_ClNameplatesTeamcolors ^= 1;
		}
	}

	// client
	{
		// headline
		Client.HSplitTop(30.0f, &Label, &Client);
		UI()->DoLabelScaled(&Label, Localize("Client"), 20.0f, -1);
		Client.Margin(5.0f, &Client);
		Client.VSplitMid(&Left, &Right);
		Left.VSplitRight(5.0f, &Left, 0);
		Right.VMargin(5.0f, &Right);

		// auto demo settings
		{
			Left.HSplitTop(20.0f, &Button, &Left);
			if(DoButton_CheckBox(&g_Config.m_ClAutoDemoRecord, Localize("Automatically record demos"), g_Config.m_ClAutoDemoRecord, &Button))
				g_Config.m_ClAutoDemoRecord ^= 1;

			Right.HSplitTop(20.0f, &Button, &Right);
			if(DoButton_CheckBox(&g_Config.m_ClAutoScreenshot, Localize("Automatically take game over screenshot"), g_Config.m_ClAutoScreenshot, &Button))
				g_Config.m_ClAutoScreenshot ^= 1;

			Left.HSplitTop(10.0f, 0, &Left);
			Left.VSplitLeft(20.0f, 0, &Left);
			Left.HSplitTop(20.0f, &Label, &Left);
			Button.VSplitRight(20.0f, &Button, 0);
			char aBuf[64];
			if(g_Config.m_ClAutoDemoMax)
				str_format(aBuf, sizeof(aBuf), "%s: %i", Localize("Max demos"), g_Config.m_ClAutoDemoMax);
			else
				str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Max demos"), Localize("no limit"));
			UI()->DoLabelScaled(&Label, aBuf, 13.0f, -1);
			Left.HSplitTop(20.0f, &Button, 0);
			Button.HMargin(2.0f, &Button);
			g_Config.m_ClAutoDemoMax = static_cast<int>(DoScrollbarH(&g_Config.m_ClAutoDemoMax, &Button, g_Config.m_ClAutoDemoMax/1000.0f)*1000.0f+0.1f);

			Right.HSplitTop(10.0f, 0, &Right);
			Right.VSplitLeft(20.0f, 0, &Right);
			Right.HSplitTop(20.0f, &Label, &Right);
			Button.VSplitRight(20.0f, &Button, 0);
			if(g_Config.m_ClAutoScreenshotMax)
				str_format(aBuf, sizeof(aBuf), "%s: %i", Localize("Max Screenshots"), g_Config.m_ClAutoScreenshotMax);
			else
				str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Max Screenshots"), Localize("no limit"));
			UI()->DoLabelScaled(&Label, aBuf, 13.0f, -1);
			Right.HSplitTop(20.0f, &Button, 0);
			Button.HMargin(2.0f, &Button);
			g_Config.m_ClAutoScreenshotMax = static_cast<int>(DoScrollbarH(&g_Config.m_ClAutoScreenshotMax, &Button, g_Config.m_ClAutoScreenshotMax/1000.0f)*1000.0f+0.1f);
		}
	}
}

void CMenus::RenderSettingsPlayer(CUIRect MainView)
{
	CUIRect Button, Label;
	MainView.HSplitTop(10.0f, 0, &MainView);

	// player name
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(150.0f, &Button, 0);
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Name"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	static float s_OffsetName = 0.0f;
	if(DoEditBox(g_Config.m_PlayerName, &Button, g_Config.m_PlayerName, sizeof(g_Config.m_PlayerName), 14.0f, &s_OffsetName))
		m_NeedSendinfo = true;

	// player clan
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(150.0f, &Button, 0);
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Clan"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	static float s_OffsetClan = 0.0f;
	if(DoEditBox(g_Config.m_PlayerClan, &Button, g_Config.m_PlayerClan, sizeof(g_Config.m_PlayerClan), 14.0f, &s_OffsetClan))
		m_NeedSendinfo = true;

	// country flag selector
	MainView.HSplitTop(20.0f, 0, &MainView);
	static float s_ScrollValue = 0.0f;
	int OldSelected = -1;
	UiDoListboxStart(&s_ScrollValue, &MainView, 50.0f, Localize("Country"), "", m_pClient->m_pCountryFlags->Num(), 6, OldSelected, s_ScrollValue);

	for(int i = 0; i < m_pClient->m_pCountryFlags->Num(); ++i)
	{
		const CCountryFlags::CCountryFlag *pEntry = m_pClient->m_pCountryFlags->GetByIndex(i);
		if(pEntry->m_CountryCode == g_Config.m_PlayerCountry)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&pEntry->m_CountryCode, OldSelected == i);
		if(Item.m_Visible)
		{
			CUIRect Label;
			Item.m_Rect.Margin(5.0f, &Item.m_Rect);
			Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
			float OldWidth = Item.m_Rect.w;
			Item.m_Rect.w = Item.m_Rect.h*2;
			Item.m_Rect.x += (OldWidth-Item.m_Rect.w)/ 2.0f;
			vec4 Color(1.0f, 1.0f, 1.0f, 1.0f);
			m_pClient->m_pCountryFlags->Render(pEntry->m_CountryCode, &Color, Item.m_Rect.x, Item.m_Rect.y, Item.m_Rect.w, Item.m_Rect.h);
			if(pEntry->m_Texture != -1)
				UI()->DoLabel(&Label, pEntry->m_aCountryCodeString, 10.0f, 0);
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		g_Config.m_PlayerCountry = m_pClient->m_pCountryFlags->GetByIndex(NewSelected)->m_CountryCode;
		m_NeedSendinfo = true;
	}
}

void CMenus::RenderSettingsTee(CUIRect MainView)
{
	CUIRect Button, Label;
	MainView.HSplitTop(10.0f, 0, &MainView);

	// skin info
	const CSkins::CSkin *pOwnSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(g_Config.m_PlayerSkin));
	CTeeRenderInfo OwnSkinInfo;
	if(g_Config.m_PlayerUseCustomColor)
	{
		OwnSkinInfo.m_Texture = pOwnSkin->m_ColorTexture;
		OwnSkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
		OwnSkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
	}
	else
	{
		OwnSkinInfo.m_Texture = pOwnSkin->m_OrgTexture;
		OwnSkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		OwnSkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	OwnSkinInfo.m_Size = 50.0f*UI()->Scale();

	MainView.HSplitTop(20.0f, &Label, &MainView);
	Label.VSplitLeft(230.0f, &Label, 0);
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Your skin"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0f, -1);

	MainView.HSplitTop(50.0f, &Label, &MainView);
	Label.VSplitLeft(230.0f, &Label, 0);
	RenderTools()->DrawUIRect(&Label, HexToVec4(g_Config.m_hcSubcontainerBackgroundColor), CUI::CORNER_ALL, 10.0f);
	RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, 0, vec2(1, 0), vec2(Label.x+30.0f, Label.y+28.0f));
	Label.HSplitTop(15.0f, 0, &Label);;
	Label.VSplitLeft(70.0f, 0, &Label);
	UI()->DoLabelScaled(&Label, g_Config.m_PlayerSkin, 14.0f, -1, 150.0f);

	// custom colour selector
	MainView.HSplitTop(20.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(230.0f, &Button, 0);
	if(DoButton_CheckBox(&g_Config.m_PlayerColorBody, Localize("Custom colors"), g_Config.m_PlayerUseCustomColor, &Button))
	{
		g_Config.m_PlayerUseCustomColor = g_Config.m_PlayerUseCustomColor?0:1;
		m_NeedSendinfo = true;
	}

	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(82.5f, &Label, &MainView);
	if(g_Config.m_PlayerUseCustomColor)
	{
		CUIRect aRects[2];
		Label.VSplitMid(&aRects[0], &aRects[1]);
		aRects[0].VSplitRight(10.0f, &aRects[0], 0);
		aRects[1].VSplitLeft(10.0f, 0, &aRects[1]);

		int *paColors[2];
		paColors[0] = &g_Config.m_PlayerColorBody;
		paColors[1] = &g_Config.m_PlayerColorFeet;

		const char *paParts[] = {
			Localize("Body"),
			Localize("Feet")};
		const char *paLabels[] = {
			Localize("Hue"),
			Localize("Sat."),
			Localize("Lht.")};
		static int s_aColorSlider[2][3] = {{0}};

		for(int i = 0; i < 2; i++)
		{
			aRects[i].HSplitTop(20.0f, &Label, &aRects[i]);
			UI()->DoLabelScaled(&Label, paParts[i], 14.0f, -1);
			aRects[i].VSplitLeft(20.0f, 0, &aRects[i]);
			aRects[i].HSplitTop(2.5f, 0, &aRects[i]);

			int PrevColor = *paColors[i];
			int Color = 0;
			for(int s = 0; s < 3; s++)
			{
				aRects[i].HSplitTop(20.0f, &Label, &aRects[i]);
				Label.VSplitLeft(100.0f, &Label, &Button);
				Button.HMargin(2.0f, &Button);

				float k = ((PrevColor>>((2-s)*8))&0xff) / 255.0f;
				k = DoScrollbarH(&s_aColorSlider[i][s], &Button, k);
				Color <<= 8;
				Color += clamp((int)(k*255), 0, 255);
				UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
			}

			if(PrevColor != Color)
				m_NeedSendinfo = true;

			*paColors[i] = Color;
		}
	}

	// skin selector
	MainView.HSplitTop(20.0f, 0, &MainView);
	static bool s_InitSkinlist = true;
	static sorted_array<const CSkins::CSkin *> s_paSkinList;
	static float s_ScrollValue = 0.0f;
	if(s_InitSkinlist)
	{
		s_paSkinList.clear();
		for(int i = 0; i < m_pClient->m_pSkins->Num(); ++i)
		{
			const CSkins::CSkin *s = m_pClient->m_pSkins->Get(i);
			// no special skins
			if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
				continue;
			s_paSkinList.add(s);
		}
		s_InitSkinlist = false;
	}

	int OldSelected = -1;
	UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Skins"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

	for(int i = 0; i < s_paSkinList.size(); ++i)
	{
		const CSkins::CSkin *s = s_paSkinList[i];
		if(s == 0)
			continue;

		if(str_comp(s->m_aName, g_Config.m_PlayerSkin) == 0)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
		if(Item.m_Visible)
		{
			CTeeRenderInfo Info;
			if(g_Config.m_PlayerUseCustomColor)
			{
				Info.m_Texture = s->m_ColorTexture;
				Info.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
				Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
			}
			else
			{
				Info.m_Texture = s->m_OrgTexture;
				Info.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				Info.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			Info.m_Size = UI()->Scale()*50.0f;
			Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, 0, vec2(1.0f, 0.0f), vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));

			if(g_Config.m_Debug)
			{
				vec3 BloodColor = g_Config.m_PlayerUseCustomColor ? m_pClient->m_pSkins->GetColorV3(g_Config.m_PlayerColorBody) : s->m_BloodColor;
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				Graphics()->SetColor(BloodColor.r, BloodColor.g, BloodColor.b, 1.0f);
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, 12.0f, 12.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		mem_copy(g_Config.m_PlayerSkin, s_paSkinList[NewSelected]->m_aName, sizeof(g_Config.m_PlayerSkin));
		m_NeedSendinfo = true;
	}
}


typedef void (*pfnAssignFuncCallback)(CConfiguration *pConfig, int Value);

typedef struct
{
	CLocConstString m_Name;
	const char *m_pCommand;
	int m_KeyId;
} CKeyInfo;

static CKeyInfo gs_aKeys[] =
{
	{ "Move left", "+left", 0},		// Localize - these strings are localized within CLocConstString
	{ "Move right", "+right", 0 },
	{ "Jump", "+jump", 0 },
	{ "Fire", "+fire", 0 },
	{ "Hook", "+hook", 0 },
	{ "Hook Collisions", "+showhookcoll", 0 },
	{ "Hammer", "+weapon1", 0 },
	{ "Pistol", "+weapon2", 0 },
	{ "Shotgun", "+weapon3", 0 },
	{ "Grenade", "+weapon4", 0 },
	{ "Rifle", "+weapon5", 0 },
	{ "Next weapon", "+nextweapon", 0 },
	{ "Prev. weapon", "+prevweapon", 0 },
	{ "Vote yes", "vote yes", 0 },
	{ "Vote no", "vote no", 0 },
	{ "Chat", "chat all", 0 },
	{ "Team chat", "chat team", 0 },
	{ "Show chat", "+show_chat", 0 },
	{ "Emoticon", "+emote", 0 },
	{ "Spectator mode", "+spectate", 0 },
	{ "Spectate next", "spectate_next", 0 },
	{ "Spectate previous", "spectate_previous", 0 },
	{ "Console", "toggle_local_console", 0 },
	{ "Remote console", "toggle_remote_console", 0 },
	{ "Screenshot", "screenshot", 0 },
	{ "Scoreboard", "+scoreboard", 0 },
	{ "Respawn", "kill", 0 },
	{ "Zoom mode", "switch_camera_mode", 0 },
};

/*	This is for scripts/update_localization.py to work, don't remove!
	Localize("Move left");Localize("Move right");Localize("Jump");Localize("Fire");Localize("Hook");Localize("Hammer");
	Localize("Pistol");Localize("Shotgun");Localize("Grenade");Localize("Rifle");Localize("Next weapon");Localize("Prev. weapon");
	Localize("Vote yes");Localize("Vote no");Localize("Chat");Localize("Team chat");Localize("Show chat");Localize("Emoticon");
	Localize("Spectator mode");Localize("Spectate next");Localize("Spectate previous");Localize("Console");Localize("Remote console");Localize("Screenshot");Localize("Scoreboard");Localize("Respawn");
*/

const int g_KeyCount = sizeof(gs_aKeys) / sizeof(CKeyInfo);

void CMenus::UiDoGetButtons(int Start, int Stop, CUIRect View)
{
	for (int i = Start; i < Stop; i++)
	{
		CKeyInfo &Key = gs_aKeys[i];
		CUIRect Button, Label;
		View.HSplitTop(20.0f, &Button, &View);
		Button.VSplitLeft(135.0f, &Label, &Button);

		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "%s:", (const char *)Key.m_Name);

		UI()->DoLabelScaled(&Label, aBuf, 13.0f, -1);
		int OldId = Key.m_KeyId;
		int NewId = DoKeyReader((void *)&gs_aKeys[i].m_Name, &Button, OldId);
		if(NewId != OldId)
		{
			if(OldId != 0 || NewId == 0)
				m_pClient->m_pBinds->Bind(OldId, "");
			if(NewId != 0)
				m_pClient->m_pBinds->Bind(NewId, gs_aKeys[i].m_pCommand);
		}
		View.HSplitTop(5.0f, 0, &View);
	}
}

// H-Client
void CMenus::RenderSettingsControls(CUIRect MainView)
{
	// this is kinda slow, but whatever
	for(int i = 0; i < g_KeyCount; i++)
		gs_aKeys[i].m_KeyId = 0;

	for(int KeyId = 0; KeyId < KEY_LAST; KeyId++)
	{
		const char *pBind = m_pClient->m_pBinds->Get(KeyId);
		if(!pBind[0])
			continue;

		for(int i = 0; i < g_KeyCount; i++)
			if(str_comp(pBind, gs_aKeys[i].m_pCommand) == 0)
			{
				gs_aKeys[i].m_KeyId = KeyId;
				break;
			}
	}

	CUIRect MovementSettings, WeaponSettings, VotingSettings, ChatSettings, MiscSettings, ResetButton;
	MainView.VSplitLeft(MainView.w/2-5.0f, &MovementSettings, &VotingSettings);

	// movement settings
	{
	    CUIRect rTitle;
		MovementSettings.HSplitTop(MainView.h/3+60.0f, &MovementSettings, &WeaponSettings);
	    MovementSettings.HSplitTop(22.0f, &rTitle, &MovementSettings);
		RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcSubcontainerHeaderBackgroundColor), CUI::CORNER_T, 10.0f);
		RenderTools()->DrawUIRect(&MovementSettings, HexToVec4(g_Config.m_hcSubcontainerBackgroundColor), 0, 0.0f);

        rTitle.Margin(5.0f, &rTitle);
		TextRender()->Text(0, rTitle.x, rTitle.y-3.0f, 14.0f*UI()->Scale(), Localize("Movement"), -1);
        MovementSettings.Margin(10.0f, &MovementSettings);

		{
			CUIRect Button, Label;
			MovementSettings.HSplitTop(20.0f, &Button, &MovementSettings);
			Button.VSplitLeft(135.0f, &Label, &Button);
			UI()->DoLabel(&Label, Localize("Mouse sens."), 14.0f*UI()->Scale(), -1);
			Button.HMargin(2.0f, &Button);
			g_Config.m_InpMousesens = (int)(DoScrollbarH(&g_Config.m_InpMousesens, &Button, (g_Config.m_InpMousesens-5)/500.0f)*500.0f)+5;
			//*key.key = ui_do_key_reader(key.key, &Button, *key.key);
			MovementSettings.HSplitTop(20.0f, 0, &MovementSettings);
		}

		UiDoGetButtons(0, 6, MovementSettings);

	}

	// weapon settings
	{
	    CUIRect rTitle;
		WeaponSettings.HSplitTop(MainView.h/3+50.0f, &WeaponSettings, &ResetButton);
		WeaponSettings.HSplitTop(10.0f, 0, &WeaponSettings);
	    WeaponSettings.HSplitTop(22.0f, &rTitle, &WeaponSettings);
		RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcSubcontainerHeaderBackgroundColor), CUI::CORNER_T, 10.0f);
		RenderTools()->DrawUIRect(&WeaponSettings, HexToVec4(g_Config.m_hcSubcontainerBackgroundColor), 0, 0.0f);

        rTitle.Margin(5.0f, &rTitle);
		TextRender()->Text(0, rTitle.x, rTitle.y-3.0f, 14.0f*UI()->Scale(), Localize("Weapon"), -1);
        WeaponSettings.Margin(10.0f, &WeaponSettings);

		UiDoGetButtons(6, 13, WeaponSettings);
	}

	// defaults
	{
		ResetButton.HSplitTop(10.0f, 0, &ResetButton);
		RenderTools()->DrawUIRect(&ResetButton, HexToVec4(g_Config.m_hcSubcontainerBackgroundColor), CUI::CORNER_ALL, 10.0f);
		ResetButton.HMargin(10.0f, &ResetButton);
		ResetButton.VMargin(30.0f, &ResetButton);
		static int s_DefaultButton = 0;
		if(DoButton_Menu((void*)&s_DefaultButton, Localize("Reset to defaults"), 0, &ResetButton))
			m_pClient->m_pBinds->SetDefaults();
	}

	// voting settings
	{
	    CUIRect rTitle;
	    VotingSettings.VSplitLeft(10.0f, 0, &VotingSettings);
		VotingSettings.HSplitTop(MainView.h/3-75.0f, &VotingSettings, &ChatSettings);
	    VotingSettings.HSplitTop(22.0f, &rTitle, &VotingSettings);
		RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcSubcontainerHeaderBackgroundColor), CUI::CORNER_T, 10.0f);
		RenderTools()->DrawUIRect(&VotingSettings, HexToVec4(g_Config.m_hcSubcontainerBackgroundColor), 0, 0.0f);

        rTitle.Margin(5.0f, &rTitle);
		TextRender()->Text(0, rTitle.x, rTitle.y-3.0f, 14.0f*UI()->Scale(), Localize("Voting"), -1);
        VotingSettings.Margin(10.0f, &VotingSettings);
		UiDoGetButtons(13, 15, VotingSettings);
	}

	// chat settings
	{
	    CUIRect rTitle;
		ChatSettings.HSplitTop(10.0f, 0, &ChatSettings);
		ChatSettings.HSplitTop(MainView.h/3-55.0f, &ChatSettings, &MiscSettings);
	    ChatSettings.HSplitTop(22.0f, &rTitle, &ChatSettings);
		RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcSubcontainerHeaderBackgroundColor), CUI::CORNER_T, 10.0f);
		RenderTools()->DrawUIRect(&ChatSettings, HexToVec4(g_Config.m_hcSubcontainerBackgroundColor), 0, 0.0f);

        rTitle.Margin(5.0f, &rTitle);
		TextRender()->Text(0, rTitle.x, rTitle.y-3.0f, 14.0f*UI()->Scale(), Localize("Chat"), -1);
        ChatSettings.Margin(10.0f, &ChatSettings);
		UiDoGetButtons(15, 18, ChatSettings);
	}

	// misc settings
	{
	    CUIRect rTitle;
		MiscSettings.HSplitTop(10.0f, 0, &MiscSettings);
	    MiscSettings.HSplitTop(22.0f, &rTitle, &MiscSettings);
		RenderTools()->DrawUIRect(&rTitle, HexToVec4(g_Config.m_hcSubcontainerHeaderBackgroundColor), CUI::CORNER_T, 10.0f);
		RenderTools()->DrawUIRect(&MiscSettings, HexToVec4(g_Config.m_hcSubcontainerBackgroundColor), 0, 0.0f);

        rTitle.Margin(5.0f, &rTitle);
		TextRender()->Text(0, rTitle.x, rTitle.y-3.0f, 14.0f*UI()->Scale(), Localize("Miscellaneus"), -1);
        MiscSettings.Margin(10.0f, &MiscSettings);
		UiDoGetButtons(18, 28, MiscSettings);
	}

}

void CMenus::RenderSettingsGraphics(CUIRect MainView)
{
	CUIRect Button;
	char aBuf[128];
	bool CheckSettings = false;

	static const int MAX_RESOLUTIONS = 256;
	static CVideoMode s_aModes[MAX_RESOLUTIONS];
	static int s_NumNodes = Graphics()->GetVideoModes(s_aModes, MAX_RESOLUTIONS);
	static int s_GfxScreenWidth = g_Config.m_GfxScreenWidth;
	static int s_GfxScreenHeight = g_Config.m_GfxScreenHeight;
	static int s_GfxColorDepth = g_Config.m_GfxColorDepth;
	static int s_GfxBorderless = g_Config.m_GfxBorderless;
	static int s_GfxFullscreen = g_Config.m_GfxFullscreen;
	static int s_GfxVsync = g_Config.m_GfxVsync;
	static int s_GfxFsaaSamples = g_Config.m_GfxFsaaSamples;
	static int s_GfxTextureQuality = g_Config.m_GfxTextureQuality;
	static int s_GfxTextureCompression = g_Config.m_GfxTextureCompression;
	static int s_GfxThreaded = g_Config.m_GfxThreaded;

	CUIRect ModeList;
	MainView.VSplitLeft(300.0f, &MainView, &ModeList);

	// draw allmodes switch
	ModeList.HSplitTop(20, &Button, &ModeList);
	if(DoButton_CheckBox(&g_Config.m_GfxDisplayAllModes, Localize("Show only supported"), g_Config.m_GfxDisplayAllModes^1, &Button))
	{
		g_Config.m_GfxDisplayAllModes ^= 1;
		s_NumNodes = Graphics()->GetVideoModes(s_aModes, MAX_RESOLUTIONS);
	}

	// display mode list
	static float s_ScrollValue = 0;
	int OldSelected = -1;
	int G = gcd(s_GfxScreenWidth, s_GfxScreenHeight);
	str_format(aBuf, sizeof(aBuf), "%s: %dx%d %d bit (%d:%d)", Localize("Current"), s_GfxScreenWidth, s_GfxScreenHeight, s_GfxColorDepth, s_GfxScreenWidth/G, s_GfxScreenHeight/G);
	UiDoListboxStart(&s_NumNodes , &ModeList, 24.0f, Localize("Display Modes"), aBuf, s_NumNodes, 1, OldSelected, s_ScrollValue);

	for(int i = 0; i < s_NumNodes; ++i)
	{
		const int Depth = s_aModes[i].m_Red+s_aModes[i].m_Green+s_aModes[i].m_Blue > 16 ? 24 : 16;
		if(g_Config.m_GfxColorDepth == Depth &&
			g_Config.m_GfxScreenWidth == s_aModes[i].m_Width &&
			g_Config.m_GfxScreenHeight == s_aModes[i].m_Height)
		{
			OldSelected = i;
		}

		CListboxItem Item = UiDoListboxNextItem(&s_aModes[i], OldSelected == i);
		if(Item.m_Visible)
		{
			int G = gcd(s_aModes[i].m_Width, s_aModes[i].m_Height);
			str_format(aBuf, sizeof(aBuf), " %dx%d %d bit (%d:%d)", s_aModes[i].m_Width, s_aModes[i].m_Height, Depth, s_aModes[i].m_Width/G, s_aModes[i].m_Height/G);
			UI()->DoLabelScaled(&Item.m_Rect, aBuf, 16.0f, -1);
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		const int Depth = s_aModes[NewSelected].m_Red+s_aModes[NewSelected].m_Green+s_aModes[NewSelected].m_Blue > 16 ? 24 : 16;
		g_Config.m_GfxColorDepth = Depth;
		g_Config.m_GfxScreenWidth = s_aModes[NewSelected].m_Width;
		g_Config.m_GfxScreenHeight = s_aModes[NewSelected].m_Height;
		CheckSettings = true;
	}

	// switches
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxBorderless, Localize("Borderless window"), g_Config.m_GfxBorderless, &Button))
	{
		g_Config.m_GfxBorderless ^= 1;
		if(g_Config.m_GfxBorderless && g_Config.m_GfxFullscreen)
			g_Config.m_GfxFullscreen = 0;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxFullscreen, Localize("Fullscreen"), g_Config.m_GfxFullscreen, &Button))
	{
		g_Config.m_GfxFullscreen ^= 1;
		if(g_Config.m_GfxFullscreen && g_Config.m_GfxBorderless)
			g_Config.m_GfxBorderless = 0;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxVsync, Localize("V-Sync"), g_Config.m_GfxVsync, &Button))
	{
		g_Config.m_GfxVsync ^= 1;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox_Number(&g_Config.m_GfxFsaaSamples, Localize("FSAA samples"), g_Config.m_GfxFsaaSamples, &Button))
	{
		g_Config.m_GfxFsaaSamples = (g_Config.m_GfxFsaaSamples+1)%17;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxThreaded, Localize("Threaded rendering"), g_Config.m_GfxThreaded, &Button))
	{
		g_Config.m_GfxThreaded ^= 1;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(g_Config.m_GfxThreaded)
	{
		Button.VSplitLeft(20.0f, 0, &Button);
		if(DoButton_CheckBox(&g_Config.m_GfxAsyncRender, Localize("Handle rendering async from updates"), g_Config.m_GfxAsyncRender, &Button))
		{
			g_Config.m_GfxAsyncRender ^= 1;
			CheckSettings = true;
		}
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxTextureQuality, Localize("Quality Textures"), g_Config.m_GfxTextureQuality, &Button))
	{
		g_Config.m_GfxTextureQuality ^= 1;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxTextureCompression, Localize("Texture Compression"), g_Config.m_GfxTextureCompression, &Button))
	{
		g_Config.m_GfxTextureCompression ^= 1;
		CheckSettings = true;
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_GfxHighDetail, Localize("High Detail"), g_Config.m_GfxHighDetail, &Button))
		g_Config.m_GfxHighDetail ^= 1;

	// check if the new settings require a restart
	if(CheckSettings)
	{
		if(s_GfxScreenWidth == g_Config.m_GfxScreenWidth &&
			s_GfxScreenHeight == g_Config.m_GfxScreenHeight &&
			s_GfxColorDepth == g_Config.m_GfxColorDepth &&
			s_GfxBorderless == g_Config.m_GfxBorderless &&
			s_GfxFullscreen == g_Config.m_GfxFullscreen &&
			s_GfxVsync == g_Config.m_GfxVsync &&
			s_GfxFsaaSamples == g_Config.m_GfxFsaaSamples &&
			s_GfxTextureQuality == g_Config.m_GfxTextureQuality &&
			s_GfxTextureCompression == g_Config.m_GfxTextureCompression &&
			s_GfxThreaded == g_Config.m_GfxThreaded)
			m_NeedRestartGraphics = false;
		else
			m_NeedRestartGraphics = true;
	}

	//
}

void CMenus::RenderSettingsSound(CUIRect MainView)
{
	CUIRect Button;
	MainView.VSplitMid(&MainView, 0);
	static int s_SndEnable = g_Config.m_SndEnable;
	static int s_SndRate = g_Config.m_SndRate;

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_SndEnable, Localize("Use sounds"), g_Config.m_SndEnable, &Button))
	{
		g_Config.m_SndEnable ^= 1;
		if(g_Config.m_SndEnable)
		{
			if(g_Config.m_SndMusic)
				m_pClient->m_pSounds->Play(CSounds::CHN_MUSIC, SOUND_MENU, 1.0f);
		}
		else
			m_pClient->m_pSounds->Stop(SOUND_MENU);
		m_NeedRestartSound = g_Config.m_SndEnable && (!s_SndEnable || s_SndRate != g_Config.m_SndRate);
	}

	if(!g_Config.m_SndEnable)
		return;

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_SndMusic, Localize("Play background music"), g_Config.m_SndMusic, &Button))
	{
		g_Config.m_SndMusic ^= 1;
		if(Client()->State() == IClient::STATE_OFFLINE)
		{
			if(g_Config.m_SndMusic)
				m_pClient->m_pSounds->Play(CSounds::CHN_MUSIC, SOUND_MENU, 1.0f);
			else
				m_pClient->m_pSounds->Stop(SOUND_MENU);
		}
	}

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_SndNonactiveMute, Localize("Mute when not active"), g_Config.m_SndNonactiveMute, &Button))
		g_Config.m_SndNonactiveMute ^= 1;

	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_ClThreadsoundloading, Localize("Threaded sound loading"), g_Config.m_ClThreadsoundloading, &Button))
		g_Config.m_ClThreadsoundloading ^= 1;

	// sample rate box
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "%d", g_Config.m_SndRate);
		MainView.HSplitTop(20.0f, &Button, &MainView);
		UI()->DoLabelScaled(&Button, Localize("Sample rate"), 14.0f, -1);
		Button.VSplitLeft(190.0f, 0, &Button);
		static float Offset = 0.0f;
		DoEditBox(&g_Config.m_SndRate, &Button, aBuf, sizeof(aBuf), 14.0f, &Offset);
		g_Config.m_SndRate = max(1, str_toint(aBuf));
		m_NeedRestartSound = !s_SndEnable || s_SndRate != g_Config.m_SndRate;
	}

	// volume slider
	{
		CUIRect Button, Label;
		MainView.HSplitTop(5.0f, &Button, &MainView);
		MainView.HSplitTop(20.0f, &Button, &MainView);
		Button.VSplitLeft(190.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);
		UI()->DoLabelScaled(&Label, Localize("Sound volume"), 14.0f, -1);
		g_Config.m_SndVolume = (int)(DoScrollbarH(&g_Config.m_SndVolume, &Button, g_Config.m_SndVolume/100.0f)*100.0f);
		MainView.HSplitTop(20.0f, 0, &MainView);
	}
}

class CLanguage
{
public:
	CLanguage() {}
	CLanguage(const char *n, const char *f, int Code) : m_Name(n), m_FileName(f), m_CountryCode(Code) {}

	string m_Name;
	string m_FileName;
	int m_CountryCode;

	bool operator<(const CLanguage &Other) { return m_Name < Other.m_Name; }
};

void LoadLanguageIndexfile(IStorage *pStorage, IConsole *pConsole, sorted_array<CLanguage> *pLanguages)
{
	IOHANDLE File = pStorage->OpenFile("languages/index.txt", IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", "couldn't open index file");
		return;
	}

	char aOrigin[128];
	char aReplacement[128];
	CLineReader LineReader;
	LineReader.Init(File);
	char *pLine;
	while((pLine = LineReader.Get()))
	{
		if(!str_length(pLine) || pLine[0] == '#') // skip empty lines and comments
			continue;

		str_copy(aOrigin, pLine, sizeof(aOrigin));

		pLine = LineReader.Get();
		if(!pLine)
		{
			pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", "unexpected end of index file");
			break;
		}

		if(pLine[0] != '=' || pLine[1] != '=' || pLine[2] != ' ')
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "malform replacement for index '%s'", aOrigin);
			pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", aBuf);
			(void)LineReader.Get();
			continue;
		}
		str_copy(aReplacement, pLine+3, sizeof(aReplacement));

		pLine = LineReader.Get();
		if(!pLine)
		{
			pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", "unexpected end of index file");
			break;
		}

		if(pLine[0] != '=' || pLine[1] != '=' || pLine[2] != ' ')
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "malform replacement for index '%s'", aOrigin);
			pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", aBuf);
			continue;
		}

		char aFileName[128];
		str_format(aFileName, sizeof(aFileName), "languages/%s.txt", aOrigin);
		pLanguages->add(CLanguage(aReplacement, aFileName, str_toint(pLine+3)));
	}
	io_close(File);
}

void CMenus::RenderLanguageSelection(CUIRect MainView)
{
	static int s_LanguageList = 0;
	static int s_SelectedLanguage = 0;
	static sorted_array<CLanguage> s_Languages;
	static float s_ScrollValue = 0;

	if(s_Languages.size() == 0)
	{
		s_Languages.add(CLanguage("English", "", 826));
		LoadLanguageIndexfile(Storage(), Console(), &s_Languages);
		for(int i = 0; i < s_Languages.size(); i++)
			if(str_comp(s_Languages[i].m_FileName, g_Config.m_ClLanguagefile) == 0)
			{
				s_SelectedLanguage = i;
				break;
			}
	}

	int OldSelected = s_SelectedLanguage;

	UiDoListboxStart(&s_LanguageList , &MainView, 24.0f, Localize("Language"), "", s_Languages.size(), 1, s_SelectedLanguage, s_ScrollValue);

	for(sorted_array<CLanguage>::range r = s_Languages.all(); !r.empty(); r.pop_front())
	{
		CListboxItem Item = UiDoListboxNextItem(&r.front());

		if(Item.m_Visible)
		{
			CUIRect Rect;
			Item.m_Rect.VSplitLeft(Item.m_Rect.h*2.0f, &Rect, &Item.m_Rect);
			Rect.VMargin(6.0f, &Rect);
			Rect.HMargin(3.0f, &Rect);
			vec4 Color(1.0f, 1.0f, 1.0f, 1.0f);
			m_pClient->m_pCountryFlags->Render(r.front().m_CountryCode, &Color, Rect.x, Rect.y, Rect.w, Rect.h);
			Item.m_Rect.HSplitTop(2.0f, 0, &Item.m_Rect);
 			UI()->DoLabelScaled(&Item.m_Rect, r.front().m_Name, 16.0f, -1);
		}
	}

	s_SelectedLanguage = UiDoListboxEnd(&s_ScrollValue, 0);

	if(OldSelected != s_SelectedLanguage)
	{
		str_copy(g_Config.m_ClLanguagefile, s_Languages[s_SelectedLanguage].m_FileName, sizeof(g_Config.m_ClLanguagefile));
		g_Localization.Load(s_Languages[s_SelectedLanguage].m_FileName, Storage(), Console());
	}
}

// H-Client
void CMenus::RenderSettings(CUIRect MainView)
{
	static int s_SettingsPage = 0;

	// render background
	CUIRect Temp, TabBar, RestartWarning;
	MainView.HSplitBottom(15.0f, &MainView, &RestartWarning);
	MainView.VSplitRight(120.0f, &MainView, &TabBar);
	RenderTools()->DrawUIRect(&MainView, HexToVec4(g_Config.m_hcContainerBackgroundColor), CUI::CORNER_B|CUI::CORNER_TL, 10.0f);
	TabBar.HSplitTop(50.0f, &Temp, &TabBar);
	RenderTools()->DrawUIRect(&Temp, HexToVec4(g_Config.m_hcContainerBackgroundColor), CUI::CORNER_R, 10.0f);

	MainView.HSplitTop(10.0f, 0, &MainView);

	CUIRect Button;

	const char *aTabs[] = {
		Localize("Language"),
		Localize("General"),
		Localize("Player"),
		("Tee"),
		Localize("Controls"),
		Localize("Graphics"),
		Localize("Sound"),
		("H-Client"),
		Localize("Themes"),
		Localize("Statistics")
	};

    const int imgSprites[] = { SPRITE_ICON_LANGUAGE, SPRITE_ICON_GENERAL, SPRITE_ICON_PLAYER, SPRITE_ICON_TEE, SPRITE_ICON_CONTROLS, SPRITE_ICON_GRAPHICS, SPRITE_ICON_SOUND, SPRITE_ICON_MOD, SPRITE_ICON_THEME, SPRITE_ICON_STATISTICS };

    ms_ColorTabbarActive = HexToVec4(g_Config.m_hcSettingsPaneltabSelectedBackgroundColor);
    ms_ColorTabbarInactive = HexToVec4(g_Config.m_hcSettingsPaneltabBackgroundColor);

    int NumTabs = (int)(sizeof(aTabs)/sizeof(*aTabs));
	for(int i = 0; i < NumTabs; i++)
	{
		//TabBar.HSplitTop(10, &Button, &TabBar);
		TabBar.HSplitTop(((i==7||i==9)?30:10), &Button, &TabBar); //H-Client
		TabBar.HSplitTop(26, &Button, &TabBar);
        if (i != s_SettingsPage && !UI()->MouseInside(&Button)) //H-Client
            Button.VSplitRight(5.0f, &Button, 0x0);
		if(DoButton_MenuTabIcon(&aTabs[i], aTabs[i], s_SettingsPage == i, &Button, CUI::CORNER_R, IMAGE_SETTINGS_ICONS, 16.0f, imgSprites[i]))
			s_SettingsPage = i;
	}
    ms_ColorTabbarActive = HexToVec4(g_Config.m_hcPaneltabSelectedBackgroundColor);
    ms_ColorTabbarInactive = HexToVec4(g_Config.m_hcPaneltabBackgroundColor);

	MainView.Margin(10.0f, &MainView);

	if(s_SettingsPage == 0)
		RenderLanguageSelection(MainView);
	else if(s_SettingsPage == 1)
		RenderSettingsGeneral(MainView);
	else if(s_SettingsPage == 2)
		RenderSettingsPlayer(MainView);
	else if(s_SettingsPage == 3)
		RenderSettingsTee(MainView);
	else if(s_SettingsPage == 4)
		RenderSettingsControls(MainView);
	else if(s_SettingsPage == 5)
		RenderSettingsGraphics(MainView);
	else if(s_SettingsPage == 6)
		RenderSettingsSound(MainView);
    // H-Client
    else if (s_SettingsPage == 7)
        RenderSettingsHClient(MainView);
    else if (s_SettingsPage == 8)
        RenderSettingsTheme(MainView);
    else if (s_SettingsPage == 9)
        RenderStatistics(MainView);

    if (!m_NeedUpdateThemesList && s_SettingsPage != 8)
        m_NeedUpdateThemesList = true;
    //

	if(m_NeedRestartGraphics || m_NeedRestartSound)
		UI()->DoLabel(&RestartWarning, Localize("You must restart the game for all settings to take effect."), 15.0f, -1);
}

// H-Client
void CMenus::RenderSettingsHClient(CUIRect MainView)
{
	//char aBuf[128];
    static vec4 s_HeaderColors[] = {HexToVec4(g_Config.m_hcSubcontainerHeaderBackgroundColor), vec4(0.0f,0.0f,0.0f,0.0f), vec4(0.0f,0.0f,0.0f,0.0f), HexToVec4(g_Config.m_hcSubcontainerHeaderBackgroundColor)};
	CUIRect PanelL, PanelR;
	CUIRect StandartGame, DDRaceGame, Ghost, Colors, HUDItem;

	MainView.VSplitLeft(300.0f, &PanelL, &PanelR);
    PanelL.VSplitRight(10.0f, &PanelL, 0x0);
    PanelR.VSplitLeft(10.0f, 0x0, &PanelR);

	//TODO: Need be change...
	float splitTop = 260.0f;
    /*if (g_Config.m_hcLaserCustomColor)
        splitTop += 105.0f;
    if (g_Config.m_hcAutoDownloadSkins)
    	splitTop += 20.0f;*/
    if (g_Config.m_hcGoreStyle)
        splitTop += 40.0f;

    //PanelL.HSplitTop(splitTop, &StandartGame, &DDRaceGame);
    StandartGame = PanelL;
    PanelR.HSplitTop(170.0f, &DDRaceGame, &Colors);
    StandartGame.HSplitTop(splitTop, &StandartGame, &Ghost);
    Ghost.HSplitTop(15.0f, 0x0, &Ghost);

    //Standart Game
    {
        //Head
        StandartGame.HSplitTop(ms_ListheaderHeight, &HUDItem, &StandartGame);
        RenderTools()->DrawUIRect(&HUDItem, s_HeaderColors);
        HUDItem.VSplitLeft(3.0f, 0x0, &HUDItem);
        UI()->DoLabel(&HUDItem, Localize("⚫·· General"), HUDItem.h*ms_FontmodHeight, -1);

        //H-Client HUD
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcUseHUD, Localize("H-Client HUD"), g_Config.m_hcUseHUD, &HUDItem))
            g_Config.m_hcUseHUD ^= 1;

        //Color Clan
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcColorClan, Localize("Highlight clan members"), g_Config.m_hcColorClan, &HUDItem))
            g_Config.m_hcColorClan ^= 1;

        //Chat Emoticons
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcChatEmoticons, Localize("Enable chat emoticons"), g_Config.m_hcChatEmoticons, &HUDItem))
            g_Config.m_hcChatEmoticons ^= 1;

        //Chat Colours
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcChatColours, Localize("Enable chat colours"), g_Config.m_hcChatColours, &HUDItem))
            g_Config.m_hcChatColours ^= 1;

        //Disable Chat Sound Notification
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcDisableChatSoundNotification, Localize("Disable chat sound notification"), g_Config.m_hcDisableChatSoundNotification, &HUDItem))
            g_Config.m_hcDisableChatSoundNotification ^= 1;

        //Show Player Info
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcPlayerInfo, Localize("Show player info"), g_Config.m_hcPlayerInfo, &HUDItem))
            g_Config.m_hcPlayerInfo ^= 1;

        //Show Map Preview
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcShowPreviewMap, Localize("Show map preview in server browser"), g_Config.m_hcShowPreviewMap, &HUDItem))
            g_Config.m_hcShowPreviewMap ^= 1;

        //Download Maps From DDNet Servers
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_ddrMapsFromHttp, Localize("Try download maps from DDNet servers"), g_Config.m_ddrMapsFromHttp, &HUDItem))
            g_Config.m_ddrMapsFromHttp ^= 1;

        /*if (g_Config.m_hcShowPreviewMap)
        {
            StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
            HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
            UI()->DoLabelScaled(&HUDItem, Localize("Max. map dimensions: "), HUDItem.h*ms_FontmodHeight*0.8f, -1);
        }*/

        //Auto-Download Skins
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcAutoDownloadSkins, Localize("Auto download skins (DDNet Database)"), g_Config.m_hcAutoDownloadSkins, &HUDItem))
            g_Config.m_hcAutoDownloadSkins ^= 1;

        if (g_Config.m_hcAutoDownloadSkins)
        {
            CUIRect Edit;
            static float Offset = 0.0f;
            StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
            HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
            HUDItem.VSplitRight(140.0f, &HUDItem, &Edit);
            Edit.VSplitMid(&Edit, 0x0);
            TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
            UI()->DoLabelScaled(&HUDItem, Localize("Speed Limit (KiBps): "), HUDItem.h*ms_FontmodHeight*0.8f, -1);
            if (DoEditBox(&g_Config.m_hcAutoDownloadSkinsSpeed, &Edit, g_Config.m_hcAutoDownloadSkinsSpeed, sizeof(g_Config.m_hcAutoDownloadSkinsSpeed), 12.0f, &Offset, false, CUI::CORNER_ALL))
            {
            	int downloadSpeed = clamp(atoi(g_Config.m_hcAutoDownloadSkinsSpeed), 0, 2048);
            	str_format(g_Config.m_hcAutoDownloadSkinsSpeed, sizeof(g_Config.m_hcAutoDownloadSkinsSpeed), "%d", downloadSpeed);
            }
        }

        //Dinamyc Camera Effect
        /*StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcDynamicCamera, Localize("Change dynamic camera"), g_Config.m_hcDynamicCamera, &HUDItem))
            g_Config.m_hcDynamicCamera ^= 1;*/

        //Auto-Update
        CUIRect Button;
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        HUDItem.VSplitMid(&HUDItem, &Button);
        if(DoButton_CheckBox(&g_Config.m_hcAutoUpdate, Localize("Auto-Update"), g_Config.m_hcAutoUpdate, &HUDItem))
            g_Config.m_hcAutoUpdate ^= 1;
        Button.Margin(2.0f, &Button);
        static int s_ButtonAutoUpdate = 0;
        if (DoButton_Menu((void*)&s_ButtonAutoUpdate, Localize("Check now!"), 0, &Button))
        {
            char aBuf[128];
            str_format(aBuf, sizeof(aBuf), "Checking for an update");
            RenderUpdating(aBuf);
            Updater()->CheckUpdates(this);
            if (Updater()->Updated())
            {
                if (Updater()->NeedResetClient())
                {
                    Client()->Quit();
                    return;
                }
                else
                    str_format(aBuf, sizeof(aBuf), "H-Client Client updated");

                RenderUpdating(aBuf);
            }
            else
            {
                str_format(aBuf, sizeof(aBuf), "No update available");
                RenderUpdating(aBuf);
            }
        }

        //teeworlds://
        /*#if defined(CONF_FAMILY_WINDOWS)
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        static bool protTW;
        protTW = W32RegExistsKey(HKEY_CLASSES_ROOT, "teeworlds");
        if(DoButton_CheckBox(&protTW, Localize("Enable 'teeworlds://' protocol in the system."), protTW, &HUDItem))
        {
            if (!protTW)
            {
                W32RegCreateKey(HKEY_CLASSES_ROOT,"teeworlds");
                W32RegSetKeyData(HKEY_CLASSES_ROOT,"teeworlds", REG_SZ, "@", (LPBYTE)"URL:teeworlds Protocol", 22);
                W32RegSetKeyData(HKEY_CLASSES_ROOT,"teeworlds", REG_SZ, "\"URL Protocol\"", (LPBYTE)"\"\"",2);
                W32RegCreateKey(HKEY_CLASSES_ROOT,"teeworlds\\shell");
                W32RegCreateKey(HKEY_CLASSES_ROOT,"teeworlds\\shell\\open");
                W32RegCreateKey(HKEY_CLASSES_ROOT,"teeworlds\\shell\\open\\command");
                W32RegSetKeyData(HKEY_CLASSES_ROOT,"teeworlds\\shell\\open\\command", REG_SZ, "@", (LPBYTE)"\"D:\\teeworlds.exe\" -s \\\"%1\\\" -p \\\"%2\\\"",39);
            }
        }
        #endif*/

        //Gore Style
        StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
        if(DoButton_CheckBox(&g_Config.m_hcGoreStyle, Localize("Blood"), g_Config.m_hcGoreStyle, &HUDItem))
            g_Config.m_hcGoreStyle ^= 1;

        //Gore Style Options
        if (g_Config.m_hcGoreStyle)
        {
            StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
            HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
            if(DoButton_CheckBox(&g_Config.m_hcGoreStyleTeeColors, Localize("Use tee colors"), g_Config.m_hcGoreStyleTeeColors, &HUDItem))
                g_Config.m_hcGoreStyleTeeColors ^= 1;

            StandartGame.HSplitTop(20.0f, &HUDItem, &StandartGame);
            HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
            if(DoButton_CheckBox(&g_Config.m_hcGoreStyleDropWeapons, Localize("Drop Weapons"), g_Config.m_hcGoreStyleDropWeapons, &HUDItem))
                g_Config.m_hcGoreStyleDropWeapons ^= 1;
        }
    }

	//Ghost
    {
    	Ghost.HSplitTop(ms_ListheaderHeight, &HUDItem, &Ghost);
        RenderTools()->DrawUIRect(&HUDItem, s_HeaderColors);
        HUDItem.VSplitLeft(3.0f, 0x0, &HUDItem);
        UI()->DoLabel(&HUDItem, "⚫·· Ghost (By Rajh, Redix and Sushi)", HUDItem.h*ms_FontmodHeight, -1);

        Ghost.HSplitTop(20.0f, &HUDItem, &Ghost);
        if(DoButton_CheckBox(&g_Config.m_hcRaceGhost, Localize("Enable ghost"), g_Config.m_hcRaceGhost, &HUDItem))
            g_Config.m_hcRaceGhost ^= 1;

        if (g_Config.m_hcRaceGhost)
        {

            Ghost.HSplitTop(20.0f, &HUDItem, &Ghost);
            HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
            if(DoButton_CheckBox(&g_Config.m_hcRaceShowGhost, Localize("Show ghost"), g_Config.m_hcRaceShowGhost, &HUDItem))
                g_Config.m_hcRaceShowGhost ^= 1;

            Ghost.HSplitTop(20.0f, &HUDItem, &Ghost);
            HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
            if(DoButton_CheckBox(&g_Config.m_hcRaceSaveGhost, Localize("Auto-Save ghost"), g_Config.m_hcRaceSaveGhost, &HUDItem))
                g_Config.m_hcRaceSaveGhost ^= 1;
        }
    }

	// DDRace/DDNet
	{
        DDRaceGame.HSplitTop(ms_ListheaderHeight, &HUDItem, &DDRaceGame);
        RenderTools()->DrawUIRect(&HUDItem, s_HeaderColors);
        HUDItem.VSplitLeft(3.0f, 0x0, &HUDItem);
        UI()->DoLabel(&HUDItem, "⚫·· DDRaceNetwork", HUDItem.h*ms_FontmodHeight, -1);

        DDRaceGame.HSplitTop(20.0f, &HUDItem, &DDRaceGame);
        if(DoButton_CheckBox(&g_Config.m_ddrShowHiddenWays, Localize("View hidden ways"), g_Config.m_ddrShowHiddenWays, &HUDItem))
            g_Config.m_ddrShowHiddenWays ^= 1;

        DDRaceGame.HSplitTop(20.0f, &HUDItem, &DDRaceGame);
        if(DoButton_CheckBox(&g_Config.m_ddrShowTeeDirection, Localize("View tee direction"), g_Config.m_ddrShowTeeDirection, &HUDItem))
            g_Config.m_ddrShowTeeDirection ^= 1;

        DDRaceGame.HSplitTop(20.0f, &HUDItem, &DDRaceGame);
        if(DoButton_CheckBox(&g_Config.m_ddrPreventPrediction, Localize("Prevent prediction in 'freezing' zones (experimental)"), g_Config.m_ddrPreventPrediction, &HUDItem))
            g_Config.m_ddrPreventPrediction ^= 1;

        DDRaceGame.HSplitTop(20.0f, &HUDItem, &DDRaceGame);
		if(DoButton_CheckBox(&g_Config.m_ddrShowOthers, Localize("Show Others"), g_Config.m_ddrShowOthers, &HUDItem))
			g_Config.m_ddrShowOthers ^= 1;

		DDRaceGame.HSplitTop(10.0f, 0x0, &DDRaceGame); // Blank Space

        CUIRect Edit;
        static float Offset = 0.0f;
        DDRaceGame.HSplitTop(20.0f, &HUDItem, &DDRaceGame);
        HUDItem.VSplitRight(260.0f, &HUDItem, &Edit);
        Edit.VSplitLeft(40.0f, &Edit, 0x0);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
        UI()->DoLabelScaled(&HUDItem, Localize("Eyes time (Secs.):"), HUDItem.h*ms_FontmodHeight*0.8f, -1);
        DoEditBox(&g_Config.m_hcEyesSelectorTime, &Edit, g_Config.m_hcEyesSelectorTime, sizeof(g_Config.m_hcEyesSelectorTime), 12.0f, &Offset, false, CUI::CORNER_ALL);

        CUIRect Button;
        static float OffsetTimeoutCode = 0.0f;
        DDRaceGame.HSplitTop(25.0f, &HUDItem, &DDRaceGame);
        HUDItem.HSplitTop(5.0f, 0x0, &HUDItem);
        HUDItem.VSplitLeft(80.0f, &HUDItem, &Edit);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
        UI()->DoLabelScaled(&HUDItem, Localize("Client Hash:"), HUDItem.h*ms_FontmodHeight*0.8f, -1);
        Edit.w = 150.0f;
        DoEditBox(&g_Config.m_ddrTimeoutHash, &Edit, g_Config.m_ddrTimeoutHash, sizeof(g_Config.m_ddrTimeoutHash), 12.0f, &OffsetTimeoutCode, false, CUI::CORNER_ALL);
        Button.x = Edit.x + Edit.w + 10.0f;
        Button.y = Edit.y;
        Button.w = 100.0f;
        Button.h = 20.0f;
        static int s_ButtonGenerateClientHash = 0;
        if (DoButton_Menu((void*)&s_ButtonGenerateClientHash, Localize("Generate"), 0, &Button))
        	for (size_t i = 0; i < sizeof(g_Config.m_ddrTimeoutHash)-1; g_Config.m_ddrTimeoutHash[i++] = (rand() % 26) + 97);
	}

    // Colors
    {
    	Colors.HSplitTop(ms_ListheaderHeight, &HUDItem, &Colors);
		RenderTools()->DrawUIRect(&HUDItem, s_HeaderColors);
		HUDItem.VSplitLeft(3.0f, 0x0, &HUDItem);
		UI()->DoLabel(&HUDItem, "⚫·· Colors", HUDItem.h*ms_FontmodHeight, -1);

    	CUIRect Text, LaserColorArea, Laser, SmokeColorArea, Smoke;
		//Laser Color
		//StandartGame.HSplitTop(5.0f, 0, &StandartGame);
		if (g_Config.m_hcLaserCustomColor)
			Colors.HSplitTop(140.0f, &LaserColorArea, &Colors);
		else
			Colors.HSplitTop(20.0f, &LaserColorArea, &Colors);

	   // RenderTools()->DrawUIRect(&LaserColorArea, vec4(0.0f,0.0f,0.0f,0.15f), CUI::CORNER_ALL, 5.0f);
		//LaserColorArea.HSplitTop(5.0f, 0, &LaserColorArea);
		LaserColorArea.HSplitTop(20.0f, &Text, &LaserColorArea);
		//Text.VSplitLeft(5.0f, 0, &Text);
		if(DoButton_CheckBox(&g_Config.m_hcLaserCustomColor, Localize("Custom laser color"), g_Config.m_hcLaserCustomColor, &Text))
			g_Config.m_hcLaserCustomColor ^= 1;

		//UI()->DoLabelScaled(&Text, Localize("Laser Color"), 14.0f, -1);

		if (g_Config.m_hcLaserCustomColor)
		{
			LaserColorArea.HSplitTop(20.0f, &Laser, &LaserColorArea);
			Laser.Margin(15.0f, &Laser);
			RenderLaser(vec2(Laser.x,Laser.y), vec2(Laser.x+Laser.w, Laser.y));
			LaserColorArea.HSplitTop(15.0f, 0x0, &LaserColorArea);

			const char *paLabels[] = {
				Localize("Hue"),
				Localize("Sat."),
				Localize("Lht."),
				Localize("Alpha")};
			int *pColorSlider[4] = {&g_Config.m_hcLaserColorHue, &g_Config.m_hcLaserColorSat, &g_Config.m_hcLaserColorLht, &g_Config.m_hcLaserColorAlpha};
			for(int s = 0; s < 4; s++)
			{
				CUIRect Text;
				LaserColorArea.HSplitTop(19.0f, &HUDItem, &LaserColorArea);
				HUDItem.VMargin(15.0f, &HUDItem);
				HUDItem.VSplitLeft(100.0f, &Text, &HUDItem);
				//Button.VSplitRight(5.0f, &Button, 0);
				HUDItem.HSplitTop(4.0f, 0, &HUDItem);

				float k = (*pColorSlider[s]) / 255.0f;
				k = DoScrollbarH(pColorSlider[s], &HUDItem, k);
				*pColorSlider[s] = (int)(k*255.0f);
				Text.y+=3.0f;
				UI()->DoLabel(&Text, paLabels[s], Text.h*ms_FontmodHeight*0.8f, -1);
			}
		}

		//Smoke Color
		if (g_Config.m_hcSmokeCustomColor)
			Colors.HSplitTop(110.0f, &SmokeColorArea, &Colors);
		else
			Colors.HSplitTop(20.0f, &SmokeColorArea, &Colors);

		SmokeColorArea.HSplitTop(20.0f, &Text, &SmokeColorArea);

		if(DoButton_CheckBox(&g_Config.m_hcSmokeCustomColor, Localize("Custom smoke color"), g_Config.m_hcSmokeCustomColor, &Text))
			g_Config.m_hcSmokeCustomColor ^= 1;

		if (g_Config.m_hcSmokeCustomColor)
		{
			SmokeColorArea.HSplitTop(20.0f, &Smoke, &SmokeColorArea);
			Smoke.Margin(15.0f, &Smoke);
			vec4 s_SmokeColors[] = {
				vec4(g_Config.m_hcSmokeColorHue/255.0f, g_Config.m_hcSmokeColorSat/255.0f, g_Config.m_hcSmokeColorLht/255.0f, g_Config.m_hcSmokeColorAlpha/255.0f),
				vec4(g_Config.m_hcSmokeColorHue/255.0f, g_Config.m_hcSmokeColorSat/255.0f, g_Config.m_hcSmokeColorLht/255.0f, g_Config.m_hcSmokeColorAlpha/255.0f),
				vec4(g_Config.m_hcSmokeColorHue/255.0f, g_Config.m_hcSmokeColorSat/255.0f, g_Config.m_hcSmokeColorLht/255.0f, g_Config.m_hcSmokeColorAlpha/255.0f),
				vec4(g_Config.m_hcSmokeColorHue/255.0f, g_Config.m_hcSmokeColorSat/255.0f, g_Config.m_hcSmokeColorLht/255.0f, g_Config.m_hcSmokeColorAlpha/255.0f)
			};
			RenderTools()->DrawUIRect(&Smoke, s_SmokeColors);

			const char *paLabels[] = {
				Localize("Hue"),
				Localize("Sat."),
				Localize("Lht."),
				Localize("Alpha")};
			int *pColorSlider[4] = {&g_Config.m_hcSmokeColorHue, &g_Config.m_hcSmokeColorSat, &g_Config.m_hcSmokeColorLht, &g_Config.m_hcSmokeColorAlpha};
			for(int s = 0; s < 4; s++)
			{
				CUIRect Text;
				SmokeColorArea.HSplitTop(19.0f, &HUDItem, &SmokeColorArea);
				HUDItem.VMargin(15.0f, &HUDItem);
				HUDItem.VSplitLeft(100.0f, &Text, &HUDItem);
				//Button.VSplitRight(5.0f, &Button, 0);
				HUDItem.HSplitTop(4.0f, 0, &HUDItem);

				float k = (*pColorSlider[s]) / 255.0f;
				k = DoScrollbarH(pColorSlider[s], &HUDItem, k);
				*pColorSlider[s] = (int)(k*255.0f);
				Text.y+=3.0f;
				UI()->DoLabel(&Text, paLabels[s], Text.h*ms_FontmodHeight*0.8f, -1);
			}
		}
		StandartGame.HSplitTop(5.0f, 0, &StandartGame);
    }

	//Feet
	{
		const char *pUrl = "http://hclient.wordpress.com";
		CUIRect TextRect, Button;
        MainView.HSplitBottom(20.0f, &MainView, &HUDItem);
        HUDItem.VSplitRight(150.0f, &HUDItem, &Button);
        Button.Margin(2.0f, &Button);
        HUDItem.VSplitRight(15.0f, &HUDItem, 0x0);
        HUDItem.VSplitRight(TextRender()->TextWidth(0, 14.0, pUrl, -1), 0x0, &TextRect);
        if (UI()->MouseInside(&TextRect))
        {
        	TextRender()->TextColor(0.0f, 1.0f, 0.39f, 1.0f);
        	if (UI()->MouseButtonClicked(0))
        		open_default_browser(pUrl);
        }
        else
        	TextRender()->TextColor(1.0f, 0.39f, 0.0f, 1.0f);
		UI()->DoLabelScaled(&TextRect, pUrl, 14.0f, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

        static int s_ButtonClearCache = 0;
        if (DoButton_Menu((void*)&s_ButtonClearCache, Localize("Clear Map-Preview Cache"), 0, &Button))
            DeleteMapPreviewCache();
	}


    TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CMenus::RenderSettingsTheme(CUIRect MainView)
{
    std::list<CTheme> themes = m_pClient->TexturePack()->GetThemes();

	static float s_ScrollValue = 0.0f;
	int OldSelected = -1;
	UiDoListboxStart(&s_ScrollValue, &MainView, 24.0f, Localize("Themes"), "", themes.size(), 1, OldSelected, s_ScrollValue);

    int i=0;
    std::list<CTheme>::iterator it = themes.begin();
	while (it != themes.end())
	{
	    CTheme theme = (*it);
		if(str_comp(theme.m_FolderName, g_Config.m_hcTheme) == 0)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&(*it), OldSelected == i);
		if(Item.m_Visible)
        {
            CUIRect Label;
            Item.m_Rect.Margin(5.0f, &Label);
            CUIRect lblName, lblBy, lblVersion, lblWeb;
            Label.VSplitLeft(150.0f, &lblName, &lblBy);
            lblBy.VSplitLeft(160.0f, &lblBy, &lblVersion);
            lblVersion.VSplitLeft(50.0f, &lblVersion, &lblWeb);
            UI()->DoLabel(&lblName, theme.m_Name, 10.0f, -1);
            char buff[100];
            if (str_length(theme.m_Author) > 0)
            {
                str_format(buff, sizeof(buff), "%s %s", Localize("by"), theme.m_Author);
                UI()->DoLabel(&lblBy, buff, 10.0f, -1);
            }
            if (str_length(theme.m_Version) > 0)
            {
                str_format(buff, sizeof(buff), "v%s", theme.m_Version);
                UI()->DoLabel(&lblVersion, buff, 10.0f, -1);
            }
            if (str_length(theme.m_Web) > 0)
                UI()->DoLabel(&lblWeb, theme.m_Web, 10.0f, -1);
        }

        it++;
		i++;
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
	    std::list<CTheme>::iterator it = themes.begin();
	    std::advance(it, NewSelected);
	    CTheme theme = (*it);
		str_copy(g_Config.m_hcTheme, theme.m_FolderName, sizeof(g_Config.m_hcTheme));
        m_pClient->TexturePack()->Load(g_Config.m_hcTheme);
        m_NeedRestartSound = true;
	}
}

void CMenus::RenderStatistics(CUIRect MainView)
{
	CUIRect HUDItem;
	char aBuf[128];

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	str_format(aBuf, sizeof(aBuf), "Started H-Client: %d times", g_Stats.m_ClientRuns);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
    time_t rawtime = g_Stats.m_ClientTimeRun;
    struct tm *timeinfo = localtime(&rawtime);
    char aTime[128];
    int days, hours, minutes, seconds;
    strftime(aTime, sizeof(aTime), "%j:%I:%M:%S", timeinfo);
    sscanf(aTime, "%d:%d:%d:%d", &days, &hours, &minutes, &seconds);
    days--; hours--; // Starts from 1
	str_format(aBuf, sizeof(aBuf), "Total time played: %d days, %02d:%02d:%02d", days, hours, minutes, seconds);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, 0x0, &MainView);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", g_Stats.m_Kills);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", g_Stats.m_Deaths);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	str_format(aBuf, sizeof(aBuf), "Total damage received: %d", g_Stats.m_TotalDamage);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, 0x0, &MainView);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	str_format(aBuf, sizeof(aBuf), "Joined on a server: %d times", g_Stats.m_ServerJoins);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	str_format(aBuf, sizeof(aBuf), "Chat messages sent: %d", g_Stats.m_ChatMessagesSent);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	str_format(aBuf, sizeof(aBuf), "Chat messages received: %d", g_Stats.m_ChatMessagesReceived);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	UI()->DoLabel(&HUDItem, "Best Score", HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
	str_format(aBuf, sizeof(aBuf), "CTF: %d", g_Stats.m_BestScore[0]);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
	str_format(aBuf, sizeof(aBuf), "TDM: %d", g_Stats.m_BestScore[1]);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
	str_format(aBuf, sizeof(aBuf), "DM: %d", g_Stats.m_BestScore[2]);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, 0x0, &MainView);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	str_format(aBuf, sizeof(aBuf), "Weapon changes: %d times", g_Stats.m_WeaponChanges);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	int TotalShoots = g_Stats.m_ShootsHammer + g_Stats.m_ShootsGun + g_Stats.m_ShootsShotgun + g_Stats.m_ShootsGrenades + g_Stats.m_ShootsRifle;
	str_format(aBuf, sizeof(aBuf), "Total Shoots: %d times", TotalShoots);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
	str_format(aBuf, sizeof(aBuf), "Hammer: %d", g_Stats.m_ShootsHammer);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
	str_format(aBuf, sizeof(aBuf), "Pistol: %d", g_Stats.m_ShootsGun);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
	str_format(aBuf, sizeof(aBuf), "Shotgun: %d", g_Stats.m_ShootsShotgun);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
	str_format(aBuf, sizeof(aBuf), "Grenades: %d", g_Stats.m_ShootsGrenades);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitTop(20.0f, &HUDItem, &MainView);
	HUDItem.VSplitLeft(20.0f, 0x0, &HUDItem);
	str_format(aBuf, sizeof(aBuf), "Rifle: %d", g_Stats.m_ShootsRifle);
	UI()->DoLabel(&HUDItem, aBuf, HUDItem.h*ms_FontmodHeight, -1);

	MainView.HSplitBottom(20.0f, &MainView, &HUDItem);
    CUIRect Button;
    HUDItem.VSplitRight(150.0f, 0x0, &Button);
    Button.Margin(2.0f, &Button);
    static int s_ButtonClearCache = 0;
    if (DoButton_Menu((void*)&s_ButtonClearCache, Localize("Reset Statistics"), 0, &Button))
    	g_Stats.Reset();
}

// H-Client
void CMenus::RenderLaser(vec2 From, vec2 Pos)
{
	vec2 Dir = normalize(Pos-From);

	vec2 Out, Border;

	Graphics()->BlendNormal();

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	From.x+=60.0f;

	//vec4 inner_color(0.15f,0.35f,0.75f,1.0f);
	//vec4 outer_color(0.65f,0.85f,1.0f,1.0f);

	vec3 Rgb = HslToRgb(vec3(g_Config.m_hcLaserColorHue/255.0f, g_Config.m_hcLaserColorSat/255.0f, g_Config.m_hcLaserColorLht/255.0f));

	// do outline
	vec4 OuterColor(0.075f, 0.075f, 0.25f, 1.0f);
	if (g_Config.m_hcLaserCustomColor)
        OuterColor = vec4(Rgb.r+0.25f, Rgb.g+0.25f, Rgb.b+0.25f, g_Config.m_hcLaserColorAlpha/255.0f);

	Graphics()->SetColor(OuterColor.r, OuterColor.g, OuterColor.b, OuterColor.a);
	Out = vec2(Dir.y, -Dir.x) * (5.0f);

	IGraphics::CFreeformItem Freeform(
			From.x-Out.x, From.y-Out.y,
			From.x+Out.x, From.y+Out.y,
			Pos.x-Out.x, Pos.y-Out.y,
			Pos.x+Out.x, Pos.y+Out.y);
	Graphics()->QuadsDrawFreeform(&Freeform, 1);

	// do inner
	vec4 InnerColor(0.5f, 0.5f, 1.0f, 1.0f);
	if (g_Config.m_hcLaserCustomColor)
        InnerColor = vec4(Rgb.r, Rgb.g, Rgb.b, g_Config.m_hcLaserColorAlpha/255.0f);

	Out = vec2(Dir.y, -Dir.x) * (3.0f);
	Graphics()->SetColor(InnerColor.r, InnerColor.g, InnerColor.b, InnerColor.a); // center

	Freeform = IGraphics::CFreeformItem(
			From.x-Out.x, From.y-Out.y,
			From.x+Out.x, From.y+Out.y,
			Pos.x-Out.x, Pos.y-Out.y,
			Pos.x+Out.x, Pos.y+Out.y);
	Graphics()->QuadsDrawFreeform(&Freeform, 1);

	Graphics()->QuadsEnd();

	// render head
	{
		Graphics()->BlendNormal();
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
		Graphics()->QuadsBegin();

		RenderTools()->SelectSprite(SPRITE_PART_SPLAT01);
		Graphics()->QuadsSetRotation(1.0f);
		Graphics()->SetColor(OuterColor.r, OuterColor.g, OuterColor.b, OuterColor.a);
		IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, 20, 20);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->SetColor(InnerColor.r, InnerColor.g, InnerColor.b, InnerColor.a);
		QuadItem = IGraphics::CQuadItem(Pos.x, Pos.y, 16, 16);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}

	// Weapon
	From.x-=30.0f;
	From.y+=5.0f;
    Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
    Graphics()->QuadsBegin();
    RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[WEAPON_RIFLE].m_pSpriteBody, Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
	RenderTools()->DrawSprite(From.x, From.y, g_pData->m_Weapons.m_aId[WEAPON_RIFLE].m_VisualSize);
	Graphics()->QuadsEnd();
}
