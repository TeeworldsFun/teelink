/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_PLAYERS_H
#define GAME_CLIENT_COMPONENTS_PLAYERS_H
#include <game/client/component.h>
#include <game/client/animstate.h> //H-Client
#include <game/client/render.h> //H-Client
#include <list> //H-Client

class CPlayers : public CComponent
{
	void RenderHand(class CTeeRenderInfo *pInfo, vec2 CenterPos, vec2 Dir, float AngleOffset, vec2 PostRotOffset);
	void RenderPlayer(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPrevInfo,
		const CNetObj_PlayerInfo *pPlayerInfo
	);
	void RenderHook(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPrevInfo,
		const CNetObj_PlayerInfo *pPlayerInfo
	);

    //H-Client
    struct PLAYERSTATE
    {
        CAnimState m_State;
        CTeeRenderInfo m_RenderInfo;
        vec2 m_Direction;
        vec2 m_Position;
        int m_Emote;
    };

    std::list<PLAYERSTATE> m_BlurEffect;
    static void ConBackgroundPaint(IConsole::IResult *pResult, void *pUserData);
    static void ConForegroundPaint(IConsole::IResult *pResult, void *pUserData);
    static void ConDropItem(IConsole::IResult *pResult, void *pUserData); //H-Client
    void SendDropItem(int Index);
    int m_BGPaint;
    int m_FGPaint;
    //

public:
	virtual void OnRender();
	virtual void OnConsoleInit(); //H-Client
	virtual void OnInit(); //H-Client

	bool IsBGPaint() const { return (m_BGPaint); } //H-Client
	bool IsFGPaint() const { return (m_FGPaint); } //H-Client
};

#endif
