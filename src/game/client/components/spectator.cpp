/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h> // H-Client
#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/generated/protocol.h>

#include <game/client/animstate.h>
#include <game/client/render.h>

#include "spectator.h"


vec4 CSpectator::ms_ColorTabbarInactive = vec4(1.0f, 1.0f, 1.0f, 0.5f);
vec4 CSpectator::ms_ColorTabbarActive = vec4(0.15f, 0.15f, 0.15f, 0.5f);
IInput::CEvent CSpectator::m_aInputEvents[MAX_INPUTEVENTS];
int CSpectator::m_NumInputEvents;

void CSpectator::ConKeySpectator(IConsole::IResult *pResult, void *pUserData)
{
	CSpectator *pSelf = (CSpectator *)pUserData;
	if(pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active &&
		(pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK || pSelf->Client()->State() != IClient::STATE_WEBM || pSelf->DemoPlayer()->GetDemoType() == IDemoPlayer::DEMOTYPE_SERVER))
		pSelf->m_Active = pResult->GetInteger(0) != 0;
}

void CSpectator::ConSpectate(IConsole::IResult *pResult, void *pUserData)
{
	((CSpectator *)pUserData)->Spectate(pResult->GetInteger(0));
}

// H-Client: DDNet
void CSpectator::ConSpectateNext(IConsole::IResult *pResult, void *pUserData)
{
	CSpectator *pSelf = (CSpectator *)pUserData;
	int NewSpectatorID;
	bool GotNewSpectatorID = false;

	int CurPos = -1;
	for (int i = 0; i < MAX_CLIENTS; i++)
		if (pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i] && pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_ClientID == pSelf->m_pClient->m_Snap.m_SpecInfo.m_SpectatorID)
		CurPos = i;

	if(pSelf->m_pClient->m_Snap.m_SpecInfo.m_SpectatorID == SPEC_FREEVIEW)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!pSelf->m_pClient->m_Snap.m_paPlayerInfos[i] || pSelf->m_pClient->m_Snap.m_paPlayerInfos[i]->m_Team == TEAM_SPECTATORS)
				continue;

			NewSpectatorID = i;
			GotNewSpectatorID = true;
			break;
		}
	}
	else
	{
		for(int i = CurPos + 1; i < MAX_CLIENTS; i++)
		{
			if(!pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i] || pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_Team == TEAM_SPECTATORS)
				continue;

			NewSpectatorID = pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_ClientID;
			GotNewSpectatorID = true;
			break;
		}

		if(!GotNewSpectatorID)
		{
			for(int i = 0; i < CurPos; i++)
			{
				if(!pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i] || pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_Team == TEAM_SPECTATORS)
					continue;

				NewSpectatorID = pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_ClientID;
				GotNewSpectatorID = true;
				break;
			}
		}
	}
	if(GotNewSpectatorID)
		pSelf->Spectate(NewSpectatorID);
}

void CSpectator::ConSpectatePrevious(IConsole::IResult *pResult, void *pUserData)
{
	CSpectator *pSelf = (CSpectator *)pUserData;
	int NewSpectatorID;
	bool GotNewSpectatorID = false;

	int CurPos = -1;
	for (int i = 0; i < MAX_CLIENTS; i++)
		if (pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i] && pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_ClientID == pSelf->m_pClient->m_Snap.m_SpecInfo.m_SpectatorID)
		CurPos = i;

	if(pSelf->m_pClient->m_Snap.m_SpecInfo.m_SpectatorID == SPEC_FREEVIEW)
	{
		for(int i = MAX_CLIENTS -1; i > -1; i--)
		{
			if(!pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i] || pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_Team == TEAM_SPECTATORS)
				continue;

			NewSpectatorID = pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_ClientID;
			GotNewSpectatorID = true;
			break;
		}
	}
	else
	{
		for(int i = CurPos - 1; i > -1; i--)
		{
			if(!pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i] || pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_Team == TEAM_SPECTATORS)
				continue;

			NewSpectatorID = pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_ClientID;
			GotNewSpectatorID = true;
			break;
		}

		if(!GotNewSpectatorID)
		{
			for(int i = MAX_CLIENTS - 1; i > CurPos; i--)
			{
				if(!pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i] || pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_Team == TEAM_SPECTATORS)
					continue;

			NewSpectatorID = pSelf->m_pClient->m_Snap.m_paInfoByDDTeam[i]->m_ClientID;
			GotNewSpectatorID = true;
				break;
			}
		}
	}
	if(GotNewSpectatorID)
		pSelf->Spectate(NewSpectatorID);
}
//

CSpectator::CSpectator()
{
	OnReset();
}

void CSpectator::OnConsoleInit()
{
	Console()->Register("+spectate", "", CFGFLAG_CLIENT, ConKeySpectator, this, "Open spectator mode selector");
	Console()->Register("spectate", "i", CFGFLAG_CLIENT, ConSpectate, this, "Switch spectator mode");
	Console()->Register("spectate_next", "", CFGFLAG_CLIENT, ConSpectateNext, this, "Spectate the next player");
	Console()->Register("spectate_previous", "", CFGFLAG_CLIENT, ConSpectatePrevious, this, "Spectate the previous player");
}

bool CSpectator::OnMouseMove(float x, float y)
{
	if(!m_Active)
		return false;

	UI()->ConvertMouseMove(&x, &y);
	m_SelectorMouse += vec2(x,y);
	return true;
}


void CSpectator::OnRelease()
{
	OnReset();
}

void CSpectator::OnRender()
{
	if(!m_Active)
	{
		if(m_WasActive)
		{
			if(m_SelectedSpectatorID != NO_SELECTION)
            {
				Spectate(m_SelectedSpectatorID);
                m_ItemSelected = m_ItemCurrentSelected;
            }
			m_WasActive = false;
		}
		return;
	}

	if(!m_pClient->m_Snap.m_SpecInfo.m_Active)
	{
		m_Active = false;
		m_WasActive = false;
		return;
	}

	m_WasActive = true;
	m_SelectedSpectatorID = NO_SELECTION;

	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	// update the ui
	float mx = (m_SelectorMouse.x/(float)Graphics()->ScreenWidth())*Screen.w;
	float my = (m_SelectorMouse.y/(float)Graphics()->ScreenHeight())*Screen.h;
	int Buttons = 0;
    if(Input()->KeyPressed(KEY_MOUSE_1)) Buttons |= 1;
    if(Input()->KeyPressed(KEY_MOUSE_2)) Buttons |= 2;
    if(Input()->KeyPressed(KEY_MOUSE_3)) Buttons |= 4;
	UI()->Update(mx, my, mx*3.0f, my*3.0f, Buttons);

    CUIRect MainView;
    Screen.Margin(150.0f, &MainView);
    MainView.VMargin(200.0f, &MainView);

	// draw background
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);

	MainView.Margin(20.0f, &MainView);

	// clamp mouse position to selector area
	//m_SelectorMouse.x = clamp(m_SelectorMouse.x, MainView.x, MainView.x+MainView.w);
	//m_SelectorMouse.y = clamp(m_SelectorMouse.y, MainView.y, MainView.y+MainView.h);

	// draw selections
	float FontSize = 20.0f;
	bool Selected = false;

    static int bid = 0;
    CUIRect Button;
    MainView.HSplitTop(30.0f, &Button, &MainView);
    MainView.HSplitTop(5.0f, 0x0, &MainView);
	if(m_pClient->m_Snap.m_SpecInfo.m_SpectatorID == SPEC_FREEVIEW)
        RenderTools()->DrawUIRect(&Button, ms_ColorTabbarInactive*ButtonColorMul(&bid), CUI::CORNER_ALL, 10.0f);

	if(UI()->MouseInside(&Button))
	{
		m_SelectedSpectatorID = SPEC_FREEVIEW;
		Selected = true;
	}
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, Selected?1.0f:0.5f);
	TextRender()->Text(0, Button.x+5.0f, Button.y+Button.h/2 - FontSize/2 - 3.0f, FontSize, Localize("Free-View"), -1);

    TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

    // H-Client
    RenderClientsList(MainView);
    //

	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

	// draw cursor
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	IGraphics::CQuadItem QuadItem(mx, my, 24.0f, 24.0f);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	m_NumInputEvents = 0;
}

void CSpectator::OnReset()
{
	m_WasActive = false;
	m_Active = false;
	m_SelectedSpectatorID = NO_SELECTION;
	m_NumInputEvents = 0;

	m_ItemCurrentSelected = -1;
	m_ItemSelected = -1;
}

void CSpectator::Spectate(int SpectatorID)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK || Client()->State() == IClient::STATE_WEBM)
	{
		m_pClient->m_DemoSpecID = clamp(SpectatorID, (int)SPEC_FREEVIEW, MAX_CLIENTS-1);
		return;
	}

	if(m_pClient->m_Snap.m_SpecInfo.m_SpectatorID == SpectatorID)
		return;

	CNetMsg_Cl_SetSpectatorMode Msg;
	Msg.m_SpectatorID = SpectatorID;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CSpectator::RenderClientsList(CUIRect MainView)
{
	int NumOptions = 0;
	static int aPlayerIDs[MAX_CLIENTS];
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!m_pClient->m_Snap.m_paPlayerInfos[i] || m_pClient->m_Snap.m_paPlayerInfos[i]->m_Team == TEAM_SPECTATORS)
			continue;

		int Index = m_pClient->m_Snap.m_paPlayerInfos[i]->m_ClientID;
		aPlayerIDs[NumOptions++] = Index;
	}

	m_ItemCurrentSelected = -1;

	static int s_UsersList = 0;
	static float s_ScrollValue = 0;
	CUIRect List = MainView;
	UiDoListboxStart(&s_UsersList, &List, 24.0f, "", "", NumOptions, 1, m_ItemSelected, s_ScrollValue);

	for(int i = 0; i < NumOptions; i++)
	{
		CListboxItem Item = UiDoListboxNextItem(&aPlayerIDs[i]);

		if(Item.m_Visible)
		{
            if (m_ItemSelected != aPlayerIDs[i] && UI()->MouseInside(&Item.m_Rect))
            {
                RenderTools()->DrawUIRect(&Item.m_Rect, ms_ColorTabbarInactive, 0, 0.0f);
                m_SelectedSpectatorID = aPlayerIDs[i];
                m_ItemCurrentSelected = i;
            }

			CTeeRenderInfo Info = m_pClient->m_aClients[aPlayerIDs[i]].m_RenderInfo;
			Info.m_Size = Item.m_Rect.h;
			Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0), vec2(Item.m_Rect.x+Item.m_Rect.h/2, Item.m_Rect.y+Item.m_Rect.h/2));
			Item.m_Rect.x +=Info.m_Size;
			UI()->DoLabelScaled(&Item.m_Rect, m_pClient->m_aClients[aPlayerIDs[i]].m_aName, 16.0f, -1);
		}
	}

    UiDoListboxEnd(&s_ScrollValue, 0);
}

vec4 CSpectator::ButtonColorMul(const void *pID)
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

void CSpectator::UiDoListboxStart(const void *pID, const CUIRect *pRect, float RowHeight, const char *pTitle, const char *pBottomText, int NumItems,
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

CSpectator::CListboxItem CSpectator::UiDoListboxNextRow()
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

CSpectator::CListboxItem CSpectator::UiDoListboxNextItem(const void *pId, bool Selected)
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

			if (UI()->ActiveItem() == pId && Input()->MouseDoubleClick())
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

int CSpectator::UiDoListboxEnd(float *pScrollValue, bool *pItemActivated)
{
	UI()->ClipDisable();
	if(pScrollValue)
		*pScrollValue = gs_ListBoxScrollValue;
	if(pItemActivated)
		*pItemActivated = gs_ListBoxItemActivated;
	return gs_ListBoxNewSelected;
}

float CSpectator::DoScrollbarV(const void *pID, const CUIRect *pRect, float Current)
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

float CSpectator::DoScrollbarH(const void *pID, const CUIRect *pRect, float Current)
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
