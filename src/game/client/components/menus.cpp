/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <engine/config.h>
#include <engine/editor.h>
#include <engine/engine.h>
#include <engine/friends.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/servermanager.h> //H-Client
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <engine/server.h> //H-Client
#include <engine/news.h> //H-Client
#include <engine/irc.h> //H-Client

#include <game/version.h>
#include <game/generated/protocol.h>

#include <game/generated/client_data.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>
#include <game/client/lineinput.h>
#include <game/client/animstate.h> //H-Client
#include <game/localization.h>
#include <mastersrv/mastersrv.h>

#include "countryflags.h"
#include "menus.h"
#include "skins.h"

vec4 CMenus::ms_GuiColor;
vec4 CMenus::ms_ColorTabbarInactiveOutgame;
vec4 CMenus::ms_ColorTabbarActiveOutgame;
vec4 CMenus::ms_ColorTabbarInactive;
vec4 CMenus::ms_ColorTabbarActive;
vec4 CMenus::ms_ColorTabbarInactiveIngame;
vec4 CMenus::ms_ColorTabbarActiveIngame;


float CMenus::ms_ButtonHeight = 25.0f;
float CMenus::ms_ListheaderHeight = 17.0f;
float CMenus::ms_FontmodHeight = 0.8f;

IInput::CEvent CMenus::m_aInputEvents[MAX_INPUTEVENTS];
int CMenus::m_NumInputEvents;

int CMenus::s_CheckSave = 0; //H-Client


CMenus::CMenus()
{
	m_Popup = POPUP_NONE;
	//m_ActivePage = PAGE_INTERNET;
	m_ActivePage = PAGE_SERVERS; //H-Client
	m_GamePage = PAGE_GAME;

	m_NeedRestartGraphics = false;
	m_NeedRestartSound = false;
	m_NeedSendinfo = false;
	m_MenuActive = true;
	m_UseMouseButtons = true;

	m_EscapePressed = false;
	m_EnterPressed = false;
	m_DeletePressed = false;
	m_NumInputEvents = 0;

	m_LastInput = time_get();

	str_copy(m_aCurrentDemoFolder, "demos", sizeof(m_aCurrentDemoFolder));
	m_aCallvoteReason[0] = 0;

	m_FriendlistSelectedIndex = -1;

	//H-Client
    m_FileDialogStorageType = 0;
    m_pFileDialogTitle = 0;
	m_pFileDialogButtonText = 0;
	m_pFileDialogUser = 0;
	m_aFileDialogFileName[0] = 0;
	m_aFileDialogCurrentFolder[0] = 0;
	m_aFileDialogCurrentLink[0] = 0;
	m_pFileDialogPath = m_aFileDialogCurrentFolder;
	m_aFileDialogActivate = false;
	m_FileDialogScrollValue = 0.0f;
	m_FilesSelectedIndex = -1;
	m_FilesStartAt = 0;
	m_FilesCur = 0;
	m_FilesStopAt = 999;

	m_SelectedLocalServer = -1;
	m_SelectedLocalExecServer = 0;

	m_Dialog = DIALOG_NONE;
	//
}

vec4 CMenus::ButtonColorMul(const void *pID)
{
	if(UI()->ActiveItem() == pID)
		return vec4(1,1,1,0.5f);
	else if(UI()->HotItem() == pID)
		return vec4(1,1,1,1.5f);
	return vec4(1,1,1,1);
}

int CMenus::DoButton_Icon(int ImageId, int SpriteId, const CUIRect *pRect)
{
	Graphics()->TextureSet(g_pData->m_aImages[ImageId].m_Id);

	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SpriteId);
	IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	return 0;
}

int CMenus::DoButton_Toggle(const void *pID, int Checked, const CUIRect *pRect)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GUIBUTTONS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(Checked?SPRITE_GUIBUTTON_ON:SPRITE_GUIBUTTON_OFF);
	IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	if((UI()->HotItem() == pID))
	{
		RenderTools()->SelectSprite(SPRITE_GUIBUTTON_HOVER);
		IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
	}
	Graphics()->QuadsEnd();

	return UI()->DoButtonLogic(pID, "", Checked, pRect);
}

int CMenus::DoButton_Menu(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	RenderTools()->DrawUIRect(pRect, vec4(1,1,1,0.5f)*ButtonColorMul(pID), CUI::CORNER_ALL, 5.0f);
	CUIRect Temp;
	pRect->HMargin(pRect->h>=20.0f?2.0f:1.0f, &Temp);
	UI()->DoLabel(&Temp, pText, Temp.h*ms_FontmodHeight, 0);
	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

void CMenus::DoButton_KeySelect(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	RenderTools()->DrawUIRect(pRect, vec4(1,1,1,0.5f)*ButtonColorMul(pID), CUI::CORNER_ALL, 5.0f);
	CUIRect Temp;
	pRect->HMargin(1.0f, &Temp);
	UI()->DoLabel(&Temp, pText, Temp.h*ms_FontmodHeight, 0);
}

int CMenus::DoButton_MenuTab(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Corners)
{
    float Seconds = 0.6f; //  0.6 seconds for fade
	float *pFade = ButtonFade(pID, Seconds, Checked);
	float FadeVal = *pFade/Seconds;

	if(Checked)
		RenderTools()->DrawUIRect(pRect, ms_ColorTabbarActive, Corners, 10.0f);
	else
	{
		RenderTools()->DrawUIRect(pRect, ms_ColorTabbarInactive, Corners, 10.0f);
		FadeVal = 1.0f;
	}
	CUIRect Temp;
	pRect->HMargin(2.0f, &Temp);
	TextRender()->TextColor(0.0f+FadeVal, 0.0f+FadeVal, 0.0f+FadeVal, 1.0f);
	UI()->DoLabel(&Temp, pText, Temp.h*ms_FontmodHeight, 0);
	TextRender()->TextColor(1, 1, 1, 1);

	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

int CMenus::DoButton_MenuTabIcon(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Corners, int imgID, float size, int sprID)
{
    float Seconds = 0.6f; //  0.6 seconds for fade
	float *pFade = ButtonFade(pID, Seconds, Checked);
	float FadeVal = *pFade/Seconds;

	if(Checked)
		RenderTools()->DrawUIRect(pRect, ms_ColorTabbarActive, Corners, 10.0f);
	else
	{
		RenderTools()->DrawUIRect(pRect, ms_ColorTabbarInactive, Corners, 10.0f);
		FadeVal = 1.0f;
	}
	CUIRect Temp;
	pRect->HMargin(2.0f, &Temp);
	Temp.x+=3.0f;
	TextRender()->TextColor(FadeVal, FadeVal, FadeVal, 1.0f);
	UI()->DoLabel(&Temp, pText, Temp.h*ms_FontmodHeight, -1);
	TextRender()->TextColor(1, 1, 1, 1);

    Graphics()->TextureSet(g_pData->m_aImages[imgID].m_Id);
    Graphics()->QuadsBegin();
    Graphics()->SetColor(1.0f-FadeVal/2, 1.0f-FadeVal/2, 1.0f-FadeVal/2, 1.0f);
    if (sprID != -1)
        RenderTools()->SelectSprite(sprID);
    RenderTools()->DrawSprite(Temp.x+(Temp.w-18.0f), Temp.y+12.0f, size);
    Graphics()->QuadsEnd();

	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

int CMenus::DoButton_GridHeader(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
//void CMenus::ui_draw_grid_header(const void *id, const char *text, int checked, const CUIRect *r, const void *extra)
{
	if(Checked)
		RenderTools()->DrawUIRect(pRect, vec4(1,1,1,0.5f), CUI::CORNER_T, 5.0f);
	CUIRect t;
	pRect->VSplitLeft(5.0f, 0, &t);
	UI()->DoLabel(&t, pText, pRect->h*ms_FontmodHeight, -1);
	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

int CMenus::DoButton_CheckBox_Common(const void *pID, const char *pText, const char *pBoxText, const CUIRect *pRect)
//void CMenus::ui_draw_checkbox_common(const void *id, const char *text, const char *boxtext, const CUIRect *r, const void *extra)
{
	CUIRect c = *pRect;
	CUIRect t = *pRect;
	c.w = c.h;
	t.x += c.w;
	t.w -= c.w;
	t.VSplitLeft(5.0f, 0, &t);

	c.Margin(2.0f, &c);
	RenderTools()->DrawUIRect(&c, vec4(1,1,1,0.25f)*ButtonColorMul(pID), CUI::CORNER_ALL, 3.0f);
	c.y += 2;
	UI()->DoLabel(&c, pBoxText, pRect->h*ms_FontmodHeight*0.6f, 0);
	UI()->DoLabel(&t, pText, pRect->h*ms_FontmodHeight*0.8f, -1);
	return UI()->DoButtonLogic(pID, pText, 0, pRect);
}

int CMenus::DoButton_CheckBox(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	return DoButton_CheckBox_Common(pID, pText, Checked?"X":"", pRect);
}


int CMenus::DoButton_CheckBox_Number(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	char aBuf[16];
	str_format(aBuf, sizeof(aBuf), "%d", Checked);
	return DoButton_CheckBox_Common(pID, pText, aBuf, pRect);
}

//H-Client
vec4 CMenus::GetButtonColor(const void *pID, int Checked)
{
	if(Checked < 0)
		return vec4(0,0,0,0.5f);

	if(Checked > 0)
	{
		if(UI()->HotItem() == pID)
			return vec4(1,0,0,0.75f);
		return vec4(1,0,0,0.5f);
	}

	if(UI()->HotItem() == pID)
		return vec4(1,1,1,0.75f);
	return vec4(1,1,1,0.5f);
}

int CMenus::DoButton_Menu_Common(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

int CMenus::DoButton_File(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Flags, const char *pToolTip)
{
	if(Checked)
		RenderTools()->DrawUIRect(pRect, GetButtonColor(pID, Checked), CUI::CORNER_ALL, 3.0f);

	CUIRect t = *pRect;
	t.VMargin(5.0f, &t);
	UI()->DoLabel(&t, pText, 10, -1, -1);
	return DoButton_Menu_Common(pID, pText, Checked, pRect, Flags, pToolTip);
}
//

int CMenus::DoEditBox(void *pID, const CUIRect *pRect, char *pStr, unsigned StrSize, float FontSize, float *Offset, bool Hidden, int Corners, bool ASCII)
{
	int Inside = UI()->MouseInside(pRect);
	bool ReturnValue = false;
	bool UpdateOffset = false;
	static int s_AtIndex = 0;
	static bool s_DoScroll = false;
	static float s_ScrollStart = 0.0f;

	FontSize *= UI()->Scale();

	if(UI()->LastActiveItem() == pID)
	{
		int Len = str_length(pStr);
		if(Len == 0)
			s_AtIndex = 0;

		if(Inside && UI()->MouseButton(0))
		{
			s_DoScroll = true;
			s_ScrollStart = UI()->MouseX();
			int MxRel = (int)(UI()->MouseX() - pRect->x);

			for(int i = 1; i <= Len; i++)
			{
				if(TextRender()->TextWidth(0, FontSize, pStr, i) - *Offset > MxRel)
				{
					s_AtIndex = i - 1;
					break;
				}

				if(i == Len)
					s_AtIndex = Len;
			}
		}
		else if(!UI()->MouseButton(0))
			s_DoScroll = false;
		else if(s_DoScroll)
		{
			// do scrolling
			if(UI()->MouseX() < pRect->x && s_ScrollStart-UI()->MouseX() > 10.0f)
			{
				s_AtIndex = max(0, s_AtIndex-1);
				s_ScrollStart = UI()->MouseX();
				UpdateOffset = true;
			}
			else if(UI()->MouseX() > pRect->x+pRect->w && UI()->MouseX()-s_ScrollStart > 10.0f)
			{
				s_AtIndex = min(Len, s_AtIndex+1);
				s_ScrollStart = UI()->MouseX();
				UpdateOffset = true;
			}
		}

		for(int i = 0; i < m_NumInputEvents; i++)
		{
			Len = str_length(pStr);

			int Code = m_aInputEvents[i].m_Unicode;
			if (ASCII && (Code < 0 || Code > 127))
                continue;

			ReturnValue |= CLineInput::Manipulate(m_aInputEvents[i], pStr, StrSize, &Len, &s_AtIndex);
		}
	}

	bool JustGotActive = false;

	if(UI()->ActiveItem() == pID)
	{
		if(!UI()->MouseButton(0))
		{
			s_AtIndex = min(s_AtIndex, str_length(pStr));
			s_DoScroll = false;
			UI()->SetActiveItem(0);
		}
	}
	else if(UI()->HotItem() == pID)
	{
		if(UI()->MouseButton(0))
		{
			if (UI()->LastActiveItem() != pID)
				JustGotActive = true;
			UI()->SetActiveItem(pID);
		}
	}

	if(Inside)
		UI()->SetHotItem(pID);

	CUIRect Textbox = *pRect;
	RenderTools()->DrawUIRect(&Textbox, vec4(1, 1, 1, 0.5f), Corners, 3.0f);
	Textbox.VMargin(2.0f, &Textbox);
	Textbox.HMargin(2.0f, &Textbox);

	const char *pDisplayStr = pStr;
	char aStars[128];

	if(Hidden)
	{
		unsigned s = str_length(pStr);
		if(s >= sizeof(aStars))
			s = sizeof(aStars)-1;
		for(unsigned int i = 0; i < s; ++i)
			aStars[i] = '*';
		aStars[s] = 0;
		pDisplayStr = aStars;
	}

	// check if the text has to be moved
	if(UI()->LastActiveItem() == pID && !JustGotActive && (UpdateOffset || m_NumInputEvents))
	{
		float w = TextRender()->TextWidth(0, FontSize, pDisplayStr, s_AtIndex);
		if(w-*Offset > Textbox.w)
		{
			// move to the left
			float wt = TextRender()->TextWidth(0, FontSize, pDisplayStr, -1);
			do
			{
				*Offset += min(wt-*Offset-Textbox.w, Textbox.w/3);
			}
			while(w-*Offset > Textbox.w);
		}
		else if(w-*Offset < 0.0f)
		{
			// move to the right
			do
			{
				*Offset = max(0.0f, *Offset-Textbox.w/3);
			}
			while(w-*Offset < 0.0f);
		}
	}
	UI()->ClipEnable(pRect);
	Textbox.x -= *Offset;

	UI()->DoLabel(&Textbox, pDisplayStr, FontSize, -1);

	// render the cursor
	if(UI()->LastActiveItem() == pID && !JustGotActive)
	{
		float w = TextRender()->TextWidth(0, FontSize, pDisplayStr, s_AtIndex);
		Textbox = *pRect;
		Textbox.VSplitLeft(2.0f, 0, &Textbox);
		Textbox.x += (w-*Offset-TextRender()->TextWidth(0, FontSize, "|", -1)/2);

		if((2*time_get()/time_freq()) % 2)	// make it blink
			UI()->DoLabel(&Textbox, "|", FontSize, -1);
	}
	UI()->ClipDisable();

	return ReturnValue;
}

float CMenus::DoScrollbarV(const void *pID, const CUIRect *pRect, float Current)
{
	CUIRect Handle;
	static float OffsetY;
	pRect->HSplitTop(33, &Handle, 0);

	Handle.y += (pRect->h-Handle.h)*Current;

	// logic
	float ReturnValue = Current;
	int Inside = UI()->MouseInside(&Handle);

	if(UI()->ActiveItem() == pID)
	{
		if(!UI()->MouseButton(0))
			UI()->SetActiveItem(0);

		float Min = pRect->y;
		float Max = pRect->h-Handle.h;
		float Cur = UI()->MouseY()-OffsetY;
		ReturnValue = (Cur-Min)/Max;
		if(ReturnValue < 0.0f) ReturnValue = 0.0f;
		if(ReturnValue > 1.0f) ReturnValue = 1.0f;
	}
	else if(UI()->HotItem() == pID)
	{
		if(UI()->MouseButton(0))
		{
			UI()->SetActiveItem(pID);
			OffsetY = UI()->MouseY()-Handle.y;
		}
	}

	if(Inside)
		UI()->SetHotItem(pID);

	// render
	CUIRect Rail;
	pRect->VMargin(5.0f, &Rail);
	RenderTools()->DrawUIRect(&Rail, vec4(1,1,1,0.25f), 0, 0.0f);

	CUIRect Slider = Handle;
	Slider.w = Rail.x-Slider.x;
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f), CUI::CORNER_L, 2.5f);
	Slider.x = Rail.x+Rail.w;
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f), CUI::CORNER_R, 2.5f);

	Slider = Handle;
	Slider.Margin(5.0f, &Slider);
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f)*ButtonColorMul(pID), CUI::CORNER_ALL, 2.5f);

	return ReturnValue;
}



float CMenus::DoScrollbarH(const void *pID, const CUIRect *pRect, float Current)
{
	CUIRect Handle;
	static float OffsetX;
	pRect->VSplitLeft(33, &Handle, 0);

	Handle.x += (pRect->w-Handle.w)*Current;

	// logic
	float ReturnValue = Current;
	int Inside = UI()->MouseInside(&Handle);

	if(UI()->ActiveItem() == pID)
	{
		if(!UI()->MouseButton(0))
			UI()->SetActiveItem(0);

		float Min = pRect->x;
		float Max = pRect->w-Handle.w;
		float Cur = UI()->MouseX()-OffsetX;
		ReturnValue = (Cur-Min)/Max;
		if(ReturnValue < 0.0f) ReturnValue = 0.0f;
		if(ReturnValue > 1.0f) ReturnValue = 1.0f;
	}
	else if(UI()->HotItem() == pID)
	{
		if(UI()->MouseButton(0))
		{
			UI()->SetActiveItem(pID);
			OffsetX = UI()->MouseX()-Handle.x;
		}
	}

	if(Inside)
		UI()->SetHotItem(pID);

	// render
	CUIRect Rail;
	pRect->HMargin(5.0f, &Rail);
	RenderTools()->DrawUIRect(&Rail, vec4(1,1,1,0.25f), 0, 0.0f);

	CUIRect Slider = Handle;
	Slider.h = Rail.y-Slider.y;
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f), CUI::CORNER_T, 2.5f);
	Slider.y = Rail.y+Rail.h;
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f), CUI::CORNER_B, 2.5f);

	Slider = Handle;
	Slider.Margin(5.0f, &Slider);
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f)*ButtonColorMul(pID), CUI::CORNER_ALL, 2.5f);

	return ReturnValue;
}

int CMenus::DoKeyReader(void *pID, const CUIRect *pRect, int Key)
{
	// process
	static void *pGrabbedID = 0;
	static bool MouseReleased = true;
	static int ButtonUsed = 0;
	int Inside = UI()->MouseInside(pRect);
	int NewKey = Key;

	if(!UI()->MouseButton(0) && !UI()->MouseButton(1) && pGrabbedID == pID)
		MouseReleased = true;

	if(UI()->ActiveItem() == pID)
	{
		if(m_Binder.m_GotKey)
		{
			// abort with escape key
			if(m_Binder.m_Key.m_Key != KEY_ESCAPE)
				NewKey = m_Binder.m_Key.m_Key;
			m_Binder.m_GotKey = false;
			UI()->SetActiveItem(0);
			MouseReleased = false;
			pGrabbedID = pID;
		}

		if(ButtonUsed == 1 && !UI()->MouseButton(1))
		{
			if(Inside)
				NewKey = 0;
			UI()->SetActiveItem(0);
		}
	}
	else if(UI()->HotItem() == pID)
	{
		if(MouseReleased)
		{
			if(UI()->MouseButton(0))
			{
				m_Binder.m_TakeKey = true;
				m_Binder.m_GotKey = false;
				UI()->SetActiveItem(pID);
				ButtonUsed = 0;
			}

			if(UI()->MouseButton(1))
			{
				UI()->SetActiveItem(pID);
				ButtonUsed = 1;
			}
		}
	}

	if(Inside)
		UI()->SetHotItem(pID);

	// draw
	if (UI()->ActiveItem() == pID && ButtonUsed == 0)
		DoButton_KeySelect(pID, "???", 0, pRect);
	else
	{
		if(Key == 0)
			DoButton_KeySelect(pID, "", 0, pRect);
		else
			DoButton_KeySelect(pID, Input()->KeyName(Key), 0, pRect);
	}
	return NewKey;
}

float *CMenus::ButtonFade(const void *pID, float Seconds, int Checked)
{
	float *pFade = (float*)pID;
	if(!Checked)
		*pFade = Seconds;
	else if(*pFade > 0.0f)
	{
		*pFade -= Client()->FrameTime();
		if(*pFade < 0.0f)
			*pFade = 0.0f;
	}
	return pFade;
}

int CMenus::RenderMenubar(CUIRect r)
{
	CUIRect Box = r;
	CUIRect Button;

	static int s_ServersButton=0; //H-Client

	m_ActivePage = g_Config.m_UiPage;
	int NewPage = -1;

	if(Client()->State() != IClient::STATE_OFFLINE)
		m_ActivePage = m_GamePage;

    ms_ColorTabbarActive = vec4(1,1,1,0.85f);
    ms_ColorTabbarInactive = vec4(1,1,1,0.27f);

	if(Client()->State() == IClient::STATE_OFFLINE)
	{
		// offline menus
        Box.VSplitLeft(75.0f, &Button, &Box);
        static int s_NewsButton=0;
        if (DoButton_MenuTab(&s_NewsButton, Localize("News"), m_ActivePage==PAGE_NEWS, &Button, CUI::CORNER_TL))
            NewPage = PAGE_NEWS;
        //H-Client
		Box.VSplitLeft(75.0f, &Button, &Box);
        if(DoButton_MenuTab(&s_ServersButton, Localize("Servers"), m_ActivePage==PAGE_SERVERS, &Button, CUI::CORNER_TR))
        {
            if (g_Config.m_UiSubPage == SUBPAGE_FAVORITES)
                ServerBrowser()->Refresh(IServerBrowser::TYPE_FAVORITES);
            else if (g_Config.m_UiSubPage == SUBPAGE_LAN)
                ServerBrowser()->Refresh(IServerBrowser::TYPE_LAN);
            else
                ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);

            NewPage = PAGE_SERVERS;
        }
        //

		/*Box.VSplitLeft(100.0f, &Button, &Box);
		static int s_InternetButton=0;
		if(DoButton_MenuTab(&s_InternetButton, Localize("Internet"), m_ActivePage==PAGE_INTERNET, &Button, CUI::CORNER_TL))
		{
			ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);
			NewPage = PAGE_INTERNET;
		}

		//Box.VSplitLeft(4.0f, 0, &Box);
		Box.VSplitLeft(80.0f, &Button, &Box);
		static int s_LanButton=0;
		if(DoButton_MenuTab(&s_LanButton, Localize("LAN"), m_ActivePage==PAGE_LAN, &Button, 0))
		{
			ServerBrowser()->Refresh(IServerBrowser::TYPE_LAN);
			NewPage = PAGE_LAN;
		}

		//box.VSplitLeft(4.0f, 0, &box);
		Box.VSplitLeft(110.0f, &Button, &Box);
		static int s_FavoritesButton=0;
		if(DoButton_MenuTab(&s_FavoritesButton, Localize("Favorites"), m_ActivePage==PAGE_FAVORITES, &Button, CUI::CORNER_TR))
		{
			ServerBrowser()->Refresh(IServerBrowser::TYPE_FAVORITES);
			NewPage = PAGE_FAVORITES;
		}*/

		Box.VSplitLeft(4.0f*5, 0, &Box);
		Box.VSplitLeft(100.0f, &Button, &Box);
		static int s_DemosButton=0;
		if(DoButton_MenuTab(&s_DemosButton, Localize("Demos"), m_ActivePage==PAGE_DEMOS, &Button, CUI::CORNER_T))
		{
			DemolistPopulate();
			NewPage = PAGE_DEMOS;
		}

		//H-Client
        /*Box.VSplitLeft(30.0f, 0, &Box);
		Box.VSplitLeft(130.0f, &Button, &Box);
		static int s_ManagerButton=0;
        if(DoButton_MenuTab(&s_ManagerButton, Localize("Create Server"), m_ActivePage==PAGE_LOCAL_SERVERS, &Button, CUI::CORNER_T))
        {
            NewPage = PAGE_LOCAL_SERVERS;
            ServerManager()->SearchServersBin();
        }*/
		//
	}
	else
	{
		// online menus
		Box.VSplitLeft(90.0f, &Button, &Box);
        if(DoButton_MenuTab(&s_ServersButton, Localize("Servers"), m_ActivePage==PAGE_SERVERS, &Button, 0))
        {
            if (g_Config.m_UiSubPage == SUBPAGE_FAVORITES)
                ServerBrowser()->Refresh(IServerBrowser::TYPE_FAVORITES);
            else if (g_Config.m_UiSubPage == SUBPAGE_LAN)
                ServerBrowser()->Refresh(IServerBrowser::TYPE_LAN);
            else
                ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);

            NewPage = PAGE_SERVERS;
        }
        //

		Box.VSplitLeft(90.0f, &Button, &Box);
		static int s_GameButton=0;
		if(DoButton_MenuTab(&s_GameButton, Localize("Game"), m_ActivePage==PAGE_GAME, &Button, 0))
			NewPage = PAGE_GAME;

		Box.VSplitLeft(90.0f, &Button, &Box);
		static int s_PlayersButton=0;
		if(DoButton_MenuTab(&s_PlayersButton, Localize("Players"), m_ActivePage==PAGE_PLAYERS, &Button, 0))
			NewPage = PAGE_PLAYERS;

		Box.VSplitLeft(130.0f, &Button, &Box);
		static int s_ServerInfoButton=0;
		if(DoButton_MenuTab(&s_ServerInfoButton, Localize("Server info"), m_ActivePage==PAGE_SERVER_INFO, &Button, 0))
			NewPage = PAGE_SERVER_INFO;

		//Box.VSplitLeft(130.0f, &Button, &Box);
		Box.VSplitLeft(90.0f, &Button, &Box); //H-Client
		static int s_CallVoteButton=0;
		if(DoButton_MenuTab(&s_CallVoteButton, Localize("Call vote"), m_ActivePage==PAGE_CALLVOTE, &Button, CUI::CORNER_TR))
			NewPage = PAGE_CALLVOTE;
	}

	/*
	box.VSplitRight(110.0f, &box, &button);
	static int system_button=0;
	if (UI()->DoButton(&system_button, "System", g_Config.m_UiPage==PAGE_SYSTEM, &button))
		g_Config.m_UiPage = PAGE_SYSTEM;

	box.VSplitRight(30.0f, &box, 0);
	*/

	Box.VSplitRight(90.0f, &Box, &Button);
	static int s_QuitButton=0;
	if(DoButton_MenuTab(&s_QuitButton, Localize("Quit"), 0, &Button, CUI::CORNER_T))
		m_Popup = POPUP_QUIT;

	Box.VSplitRight(10.0f, &Box, &Button);
	Box.VSplitRight(130.0f, &Box, &Button);
	static int s_SettingsButton=0;
	if(DoButton_MenuTab(&s_SettingsButton, Localize("Settings"), m_ActivePage==PAGE_SETTINGS, &Button, CUI::CORNER_T))
		NewPage = PAGE_SETTINGS;

	if(Client()->State() == IClient::STATE_OFFLINE)
	{

		//H-Client
        Box.VSplitRight(10.0f, &Box, &Button);
        Box.VSplitRight(130.0f, &Box, &Button);
		static int s_MapEditor=0;
		if(DoButton_MenuTab(&s_MapEditor, Localize("Map Editor"), 0, &Button, CUI::CORNER_T))
		{
			g_Config.m_ClEditor = g_Config.m_ClEditor^1;
			Input()->MouseModeRelative();
		}
	}

	if(NewPage != -1)
	{
		if(Client()->State() == IClient::STATE_OFFLINE)
			g_Config.m_UiPage = NewPage;
		else
			m_GamePage = NewPage;
	}

    ms_ColorTabbarActive = vec4(0,0,0,0.85f);
    ms_ColorTabbarInactive = vec4(0,0,0,0.15f);
	return 0;
}

void CMenus::RenderLoading()
{
	static int64 LastLoadRender = 0;
	float Percent = m_LoadCurrent++/(float)m_LoadTotal;

	// make sure that we don't render for each little thing we load
	// because that will slow down loading if we have vsync
	if(time_get()-LastLoadRender < time_freq()/60)
		return;

	LastLoadRender = time_get();

	// need up date this here to get correct
	vec3 Rgb = HslToRgb(vec3(g_Config.m_UiColorHue/255.0f, g_Config.m_UiColorSat/255.0f, g_Config.m_UiColorLht/255.0f));
	ms_GuiColor = vec4(Rgb.r, Rgb.g, Rgb.b, g_Config.m_UiColorAlpha/255.0f);

	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	RenderBackground();

	float w = 700;
	float h = 200;
	float x = Screen.w/2-w/2;
	float y = Screen.h/2-h/2;

	Graphics()->BlendNormal();

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.50f);
	RenderTools()->DrawRoundRect(0, y, Screen.w, h, 0.0f);
	Graphics()->QuadsEnd();


	//const char *pCaption = Localize("Loading");
	//H-Client
	char pCaption[25];
	str_format(pCaption, sizeof(pCaption), "%s H-Client", Localize("Loading"));
    float percent = (m_LoadCurrent*100.0f)/(float)m_LoadTotal;
	//

	CUIRect r;
	r.x = x;
	r.y = y+20;
	r.w = w;
	r.h = h;
	UI()->DoLabel(&r, pCaption, 48.0f, 0, -1);

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.15f,0.15f,0.15f,0.75f);
	RenderTools()->DrawRoundRect(x+40, y+h-75, w-80, 30, 5.0f);
	Graphics()->SetColor(1,1,1,0.75f);
	RenderTools()->DrawRoundRect(x+45, y+h-70, (w-85)*Percent, 20, 5.0f);
	Graphics()->QuadsEnd();

	//H-Client
    {
        char aBuf[50];
        str_format(aBuf, sizeof(aBuf), "%.2f%c", percent, '%');
        float perLen = TextRender()->TextWidth(0, 16.0f, aBuf, str_length(aBuf));
        r.x = x-perLen/2;
        r.y = y+h-70;
        r.w = w;
        r.h = 20;
        vec3 RgbBar = HslToRgb(vec3(((m_LoadCurrent * 0.35f) / m_LoadTotal), 1.0f, 0.5f));
        TextRender()->TextColor(RgbBar.r, RgbBar.g, RgbBar.b, 1.0f);
        UI()->DoLabelScaled(&r, aBuf, 16.0f, 0, 0);

        //Teeworlds
        str_copy(aBuf, "TEEWORLDS", sizeof(aBuf));
        perLen = TextRender()->TextWidth(0, 92.0f, aBuf, str_length(aBuf));
        r.x = 0;
        r.y = 50.0f;
        r.w = Screen.w;
        r.h = 120.0f;
        TextRender()->TextColor(0.0f, 0.0f, 0.0f, 1.0f);
        UI()->DoLabel(&r, aBuf, 92.0f, 0, -1);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
	//

	Graphics()->Swap();
}

void CMenus::RenderNews(CUIRect MainView)
{
    static int lastState = m_pClient->News()->GetState();
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);

	CUIRect NewsPost, Sections, Section, Footer;
	MainView.Margin(10.0f, &NewsPost);
	NewsPost.HSplitTop(35.0f, &Sections, &NewsPost);
	ms_ColorTabbarActive = vec4(1,1,1,0.85f);
    ms_ColorTabbarInactive = vec4(1,1,1,0.15f);
    Sections.VSplitMid(&Section, &Sections);
    static int s_NewsHeroiAmarelo=0;
	if (DoButton_MenuTab(&s_NewsHeroiAmarelo, Localize("Forum Fails"), g_Config.m_UiSubPage == SUBPAGE_HEROIAMARELO, &Section, CUI::CORNER_L))
	{
        g_Config.m_UiSubPage = SUBPAGE_HEROIAMARELO;
        thread_create(ThreadRefreshForumFails, m_pClient->News());
	}
	static int s_NewsTeeworlds=0;
	if (DoButton_MenuTab(&s_NewsTeeworlds, Localize("Teeworlds"), g_Config.m_UiSubPage == SUBPAGE_TEEWORLDS, &Sections, CUI::CORNER_R))
	{
        g_Config.m_UiSubPage = SUBPAGE_TEEWORLDS;
        m_pClient->News()->RefreshTeeworlds();
	}
	ms_ColorTabbarActive = vec4(0,0,0,0.85f);
    ms_ColorTabbarInactive = vec4(0,0,0,0.15f);

	NewsPost.HSplitTop(10.0f, 0x0, &NewsPost);
	NewsPost.HSplitBottom(20.0f, &NewsPost, &Footer);
	CTextCursor Cursor;

	int state = m_pClient->News()->GetState();
	if (state == INews::STATE_READY)
	{
        //Load Textures
        if (lastState != INews::STATE_READY)
            m_pClient->News()->LoadTextures();

        if (g_Config.m_UiSubPage == SUBPAGE_HEROIAMARELO)
        {
            CUIRect Author;
            NewsPost.HSplitTop(20.0f, &Author, &NewsPost);
            UI()->DoLabel(&Author, "Channel Author: HeroiAmarelo", 16.0f, -1, -1);
        }

	    CUIRect View = NewsPost;
	    CUIRect Headers;

        struct CColumn
        {
            int m_Id;
            CLocConstString m_Caption;
            float m_Width;
            CUIRect m_Rect;
            CUIRect m_Spacer;
        };

        enum
        {
            COL_NEWS=0,
        };

        static CColumn s_aCols[] = {
            {-1,			" ",		            20.0f,		{0}, {0}},
            {COL_NEWS,	    " ",		            View.w,		{0}, {0}},
        };

        int NumCols = sizeof(s_aCols)/sizeof(CColumn);

        // do layout
        for(int i = 0; i < NumCols; i++)
        {
            Headers.VSplitLeft(s_aCols[i].m_Width, &s_aCols[i].m_Rect, &Headers);

            if(i+1 < NumCols)
                Headers.VSplitLeft(2, &s_aCols[i].m_Spacer, &Headers);
        }

        s_aCols[0].m_Rect.h = 450.0f;

        RenderTools()->DrawUIRect(&View, vec4(0,0,0,0.15f), 0, 0);

        CUIRect Scroll;
        View.VSplitRight(15, &View, &Scroll);

        int Num = (int)(View.h/s_aCols[0].m_Rect.h) + 1;
        static int s_ScrollBar = 0;
        static float s_ScrollValue = 0;

        Scroll.HMargin(5.0f, &Scroll);
        s_ScrollValue = DoScrollbarV(&s_ScrollBar, &Scroll, s_ScrollValue);

        int ScrollNum = m_pClient->News()->TotalNews()-Num+1;
        if(ScrollNum > 0)
        {
            if(Input()->KeyPresses(KEY_MOUSE_WHEEL_UP))
                s_ScrollValue -= 1.0f/ScrollNum;
            if(Input()->KeyPresses(KEY_MOUSE_WHEEL_DOWN))
                s_ScrollValue += 1.0f/ScrollNum;
        }
        else
            ScrollNum = 0;

        static int s_SelectedIndex = 0;
        for(int i = 0; i < m_NumInputEvents; i++)
        {
            int NewIndex = -1;
            if(m_aInputEvents[i].m_Flags&IInput::FLAG_PRESS)
            {
                if(m_aInputEvents[i].m_Key == KEY_DOWN) NewIndex = s_SelectedIndex + 1;
                if(m_aInputEvents[i].m_Key == KEY_UP) NewIndex = s_SelectedIndex - 1;
            }
            if(NewIndex > -1 && NewIndex < m_pClient->News()->TotalNews())
            {
                //scroll
                float IndexY = View.y - s_ScrollValue*ScrollNum*s_aCols[0].m_Rect.h + NewIndex*s_aCols[0].m_Rect.h;
                int Scroll = View.y > IndexY ? -1 : View.y+View.h < IndexY+s_aCols[0].m_Rect.h ? 1 : 0;
                if(Scroll)
                {
                    if(Scroll < 0)
                    {
                        int NumScrolls = (View.y-IndexY+s_aCols[0].m_Rect.h-1.0f)/s_aCols[0].m_Rect.h;
                        s_ScrollValue -= (1.0f/ScrollNum)*NumScrolls;
                    }
                    else
                    {
                        int NumScrolls = (IndexY+s_aCols[0].m_Rect.h-(View.y+View.h)+s_aCols[0].m_Rect.h-1.0f)/s_aCols[0].m_Rect.h;
                        s_ScrollValue += (1.0f/ScrollNum)*NumScrolls;
                    }
                }

                s_SelectedIndex = NewIndex;
            }
        }

        if(s_ScrollValue < 0) s_ScrollValue = 0;
        if(s_ScrollValue > 1) s_ScrollValue = 1;

        // set clipping
        UI()->ClipEnable(&View);

        CUIRect OriginalView = View;
        View.y -= s_ScrollValue*ScrollNum*s_aCols[0].m_Rect.h;

        int NewSelected = -1;

        if (g_Config.m_UiSubPage == SUBPAGE_HEROIAMARELO)
        {
            for (size_t i=0; i<m_pClient->News()->TotalNews(); i++)
            {
                SNewsPost CurPost = m_pClient->News()->GetNew(i);
                CUIRect Row;
                CUIRect SelectHitBox;

                int imgH = Graphics()->GetTextureHeight(CurPost.m_ImageID);
                View.HSplitTop(10.0f, 0x0, &View);
                View.HSplitTop(105.0f+imgH, &Row, &View);
                SelectHitBox = Row;

                // make sure that only those in view can be selected
                if(Row.y+Row.h > OriginalView.y && Row.y < OriginalView.y+OriginalView.h)
                {

                    CUIRect r = Row;
                    r.Margin(1.5f, &r);
                    RenderTools()->DrawUIRect(&r, vec4((i%2)?1.0f:0.45f,(i%2)?1.0f:0.45f,(i%2)?1.0f:0.45f,0.75f), 0, 0.0f);

                    // clip the selection
                    if(SelectHitBox.y < OriginalView.y) // top
                    {
                        SelectHitBox.h -= OriginalView.y-SelectHitBox.y;
                        SelectHitBox.y = OriginalView.y;
                    }
                    else if(SelectHitBox.y+SelectHitBox.h > OriginalView.y+OriginalView.h) // bottom
                        SelectHitBox.h = OriginalView.y+OriginalView.h-SelectHitBox.y;
                }

                for(int c = 0; c < NumCols; c++)
                {
                    CUIRect Button;
                    Button.x = s_aCols[c].m_Rect.x;
                    Button.y = Row.y;
                    Button.h = Row.h;
                    Button.w = s_aCols[c].m_Rect.w;

                    int Id = s_aCols[c].m_Id;
                    if(Id == COL_NEWS)
                    {
                        CUIRect PostRect, Head, Body, Date, Image;
                        PostRect = Button;
                        //RenderTools()->DrawUIRect(&PostRect, ms_ColorTabbarInactive, CUI::CORNER_ALL, 10.0f);

                        PostRect.HSplitTop(30.0f, &Head, &Body);
                        RenderTools()->DrawUIRect(&Head, vec4(0.0f,0.0f,0.75f, 0.75f), 0, 0);
                        Body.HSplitTop(75.0f, &Body, &Image);
                        Head.Margin(5.0f, &Head);
                        Body.Margin(5.0f, &Body);
                        Head.VSplitRight(150.0f, &Head, &Date);

                        UI()->DoLabel(&Head,  CurPost.m_Title, 16.0f, -1, -1);
                        UI()->DoLabel(&Date,  CurPost.m_Date, 16.0f, 1, -1);

                        TextRender()->SetCursor(&Cursor, Body.x, Body.y, 12.0f, TEXTFLAG_RENDER);
                        Cursor.m_LineWidth = Body.w-Body.x;
                        TextRender()->TextEx(&Cursor, CurPost.m_Text, -1);

                        if (CurPost.m_ImageID != -1)
                        {
                            Graphics()->BlendNormal();
                            Graphics()->TextureSet(CurPost.m_ImageID);
                            Graphics()->QuadsBegin();
                                Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
                                Graphics()->QuadsSetRotation(0.0f);
                                IGraphics::CQuadItem QuadItem(Image.x+Image.w/2, Image.y+Image.h/2, Image.w, Image.h);
                                Graphics()->QuadsDraw(&QuadItem, 1);
                            Graphics()->QuadsEnd();
                            Graphics()->TextureSet(-1);
                        }
                    }
                }
            }
        }
        else
        {
            UI()->DoLabel(&NewsPost, "CHANNEL NOT IMPLEMENTED :S", 16.0f, 0, -1);
        }

        if(NewSelected != -1)
            s_SelectedIndex = NewSelected;

        UI()->ClipDisable();

        CUIRect ButtonL, ButtonR;
        Footer.VSplitLeft(200.0f, &ButtonL, &Footer);
        Footer.VSplitRight(200.0f, &Footer, &ButtonR);
        static int s_PageIndex = 1;
        static int s_ButtonPrev=0;
        if (DoButton_Menu(&s_ButtonPrev, Localize("<<"), 0, &ButtonL))
        {
            if (s_PageIndex == 1)
                return;
            m_pClient->News()->SetPage(--s_PageIndex);
            thread_create(ThreadRefreshForumFails, m_pClient->News());
        }
        static int s_ButtonNext=0;
        if (DoButton_Menu(&s_ButtonNext, Localize(">>"), 0, &ButtonR))
        {
            m_pClient->News()->SetPage(++s_PageIndex);
            thread_create(ThreadRefreshForumFails, m_pClient->News());
        }

        char aBuf[128];
        str_format(aBuf, sizeof(aBuf), "Page %d", s_PageIndex);
        UI()->DoLabel(&Footer, aBuf, 16.0f, 0, -1);

	}
    if (state == INews::STATE_EMPTY)
    {
        UI()->DoLabel(&NewsPost, "SELECT CHANNEL", 16.0f, 0, -1);
    }
    else if (state == INews::STATE_DOWNLOADING)
    {
        UI()->DoLabel(&NewsPost, "DOWNLOADING NEWS, PLEASE WAIT...", 16.0f, 0, -1);
    }

    lastState = state;
}

void CMenus::RenderIrc(CUIRect MainView)
{
    MainView.Margin(5.0f, &MainView);
    RenderTools()->DrawUIRect(&MainView, vec4(0.2,0.4,0.4,0.5), CUI::CORNER_ALL, 10.0f);

    CUIRect MainIrc, EntryBox, Button;
    MainView.Margin(10.0f, &MainIrc);

    if (m_pClient->Irc()->GetState() == IIrc::STATE_DISCONNECTED)
    {
        EntryBox.x = MainIrc.x+(MainIrc.w/2.0f-300.0f/2.0f);
        EntryBox.w = 300.0f;
        EntryBox.y = MainIrc.y+(MainIrc.h/2.0f-55.0f/2.0f);
        EntryBox.h = 55.0f;

        RenderTools()->DrawUIRect(&EntryBox, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);
        EntryBox.Margin(5.0f, &EntryBox);

        EntryBox.HSplitTop(18.0f, &Button, &EntryBox);
        CUIRect Label;
        Button.VSplitLeft(40.0f, &Label, &Button);
        UI()->DoLabelScaled(&Label, Localize("Nick:"), 14.0f, -1);
        static float OffsetNick;
        DoEditBox(&g_Config.m_IrcNick, &Button, g_Config.m_IrcNick, sizeof(g_Config.m_IrcNick), 12.0f, &OffsetNick, false, CUI::CORNER_ALL, true);

        EntryBox.HSplitTop(5.0f, 0x0, &EntryBox);
        EntryBox.HSplitTop(20.0f, &Button, &EntryBox);
        static float s_ButtonConnect = 0;
        if (DoButton_Menu(&s_ButtonConnect, Localize("Connect"), 0, &Button))
        {
            m_pClient->Irc()->SetNick(g_Config.m_IrcNick);
            thread_create(ThreadIrcConnection, m_pClient->Irc());
        }
    }
    else if (m_pClient->Irc()->GetState() == IIrc::STATE_CONNECTING)
    {
        EntryBox.x = MainIrc.x+(MainIrc.w/2.0f-300.0f/2.0f);
        EntryBox.w = 300.0f;
        EntryBox.y = MainIrc.y+(MainIrc.h/2.0f-25.0f/2.0f);
        EntryBox.h = 25.0f;

        RenderTools()->DrawUIRect(&EntryBox, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);
        EntryBox.Margin(5.0f, &EntryBox);
        UI()->DoLabelScaled(&EntryBox, Localize("Connecting, please wait..."), 14.0f, -1);
    }
    else if (m_pClient->Irc()->GetState() == IIrc::STATE_CONNECTED)
    {
        CUIRect ButtonBox, InputBox;

        //Channel List
        MainIrc.HSplitTop(20.0f, &ButtonBox, &EntryBox);
        ButtonBox.VSplitRight(80.0f, &ButtonBox, &Button);
        static float s_ButtonDisc = 0;
        if (DoButton_Menu(&s_ButtonDisc, Localize("Disconnect"), 0, &Button))
            m_pClient->Irc()->EndConnection();

        float LW = (ButtonBox.w-ButtonBox.x)/m_pClient->Irc()->GetNumComs();
        static int s_ButsID[100];
        for (int i=0; i<m_pClient->Irc()->GetNumComs(); i++)
        {
            CIrcCom *pCom = m_pClient->Irc()->GetCom(i);

            if (pCom == m_pClient->Irc()->GetActiveCom())
                ButtonBox.VSplitLeft(LW-25.0f, &Button, &ButtonBox);
            else
            {
                ButtonBox.VSplitLeft(LW, &Button, &ButtonBox);
                Button.VSplitRight(2.0f, &Button, 0x0);
            }

            if (pCom->m_UnreadMsg)
            {
                ms_ColorTabbarActive = vec4(0.0f,1.0f,0.0f,1.0f);
                ms_ColorTabbarInactive = vec4(0.0f,1.0f,0.0f,1.0f);
            }
            else
            {
                if (pCom == m_pClient->Irc()->GetActiveCom())
                {
                    ms_ColorTabbarActive = vec4(0.35f,0.35f,0.35f,1.0f);
                    ms_ColorTabbarInactive = vec4(0.35f,0.35f,0.35f,1.0f);
                }
                else
                {
                    ms_ColorTabbarActive = vec4(0.0f,0.0f,0.0f,1.0f);
                    ms_ColorTabbarInactive = vec4(0.0f,0.0f,0.0f,1.0f);
                }
            }

            if (pCom->GetType() == CIrcCom::TYPE_CHANNEL)
            {
                CComChan *pChan = static_cast<CComChan*>(pCom);
                char aTab[255];
                if (pCom->m_UnreadMsg)
                    str_format(aTab, sizeof(aTab), "%s [%d]", pChan->m_Channel, pCom->m_NumUnreadMsg);
                else
                    str_copy(aTab, pChan->m_Channel, sizeof(aTab));

                if (DoButton_MenuTab(&s_ButsID[i], aTab, 0, &Button, 0))
                    m_pClient->Irc()->SetActiveCom(i);
            }
            else if (pCom->GetType() == CIrcCom::TYPE_QUERY)
            {
                CComQuery *pQuery = static_cast<CComQuery*>(pCom);
                char aTab[255];
                if (pCom->m_UnreadMsg)
                    str_format(aTab, sizeof(aTab), "%s [%d]", pQuery->m_User, pCom->m_NumUnreadMsg);
                else
                    str_copy(aTab, pQuery->m_User, sizeof(aTab));

                if (DoButton_MenuTab(&s_ButsID[i], aTab, 0, &Button, 0))
                    m_pClient->Irc()->SetActiveCom(i);
            }

            if (pCom == m_pClient->Irc()->GetActiveCom())
            {
                ButtonBox.VSplitLeft(25.0f, &Button, &ButtonBox);
                Button.VSplitRight(2.0f, &Button, 0x0);
                static int sCloseButton=0;
                if (DoButton_MenuTab(&sCloseButton, "X", 0, &Button, 0))
                    m_pClient->Irc()->Part();
            }
        }

        //Input Box
        EntryBox.HSplitBottom(20.0f, &EntryBox, &InputBox);
        InputBox.VSplitRight(50.0f, &InputBox, &Button);
        Button.VSplitLeft(5.0f, 0x0, &Button);
        static char EntryText[255];
        static float Offset;
        DoEditBox(&EntryText, &InputBox, EntryText, sizeof(EntryText), 12.0f, &Offset, false, CUI::CORNER_L);
        static float s_ButtonSend = 0;
        if (DoButton_Menu(&s_ButtonSend, Localize("Send"), 0, &Button) || Input()->KeyPressed(KEY_RETURN))
        {
            if (EntryText[0] == '/')
            {
                std::string aCmdRaw = EntryText;
                std::string aCmd = aCmdRaw;
                int del, ldel=0;
                del = aCmdRaw.find_first_of(" ");
                if (del != std::string::npos)
                    aCmd = aCmdRaw.substr(0, del);

                if (aCmd.compare("/join") == 0 || aCmd.compare("/j") == 0)
                {
                    if (del <= 0)
                        return;

                    std::string aCmdChan = aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                    m_pClient->Irc()->JoinTo(aCmdChan.c_str());
                }
                else if (aCmd.compare("/query") == 0 || aCmd.compare("/q") == 0)
                {
                    if (del <= 0)
                        return;

                    std::string aCmdUser = aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                    m_pClient->Irc()->OpenQuery(aCmdUser.c_str());
                }
                else if (aCmd.compare("/topic") == 0 || aCmd.compare("/t") == 0)
                {
                    if (del <= 0)
                        return;

                    std::string aCmdTopic = aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                    m_pClient->Irc()->SetTopic(aCmdTopic.c_str());
                }
                else if (aCmd.compare("/part") == 0 || aCmd.compare("/p") == 0)
                {
                    //std::string aCmdChan = aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                    m_pClient->Irc()->Part();
                }
                else if (aCmd.compare("/nick") == 0)
                {
                    if (del <= 0)
                        return;

                    std::string aCmdNewNick = aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                    m_pClient->Irc()->SetNick(aCmdNewNick.c_str());
                }
                else if (aCmd.compare("/op") == 0)
                {
                    if (del > 0)
                    {
                        std::string aCmdTo= aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                        m_pClient->Irc()->SetMode("+o", aCmdTo.c_str());
                    }
                    else
                        m_pClient->Irc()->SetMode("+o", 0x0);
                }
                else if (aCmd.compare("/deop") == 0)
                {
                    if (del > 0)
                    {
                        std::string aCmdTo= aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                        m_pClient->Irc()->SetMode("-o", aCmdTo.c_str());
                    }
                    else
                        m_pClient->Irc()->SetMode("-o", 0x0);
                }
                else if (aCmd.compare("/voz") == 0)
                {
                    if (del > 0)
                    {
                        std::string aCmdTo= aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                        m_pClient->Irc()->SetMode("+v", aCmdTo.c_str());
                    }
                    else
                        m_pClient->Irc()->SetMode("+v", 0x0);
                }
                else if (aCmd.compare("/devoz") == 0)
                {
                    if (del > 0)
                    {
                        std::string aCmdTo= aCmdRaw.substr(del+1, aCmdRaw.length()-del-1);
                        m_pClient->Irc()->SetMode("-v", aCmdTo.c_str());
                    }
                    else
                        m_pClient->Irc()->SetMode("-v", 0x0);
                }
                else
                    m_pClient->Irc()->SendRaw(EntryText);

                EntryText[0]=0;
                return;
            }

            m_pClient->Irc()->SendMsg(0x0, EntryText);
            EntryText[0]=0;
        }

        //Channel/Query
        CIrcCom *pCom = m_pClient->Irc()->GetActiveCom();
        if (!pCom)
            return;

        if (pCom->GetType() == CIrcCom::TYPE_CHANNEL)
        {
            CComChan *pChan = static_cast<CComChan*>(pCom);

            CUIRect Chat, UserList;
            EntryBox.Margin(5.0f, &EntryBox);
            EntryBox.VSplitRight(150.0f, &Chat, &UserList);

            static int Selected = 0;
            static int s_UsersList = 0;
            static float s_UsersScrollValue = 0;
            char aBuff[50];
            str_format(aBuff, sizeof(aBuff), "Total: %d", pChan->m_Users.size());
            UiDoListboxStart(&s_UsersList, &UserList, 18.0f, "Users", aBuff, pChan->m_Users.size(), 1, Selected, s_UsersScrollValue);

            int o=0;
            std::list<std::string>::iterator it = pChan->m_Users.begin();
            while (it != pChan->m_Users.end())
            {
                CListboxItem Item = UiDoListboxNextItem(&(*it));

                if(Item.m_Visible)
                {
                    UI()->DoLabelScaled(&Item.m_Rect, (*it).c_str(), 12.0f, -1);
                    if (Selected == o)
                    {
                        if(UI()->DoButtonLogic(&Item, "", Selected, &Item.m_Rect))
                        {
                            std::list<std::string>::iterator it = pChan->m_Users.begin();
                            std::advance(it, o);

                            m_pClient->Irc()->OpenQuery((*it).c_str());
                        }
                    }
                }

                o++; it++;
            }
            Selected = UiDoListboxEnd(&s_UsersScrollValue, 0);

            static int s_Chat = 0;
            static float s_ChatScrollValue = 100.0f;
            UiDoListboxStart(&s_Chat, &Chat, 12.0f, pChan->m_Topic.c_str(), "", pChan->m_Buffer.size(), 1, -1, s_ChatScrollValue);
            for(int i = 0; i < pChan->m_Buffer.size(); i++)
            {
                CListboxItem Item = UiDoListboxNextItem(&pChan->m_Buffer[i]);

                if(Item.m_Visible)
                    UI()->DoLabelScaled(&Item.m_Rect, pChan->m_Buffer[i].c_str(), 8.0f, -1);
            }
            UiDoListboxEnd(&s_ChatScrollValue, 0);
        }
        else if (pCom->GetType() == CIrcCom::TYPE_QUERY)
        {
            CComQuery *pQuery = static_cast<CComQuery*>(pCom);
            CUIRect Chat;
            EntryBox.Margin(5.0f, &Chat);

            static int s_Chat = 0;
            static float s_ChatScrollValue = 100.0f;
            UiDoListboxStart(&s_Chat, &Chat, 12.0f, pQuery->m_User, "", pQuery->m_Buffer.size(), 1, -1, s_ChatScrollValue);
            for(int i = 0; i < pQuery->m_Buffer.size(); i++)
            {
                CListboxItem Item = UiDoListboxNextItem(&pQuery->m_Buffer[i]);

                if(Item.m_Visible)
                    UI()->DoLabelScaled(&Item.m_Rect, pQuery->m_Buffer[i].c_str(), 8.0f, -1);
            }
            UiDoListboxEnd(&s_ChatScrollValue, 0);
        }
    }
}

void CMenus::OnInit()
{

	/*
	array<string> my_strings;
	array<string>::range r2;
	my_strings.add("4");
	my_strings.add("6");
	my_strings.add("1");
	my_strings.add("3");
	my_strings.add("7");
	my_strings.add("5");
	my_strings.add("2");

	for(array<string>::range r = my_strings.all(); !r.empty(); r.pop_front())
		dbg_msg("", "%s", r.front().cstr());

	sort(my_strings.all());

	dbg_msg("", "after:");
	for(array<string>::range r = my_strings.all(); !r.empty(); r.pop_front())
		dbg_msg("", "%s", r.front().cstr());


	array<int> myarray;
	myarray.add(4);
	myarray.add(6);
	myarray.add(1);
	myarray.add(3);
	myarray.add(7);
	myarray.add(5);
	myarray.add(2);

	for(array<int>::range r = myarray.all(); !r.empty(); r.pop_front())
		dbg_msg("", "%d", r.front());

	sort(myarray.all());
	sort_verify(myarray.all());

	dbg_msg("", "after:");
	for(array<int>::range r = myarray.all(); !r.empty(); r.pop_front())
		dbg_msg("", "%d", r.front());

	exit(-1);
	// */

	if(g_Config.m_ClShowWelcome)
		m_Popup = POPUP_LANGUAGE;
	g_Config.m_ClShowWelcome = 0;

	Console()->Chain("add_favorite", ConchainServerbrowserUpdate, this);
	Console()->Chain("remove_favorite", ConchainServerbrowserUpdate, this);
	Console()->Chain("add_friend", ConchainFriendlistUpdate, this);
	Console()->Chain("remove_friend", ConchainFriendlistUpdate, this);

	// setup load amount
	m_LoadCurrent = 0;
	m_LoadTotal = g_pData->m_NumImages;
	if(!g_Config.m_ClThreadsoundloading)
		m_LoadTotal += g_pData->m_NumSounds;
}

void CMenus::PopupMessage(const char *pTopic, const char *pBody, const char *pButton)
{
	// reset active item
	UI()->SetActiveItem(0);

	str_copy(m_aMessageTopic, pTopic, sizeof(m_aMessageTopic));
	str_copy(m_aMessageBody, pBody, sizeof(m_aMessageBody));
	str_copy(m_aMessageButton, pButton, sizeof(m_aMessageButton));
	m_Popup = POPUP_MESSAGE;
}


int CMenus::Render()
{
	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	static bool s_First = true;
	if(s_First)
	{
	    //H-Client
	    if (g_Config.m_UiPage == PAGE_SERVERS)
	    {
            if(g_Config.m_UiSubPage == SUBPAGE_INTERNET)
                ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);
            else if(g_Config.m_UiSubPage == SUBPAGE_LAN)
                ServerBrowser()->Refresh(IServerBrowser::TYPE_LAN);
            else if(g_Config.m_UiSubPage == SUBPAGE_FAVORITES)
                ServerBrowser()->Refresh(IServerBrowser::TYPE_FAVORITES);
	    }
	    //

		/*if(g_Config.m_UiPage == PAGE_INTERNET)
			ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);
		else if(g_Config.m_UiPage == PAGE_LAN)
			ServerBrowser()->Refresh(IServerBrowser::TYPE_LAN);
		else if(g_Config.m_UiPage == PAGE_FAVORITES)
			ServerBrowser()->Refresh(IServerBrowser::TYPE_FAVORITES);
		m_pClient->m_pSounds->Enqueue(CSounds::CHN_MUSIC, SOUND_MENU);*/
		s_First = false;
	}

	if(Client()->State() == IClient::STATE_ONLINE)
	{
		ms_ColorTabbarInactive = ms_ColorTabbarInactiveIngame;
		ms_ColorTabbarActive = ms_ColorTabbarActiveIngame;
	}
	else
	{
	    if (!Client()->BackgroundLoaded())
            RenderBackground();
		ms_ColorTabbarInactive = ms_ColorTabbarInactiveOutgame;
		ms_ColorTabbarActive = ms_ColorTabbarActiveOutgame;
	}

	CUIRect TabBar;
	CUIRect MainView;

	// some margin around the screen
	Screen.Margin(10.0f, &Screen);

	static bool s_SoundCheck = false;
	if(!s_SoundCheck && m_Popup == POPUP_NONE)
	{
		if(Client()->SoundInitFailed())
			m_Popup = POPUP_SOUNDERROR;
		s_SoundCheck = true;
	}

	if(m_Popup == POPUP_NONE)
	{
		// do tab bar
		Screen.HSplitTop(24.0f, &TabBar, &MainView);
		TabBar.VMargin(20.0f, &TabBar);
		RenderMenubar(TabBar);

		// news is not implemented yet
		if(g_Config.m_UiPage < PAGE_NEWS || g_Config.m_UiPage > PAGE_SETTINGS || (Client()->State() == IClient::STATE_OFFLINE && g_Config.m_UiPage >= PAGE_GAME && g_Config.m_UiPage <= PAGE_CALLVOTE))
		{
			ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);
			//g_Config.m_UiPage = PAGE_INTERNET;
			g_Config.m_UiPage = PAGE_SERVERS; // H-Client
		}

		// render current page
		if(Client()->State() != IClient::STATE_OFFLINE)
		{
			if(m_GamePage == PAGE_GAME)
				RenderGame(MainView);
			else if(m_GamePage == PAGE_PLAYERS)
				RenderPlayers(MainView);
			else if(m_GamePage == PAGE_SERVER_INFO)
				RenderServerInfo(MainView);
			else if(m_GamePage == PAGE_CALLVOTE)
				RenderServerControl(MainView);
			else if(m_GamePage == PAGE_SETTINGS)
				RenderSettings(MainView);
            else if(m_GamePage == PAGE_SERVERS) //H-Client
                RenderServerbrowser(MainView);
		}
		else if(g_Config.m_UiPage == PAGE_NEWS)
			RenderNews(MainView);
		else if(g_Config.m_UiPage == PAGE_SERVERS) //H-Client
            RenderServerbrowser(MainView);
		/*else if(g_Config.m_UiPage == PAGE_INTERNET)
			RenderServerbrowser(MainView);
		else if(g_Config.m_UiPage == PAGE_LAN)
			RenderServerbrowser(MainView);*/
		else if(g_Config.m_UiPage == PAGE_DEMOS)
			RenderDemoList(MainView);
		/*else if(g_Config.m_UiPage == PAGE_FAVORITES)
			RenderServerbrowser(MainView);*/
		else if(g_Config.m_UiPage == PAGE_SETTINGS)
			RenderSettings(MainView);
        else if (g_Config.m_UiPage == PAGE_LOCAL_SERVERS) //H-Client
            RenderServerManager(MainView);
	}
	else
	{
		// make sure that other windows doesn't do anything funnay!
		//UI()->SetHotItem(0);
		//UI()->SetActiveItem(0);
		char aBuf[128];
		const char *pTitle = "";
		const char *pExtraText = "";
		const char *pButtonText = "";
		int ExtraAlign = 0;

		if(m_Popup == POPUP_MESSAGE)
		{
			pTitle = m_aMessageTopic;
			pExtraText = m_aMessageBody;
			pButtonText = m_aMessageButton;
		}
		else if(m_Popup == POPUP_CONNECTING)
		{
			pTitle = Localize("Connecting to");
			pExtraText = g_Config.m_UiServerAddress; // TODO: query the client about the address
			pButtonText = Localize("Abort");
			if(Client()->MapDownloadTotalsize() > 0)
			{
				pTitle = Localize("Downloading map");
				pExtraText = "";
			}
		}
		else if(m_Popup == POPUP_DISCONNECTED)
		{
			pTitle = Localize("Disconnected");
			pExtraText = Client()->ErrorString();
			pButtonText = Localize("Ok");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_PURE)
		{
			pTitle = Localize("Disconnected");
			pExtraText = Localize("The server is running a non-standard tuning on a pure game type.");
			pButtonText = Localize("Ok");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_DELETE_DEMO)
		{
			pTitle = Localize("Delete demo");
			pExtraText = Localize("Are you sure that you want to delete the demo?");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_RENAME_DEMO)
		{
			pTitle = Localize("Rename demo");
			pExtraText = "";
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_REMOVE_FRIEND)
		{
			pTitle = Localize("Remove friend");
			pExtraText = Localize("Are you sure that you want to remove the player from your friends list?");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_SOUNDERROR)
		{
			pTitle = Localize("Sound error");
			pExtraText = Localize("The audio device couldn't be initialised.");
			pButtonText = Localize("Ok");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_PASSWORD)
		{
			pTitle = Localize("Password incorrect");
			pExtraText = "";
			pButtonText = Localize("Try again");
		}
		else if(m_Popup == POPUP_QUIT)
		{
			pTitle = Localize("Quit");
			pExtraText = Localize("Are you sure that you want to quit?");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_FIRST_LAUNCH)
		{
			pTitle = Localize("Welcome to Teeworlds");
			pExtraText = Localize("As this is the first time you launch the game, please enter your nick name below. It's recommended that you check the settings to adjust them to your liking before joining a server.");
			pButtonText = Localize("Ok");
			ExtraAlign = -1;
		}

		CUIRect Box, Part;
		Box = Screen;
		Box.VMargin(150.0f/UI()->Scale(), &Box);
		Box.HMargin(150.0f/UI()->Scale(), &Box);

		// render the box
		RenderTools()->DrawUIRect(&Box, vec4(0,0,0,0.5f), CUI::CORNER_ALL, 15.0f);

		Box.HSplitTop(30.f, &Part, &Box);
		RenderTools()->DrawUIRect(&Part, vec4(0,0,0,0.5f), CUI::CORNER_T, 15.0f);
		UI()->DoLabelScaled(&Part, pTitle, 24.f, 0);
		/*Box.HSplitTop(24.f/UI()->Scale(), &Part, &Box);
		UI()->DoLabelScaled(&Part, pTitle, 24.f, 0);*/
		Box.HSplitTop(20.f/UI()->Scale(), &Part, &Box);
		Box.HSplitTop(24.f/UI()->Scale(), &Part, &Box);
		Part.VMargin(20.f/UI()->Scale(), &Part);

		if(ExtraAlign == -1)
			UI()->DoLabelScaled(&Part, pExtraText, 20.f, -1, (int)Part.w);
		else
			UI()->DoLabelScaled(&Part, pExtraText, 20.f, 0, -1);

		if(m_Popup == POPUP_QUIT)
		{
			CUIRect Yes, No;
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			// additional info
			Box.HSplitTop(10.0f, 0, &Box);
			Box.VMargin(20.f/UI()->Scale(), &Box);
			if(m_pClient->Editor()->HasUnsavedData())
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "%s\n%s", Localize("There's an unsaved map in the editor, you might want to save it before you quit the game."), Localize("Quit anyway?"));
				UI()->DoLabelScaled(&Box, aBuf, 20.f, -1, Part.w-20.0f);
			}

			// buttons
			Part.VMargin(80.0f, &Part);
			Part.VSplitMid(&No, &Yes);
			Yes.VMargin(20.0f, &Yes);
			No.VMargin(20.0f, &No);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("No"), 0, &No) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Yes"), 0, &Yes) || m_EnterPressed)
				Client()->Quit();
		}
		else if(m_Popup == POPUP_PASSWORD)
		{
			CUIRect Label, TextBox, TryAgain, Abort, Save;

			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&Abort, &TryAgain);

			TryAgain.VMargin(20.0f, &TryAgain);
			Abort.VMargin(20.0f, &Abort);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("Abort"), 0, &Abort) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Try again"), 0, &TryAgain) || m_EnterPressed)
			{
				Client()->Connect(g_Config.m_UiServerAddress);
			}

			Box.HSplitBottom(80.f, &Box, &Part);
			Box.HSplitBottom(44.f, &Box, &Part);

			//H-Client
			Part.HSplitBottom(20.0f, &Part, &Save); //H-Client
			Save.VSplitLeft(180.0f, 0, &Save); //H-Client
            if(DoButton_CheckBox(&s_CheckSave, Localize("Remember Password"), s_CheckSave, &Save))
                s_CheckSave ^= 1;
			//

			Part.VSplitLeft(60.0f, 0, &Label);
			Label.VSplitLeft(100.0f, 0, &TextBox);
			TextBox.VSplitLeft(20.0f, 0, &TextBox);
			TextBox.VSplitRight(60.0f, &TextBox, 0);
			UI()->DoLabel(&Label, Localize("Password"), 18.0f, -1);
			static float Offset = 0.0f;
			DoEditBox(&g_Config.m_Password, &TextBox, g_Config.m_Password, sizeof(g_Config.m_Password), 12.0f, &Offset, true);
		}
		else if(m_Popup == POPUP_CONNECTING)
		{
			Box = Screen;
			Box.VMargin(150.0f, &Box);
			Box.HMargin(150.0f, &Box);
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(120.0f, &Part);

			static int s_Button = 0;
			if(DoButton_Menu(&s_Button, pButtonText, 0, &Part) || m_EscapePressed || m_EnterPressed)
			{
				Client()->Disconnect();
				m_Popup = POPUP_NONE;
			}

			if(Client()->MapDownloadTotalsize() > 0)
			{
				int64 Now = time_get();
				if(Now-m_DownloadLastCheckTime >= time_freq())
				{
					if(m_DownloadLastCheckSize > Client()->MapDownloadAmount())
					{
						// map downloaded restarted
						m_DownloadLastCheckSize = 0;
					}

					// update download speed
					float Diff = Client()->MapDownloadAmount()-m_DownloadLastCheckSize;
					float StartDiff = m_DownloadLastCheckSize-0.0f;
					if(StartDiff+Diff > 0.0f)
						m_DownloadSpeed = (Diff/(StartDiff+Diff))*(Diff/1.0f) + (StartDiff/(Diff+StartDiff))*m_DownloadSpeed;
					else
						m_DownloadSpeed = 0.0f;
					m_DownloadLastCheckTime = Now;
					m_DownloadLastCheckSize = Client()->MapDownloadAmount();
				}

				Box.HSplitTop(64.f, 0, &Box);
				Box.HSplitTop(24.f, &Part, &Box);
				str_format(aBuf, sizeof(aBuf), "%d/%d KiB (%.1f KiB/s)", Client()->MapDownloadAmount()/1024, Client()->MapDownloadTotalsize()/1024,	m_DownloadSpeed/1024.0f);
				UI()->DoLabel(&Part, aBuf, 20.f, 0, -1);

				// time left
				const char *pTimeLeftString;
				int TimeLeft = max(1, m_DownloadSpeed > 0.0f ? static_cast<int>((Client()->MapDownloadTotalsize()-Client()->MapDownloadAmount())/m_DownloadSpeed) : 1);
				if(TimeLeft >= 60)
				{
					TimeLeft /= 60;
					pTimeLeftString = TimeLeft == 1 ? Localize("%i minute left") : Localize("%i minutes left");
				}
				else
					pTimeLeftString = TimeLeft == 1 ? Localize("%i second left") : Localize("%i seconds left");
				Box.HSplitTop(20.f, 0, &Box);
				Box.HSplitTop(24.f, &Part, &Box);
				str_format(aBuf, sizeof(aBuf), pTimeLeftString, TimeLeft);
				UI()->DoLabel(&Part, aBuf, 20.f, 0, -1);

				// progress bar
				Box.HSplitTop(20.f, 0, &Box);
				Box.HSplitTop(24.f, &Part, &Box);
				Part.VMargin(40.0f, &Part);
				RenderTools()->DrawUIRect(&Part, vec4(1.0f, 1.0f, 1.0f, 0.25f), CUI::CORNER_ALL, 5.0f);

				//H-Client
				float SizeBlock = (Part.w-55)/10;
				float SizeVBlock = (Part.w-55)/9;
				int NumEBlocks = (((Part.w-55)*Client()->MapDownloadAmount())/Client()->MapDownloadTotalsize())/SizeVBlock;
				CUIRect PBlock;
				PBlock.w = SizeBlock;
				PBlock.h = Part.h-10;
				PBlock.y = Part.y+5;
				for (int i=0; i<10; i++)
				{
				    int CornerType = 0;
                    PBlock.x = Part.x+(i*SizeBlock)+((i+1)*5.0f);

				    if (i == 0)
                        CornerType = CUI::CORNER_L;
                    else if (i == 9)
                        CornerType = CUI::CORNER_R;

				    if (i <= NumEBlocks)
                        RenderTools()->DrawUIRect(&PBlock, vec4(1.0f, 1.0f, 1.0f, 0.5f), CornerType, 5.0f);
                    else
                        RenderTools()->DrawUIRect(&PBlock, vec4(0.85f, 0.85f, 0.85f, 0.2f), CornerType, 5.0f);
				}

                float Prog = (Client()->MapDownloadAmount()*100.0f)/Client()->MapDownloadTotalsize();
                str_format(aBuf, sizeof(aBuf), "%.2f%c", Prog, '%');
				UI()->DoLabel(&Part, aBuf, 20.f, 0, -1);
				//

				//Part.w = max(10.0f, (Part.w*Client()->MapDownloadAmount())/Client()->MapDownloadTotalsize());
				//RenderTools()->DrawUIRect(&Part, vec4(1.0f, 1.0f, 1.0f, 0.5f), CUI::CORNER_ALL, 5.0f);
			}
		}
		else if(m_Popup == POPUP_LANGUAGE)
		{
			Box = Screen;
			Box.VMargin(150.0f, &Box);
			Box.HMargin(150.0f, &Box);
			Box.HSplitTop(20.f, &Part, &Box);
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Box.HSplitBottom(20.f, &Box, 0);
			Box.VMargin(20.0f, &Box);
			RenderLanguageSelection(Box);
			Part.VMargin(120.0f, &Part);

			static int s_Button = 0;
			if(DoButton_Menu(&s_Button, Localize("Ok"), 0, &Part) || m_EscapePressed || m_EnterPressed)
				m_Popup = POPUP_FIRST_LAUNCH;
		}
		else if(m_Popup == POPUP_COUNTRY)
		{
			Box = Screen;
			Box.VMargin(150.0f, &Box);
			Box.HMargin(150.0f, &Box);
			Box.HSplitTop(20.f, &Part, &Box);
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Box.HSplitBottom(20.f, &Box, 0);
			Box.VMargin(20.0f, &Box);

			static int ActSelection = -2;
			if(ActSelection == -2)
				ActSelection = g_Config.m_BrFilterCountryIndex;
			static float s_ScrollValue = 0.0f;
			int OldSelected = -1;
			UiDoListboxStart(&s_ScrollValue, &Box, 50.0f, Localize("Country"), "", m_pClient->m_pCountryFlags->Num(), 6, OldSelected, s_ScrollValue);

			for(int i = 0; i < m_pClient->m_pCountryFlags->Num(); ++i)
			{
				const CCountryFlags::CCountryFlag *pEntry = m_pClient->m_pCountryFlags->GetByIndex(i);
				if(pEntry->m_CountryCode == ActSelection)
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
					Graphics()->TextureSet(pEntry->m_Texture);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, Item.m_Rect.w, Item.m_Rect.h);
					Graphics()->QuadsDrawTL(&QuadItem, 1);
					Graphics()->QuadsEnd();
					UI()->DoLabel(&Label, pEntry->m_aCountryCodeString, 10.0f, 0);
				}
			}

			const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
			if(OldSelected != NewSelected)
				ActSelection = m_pClient->m_pCountryFlags->GetByIndex(NewSelected)->m_CountryCode;

			Part.VMargin(120.0f, &Part);

			static int s_Button = 0;
			if(DoButton_Menu(&s_Button, Localize("Ok"), 0, &Part) || m_EnterPressed)
			{
				g_Config.m_BrFilterCountryIndex = ActSelection;
				Client()->ServerBrowserUpdate();
				m_Popup = POPUP_NONE;
			}

			if(m_EscapePressed)
			{
				ActSelection = g_Config.m_BrFilterCountryIndex;
				m_Popup = POPUP_NONE;
			}
		}
		else if(m_Popup == POPUP_DELETE_DEMO)
		{
			CUIRect Yes, No;
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&No, &Yes);

			Yes.VMargin(20.0f, &Yes);
			No.VMargin(20.0f, &No);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("No"), 0, &No) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Yes"), 0, &Yes) || m_EnterPressed)
			{
				m_Popup = POPUP_NONE;
				// delete demo
				if(m_DemolistSelectedIndex >= 0 && !m_DemolistSelectedIsDir)
				{
					char aBuf[512];
					str_format(aBuf, sizeof(aBuf), "%s/%s", m_aCurrentDemoFolder, m_lDemos[m_DemolistSelectedIndex].m_aFilename);
					if(Storage()->RemoveFile(aBuf, m_lDemos[m_DemolistSelectedIndex].m_StorageType))
					{
						DemolistPopulate();
						DemolistOnUpdate(false);
					}
					else
						PopupMessage(Localize("Error"), Localize("Unable to delete the demo"), Localize("Ok"));
				}
			}
		}
		else if(m_Popup == POPUP_RENAME_DEMO)
		{
			CUIRect Label, TextBox, Ok, Abort;

			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&Abort, &Ok);

			Ok.VMargin(20.0f, &Ok);
			Abort.VMargin(20.0f, &Abort);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("Abort"), 0, &Abort) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonOk = 0;
			if(DoButton_Menu(&s_ButtonOk, Localize("Ok"), 0, &Ok) || m_EnterPressed)
			{
				m_Popup = POPUP_NONE;
				// rename demo
				if(m_DemolistSelectedIndex >= 0 && !m_DemolistSelectedIsDir)
				{
					char aBufOld[512];
					str_format(aBufOld, sizeof(aBufOld), "%s/%s", m_aCurrentDemoFolder, m_lDemos[m_DemolistSelectedIndex].m_aFilename);
					int Length = str_length(m_aCurrentDemoFile);
					char aBufNew[512];
					if(Length <= 4 || m_aCurrentDemoFile[Length-5] != '.' || str_comp_nocase(m_aCurrentDemoFile+Length-4, "demo"))
						str_format(aBufNew, sizeof(aBufNew), "%s/%s.demo", m_aCurrentDemoFolder, m_aCurrentDemoFile);
					else
						str_format(aBufNew, sizeof(aBufNew), "%s/%s", m_aCurrentDemoFolder, m_aCurrentDemoFile);
					if(Storage()->RenameFile(aBufOld, aBufNew, m_lDemos[m_DemolistSelectedIndex].m_StorageType))
					{
						DemolistPopulate();
						DemolistOnUpdate(false);
					}
					else
						PopupMessage(Localize("Error"), Localize("Unable to rename the demo"), Localize("Ok"));
				}
			}

			Box.HSplitBottom(60.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			Part.VSplitLeft(60.0f, 0, &Label);
			Label.VSplitLeft(120.0f, 0, &TextBox);
			TextBox.VSplitLeft(20.0f, 0, &TextBox);
			TextBox.VSplitRight(60.0f, &TextBox, 0);
			UI()->DoLabel(&Label, Localize("New name:"), 18.0f, -1);
			static float Offset = 0.0f;
			DoEditBox(&Offset, &TextBox, m_aCurrentDemoFile, sizeof(m_aCurrentDemoFile), 12.0f, &Offset);
		}
		else if(m_Popup == POPUP_REMOVE_FRIEND)
		{
			CUIRect Yes, No;
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&No, &Yes);

			Yes.VMargin(20.0f, &Yes);
			No.VMargin(20.0f, &No);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("No"), 0, &No) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Yes"), 0, &Yes) || m_EnterPressed)
			{
				m_Popup = POPUP_NONE;
				// remove friend
				if(m_FriendlistSelectedIndex >= 0)
				{
					m_pClient->Friends()->RemoveFriend(m_lFriends[m_FriendlistSelectedIndex].m_pFriendInfo->m_aName,
						m_lFriends[m_FriendlistSelectedIndex].m_pFriendInfo->m_aClan);
					FriendlistOnUpdate();
					Client()->ServerBrowserUpdate();
				}
			}
		}
		else if(m_Popup == POPUP_FIRST_LAUNCH)
		{
			CUIRect Label, TextBox;

			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			static int s_EnterButton = 0;
			if(DoButton_Menu(&s_EnterButton, Localize("Enter"), 0, &Part) || m_EnterPressed)
				m_Popup = POPUP_NONE;

			Box.HSplitBottom(40.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			Part.VSplitLeft(60.0f, 0, &Label);
			Label.VSplitLeft(100.0f, 0, &TextBox);
			TextBox.VSplitLeft(20.0f, 0, &TextBox);
			TextBox.VSplitRight(60.0f, &TextBox, 0);
			UI()->DoLabel(&Label, Localize("Nickname"), 18.0f, -1);
			static float Offset = 0.0f;
			DoEditBox(&g_Config.m_PlayerName, &TextBox, g_Config.m_PlayerName, sizeof(g_Config.m_PlayerName), 12.0f, &Offset);
		}
		else
		{
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(120.0f, &Part);

			static int s_Button = 0;
			if(DoButton_Menu(&s_Button, pButtonText, 0, &Part) || m_EscapePressed || m_EnterPressed)
				m_Popup = POPUP_NONE;
		}
	}

	return 0;
}


void CMenus::SetActive(bool Active)
{
	m_MenuActive = Active;
	if(!m_MenuActive)
	{
		if(m_NeedSendinfo)
		{
			m_pClient->SendInfo(false);
			m_NeedSendinfo = false;
		}

		if(Client()->State() == IClient::STATE_ONLINE)
		{
			m_pClient->OnRelease();
		}
	}
	else if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		m_pClient->OnRelease();
	}
}

void CMenus::OnReset()
{
}

bool CMenus::OnMouseMove(float x, float y)
{
	m_LastInput = time_get();

	if(!m_MenuActive)
		return false;

	UI()->ConvertMouseMove(&x, &y);
	m_MousePos.x += x;
	m_MousePos.y += y;
	if(m_MousePos.x < 0) m_MousePos.x = 0;
	if(m_MousePos.y < 0) m_MousePos.y = 0;
	if(m_MousePos.x > Graphics()->ScreenWidth()) m_MousePos.x = Graphics()->ScreenWidth();
	if(m_MousePos.y > Graphics()->ScreenHeight()) m_MousePos.y = Graphics()->ScreenHeight();

	return true;
}

bool CMenus::OnInput(IInput::CEvent e)
{
	m_LastInput = time_get();

	// special handle esc and enter for popup purposes
	if(e.m_Flags&IInput::FLAG_PRESS)
	{
		if(e.m_Key == KEY_ESCAPE)
		{
			m_EscapePressed = true;
			SetActive(!IsActive());
			return true;
		}
	}

	if(IsActive())
	{
		if(e.m_Flags&IInput::FLAG_PRESS)
		{
			// special for popups
			if(e.m_Key == KEY_RETURN || e.m_Key == KEY_KP_ENTER)
				m_EnterPressed = true;
			else if(e.m_Key == KEY_DELETE)
				m_DeletePressed = true;
		}

		if(m_NumInputEvents < MAX_INPUTEVENTS)
			m_aInputEvents[m_NumInputEvents++] = e;
		return true;
	}
	return false;
}

void CMenus::OnStateChange(int NewState, int OldState)
{
	// reset active item
	UI()->SetActiveItem(0);

	if(NewState == IClient::STATE_OFFLINE)
	{
		if(OldState >= IClient::STATE_ONLINE)
			m_pClient->m_pSounds->Play(CSounds::CHN_MUSIC, SOUND_MENU, 1.0f, vec2(0, 0));
		m_Popup = POPUP_NONE;
		if(Client()->ErrorString() && Client()->ErrorString()[0] != 0)
		{
			if(str_find_nocase(Client()->ErrorString(), "password"))
			{
				m_Popup = POPUP_PASSWORD;
				UI()->SetHotItem(&g_Config.m_Password);
				UI()->SetActiveItem(&g_Config.m_Password);
			}
			else if(str_find(Client()->ErrorString(), "H-Client"))
                return;
			else
				m_Popup = POPUP_DISCONNECTED;
		}
	}
	else if(NewState == IClient::STATE_LOADING)
	{
		m_Popup = POPUP_CONNECTING;
		m_DownloadLastCheckTime = time_get();
		m_DownloadLastCheckSize = 0;
		m_DownloadSpeed = 0.0f;
		//client_serverinfo_request();
	}
	else if(NewState == IClient::STATE_CONNECTING)
		m_Popup = POPUP_CONNECTING;
	else if (NewState == IClient::STATE_ONLINE || NewState == IClient::STATE_DEMOPLAYBACK)
	{
	    char aBuf[125];
	    Client()->GetServerPassword(g_Config.m_UiServerAddress, aBuf, sizeof(aBuf));

	    if (NewState == IClient::STATE_ONLINE && s_CheckSave == 1 && str_comp(aBuf, g_Config.m_Password) != 0)
	    {
	        dbg_msg("H-Client", "Saveing Server Password");
            if (!Client()->SaveServerPassword(g_Config.m_UiServerAddress, g_Config.m_Password))
                dbg_msg("H-Client", "ERROR: Can't save server password! :S");
	    }

		m_Popup = POPUP_NONE;
		SetActive(false);
	}
}

extern "C" void font_debug_render();

void CMenus::OnRender()
{
	/*
	// text rendering test stuff
	render_background();

	CTextCursor cursor;
	TextRender()->SetCursor(&cursor, 10, 10, 20, TEXTFLAG_RENDER);
	TextRender()->TextEx(&cursor, "ようこそ - ガイド", -1);

	TextRender()->SetCursor(&cursor, 10, 30, 15, TEXTFLAG_RENDER);
	TextRender()->TextEx(&cursor, "ようこそ - ガイド", -1);

	//Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->QuadsDrawTL(60, 60, 5000, 5000);
	Graphics()->QuadsEnd();
	return;*/

	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		SetActive(true);

	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		CUIRect Screen = *UI()->Screen();
		Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
		RenderDemoPlayer(Screen);
	}

	if(Client()->State() == IClient::STATE_ONLINE && m_pClient->m_ServerMode == m_pClient->SERVERMODE_PUREMOD)
	{
		Client()->Disconnect();
		SetActive(true);
		m_Popup = POPUP_PURE;
	}

	if(!IsActive())
	{
		m_EscapePressed = false;
		m_EnterPressed = false;
		m_DeletePressed = false;
		m_NumInputEvents = 0;
		return;
	}

	// update colors
	vec3 Rgb = HslToRgb(vec3(g_Config.m_UiColorHue/255.0f, g_Config.m_UiColorSat/255.0f, g_Config.m_UiColorLht/255.0f));
	ms_GuiColor = vec4(Rgb.r, Rgb.g, Rgb.b, g_Config.m_UiColorAlpha/255.0f);

	ms_ColorTabbarInactiveOutgame = vec4(0,0,0,0.55f);
	ms_ColorTabbarActiveOutgame = vec4(0,0,0,0.85f);

	float ColorIngameScaleI = 0.5f;
	float ColorIngameAcaleA = 0.2f;
	ms_ColorTabbarInactiveIngame = vec4(
		ms_GuiColor.r*ColorIngameScaleI,
		ms_GuiColor.g*ColorIngameScaleI,
		ms_GuiColor.b*ColorIngameScaleI,
		ms_GuiColor.a*0.8f);

	ms_ColorTabbarActiveIngame = vec4(
		ms_GuiColor.r*ColorIngameAcaleA,
		ms_GuiColor.g*ColorIngameAcaleA,
		ms_GuiColor.b*ColorIngameAcaleA,
		ms_GuiColor.a);

	// update the ui
	CUIRect *pScreen = UI()->Screen();
	float mx = (m_MousePos.x/(float)Graphics()->ScreenWidth())*pScreen->w;
	float my = (m_MousePos.y/(float)Graphics()->ScreenHeight())*pScreen->h;

	int Buttons = 0;
	if(m_UseMouseButtons)
	{
		if(Input()->KeyPressed(KEY_MOUSE_1)) Buttons |= 1;
		if(Input()->KeyPressed(KEY_MOUSE_2)) Buttons |= 2;
		if(Input()->KeyPressed(KEY_MOUSE_3)) Buttons |= 4;
	}

	UI()->Update(mx,my,mx*3.0f,my*3.0f,Buttons);

	// render
	if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
		Render();

	// render cursor
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);
	IGraphics::CQuadItem QuadItem(mx, my, 24, 24);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	// render debug information
	if(g_Config.m_Debug)
	{
		CUIRect Screen = *UI()->Screen();
		Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "%p %p %p", UI()->HotItem(), UI()->ActiveItem(), UI()->LastActiveItem());
		CTextCursor Cursor;
		TextRender()->SetCursor(&Cursor, 10, 10, 10, TEXTFLAG_RENDER);
		TextRender()->TextEx(&Cursor, aBuf, -1);
	}

	m_EscapePressed = false;
	m_EnterPressed = false;
	m_DeletePressed = false;
	m_NumInputEvents = 0;
}

static int gs_TextureBlob = -1;

void CMenus::RenderBackground()
{
	//Graphics()->Clear(1,1,1);
	//render_sunrays(0,0);
	if(gs_TextureBlob == -1)
		gs_TextureBlob = Graphics()->LoadTexture("blob.png", IStorageTW::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);


	float sw = 300*Graphics()->ScreenAspect();
	float sh = 300;
	Graphics()->MapScreen(0, 0, sw, sh);

	// render background color
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
		//vec4 bottom(gui_color.r*0.3f, gui_color.g*0.3f, gui_color.b*0.3f, 1.0f);
		//vec4 bottom(0, 0, 0, 1.0f);
		vec4 Bottom(ms_GuiColor.r, ms_GuiColor.g, ms_GuiColor.b, 1.0f);
		vec4 Top(ms_GuiColor.r, ms_GuiColor.g, ms_GuiColor.b, 1.0f);
		IGraphics::CColorVertex Array[4] = {
			IGraphics::CColorVertex(0, Top.r, Top.g, Top.b, Top.a),
			IGraphics::CColorVertex(1, Top.r, Top.g, Top.b, Top.a),
			IGraphics::CColorVertex(2, Bottom.r, Bottom.g, Bottom.b, Bottom.a),
			IGraphics::CColorVertex(3, Bottom.r, Bottom.g, Bottom.b, Bottom.a)};
		Graphics()->SetColorVertex(Array, 4);
		IGraphics::CQuadItem QuadItem(0, 0, sw, sh);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	// render the tiles
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
		float Size = 15.0f;
		float OffsetTime = fmod(Client()->LocalTime()*0.15f, 2.0f);
		for(int y = -2; y < (int)(sw/Size); y++)
			for(int x = -2; x < (int)(sh/Size); x++)
			{
				Graphics()->SetColor(0,0,1,0.045f);
				IGraphics::CQuadItem QuadItem((x-OffsetTime)*Size*2+(y&1)*Size, (y+OffsetTime)*Size, Size, Size);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
			}
	Graphics()->QuadsEnd();

	// render border fade
	Graphics()->TextureSet(gs_TextureBlob);
	Graphics()->QuadsBegin();
		Graphics()->SetColor(0,0,0,0.5f);
		QuadItem = IGraphics::CQuadItem(-100, -100, sw+200, sh+200);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	// restore screen
	{CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);}
}

//H-Client
int CMenus::DeleteMapPreviewCacheCallback(const char *pName, int IsDir, int StorageType, void *pUser)
{
	CMenus *pSelf = (CMenus *)pUser;
	int Length = str_length(pName);
	if((pName[0] == '.' && (pName[1] == 0 ||
		(pName[1] == '.' && pName[2] == 0))) ||
		(!IsDir && (Length < 4 || str_comp(pName+Length-4, ".png"))))
		return 0;


    char aPath[512];
    str_format(aPath, sizeof(aPath), "mappreviews/%s", pName);
    dbg_msg("h-client", "Deleting: %s", aPath);

    pSelf->Storage()->RemoveFile(aPath, IStorageTW::TYPE_SAVE);
	return 0;
}

void CMenus::DeleteMapPreviewCache()
{
    dbg_msg("h-client", "Starting clear cache...");
	Storage()->ListDirectory(IStorageTW::TYPE_ALL, "mappreviews", DeleteMapPreviewCacheCallback, this);
}

void CMenus::RenderUpdating(const char *pCaption, int current, int total)
{
	// make sure that we don't render for each little thing we load
	// because that will slow down loading if we have vsync
	static int64 LastLoadRender = 0;
	if(time_get()-LastLoadRender < time_freq()/60)
		return;
	LastLoadRender = time_get();

	// need up date this here to get correct
	vec3 Rgb = HslToRgb(vec3(g_Config.m_UiColorHue/255.0f, g_Config.m_UiColorSat/255.0f, g_Config.m_UiColorLht/255.0f));
	ms_GuiColor = vec4(Rgb.r, Rgb.g, Rgb.b, g_Config.m_UiColorAlpha/255.0f);

	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	RenderBackground();

	float w = 700;
	float h = 200;
	float x = Screen.w/2-w/2;
	float y = Screen.h/2-h/2;

	Graphics()->BlendNormal();

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.50f);
	RenderTools()->DrawRoundRect(0, y, Screen.w, h, 0.0f);
	Graphics()->QuadsEnd();

	CUIRect r;
	r.x = x;
	r.y = y+20;
	r.w = w;
	r.h = h;
	UI()->DoLabel(&r, Localize(pCaption), 32.0f, 0, -1);

    if (total>0)
    {
        float Percent = current/(float)total;
        Graphics()->TextureSet(-1);
        Graphics()->QuadsBegin();
        Graphics()->SetColor(0.15f,0.15f,0.15f,0.75f);
        RenderTools()->DrawRoundRect(x+40, y+h-75, w-80, 30, 5.0f);
        Graphics()->SetColor(1,1,1,0.75f);
        RenderTools()->DrawRoundRect(x+45, y+h-70, (w-85)*Percent, 20, 5.0f);
        Graphics()->QuadsEnd();
    }

	Graphics()->Swap();
}
