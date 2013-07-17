/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SERVER_ADMIN_H
#define GAME_CLIENT_COMPONENTS_SERVER_ADMIN_H
#include <base/vmath.h>

#include <game/client/component.h>
#include <game/client/ui.h>

class CServerAdmin : public CComponent
{
	static vec4 ms_ColorTabbarInactive;
	static vec4 ms_ColorTabbarActive;

    bool m_Active;
	bool m_WasActive;
	bool m_NeedSendinfo;
	int m_SelectedPlayer;

    bool m_EnterPressed;
	vec2 m_SelectorMouse;

	char m_strError[255];

	static void ConKeyServerAdmin(IConsole::IResult *pResult, void *pUserData);

	int DoButton_Tab(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Corners);
	int DoEditBox(void *pID, const CUIRect *pRect, char *pStr, unsigned StrSize, float FontSize, float *Offset, bool Hidden=false, int Corners=CUI::CORNER_ALL);
	int DoButton(const void *pID, const char *pText, int Checked, const CUIRect *pRect);
	float DoScrollbarV(const void *pID, const CUIRect *pRect, float Current);
	float DoScrollbarH(const void *pID, const CUIRect *pRect, float Current);

	enum { MAX_INPUTEVENTS = 32 };
	static IInput::CEvent m_aInputEvents[MAX_INPUTEVENTS];
	static int m_NumInputEvents;

	vec4 ButtonColorMul(const void *pID);

	struct CListboxItem
	{
		int m_Visible;
		int m_Selected;
		CUIRect m_Rect;
		CUIRect m_HitRect;
	};

	void UiDoListboxStart(const void *pID, const CUIRect *pRect, float RowHeight, const char *pTitle, const char *pBottomText, int NumItems,
						int ItemsPerRow, int SelectedIndex, float ScrollValue);
	CListboxItem UiDoListboxNextItem(const void *pID, bool Selected = false);
	CListboxItem UiDoListboxNextRow();
	int UiDoListboxEnd(float *pScrollValue, bool *pItemActivated);

	void RenderServerControlPanel(CUIRect MainView);
	void RenderClientsList(CUIRect MainView, bool FilterSpectators = false);
	void RenderNoAuth(CUIRect MainView);
	void RenderAdmin(CUIRect MainView);
	void RenderSettings(CUIRect MainView);
	void RenderTuning(CUIRect MainView);
public:
	CServerAdmin();

    bool IsActive() const { return m_Active; }
    void SetError(const char *strError) { str_copy(m_strError, strError, sizeof(m_strError)); }

	virtual void OnConsoleInit();
	virtual bool OnMouseMove(float x, float y);
	virtual bool OnInput(IInput::CEvent Event);
	virtual void OnRender();
	virtual void OnRelease();
	virtual void OnReset();
};

#endif
