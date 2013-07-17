/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_LAYERS_H
#define GAME_LAYERS_H

#include <engine/map.h>
#include <game/mapitems.h>
#include <base/math.h>
#include <base/vmath.h>
#include <game/generated/protocol.h>
#include <deque>

class CLayers
{
	int m_GroupsNum;
	int m_GroupsStart;
	int m_LayersNum;
	int m_LayersStart;
	CMapItemGroup *m_pGameGroup;
	CMapItemLayerTilemap *m_pGameLayer;
	CTile *m_pMineTeeOrigin; //H-Client
	CMapItemLayerTilemap *m_pFrontLayer; //H-Client: DDRace
	CMapItemLayerTilemap *m_pMineTeeLayer; //H-Client
	CMapItemLayerTilemap *m_pMineTeeBGLayer; //H-Client
	CMapItemLayerTilemap *m_pMineTeeFGLayer; //H-Client
	CMapItemLayerTilemap *m_pMineTeeLights; //H-Client
	CTile *m_pMineTeeLightsTiles; //H-Client
	class IMap *m_pMap;

public:
	CLayers();
	void Init(class IKernel *pKernel);
	int NumGroups() const { return m_GroupsNum; };
	class IMap *Map() const { return m_pMap; };
	CMapItemGroup *GameGroup() const { return m_pGameGroup; };
	CMapItemLayerTilemap *GameLayer() const { return m_pGameLayer; };
	CMapItemGroup *GetGroup(int Index) const;
	CMapItemLayer *GetLayer(int Index) const;

    //H-Client
    std::deque<CNetMsg_Sv_TileChangeExt> m_BuffNetTileChange;
    CMapItemLayerTilemap *Lights() const { return m_pMineTeeLights; };
    CTile *TileLights() const { return m_pMineTeeLightsTiles; };
    CMapItemLayerTilemap *MineTeeLayer() const { return m_pMineTeeLayer; };
    CMapItemLayerTilemap *MineTeeFGLayer() const { return m_pMineTeeFGLayer; };
    CMapItemLayerTilemap *MineTeeBGLayer() const { return m_pMineTeeBGLayer; };
    CTile *MineTeeOrigin() const { return m_pMineTeeOrigin; };
    CMapItemLayerTilemap *FrontLayer() const { return m_pFrontLayer; };
    int GetMineTeeIndex(vec2 Pos);
	int DestroyTile(vec2 Pos);
	void CreateTile(vec2 Pos, int ITile, bool Coll = true, int State = 0);
	//
};

#endif
