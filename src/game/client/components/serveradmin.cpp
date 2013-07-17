/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h>
#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/generated/protocol.h>

#include <game/client/animstate.h>
#include <game/client/render.h>
#include <game/client/lineinput.h>

#include "serveradmin.h"

#include <string.h>

vec4 CServerAdmin::ms_ColorTabbarInactive;
vec4 CServerAdmin::ms_ColorTabbarActive;

IInput::CEvent CServerAdmin::m_aInputEvents[MAX_INPUTEVENTS];
int CServerAdmin::m_NumInputEvents;

void CServerAdmin::ConKeyServerAdmin(IConsole::IResult *pResult, void *pUserData)
{
	CServerAdmin *pSelf = (CServerAdmin *)pUserData;

	if (pResult->GetInteger(0) != 0)
        pSelf->m_Active ^= 1;
}


CServerAdmin::CServerAdmin()
{
    ms_ColorTabbarInactive = vec4(1.0f, 1.0f, 1.0f, 0.5f);
    ms_ColorTabbarActive = vec4(0.15f, 0.15f, 0.15f, 0.5f);

	OnReset();
}

void CServerAdmin::OnReset()
{
	m_WasActive = false;
	m_Active = false;
	m_NeedSendinfo = false;
	m_NumInputEvents = 0;
	m_SelectedPlayer = -1;

	m_EnterPressed = false;
}

void CServerAdmin::OnConsoleInit()
{
	Console()->Register("+adminpanel", "", CFGFLAG_CLIENT, ConKeyServerAdmin, this, "Open administrator panel");
}

bool CServerAdmin::OnMouseMove(float x, float y)
{
	if(!m_Active)
		return false;

	UI()->ConvertMouseMove(&x, &y);
	m_SelectorMouse += vec2(x,y);

	if(m_SelectorMouse.x < 0) m_SelectorMouse.x = 0;
	if(m_SelectorMouse.y < 0) m_SelectorMouse.y = 0;
	if(m_SelectorMouse.x > Graphics()->ScreenWidth()) m_SelectorMouse.x = Graphics()->ScreenWidth();
	if(m_SelectorMouse.y > Graphics()->ScreenHeight()) m_SelectorMouse.y = Graphics()->ScreenHeight();

	return true;
}

void CServerAdmin::OnRelease()
{
	OnReset();
}

void CServerAdmin::OnRender()
{
    if(!m_Active )
		return;

	m_WasActive = true;

	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	// update the ui
	float mx = (m_SelectorMouse.x/(float)Graphics()->ScreenWidth())*Screen.w;
	float my = (m_SelectorMouse.y/(float)Graphics()->ScreenHeight())*Screen.h;
	int Buttons = 0;
    if(Input()->KeyPressed(KEY_MOUSE_1)) Buttons |= 1;
    if(Input()->KeyPressed(KEY_MOUSE_2)) Buttons |= 2;
    if(Input()->KeyPressed(KEY_MOUSE_3)) Buttons |= 4;
	UI()->Update(mx,my,mx*3.0f,my*3.0f,Buttons);


    CUIRect MainView;
    Screen.Margin(150.0f, &MainView);

    RenderServerControlPanel(MainView);

	// draw cursor
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	IGraphics::CQuadItem QuadItem(mx, my, 24.0f, 24.0f);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

    m_EnterPressed = false;
	m_NumInputEvents = 0;
}

bool CServerAdmin::OnInput(IInput::CEvent Event)
{
	if(!m_Active)
		return false;

    if(Event.m_Flags&IInput::FLAG_PRESS)
    {
        switch(Event.m_Key)
        {
            case KEY_ESCAPE:
                m_Active = false;
                m_pClient->OnRelease();
                break;

            case KEY_RETURN:
            case KEY_KP_ENTER:
                m_EnterPressed = true;
                break;
        }
    }

    if(m_NumInputEvents < MAX_INPUTEVENTS)
        m_aInputEvents[m_NumInputEvents++] = Event;

	return true;
}

int CServerAdmin::DoButton_Tab(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Corners)
{
	if(Checked)
		RenderTools()->DrawUIRect(pRect, ms_ColorTabbarActive, Corners, 10.0f);
	else
		RenderTools()->DrawUIRect(pRect, ms_ColorTabbarInactive, Corners, 10.0f);
	CUIRect Temp;
	pRect->HMargin(2.0f, &Temp);
	UI()->DoLabel(&Temp, pText, Temp.h*0.8f, 0);

	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

int CServerAdmin::DoEditBox(void *pID, const CUIRect *pRect, char *pStr, unsigned StrSize, float FontSize, float *Offset, bool Hidden, int Corners)
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
	RenderTools()->DrawUIRect(&Textbox, ms_ColorTabbarInactive, Corners, 3.0f);
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

int CServerAdmin::DoButton(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	RenderTools()->DrawUIRect(pRect, ms_ColorTabbarInactive*ButtonColorMul(pID), CUI::CORNER_ALL, 5.0f);
	CUIRect Temp;
	pRect->HMargin(pRect->h>=20.0f?2.0f:1.0f, &Temp);
	UI()->DoLabel(&Temp, pText, Temp.h*0.8f, 0);
	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

vec4 CServerAdmin::ButtonColorMul(const void *pID)
{
	if(UI()->ActiveItem() == pID)
		return ms_ColorTabbarInactive;
	else if(UI()->HotItem() == pID)
		return ms_ColorTabbarInactive;
	return vec4(1,1,1,1);
}

static CUIRect gs_ListBoxOriginalView;
static CUIRect gs_ListBoxView;
static float gs_ListBoxRowHeight;
static int gs_ListBoxItemIndex;
static int gs_ListBoxSelectedIndex;
static int gs_ListBoxNewSelected;
static int gs_ListBoxDoneEvents;
static int gs_ListBoxNumItems;
static int gs_ListBoxItemsPerRow;
static float gs_ListBoxScrollValue;
static bool gs_ListBoxItemActivated;

void CServerAdmin::UiDoListboxStart(const void *pID, const CUIRect *pRect, float RowHeight, const char *pTitle, const char *pBottomText, int NumItems,
								int ItemsPerRow, int SelectedIndex, float ScrollValue)
{
	CUIRect Scroll, Row;
	CUIRect View = *pRect;

	// background
	RenderTools()->DrawUIRect(&View, ms_ColorTabbarActive, 0, 0);

	// prepare the scroll
	View.VSplitRight(15, &View, &Scroll);

	// setup the variables
	gs_ListBoxOriginalView = View;
	gs_ListBoxSelectedIndex = SelectedIndex;
	gs_ListBoxNewSelected = SelectedIndex;
	gs_ListBoxItemIndex = 0;
	gs_ListBoxRowHeight = RowHeight;
	gs_ListBoxNumItems = NumItems;
	gs_ListBoxItemsPerRow = ItemsPerRow;
	gs_ListBoxDoneEvents = 0;
	gs_ListBoxScrollValue = ScrollValue;
	gs_ListBoxItemActivated = false;

	// do the scrollbar
	View.HSplitTop(gs_ListBoxRowHeight, &Row, 0);

	int NumViewable = (int)(gs_ListBoxOriginalView.h/Row.h) + 1;
	int Num = (NumItems+gs_ListBoxItemsPerRow-1)/gs_ListBoxItemsPerRow-NumViewable+1;
	if(Num < 0)
		Num = 0;
	if(Num > 0)
	{
		if(Input()->KeyPresses(KEY_MOUSE_WHEEL_UP) && UI()->MouseInside(&View))
			gs_ListBoxScrollValue -= 3.0f/Num;
		if(Input()->KeyPresses(KEY_MOUSE_WHEEL_DOWN) && UI()->MouseInside(&View))
			gs_ListBoxScrollValue += 3.0f/Num;

		if(gs_ListBoxScrollValue < 0.0f) gs_ListBoxScrollValue = 0.0f;
		if(gs_ListBoxScrollValue > 1.0f) gs_ListBoxScrollValue = 1.0f;
	}

	Scroll.HMargin(5.0f, &Scroll);
	gs_ListBoxScrollValue = DoScrollbarV(pID, &Scroll, gs_ListBoxScrollValue);

	// the list
	gs_ListBoxView = gs_ListBoxOriginalView;
	gs_ListBoxView.VMargin(5.0f, &gs_ListBoxView);
	UI()->ClipEnable(&gs_ListBoxView);
	gs_ListBoxView.y -= gs_ListBoxScrollValue*Num*Row.h;
}

CServerAdmin::CListboxItem CServerAdmin::UiDoListboxNextRow()
{
	static CUIRect s_RowView;
	CListboxItem Item = {0};
	if(gs_ListBoxItemIndex%gs_ListBoxItemsPerRow == 0)
		gs_ListBoxView.HSplitTop(gs_ListBoxRowHeight /*-2.0f*/, &s_RowView, &gs_ListBoxView);

	s_RowView.VSplitLeft(s_RowView.w/(gs_ListBoxItemsPerRow-gs_ListBoxItemIndex%gs_ListBoxItemsPerRow)/(UI()->Scale()), &Item.m_Rect, &s_RowView);

	Item.m_Visible = 1;
	//item.rect = row;

	Item.m_HitRect = Item.m_Rect;

	//CUIRect select_hit_box = item.rect;

	if(gs_ListBoxSelectedIndex == gs_ListBoxItemIndex)
		Item.m_Selected = 1;

	// make sure that only those in view can be selected
	if(Item.m_Rect.y+Item.m_Rect.h > gs_ListBoxOriginalView.y)
	{

		if(Item.m_HitRect.y < Item.m_HitRect.y) // clip the selection
		{
			Item.m_HitRect.h -= gs_ListBoxOriginalView.y-Item.m_HitRect.y;
			Item.m_HitRect.y = gs_ListBoxOriginalView.y;
		}

	}
	else
		Item.m_Visible = 0;

	// check if we need to do more
	if(Item.m_Rect.y > gs_ListBoxOriginalView.y+gs_ListBoxOriginalView.h)
		Item.m_Visible = 0;

	gs_ListBoxItemIndex++;
	return Item;
}

CServerAdmin::CListboxItem CServerAdmin::UiDoListboxNextItem(const void *pId, bool Selected)
{
	int ThisItemIndex = gs_ListBoxItemIndex;
	if(Selected)
	{
		if(gs_ListBoxSelectedIndex == gs_ListBoxNewSelected)
			gs_ListBoxNewSelected = ThisItemIndex;
		gs_ListBoxSelectedIndex = ThisItemIndex;
	}

	CListboxItem Item = UiDoListboxNextRow();

	if(Item.m_Visible && UI()->DoButtonLogic(pId, "", gs_ListBoxSelectedIndex == gs_ListBoxItemIndex, &Item.m_HitRect))
		gs_ListBoxNewSelected = ThisItemIndex;

	// process input, regard selected index
	if(gs_ListBoxSelectedIndex == ThisItemIndex)
	{
		if(!gs_ListBoxDoneEvents)
		{
			gs_ListBoxDoneEvents = 1;

			if(m_EnterPressed || (UI()->ActiveItem() == pId && Input()->MouseDoubleClick()))
			{
				gs_ListBoxItemActivated = true;
				UI()->SetActiveItem(0);
			}
			else
			{
				for(int i = 0; i < m_NumInputEvents; i++)
				{
					int NewIndex = -1;
					if(m_aInputEvents[i].m_Flags&IInput::FLAG_PRESS)
					{
						if(m_aInputEvents[i].m_Key == KEY_DOWN) NewIndex = gs_ListBoxNewSelected + 1;
						if(m_aInputEvents[i].m_Key == KEY_UP) NewIndex = gs_ListBoxNewSelected - 1;
					}
					if(NewIndex > -1 && NewIndex < gs_ListBoxNumItems)
					{
						// scroll
						float Offset = (NewIndex/gs_ListBoxItemsPerRow-gs_ListBoxNewSelected/gs_ListBoxItemsPerRow)*gs_ListBoxRowHeight;
						int Scroll = gs_ListBoxOriginalView.y > Item.m_Rect.y+Offset ? -1 :
										gs_ListBoxOriginalView.y+gs_ListBoxOriginalView.h < Item.m_Rect.y+Item.m_Rect.h+Offset ? 1 : 0;
						if(Scroll)
						{
							int NumViewable = (int)(gs_ListBoxOriginalView.h/gs_ListBoxRowHeight) + 1;
							int ScrollNum = (gs_ListBoxNumItems+gs_ListBoxItemsPerRow-1)/gs_ListBoxItemsPerRow-NumViewable+1;
							if(Scroll < 0)
							{
								int Num = (gs_ListBoxOriginalView.y-Item.m_Rect.y-Offset+gs_ListBoxRowHeight-1.0f)/gs_ListBoxRowHeight;
								gs_ListBoxScrollValue -= (1.0f/ScrollNum)*Num;
							}
							else
							{
								int Num = (Item.m_Rect.y+Item.m_Rect.h+Offset-(gs_ListBoxOriginalView.y+gs_ListBoxOriginalView.h)+gs_ListBoxRowHeight-1.0f)/
									gs_ListBoxRowHeight;
								gs_ListBoxScrollValue += (1.0f/ScrollNum)*Num;
							}
							if(gs_ListBoxScrollValue < 0.0f) gs_ListBoxScrollValue = 0.0f;
							if(gs_ListBoxScrollValue > 1.0f) gs_ListBoxScrollValue = 1.0f;
						}

						gs_ListBoxNewSelected = NewIndex;
					}
				}
			}
		}

		//selected_index = i;
		CUIRect r = Item.m_Rect;
		r.x -= 5.0f; r.h += 5.0f; //ugly.. but fix the problem
		r.Margin(1.5f, &r);
		RenderTools()->DrawUIRect(&r, ms_ColorTabbarInactive, CUI::CORNER_ALL, 4.0f);
	}

	return Item;
}

int CServerAdmin::UiDoListboxEnd(float *pScrollValue, bool *pItemActivated)
{
	UI()->ClipDisable();
	if(pScrollValue)
		*pScrollValue = gs_ListBoxScrollValue;
	if(pItemActivated)
		*pItemActivated = gs_ListBoxItemActivated;
	return gs_ListBoxNewSelected;
}

float CServerAdmin::DoScrollbarV(const void *pID, const CUIRect *pRect, float Current)
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

float CServerAdmin::DoScrollbarH(const void *pID, const CUIRect *pRect, float Current)
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

void CServerAdmin::RenderServerControlPanel(CUIRect MainView)
{
	static int s_SettingsPage = 0;

	// render background
	CUIRect TabBar, RestartWarning;
	MainView.HSplitBottom(15.0f, &MainView, &RestartWarning);
	MainView.VSplitRight(120.0f, &MainView, &TabBar);
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);

	MainView.HSplitTop(10.0f, 0, &MainView);

	CUIRect Button;

	const char *aTabs[] = {
		Localize("Admin"),
		Localize("Tuning")};

	int NumTabs = (int)(sizeof(aTabs)/sizeof(*aTabs));

	for(int i = 0; i < NumTabs; i++)
	{
		TabBar.HSplitTop((i==7?30:10), &Button, &TabBar);
		TabBar.HSplitTop(26, &Button, &TabBar);
        if (i != s_SettingsPage && !UI()->MouseInside(&Button))
            Button.VSplitRight(15, &Button, 0x0);
		if(DoButton_Tab(aTabs[i], aTabs[i], s_SettingsPage == i, &Button, CUI::CORNER_R))
			s_SettingsPage = i;
	}

	MainView.Margin(2.0f, &MainView);

	//Render Title
    CUIRect Title;
    MainView.HSplitTop(20.0f, &Title, &MainView);
    UI()->DoLabel(&Title, "SERVER CONTROL PANEL", Title.h*0.8f, 0);

    MainView.Margin(5.0f, &MainView);

    //Render Tabs
	if (Client()->RconAuthed())
	{
        if(s_SettingsPage == 0)
            RenderAdmin(MainView);
        else if(s_SettingsPage == 1)
            RenderTuning(MainView);
	}
	else
        RenderNoAuth(MainView);
}


void CServerAdmin::RenderClientsList(CUIRect MainView, bool FilterSpectators)
{
	int NumOptions = 0;
	int Selected = -1;
	static int aPlayerIDs[MAX_CLIENTS];
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_pClient->m_Snap.m_paInfoByTeam[i])
			continue;

		int Index = m_pClient->m_Snap.m_paInfoByTeam[i]->m_ClientID;
		if(Index == m_pClient->m_Snap.m_LocalClientID || (FilterSpectators && m_pClient->m_Snap.m_paInfoByTeam[i]->m_Team == TEAM_SPECTATORS))
			continue;
		if(m_SelectedPlayer == Index)
			Selected = NumOptions;
		aPlayerIDs[NumOptions++] = Index;
	}

	static int s_VoteList = 0;
	static float s_ScrollValue = 0;
	CUIRect List = MainView;
	UiDoListboxStart(&s_VoteList, &List, 24.0f, "", "", NumOptions, 1, Selected, s_ScrollValue);

	for(int i = 0; i < NumOptions; i++)
	{
		CListboxItem Item = UiDoListboxNextItem(&aPlayerIDs[i]);

		if(Item.m_Visible)
		{
			CTeeRenderInfo Info = m_pClient->m_aClients[aPlayerIDs[i]].m_RenderInfo;
			Info.m_Size = Item.m_Rect.h;
			Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0), vec2(Item.m_Rect.x+Item.m_Rect.h/2, Item.m_Rect.y+Item.m_Rect.h/2));
			Item.m_Rect.x +=Info.m_Size;
			UI()->DoLabelScaled(&Item.m_Rect, m_pClient->m_aClients[aPlayerIDs[i]].m_aName, 16.0f, -1);
		}
	}

	Selected = UiDoListboxEnd(&s_ScrollValue, 0);
	m_SelectedPlayer = Selected != -1 ? aPlayerIDs[Selected] : -1;
}

void CServerAdmin::RenderNoAuth(CUIRect MainView)
{
    static char rconPassword[64];
    CUIRect Item, Area;
    MainView.Margin(60.0f, &Area);
    static float s_OffsetAuthPass = 0.0f;

    Area.HSplitTop(25.0f, &Item, &Area);
    UI()->DoLabel(&Item, "RCON Password:", Item.h*0.8f, -1);
    Area.HSplitTop(22.0f, &Item, &Area);
    if(DoEditBox(rconPassword, &Item, rconPassword, sizeof(rconPassword), 14.0f, &s_OffsetAuthPass, true))
        m_NeedSendinfo = true;
    Area.HSplitTop(2.0f, 0, &Area);
    Area.HSplitTop(20.0f, &Item, &Area);
    Item.VSplitRight(150.0f, &Item, 0x0);
    static int s_ButtonRconAuth = 0;
    if (DoButton((void*)&s_ButtonRconAuth, Localize("Accept"), 0, &Item) || m_EnterPressed)
        Client()->RconAuth("", rconPassword);

    if (m_strError[0] != '\0')
    {
        Area.HSplitTop(35.0f, 0x0, &Item);
        TextRender()->TextColor(1.0f, 0.0f, 0.0f, 1.0f);
        UI()->DoLabel(&Item, m_strError, Item.h*0.8f, 0);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void CServerAdmin::RenderAdmin(CUIRect MainView)
{

    CUIRect Area, Item, Button;
    Area = MainView;

    //Render Foot Options
    Area.HSplitBottom(30.0f, &Area, &Item);
    RenderTools()->DrawUIRect(&Item, vec4(0.0f,0.0f,0.0f,0.15f), CUI::CORNER_ALL, 5.0f);
    Item.Margin(5.0f, &Item);
    Item.VSplitLeft(Item.w/3, &Button, &Item);
    Button.Margin(2.0f, &Button);
    static int s_ButtonRestart = 0;
    if (DoButton((void*)&s_ButtonRestart, Localize("Restart"), 0, &Button))
        Client()->Rcon("restart");
    Item.VSplitMid(&Button, &Item);
    Button.Margin(2.0f, &Button);
    static int s_ButtonReload = 0;
    if (DoButton((void*)&s_ButtonReload, Localize("Reload"), 0, &Button))
        Client()->Rcon("reload");
    Item.Margin(2.0f, &Item);
    static int s_ButtonShutdown = 0;
    if (DoButton((void*)&s_ButtonShutdown, Localize("Shutdown"), 0, &Item))
        Client()->Rcon("shutdown");

    //Render Client list
    Area.HSplitTop(130.0f, &Item, &Area);
    RenderTools()->DrawUIRect(&Item, vec4(0.0f,0.0f,0.0f,0.15f), CUI::CORNER_ALL, 5.0f);
    Item.Margin(5.0f, &Item);
    CUIRect List, Options;
    Item.VSplitRight(70.0f, &List, &Options);
    RenderClientsList(List);
    //Render Options
    Options.VSplitLeft(5.0f, 0x0, &Options);
    Options.HSplitTop(20.0f, &Button, &Options);
    static int s_ButtonKick = 0;
    if (DoButton((void*)&s_ButtonKick, Localize("Kick"), 0, &Button))
    {
        char aBuf[64];
        str_format(aBuf, sizeof(aBuf), "kick %d \"Kicked by H-Client Server Control Panel\"", m_SelectedPlayer);
        Client()->Rcon(aBuf);
    }
    Options.HSplitTop(5.0f, 0x0, &Options);
    Options.HSplitTop(20.0f, &Button, &Options);
    static int s_ButtonBan = 0;
    if (DoButton((void*)&s_ButtonBan, Localize("Ban"), 0, &Button))
    {
        char aBuf[64];
        str_format(aBuf, sizeof(aBuf), "ban %d \"Banned by H-Client Server Control Panel\"", m_SelectedPlayer);
        Client()->Rcon(aBuf);
    }
    Options.HSplitTop(5.0f, 0x0, &Options);
    Options.HSplitTop(20.0f, &Button, &Options);
    static int s_ButtonSpectate = 0;
    if (DoButton((void*)&s_ButtonSpectate, Localize("Spectate"), 0, &Button))
    {
        char aBuf[64];
        str_format(aBuf, sizeof(aBuf), "force_vote spectate %d", m_SelectedPlayer);
        Client()->Rcon(aBuf);
    }
    Options.HSplitTop(5.0f, 0x0, &Options);
    Options.HSplitTop(20.0f, &Button, &Options);
    static int s_ButtonMute = 0;
    if (DoButton((void*)&s_ButtonMute, Localize("Mute"), 0, &Button))
    {
        char aBuf[64];
        str_format(aBuf, sizeof(aBuf), "muteid %d 0", m_SelectedPlayer);
        Client()->Rcon(aBuf);
    }


    //Render Say Admin
    Area.HSplitTop(10.0f, 0x0, &Area);
    Area.HSplitTop(30.0f, &Item, &Area);
    RenderTools()->DrawUIRect(&Item, vec4(0.0f,0.0f,0.0f,0.15f), CUI::CORNER_ALL, 5.0f);
    Item.Margin(5.0f, &Item);
    Item.VSplitRight(60.0f, &Item, &Button);
    static char s_MessageAdmin[255];
    static float s_OffsetMessageAdmin = 0.0f;
    DoEditBox(s_MessageAdmin, &Item, s_MessageAdmin, sizeof(s_MessageAdmin), 14.0f, &s_OffsetMessageAdmin, false);
    static int s_ButtonSendMessageAdmin = 0;
    Button.VSplitLeft(4.0f, 0x0, &Button);
    if (DoButton((void*)&s_ButtonSendMessageAdmin, Localize("Say"), 0, &Button))
    {
        char aBuf[64];
        str_format(aBuf, sizeof(aBuf), "say %s", s_MessageAdmin);
        Client()->Rcon(aBuf);
        memset(s_MessageAdmin, 0, sizeof(s_MessageAdmin));
    }
}

void CServerAdmin::RenderTuning(CUIRect MainView)
{
    CUIRect Area, Item, Button;
    Area = MainView;

    static char s_TuningValue[64];
    static float s_OffsetTuningValue = 0;
    static int s_OptionTuningSelected = 0;
    char aBuf[100];

    //draw sections
    static int s_TuneSection = 0;
    Area.HSplitTop(30.0f, &Item, &Area);
    RenderTools()->DrawUIRect(&Item, vec4(0.0f,0.0f,0.0f,0.15f), CUI::CORNER_T, 5.0f);
    RenderTools()->DrawUIRect(&Area, vec4(0.0f,0.0f,0.0f,0.1f), CUI::CORNER_B, 5.0f);
    Item.Margin(5.0f, &Item);
    Item.VSplitMid(&Item, &Button);
    static int s_ButtonTuneSectionP = 0;
    if (DoButton_Tab((void*)&s_ButtonTuneSectionP, Localize("Physics"), (s_TuneSection == 0)?1:0, &Item, CUI::CORNER_L))
    {
        s_TuneSection = 0;
        s_OptionTuningSelected = 0;
    }
    static int s_ButtonTuneSectionW = 0;
    if (DoButton_Tab((void*)&s_ButtonTuneSectionW, Localize("Weapons"), (s_TuneSection == 0)?0:1, &Button, CUI::CORNER_R))
    {
        s_TuneSection = 1;
        s_OptionTuningSelected = 0;
    }

    //draw physics section
    if (s_TuneSection == 0)
    {
        const char *paOptions[][2] = {
            { "Ground ctrl speed", "10.0" },
            { "Ground ctrl accel", "2.0" },
            { "Ground friction", "0.5" },
            { "Ground jump imp.", "13.2" },
            { "Air jump impulse", "12.0" },
            { "Air control speed", "5.0" },
            { "Air control accel", "1.5" },
            { "Air friction", "0.95" },
            { "Hook length", "380.0" },
            { "Hook fire speed", "80.0" },
            { "Hook drag accel", "3.0" },
            { "Hook drag speed", "15.0" },
            { "Gravity", "0.5" },
            { "Velramp start", "550.0" },
            { "Velramp range", "2000.0" },
            { "Velramp curvature", "1.4" },
            { "Player collision", "1" },
            { "Player hooking", "1" }
        };
        const char *paParams[] = {
            "ground_control_speed",
            "ground_control_accel",
            "ground_friction",
            "ground_jump_impulse",
            "air_jump_impulse",
            "air_control_speed",
            "air_control_accel",
            "air_friction",
            "hook_length",
            "hook_fire_speed",
            "hook_drag_accel",
            "hook_drag_speed",
            "gravity",
            "velramp_start",
            "velramp_range",
            "velramp_curvature",
            "player_collision",
            "player_hooking",
        };

        //draw bottom
        Area.HSplitBottom(30.0f, &Area, &Item);
        Item.Margin(5.0f, &Item);
        Item.VSplitLeft(55.0f, &Button, &Item);
        UI()->DoLabel(&Button, "Value:", Button.h*0.8f, -1);
        Item.VSplitRight(70.0f, &Item, &Button);
        DoEditBox(s_TuningValue, &Item, s_TuningValue, sizeof(s_TuningValue), 14.0f, &s_OffsetTuningValue, false);
        static int s_ButtonApplyTuning = 0;
        Button.VMargin(5.0f, &Button);
        if (DoButton((void*)&s_ButtonApplyTuning, Localize("Apply"), 0, &Button))
        {
            char aBuf[64];
            str_format(aBuf, sizeof(aBuf), "tune %s %s", paParams[s_OptionTuningSelected], s_TuningValue);
            Client()->Rcon(aBuf);
            memset(s_TuningValue, 0, sizeof(s_TuningValue));
        }

        //draw options
        Area.Margin(5.0f, &Area);
        Area.HMargin(10.0f, &Area);

        static int s_aTuningIDs[18] = {0};

        float sizeTemp = 0.0f;
        for (int o=0; o<18; o++)
        {
            if (o%3 == 0)
            {
                Area.HSplitTop(20.0f, &Item, &Area);
                sizeTemp = Item.w/3;
            }

            Item.VSplitLeft(sizeTemp, &Button, &Item);
            str_format(aBuf, sizeof(aBuf), "%s", paOptions[o][0]);
            if (DoButton_Tab((void*)&s_aTuningIDs[o], aBuf, (o==s_OptionTuningSelected)?1:0, &Button, 0))
                s_OptionTuningSelected = o;
        }

        //draw info
        Area.HSplitTop(10.0f, 0x0,&Area);
        Area.HSplitTop(15.0f, &Item, &Area);
        TextRender()->TextColor(181.0f/255.0f, 230.0f/255.0f, 29.0f/255.0f, 1.0f);
        str_format(aBuf, sizeof(aBuf), "Default Value: %s", paOptions[s_OptionTuningSelected][1]);
        UI()->DoLabel(&Item, aBuf, Item.h*0.8f, 0);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
    //draw weapons section
    else if (s_TuneSection == 1)
    {
        const char *paOptions[][2] = {
            { "Gun curvature", "1.25" },
            { "Gun speed", "2200.0" },
            { "Gun lifetime", "2.0" },
            { "Shotgun curvature", "1.25" },
            { "Shotgun speed", "2750" },
            { "Shotgun speeddiff", "0.8" },
            { "Shotgun lifetime", "0.20" },
            { "Grenade curvature", "7.0" },
            { "Grenade speed", "1000.0" },
            { "Grenade lifetime", "2.0" },
            { "Laser reach", "800.0" },
            { "Laser bounce delay", "150.0" },
            { "Laser bounce num", "1.0" },
            { "Laser bounce cost", "0.0" },
            { "Laser damage", "5.0" },
        };
        const char *paParams[] = {
            "gun_curvature",
            "gun_speed",
            "gun_lifetime",
            "shotgun_curvature",
            "shotgun_speed",
            "shotgun_speeddiff",
            "shotgun_lifetime",
            "grenade_curvature",
            "grenade_speed",
            "grenade_lifetime",
            "laser_reach",
            "laser_bounce_delay",
            "laser_bounce_num",
            "laser_bounce_cost",
            "laser_damage",
        };

        //draw bottom
        Area.HSplitBottom(30.0f, &Area, &Item);
        Item.Margin(5.0f, &Item);
        Item.VSplitLeft(55.0f, &Button, &Item);
        UI()->DoLabel(&Button, "Value:", Button.h*0.8f, -1);
        Item.VSplitRight(70.0f, &Item, &Button);
        DoEditBox(s_TuningValue, &Item, s_TuningValue, sizeof(s_TuningValue), 14.0f, &s_OffsetTuningValue, false);
        static int s_ButtonApplyTuning = 0;
        Button.VMargin(5.0f, &Button);
        if (DoButton((void*)&s_ButtonApplyTuning, Localize("Apply"), 0, &Button))
        {
            char aBuf[64];
            str_format(aBuf, sizeof(aBuf), "tune %s %s", paParams[s_OptionTuningSelected], s_TuningValue);
            Client()->Rcon(aBuf);
            memset(s_TuningValue, 0, sizeof(s_TuningValue));
        }

        //draw options
        Area.Margin(5.0f, &Area);
        Area.HMargin(10.0f, &Area);

        static int s_aTuningIDs[15] = {0};

        float sizeTemp = 0.0f;
        for (int o=0; o<15; o++)
        {
            if (o%3 == 0)
            {
                Area.HSplitTop(20.0f, &Item, &Area);
                sizeTemp = Item.w/3;
            }

            Item.VSplitLeft(sizeTemp, &Button, &Item);
            str_format(aBuf, sizeof(aBuf), "%s", paOptions[o][0]);
            if (DoButton_Tab((void*)&s_aTuningIDs[o], aBuf, (o==s_OptionTuningSelected)?1:0, &Button, 0))
                s_OptionTuningSelected = o;
        }

        //draw info
        Area.HSplitTop(10.0f, 0x0,&Area);
        Area.HSplitTop(15.0f, &Item, &Area);
        TextRender()->TextColor(181.0f/255.0f, 230.0f/255.0f, 29.0f/255.0f, 1.0f);
        str_format(aBuf, sizeof(aBuf), "Default Value: %s", paOptions[s_OptionTuningSelected][1]);
        UI()->DoLabel(&Item, aBuf, Item.h*0.8f, 0);
        TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
}
