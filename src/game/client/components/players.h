/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_PLAYERS_H
#define GAME_CLIENT_COMPONENTS_PLAYERS_H
#include <game/client/component.h>

class CPlayers : public CComponent
{
	enum CPlayerFlags
	{
		PLAYERFLAG_ANTIPING = 1
	};

	CTeeRenderInfo m_RenderFreezeInfo; // H-Client
	CTeeRenderInfo m_aRenderInfo[MAX_CLIENTS];
	void RenderHand(class CTeeRenderInfo *pInfo, const vec2 &CenterPos, vec2 Dir, float AngleOffset, const vec2 &PostRotOffset);
	void RenderPlayer(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPrevInfo,
		const CNetObj_PlayerInfo *pPlayerInfo,
		int flags = 0
	);
	void RenderHook(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPrevInfo,
		const CNetObj_PlayerInfo *pPlayerInfo
	);

public:
	virtual void OnRender();
	virtual void OnMapLoad();
};

#endif
