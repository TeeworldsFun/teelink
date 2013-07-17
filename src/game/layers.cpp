/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <game/collision.h> //H-Client
#include <engine/serverbrowser.h> //H-Client
#include <game/client/gameclient.h> //H-Client
#include "layers.h"

CLayers::CLayers()
{
	m_GroupsNum = 0;
	m_GroupsStart = 0;
	m_LayersNum = 0;
	m_LayersStart = 0;
	m_pGameGroup = 0;
	m_pGameLayer = 0;
	m_pMineTeeLayer = 0;
	m_pMineTeeBGLayer = 0;
	m_pMineTeeFGLayer = 0;
	m_pMineTeeLights = 0;
	m_pMineTeeLightsTiles = 0;
	m_pFrontLayer = 0;
	m_pMap = 0;
	m_pMineTeeOrigin = 0x0;
}

void CLayers::Init(class IKernel *pKernel)
{
    m_BuffNetTileChange.clear(); //H-Client

    //TODO: Need be change to free memory
	m_pGameGroup = 0;
	m_pGameLayer = 0;
	m_pMineTeeLayer = 0; //H-Client
	m_pMineTeeBGLayer = 0;  //H-Client
	m_pMineTeeFGLayer = 0;  //H-Client
	m_pMineTeeLights = 0;  //H-Client
	m_pMineTeeLightsTiles = 0;  //H-Client
	m_pFrontLayer = 0;

	if (m_pMineTeeOrigin)
	{
	    mem_free(m_pMineTeeOrigin);
	    m_pMineTeeOrigin = 0x0;
	}

	m_pMap = pKernel->RequestInterface<IMap>();
	m_pMap->GetType(MAPITEMTYPE_GROUP, &m_GroupsStart, &m_GroupsNum);
	m_pMap->GetType(MAPITEMTYPE_LAYER, &m_LayersStart, &m_LayersNum);

	for(int g = 0; g < NumGroups(); g++)
	{
		CMapItemGroup *pGroup = GetGroup(g);

		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			CMapItemLayer *pLayer = GetLayer(pGroup->m_StartLayer+l);

			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);
                char layerName[128]={0};
                IntsToStr(pTilemap->m_aName, sizeof(pTilemap->m_aName)/sizeof(int), layerName);

				if (!m_pMineTeeLayer && str_comp_nocase(layerName, "mt-break") == 0)
				{
				    m_pMineTeeLayer = pTilemap;

                    //H-Client: Save copy in memory
                    unsigned int memSize = sizeof(CTile)*m_pMineTeeLayer->m_Width*m_pMineTeeLayer->m_Height;
					m_pMineTeeOrigin = static_cast<CTile*>(mem_alloc(memSize, 1));
					CTile *pMTiles = (CTile *)m_pMap->GetData(m_pMineTeeLayer->m_Data);
					mem_copy(m_pMineTeeOrigin, pMTiles, memSize);
				}
                else if(!m_pMineTeeLights && str_comp_nocase(layerName, "mt-light") == 0)
                {
                    m_pMineTeeLights = pTilemap;
                    if (m_pMineTeeLights)
                    {
                        CTile *pLTiles = (CTile *)m_pMap->GetData(m_pMineTeeLights->m_Data);
                        m_pMineTeeLightsTiles = pLTiles;
                    }
                }
                else if (!m_pMineTeeBGLayer && str_comp_nocase(layerName, "mt-bg") == 0)
                {
                    m_pMineTeeBGLayer = pTilemap;
                }
                else if (!m_pMineTeeFGLayer && str_comp_nocase(layerName, "mt-fg") == 0)
                {
                    m_pMineTeeFGLayer = pTilemap;
                }

				if(pTilemap->m_Flags&TILESLAYERFLAG_GAME)
				{
					m_pGameLayer = pTilemap;
					m_pGameGroup = pGroup;

					// make sure the game group has standard settings
					m_pGameGroup->m_OffsetX = 0;
					m_pGameGroup->m_OffsetY = 0;
					m_pGameGroup->m_ParallaxX = 100;
					m_pGameGroup->m_ParallaxY = 100;

					if(m_pGameGroup->m_Version >= 2)
					{
						m_pGameGroup->m_UseClipping = 0;
						m_pGameGroup->m_ClipX = 0;
						m_pGameGroup->m_ClipY = 0;
						m_pGameGroup->m_ClipW = 0;
						m_pGameGroup->m_ClipH = 0;
					}
					//break;
				}
				else if(pTilemap->m_Flags&TILESLAYERFLAG_FRONT)
				{
					if(pTilemap->m_Version <= 2)
						pTilemap->m_Front = *((int*)(pTilemap) + 17);

					m_pFrontLayer = pTilemap;
				}
			}
		}
	}
}

CMapItemGroup *CLayers::GetGroup(int Index) const
{
	return static_cast<CMapItemGroup *>(m_pMap->GetItem(m_GroupsStart+Index, 0, 0));
}

CMapItemLayer *CLayers::GetLayer(int Index) const
{
	return static_cast<CMapItemLayer *>(m_pMap->GetItem(m_LayersStart+Index, 0, 0));
}

int CLayers::DestroyTile(vec2 Pos)
{
    int ITile = 0;
    CMapItemLayerTilemap *pTMap = MineTeeLayer();
    CTile *pTiles = (CTile *)Map()->GetData(pTMap->m_Data);

    int Index = static_cast<int>(Pos.y*pTMap->m_Width+Pos.x);

    ITile = pTiles[Index].m_Index;
    pTiles[Index].m_Flags = 0x0;
    pTiles[Index].m_Index = 0;

    //Game Layer
    CMapItemLayerTilemap *pTMapGame = GameLayer();
    CTile *pTilesGame = (CTile *)Map()->GetData(pTMapGame->m_Data);

    Index = static_cast<int>(Pos.y*pTMapGame->m_Width+Pos.x);
    pTilesGame[Index].m_Flags = 0x0;
    pTilesGame[Index].m_Index = 0;

    return ITile;
}

int CLayers::GetMineTeeIndex(vec2 Pos)
{
    CMapItemLayerTilemap *pTMap = MineTeeLayer();
    CTile *pTiles = (CTile *)Map()->GetData(pTMap->m_Data);

    Pos.x = clamp((int)Pos.x/32, 0, (int)pTMap->m_Width-1);
    Pos.y = clamp((int)Pos.y/32, 0, (int)pTMap->m_Height-1);

    int Index = static_cast<int>(Pos.y*pTMap->m_Width+Pos.x);

    return pTiles[Index].m_Index;
}

void CLayers::CreateTile(vec2 Pos, int ITile, bool Coll, int State)
{
    if (!MineTeeLayer())
        return;

    CMapItemLayerTilemap *pTMap = 0x0;

    if (State == 0)
        pTMap = MineTeeLayer();
    else if (State == 1)
        pTMap = MineTeeBGLayer();
    else if (State == 2)
        pTMap = MineTeeFGLayer();

    if (pTMap)
    {
        CTile *pTiles = (CTile *)Map()->GetData(pTMap->m_Data);

        int Index = static_cast<int>(Pos.y*pTMap->m_Width+Pos.x);
        pTiles[Index].m_Flags = 0x0;
        pTiles[Index].m_Index = ITile;
    }

    //Game Layer
    CMapItemLayerTilemap *pTMapGame = GameLayer();
    CTile *pTilesGame = (CTile *)Map()->GetData(pTMapGame->m_Data);

    int Index = static_cast<int>(Pos.y*pTMapGame->m_Width+Pos.x);
    pTilesGame[Index].m_Flags = 0x0;

    if (!Coll)
        pTilesGame[Index].m_Index = 0;
    else
        pTilesGame[Index].m_Index = (!State)?TILE_SOLID:0;
}
