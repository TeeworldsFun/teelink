#include "cem.h"
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/entities/character.h>
#include <game/server/entities/pickup.h>
#include <game/server/gamecontext.h>

#include <cstring>

CGameControllerMINETEE::CGameControllerMINETEE(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "MineTee";
	m_TimeVegetal = Server()->Tick();
	m_TimeEnv = Server()->Tick();
	m_TimeDestruction = Server()->Tick();
	m_TimeCook = Server()->Tick();
	m_TimeWear = Server()->Tick();
}

void CGameControllerMINETEE::Tick()
{
    bool Vegetal=false, Envirionment=false, Destruction=false, Cook = false, Wear = false;

    //Control Actions
    if (Server()->Tick() - m_TimeVegetal > Server()->TickSpeed()*60.0f)
    {
        Vegetal = true;
        m_TimeVegetal = Server()->Tick();
    }
    if (Server()->Tick() - m_TimeEnv > Server()->TickSpeed()*0.75f)
    {
        Envirionment = true;
        m_TimeEnv = Server()->Tick();
    }
    if (Server()->Tick() - m_TimeDestruction > Server()->TickSpeed()*0.5f)
    {
        Destruction = true;
        m_TimeDestruction = Server()->Tick();
    }
    if (Server()->Tick() - m_TimeCook > Server()->TickSpeed()*5.0f)
    {
        Cook = true;
        m_TimeCook = Server()->Tick();
    }
    if (Server()->Tick() - m_TimeWear > Server()->TickSpeed()*75.0f)
    {
        Wear = true;
        m_TimeWear = Server()->Tick();
    }


    //Actions
    bool ActionRun = false;
    if (Vegetal || Envirionment || Destruction || Cook || Wear)
    {
        CMapItemLayerTilemap *pTmap = (CMapItemLayerTilemap *)GameServer()->Layers()->MineTeeLayer();
        if (pTmap)
        {
            CTile *pTiles = (CTile *)GameServer()->Layers()->Map()->GetData(pTmap->m_Data);
            CTile *pTempTiles = static_cast<CTile*>(mem_alloc(sizeof(CTile)*pTmap->m_Width*pTmap->m_Height,1));
            mem_copy(pTempTiles, pTiles, sizeof(CTile)*pTmap->m_Width*pTmap->m_Height);

            for(int y = 0; y < pTmap->m_Height-1; y++)
            {
                for(int x = 0; x < pTmap->m_Width-1; x++)
                {
                    int c = y*pTmap->m_Width+x;

                    if (Envirionment)
                    {
                        if (((pTempTiles[c].m_Index >= BLOCK_UNDEF82 && pTempTiles[c].m_Index <= BLOCK_AGUA) || (pTempTiles[c].m_Index >= BLOCK_UNDEF104 && pTempTiles[c].m_Index <= BLOCK_LAVA)) && y+1 < pTmap->m_Height && x+1 < pTmap->m_Width)
                        {
                            int tc = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[tc].m_Index == 0 || (pTempTiles[tc].m_Index >= BLOCK_UNDEF82 && pTempTiles[tc].m_Index < BLOCK_UNDEF84) || (pTempTiles[tc].m_Index >= BLOCK_UNDEF104 && pTempTiles[tc].m_Index < BLOCK_UNDEF106))
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y+1;
                                if (pTempTiles[c].m_Index >= BLOCK_UNDEF82 && pTempTiles[c].m_Index <= BLOCK_AGUA)
                                    TileInfo.m_ITile = BLOCK_UNDEF84;
                                else
                                    TileInfo.m_ITile = BLOCK_UNDEF106;
                                TileInfo.m_Col = 0;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);

                                if (pTempTiles[c].m_Index >= BLOCK_UNDEF82 && pTempTiles[c].m_Index <= BLOCK_AGUA)
                                    GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_UNDEF84, 0);
                                else
                                   GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_UNDEF106, 0);
                            }
                            else if ((pTempTiles[c].m_Index >= BLOCK_UNDEF104 && pTempTiles[c].m_Index <= BLOCK_LAVA && pTempTiles[tc].m_Index >= BLOCK_UNDEF82 && pTempTiles[tc].m_Index <= BLOCK_AGUA) ||
                                     (pTempTiles[c].m_Index >= BLOCK_UNDEF82 && pTempTiles[c].m_Index <= BLOCK_AGUA && pTempTiles[tc].m_Index >= BLOCK_UNDEF104 && pTempTiles[tc].m_Index <= BLOCK_LAVA))
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y+1;
                                TileInfo.m_ITile = BLOCK_RUDINIUM;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_RUDINIUM);
                            }

                            tc = y*pTmap->m_Width+(x+1);
                            int tb = (y+1)*pTmap->m_Width+(x+1);
                            int te = (y-1)*pTmap->m_Width+(x+1);
                            int tl = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[tl].m_Index != 0 && (pTempTiles[tl].m_Index < BLOCK_UNDEF82 || pTempTiles[tl].m_Index > BLOCK_AGUA) && (pTempTiles[tl].m_Index < BLOCK_UNDEF104 || pTempTiles[tl].m_Index > BLOCK_LAVA) &&
                                pTempTiles[tc].m_Index == 0 &&
                                (pTempTiles[tb].m_Index < BLOCK_UNDEF82 || pTempTiles[tb].m_Index > BLOCK_AGUA) && (pTempTiles[tb].m_Index < BLOCK_UNDEF104 || pTempTiles[tb].m_Index > BLOCK_LAVA) &&
                                pTempTiles[c].m_Index-1 != BLOCK_UNDEF82-1 && pTempTiles[c].m_Index-1 != BLOCK_UNDEF104-1 &&
                                (pTempTiles[te].m_Index < BLOCK_UNDEF82 || pTempTiles[te].m_Index > BLOCK_AGUA) && (pTempTiles[te].m_Index < BLOCK_UNDEF104 || pTempTiles[te].m_Index > BLOCK_LAVA))
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x+1;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = pTiles[c].m_Index-1;
                                TileInfo.m_Col = 0;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), pTiles[c].m_Index-1, 0);
                            }
                            tc = y*pTmap->m_Width+(x-1);
                            tb = (y+1)*pTmap->m_Width+(x-1);
                            te = (y-1)*pTmap->m_Width+(x-1);
                            if (pTempTiles[tl].m_Index != 0 && pTempTiles[tl].m_Index != BLOCK_AGUA && pTempTiles[tl].m_Index != BLOCK_LAVA &&
                                pTempTiles[tc].m_Index == 0 &&
                                (pTempTiles[tb].m_Index < BLOCK_UNDEF82 || pTempTiles[tb].m_Index > BLOCK_AGUA) && (pTempTiles[tb].m_Index < BLOCK_UNDEF104 || pTempTiles[tb].m_Index > BLOCK_LAVA) &&
                                pTempTiles[c].m_Index-1 != BLOCK_UNDEF82-1 && pTempTiles[c].m_Index-1 != BLOCK_UNDEF104-1 &&
                                (pTempTiles[te].m_Index < BLOCK_UNDEF82 || pTempTiles[te].m_Index > BLOCK_AGUA) && (pTempTiles[te].m_Index < BLOCK_UNDEF104 || pTempTiles[te].m_Index > BLOCK_LAVA))
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x-1;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = pTiles[c].m_Index-1;
                                TileInfo.m_Col = 0;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), pTiles[c].m_Index-1, 0);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_ROSAR || pTempTiles[c].m_Index == BLOCK_ROSAY)
                        {
                            int indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index == BLOCK_GROUND)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y+1;
                                TileInfo.m_ITile = BLOCK_APGRASS;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_APGRASS);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_BED)
                        {
                            int indexT = y*pTmap->m_Width+(x+1);
                            if (pTempTiles[indexT].m_Index == BLOCK_BED)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = BLOCK_UNDEF48;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_UNDEF48);

                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x+1;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = BLOCK_UNDEF49;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_UNDEF49);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_INVENTARY)
                        {
                            int indexT = y*pTmap->m_Width+(x+1);
                            if (pTempTiles[indexT].m_Index == BLOCK_INVENTARY)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = BLOCK_INVTA;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_INVTA);

                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x+1;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = BLOCK_INVTB;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_INVTB);
                            }
                        }


                        if (y>0)
                        {
                            if (pTempTiles[c].m_Index == BLOCK_GROUND)
                            {
                                bool found = false;
                                for (int o=y-1; o>=0; o--)
                                {
                                    int indexT = o*pTmap->m_Width+x;
                                    if (pTempTiles[indexT].m_Index != 0 && (pTempTiles[indexT].m_Index < BLOCK_SEED2 || pTempTiles[indexT].m_Index > BLOCK_SEED8))
                                    {
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y;
                                    TileInfo.m_ITile = BLOCK_GRASSGROUND;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_CREATE;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_GRASSGROUND);
                                }
                            }
                            else if (pTempTiles[c].m_Index == BLOCK_GRASSGROUND)
                            {
                                int indexT = (y-1)*pTmap->m_Width+x;
                                if (pTempTiles[indexT].m_Index != 0 && (pTempTiles[indexT].m_Index < BLOCK_SEED2 || pTempTiles[indexT].m_Index > BLOCK_SEED8))
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y;
                                    if (pTempTiles[indexT].m_Index == BLOCK_NIEVE)
                                        TileInfo.m_ITile = BLOCK_BNGRASS;
                                    else
                                        TileInfo.m_ITile = BLOCK_GROUND;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_CREATE;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    if (pTempTiles[indexT].m_Index == BLOCK_NIEVE)
                                        GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_BNGRASS);
                                    else
                                        GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_GROUND);
                                }
                            }
                            else if (pTempTiles[c].m_Index == BLOCK_HORNO_OFF)
                            {
                                int indexT = (y-1)*pTmap->m_Width+x;
                                int indexR = y*pTmap->m_Width+(x-1);
                                int indexS = y*pTmap->m_Width+(x+1);
                                if (pTempTiles[indexT].m_Index == BLOCK_CARBONP || pTempTiles[indexR].m_Index == BLOCK_CARBONP || pTempTiles[indexS].m_Index == BLOCK_CARBONP)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y;
                                    TileInfo.m_ITile = BLOCK_HORNO_ON;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_CREATE;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_HORNO_ON);

                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    if (pTempTiles[indexT].m_Index == BLOCK_CARBONP)
                                    {
                                        TileInfo.m_X = x;
                                        TileInfo.m_Y = y-1;
                                    }
                                    else if (pTempTiles[indexR].m_Index == BLOCK_CARBONP)
                                    {
                                        TileInfo.m_X = x-1;
                                        TileInfo.m_Y = y;
                                    }
                                    else if (pTempTiles[indexS].m_Index == BLOCK_CARBONP)
                                    {
                                        TileInfo.m_X = x+1;
                                        TileInfo.m_Y = y;
                                    }
                                    TileInfo.m_ITile = -1;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_DESTROY;
                                    TileInfo.m_State = 0;
                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                }
                            }
                        }

                        //BLOCK FALL
                        if (pTempTiles[c].m_Index == BLOCK_ARENA || pTempTiles[c].m_Index == BLOCK_ARENAB || pTempTiles[c].m_Index == BLOCK_NIEVE || pTempTiles[c].m_Index == BLOCK_CARBONP)
                        {
                            int indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;
                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));

                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y+1;
                                TileInfo.m_ITile = pTempTiles[c].m_Index;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;
                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), pTempTiles[c].m_Index);
                            }
                        }

                        //BLOCK DAMAGE
                        if (pTempTiles[c].m_Index == BLOCK_CACTUS)
                        {
                            for (size_t e=0; e<MAX_CLIENTS; e++)
                            {
                                if (!GameServer()->m_apPlayers[e] || !GameServer()->m_apPlayers[e]->GetCharacter() || !GameServer()->m_apPlayers[e]->GetCharacter()->IsAlive())
                                    continue;

                                if (distance(GameServer()->m_apPlayers[e]->GetCharacter()->m_Pos, vec2((x<<5)+16.0f,(y<<5)+16.0f)) <= 36.0f)
                                    GameServer()->m_apPlayers[e]->GetCharacter()->TakeDamage(vec2(0.0f, 0.0f), 1.0f, GameServer()->m_apPlayers[e]->GetCID(), WEAPON_WORLD);
                            }
                        }
                    }

                    if (Destruction)
                    {
                        if (pTempTiles[c].m_Index == BLOCK_AZUCAR)
                        {
                            int indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index != BLOCK_AZUCAR && pTempTiles[indexT].m_Index != BLOCK_APGRASS)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_AZUCAR);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_CACTUS)
                        {
                            int indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index != BLOCK_CACTUS && pTempTiles[indexT].m_Index != BLOCK_ARENA && pTempTiles[indexT].m_Index != BLOCK_ARENAB)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_CACTUS);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_LUZ)
                        {
                            bool found = false;

                            int indexT = y*pTmap->m_Width+(x-1);
                            if (pTempTiles[indexT].m_Index != 0 && pTempTiles[indexT].m_Index != BLOCK_LAVA && pTempTiles[indexT].m_Index != BLOCK_AGUA)
                                found = true;
                            indexT = y*pTmap->m_Width+(x+1);
                            if (pTempTiles[indexT].m_Index != 0 && pTempTiles[indexT].m_Index != BLOCK_LAVA && pTempTiles[indexT].m_Index != BLOCK_AGUA)
                                found = true;
                            indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index != 0 && pTempTiles[indexT].m_Index != BLOCK_LAVA && pTempTiles[indexT].m_Index != BLOCK_AGUA)
                                found = true;

                            if (!found)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_LUZ);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                        else if (pTempTiles[c].m_Index >= BLOCK_SEED1 && pTempTiles[c].m_Index <= BLOCK_SEED8)
                        {
                            int indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_SEEDM);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_UNDEF48)
                        {
                            int indexT = y*pTmap->m_Width+(x+1);
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_BED);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_UNDEF49)
                        {
                            int indexT = y*pTmap->m_Width+(x-1);
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_BED);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_INVTA)
                        {
                            int indexT = y*pTmap->m_Width+(x+1);
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_INVENTARY);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_INVTB)
                        {
                            int indexT = y*pTmap->m_Width+(x-1);
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_INVENTARY);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_UNDEF84)
                        {
                            int indexT = (y-1)*pTmap->m_Width+x;
                            int indexR = y*pTmap->m_Width+(x-1);
                            int indexS = y*pTmap->m_Width+(x+1);
                            if ((pTempTiles[indexT].m_Index < BLOCK_UNDEF82 || pTempTiles[indexT].m_Index > BLOCK_AGUA) &&
                                (pTempTiles[indexR].m_Index < BLOCK_UNDEF82 || pTempTiles[indexR].m_Index > BLOCK_AGUA) &&
                                (pTempTiles[indexS].m_Index < BLOCK_UNDEF82 || pTempTiles[indexS].m_Index > BLOCK_AGUA))
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_UNDEF106)
                        {
                            int indexT = (y-1)*pTmap->m_Width+x;
                            int indexR = y*pTmap->m_Width+(x-1);
                            int indexS = y*pTmap->m_Width+(x+1);
                            if ((pTempTiles[indexT].m_Index < BLOCK_UNDEF104 || pTempTiles[indexT].m_Index > BLOCK_LAVA) &&
                                (pTempTiles[indexR].m_Index < BLOCK_UNDEF104 || pTempTiles[indexR].m_Index > BLOCK_LAVA) &&
                                (pTempTiles[indexS].m_Index < BLOCK_UNDEF104 || pTempTiles[indexS].m_Index > BLOCK_LAVA))
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                            }
                        }
                        else if (pTempTiles[c].m_Index == BLOCK_ROSAR || pTempTiles[c].m_Index == BLOCK_ROSAY)
                        {
                            int indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = -1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_DESTROY;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, pTempTiles[c].m_Index);
                                pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                            }
                        }
                    }

                    if (Vegetal)
                    {
                        /** Crecimiento **/
                        if (pTempTiles[c].m_Index == BLOCK_AZUCAR)
                        {
                            int indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index == BLOCK_AZUCAR || pTempTiles[indexT].m_Index == BLOCK_APGRASS)
                            {
                                indexT = (y-1)*pTmap->m_Width+x;
                                if (pTempTiles[indexT].m_Index == 0)
                                {
                                    int tam=0;
                                    bool canC = false;
                                    for(int u=1; u<=5; u++)
                                    {
                                        int indexA = (y+u)*pTmap->m_Width+x;
                                        if (pTempTiles[indexA].m_Index == BLOCK_AZUCAR)
                                            tam++;
                                        else
                                        {
                                            if (pTempTiles[indexA].m_Index == BLOCK_APGRASS)
                                                canC = true;
                                            break;
                                        }
                                    }

                                    if ((rand()%10) == 7 && canC && tam < 5)
                                    {
                                        CNetMsg_Sv_TileChangeExt TileInfo;
                                        TileInfo.m_Size = -1;
                                        TileInfo.m_Index = -1;
                                        TileInfo.m_X = x;
                                        TileInfo.m_Y = y-1;
                                        TileInfo.m_ITile = BLOCK_AZUCAR;
                                        TileInfo.m_Col = 1;
                                        TileInfo.m_Act = TILE_CREATE;
                                        TileInfo.m_State = 0;

                                        Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                        GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_AZUCAR);
                                    }
                                }
                            }
                        }

                        if (pTempTiles[c].m_Index == BLOCK_CACTUS)
                        {
                            int indexT = (y+1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index == BLOCK_CACTUS || pTempTiles[indexT].m_Index == BLOCK_ARENA || pTempTiles[indexT].m_Index == BLOCK_ARENAB)
                            {
                                indexT = (y-1)*pTmap->m_Width+x;
                                if (pTempTiles[indexT].m_Index == 0)
                                {
                                    int tam=0;
                                    bool canC = false;
                                    for(int u=1; u<=5; u++)
                                    {
                                        int indexA = (y+u)*pTmap->m_Width+x;
                                        if (pTempTiles[indexA].m_Index == BLOCK_CACTUS)
                                            tam++;
                                        else
                                        {
                                            if (pTempTiles[indexA].m_Index == BLOCK_ARENA || pTempTiles[indexA].m_Index == BLOCK_ARENAB)
                                                canC = true;
                                            break;
                                        }
                                    }

                                    if ((rand()%10) == 7 && canC && tam < 8)
                                    {
                                        CNetMsg_Sv_TileChangeExt TileInfo;
                                        TileInfo.m_Size = -1;
                                        TileInfo.m_Index = -1;
                                        TileInfo.m_X = x;
                                        TileInfo.m_Y = y-1;
                                        TileInfo.m_ITile = BLOCK_CACTUS;
                                        TileInfo.m_Col = 1;
                                        TileInfo.m_Act = TILE_CREATE;
                                        TileInfo.m_State = 0;

                                        Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                        GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_CACTUS);
                                    }
                                }
                            }
                        }
                        else if (pTempTiles[c].m_Index >= BLOCK_SEED1 && pTempTiles[c].m_Index < BLOCK_SEED8)
                        {
                            if ((rand()%1) == 0)
                            {
                                int nindex = pTempTiles[c].m_Index+1;
                                int cindex = (y+1)*pTmap->m_Width+x;
                                if (pTempTiles[cindex].m_Index != BLOCK_APGRASS && nindex > BLOCK_SEED4)
                                    nindex = BLOCK_SEED4;

                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = nindex;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), nindex);
                            }
                        }

                        /** Creacion **/
                        if ((rand()%100)==3 && (pTempTiles[c].m_Index == BLOCK_ARENA || pTempTiles[c].m_Index == BLOCK_ARENAB))
                        {
                            int indexT = (y-1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                bool found = false;

                                for (int i=-4; i<5; i++)
                                {
                                    int indexT = (y-1)*pTmap->m_Width+(x+i);
                                    if (pTempTiles[indexT].m_Index == BLOCK_CACTUS)
                                    {
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y-1;
                                    TileInfo.m_ITile = BLOCK_CACTUS;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_CREATE;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_CACTUS);
                                }
                            }
                        }
                        else if ((rand()%100)==3 && pTempTiles[c].m_Index == BLOCK_SEED4)
                        {
                            int indexT = y*pTmap->m_Width+(x+1);
                            int indexE = (y+1)*pTmap->m_Width+(x+1);
                            if (pTempTiles[indexT].m_Index == 0 && pTempTiles[indexE].m_Index == BLOCK_GRASSGROUND)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x+1;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = BLOCK_SEED1;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_SEED1);
                            }
                        }

                        /** Muerte **/
                        if (pTempTiles[c].m_Index == BLOCK_APGRASS)
                        {
                            int indexT = (y-1)*pTmap->m_Width+x;
                            if (pTempTiles[indexT].m_Index == 0)
                            {
                                CNetMsg_Sv_TileChangeExt TileInfo;
                                TileInfo.m_Size = -1;
                                TileInfo.m_Index = -1;
                                TileInfo.m_X = x;
                                TileInfo.m_Y = y;
                                TileInfo.m_ITile = BLOCK_GROUND;
                                TileInfo.m_Col = 1;
                                TileInfo.m_Act = TILE_CREATE;
                                TileInfo.m_State = 0;

                                Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_GROUND);
                            }
                        }
                    }

                    if (Cook)
                    {
                        if (y > 0)
                        {
                            if (pTempTiles[c].m_Index == BLOCK_HORNO_ON)
                            {
                                int indexT = (y-1)*pTmap->m_Width+x;
                                if (pTempTiles[indexT].m_Index == BLOCK_ARENA || pTempTiles[indexT].m_Index == BLOCK_ARENAB)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y-1;
                                    TileInfo.m_ITile = -1;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_DESTROY;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                    CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_CRISTAL);
                                    pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                                }
                                else if (pTempTiles[indexT].m_Index == BLOCK_STONE)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y-1;
                                    TileInfo.m_ITile = -1;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_DESTROY;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                    CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_STONE2);
                                    pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                                }
                                else if (pTempTiles[indexT].m_Index == BLOCK_UNDEF63)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y-1;
                                    TileInfo.m_ITile = -1;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_DESTROY;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                    CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_TARENA);
                                    pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                                }
                                else if (pTempTiles[indexT].m_Index == BLOCK_GOLD)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y-1;
                                    TileInfo.m_ITile = -1;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_DESTROY;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                    CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_OROP);
                                    pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                                }
                                else if (pTempTiles[indexT].m_Index == BLOCK_PLATA)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y-1;
                                    TileInfo.m_ITile = -1;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_DESTROY;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                    CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_PLATAP);
                                    pPickup->m_Pos = vec2(TileInfo.m_X*32.0f + 8.0f, TileInfo.m_Y*32.0f + 8.0f);
                                }
                                else if (pTempTiles[indexT].m_Index != 0 && pTempTiles[indexT].m_Index != BLOCK_CRISTAL && pTempTiles[indexT].m_Index != BLOCK_STONE2)
                                {
                                    CNetMsg_Sv_TileChangeExt TileInfo;
                                    TileInfo.m_Size = -1;
                                    TileInfo.m_Index = -1;
                                    TileInfo.m_X = x;
                                    TileInfo.m_Y = y-1;
                                    TileInfo.m_ITile = -1;
                                    TileInfo.m_Col = 1;
                                    TileInfo.m_Act = TILE_DESTROY;
                                    TileInfo.m_State = 0;

                                    Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                                    GameServer()->Collision()->DestroyTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5));
                                }
                            }
                        }
                    }

                    if (Wear)
                    {

                        if (pTempTiles[c].m_Index == BLOCK_HORNO_ON)
                        {
                            CNetMsg_Sv_TileChangeExt TileInfo;
                            TileInfo.m_Size = -1;
                            TileInfo.m_Index = -1;
                            TileInfo.m_X = x;
                            TileInfo.m_Y = y;
                            TileInfo.m_ITile = BLOCK_HORNO_OFF;
                            TileInfo.m_Col = 1;
                            TileInfo.m_Act = TILE_CREATE;
                            TileInfo.m_State = 0;

                            Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                            GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_HORNO_OFF);
                        }
                        else if ((rand()%100) == 2 && pTempTiles[c].m_Index == BLOCK_STONE2)
                        {
                            CNetMsg_Sv_TileChangeExt TileInfo;
                            TileInfo.m_Size = -1;
                            TileInfo.m_Index = -1;
                            TileInfo.m_X = x;
                            TileInfo.m_Y = y;
                            TileInfo.m_ITile = BLOCK_STONE2BREAK;
                            TileInfo.m_Col = 1;
                            TileInfo.m_Act = TILE_CREATE;
                            TileInfo.m_State = 0;

                            Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                            GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_STONE2BREAK);
                        }
                        else if ((rand()%100) == 2 && pTempTiles[c].m_Index == BLOCK_STONE2BREAK)
                        {
                            CNetMsg_Sv_TileChangeExt TileInfo;
                            TileInfo.m_Size = -1;
                            TileInfo.m_Index = -1;
                            TileInfo.m_X = x;
                            TileInfo.m_Y = y;
                            TileInfo.m_ITile = BLOCK_STONE2MOO;
                            TileInfo.m_Col = 1;
                            TileInfo.m_Act = TILE_CREATE;
                            TileInfo.m_State = 0;

                            Server()->SendPackMsg(&TileInfo, MSGFLAG_VITAL, -1);
                            GameServer()->Collision()->CreateTile(vec2(TileInfo.m_X<<5, TileInfo.m_Y<<5), BLOCK_STONE2MOO);
                        }
                    }
                }
            }

            mem_free(pTempTiles);
        }
    }

	IGameController::Tick();
}

void CGameControllerMINETEE::OnCharacterSpawn(class CCharacter *pChr)
{
	pChr->IncreaseHealth(10);

	// Zombies Weapons
	switch (pChr->GetPlayer()->GetTeam())
	{
        case TEAM_ENEMY_TEEPER:
            pChr->GiveBlock(BLOCK_TNT, 1);
            pChr->SetWeapon(NUM_WEAPONS+BLOCK_TNT);
            break;

        case TEAM_ENEMY_ZOMBITEE:
        case TEAM_ENEMY_SPIDERTEE:
            pChr->GiveWeapon(WEAPON_HAMMER, -1);
            pChr->SetWeapon(WEAPON_HAMMER);
            break;

        case TEAM_ENEMY_SKELETEE:
            pChr->GiveWeapon(WEAPON_GRENADE, -1);
            pChr->SetWeapon(WEAPON_GRENADE);
            break;

        default:
            pChr->GiveWeapon(WEAPON_HAMMER, -1);
            pChr->SetWeapon(WEAPON_HAMMER);
	}
}

int CGameControllerMINETEE::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
    IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

    for (size_t i=0; i<10; i++)
    {
        int Index = pVictim->m_Inventory.m_Items[i];

        if (Index != NUM_WEAPONS+NUM_BLOCKS)
        {
            if (Index >= NUM_WEAPONS)
            {
                if (pVictim->m_aBlocks[Index-NUM_WEAPONS].m_Got)
                {
                    CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_DROPITEM, Index-NUM_WEAPONS);
                    pPickup->m_Pos = pVictim->m_Pos;
                    pPickup->m_Pos.y -= 18.0f;
                    pPickup->m_Vel = vec2((((rand()%2)==0)?1:-1)*(rand()%10), -5);
                    pPickup->m_Amount = pVictim->m_aBlocks[Index-NUM_WEAPONS].m_Amount;
                    pPickup->m_Owner = pVictim->GetPlayer()->GetCID();
                }
            }
        }
    }

    if (pVictim->GetPlayer() == pKiller)
        return 1;

    if (g_Config.m_SvGameMode == 1 && pKiller->GetCharacter() && pVictim->GetPlayer()->GetTeam() > TEAM_BLUE && pVictim->GetPlayer()->GetTeam() < TEAM_ANIMAL_TEECOW)
    {
        int Kills = pKiller->GetCharacter()->m_Kills;
        if (Kills > 0 && (Kills%8) == 0)
        {
            char buff[150];
            str_format(buff, sizeof(buff), "'%s' kill %i bots without dead!", Server()->ClientName(pKiller->GetCID()), Kills);
            GameServer()->SendChat(-1, CGameContext::CHAT_ALL, buff);
            pKiller->GetCharacter()->GiveWeapon(WEAPON_RIFLE, 10);
        }
    }

    if (pVictim->GetPlayer()->GetTeam() == TEAM_ANIMAL_TEECOW)
    {
		CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_FOOD, FOOD_COW);
		pPickup->m_Pos = pVictim->m_Pos;
        pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_CUERO);
		pPickup->m_Pos = pVictim->m_Pos;

		return 0;
    }
    else if (pVictim->GetPlayer()->GetTeam() == TEAM_ANIMAL_TEEPIG)
    {
		CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_FOOD, FOOD_PIG);
		pPickup->m_Pos = pVictim->m_Pos;
		return 0;
    }
    else if (pVictim->GetPlayer()->GetTeam() == TEAM_ENEMY_TEEPER)
    {
		CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_POLVORA);
		pPickup->m_Pos = pVictim->m_Pos;
		return 0;
    }
    else if (pVictim->GetPlayer()->GetTeam() == TEAM_ENEMY_SKELETEE)
    {
		CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_BLOCK, BLOCK_HUESO);
		pPickup->m_Pos = pVictim->m_Pos;
		return 0;
    }

    return 1;
}

bool CGameControllerMINETEE::CanJoinTeam(int Team, int NotThisID)
{
    if (!Server()->IsAuthed(NotThisID) && Team == TEAM_SPECTATORS)
    {
        GameServer()->SendChatTarget(NotThisID, "Players can't join to spectators team.");
        return false;
    }

    return IGameController::CanJoinTeam(Team, NotThisID);
}

bool CGameControllerMINETEE::OnChat(int cid, int team, const char *msg)
{
    char buff[255];
	char *ptr;

	str_format(buff, sizeof(buff), "%s", msg);

	if (!(ptr = strtok(buff, " \n\t")) || msg[0] != '/')
		return true;

	if (str_comp_nocase(ptr,"/cmdlist") == 0 || str_comp_nocase(ptr,"/help") == 0)
	{
        GameServer()->SendChatTarget(cid," ");
        GameServer()->SendChatTarget(cid," ");
        GameServer()->SendChatTarget(cid," ");
        GameServer()->SendChatTarget(cid,"COMMANDS LIST");
        GameServer()->SendChatTarget(cid,"======================================");
		GameServer()->SendChatTarget(cid," ");
		GameServer()->SendChatTarget(cid," - /info          > View general info of this mod.");
		GameServer()->SendChatTarget(cid," - /info about    > About of this mod.");
		GameServer()->SendChatTarget(cid," - /info <cmd>    > Info about selected command.");
		GameServer()->SendChatTarget(cid," - /craft         > Craft items!");
        GameServer()->SendChatTarget(cid," ");

		return false;
	}
	else if (str_comp_nocase(ptr,"/info") == 0)
	{
		if ((ptr=strtok(NULL, " \n\t")) == NULL)
		{
			GameServer()->SendChatTarget(cid," ");
			GameServer()->SendChatTarget(cid," ");
			GameServer()->SendChatTarget(cid," ");
			GameServer()->SendChatTarget(cid," ");
			GameServer()->SendChatTarget(cid," ");
			GameServer()->SendChatTarget(cid,"INFO *** [MineTee v"MINETEE_VERSION"] by unsigned char*");
			GameServer()->SendChatTarget(cid,"======================================");
			GameServer()->SendChatTarget(cid," ");
			GameServer()->SendChatTarget(cid," Build, Destroy, Imagine, Discover....");
			GameServer()->SendChatTarget(cid," ");

			return false;
		}
		else
		{
			do
			{
				if (str_comp_nocase(ptr,"about") == 0)
				{
                    GameServer()->SendChatTarget(cid," ");
                    GameServer()->SendChatTarget(cid," ");
                    GameServer()->SendChatTarget(cid," ");
                    GameServer()->SendChatTarget(cid," ");
					GameServer()->SendChatTarget(cid, "MineTee v"MINETEE_VERSION" **");
					GameServer()->SendChatTarget(cid, "Author: unsigned char*");
					GameServer()->SendChatTarget(cid, "Location: Galician/Spain");
					GameServer()->SendChatTarget(cid, "Year: 2013");
					GameServer()->SendChatTarget(cid, "Teeworlds Version: "GAME_VERSION);
                    GameServer()->SendChatTarget(cid," ");

					return false;
				}
                if (str_comp_nocase(ptr,"craft") == 0)
				{
                    GameServer()->SendChatTarget(cid," ");
                    GameServer()->SendChatTarget(cid," ");
                    GameServer()->SendChatTarget(cid," ");
                    GameServer()->SendChatTarget(cid,"- CRAFTING");
					GameServer()->SendChatTarget(cid, "======================================");
					GameServer()->SendChatTarget(cid, "Syntaxis: /craft <item>");
					GameServer()->SendChatTarget(cid, "Items: Horn, Torch, Bed, Chest, GLauncher, Grenade, Gun, Grille, Gold_Block, Iron_Fence, Iron_Window, Iron_Block, Wood_Window, Wood_Fence, Dust_A");
					GameServer()->SendChatTarget(cid, " ");
					GameServer()->SendChatTarget(cid, "Usage Example: /craft torch");
                    GameServer()->SendChatTarget(cid," ");

					return false;
				}
				else
                {
					GameServer()->SendChatTarget(cid,"Oops!: Unknown command or item");
                    return false;
                }
			} while ((ptr = strtok(NULL, " \n\t")) != NULL);
		}
	}
	else if (str_comp_nocase(ptr,"/craft") == 0)
    {
		if ((ptr=strtok(NULL, " \n\t")) == NULL)
		{
			GameServer()->SendChatTarget(cid,"** Need item! Please, write '/info craft' to see more information about crafting.");

			return false;
		}
		else
		{
			do
			{
				if (str_comp_nocase(ptr,"Torch") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_CARBONP].m_Got || pChar->m_aBlocks[BLOCK_CARBONP].m_Amount <= 0 || !pChar->m_aBlocks[BLOCK_TRONCO1].m_Got || pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount <= 0)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: TORCH   [4 Units]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 1 Wood [Brown]");
                            GameServer()->SendChatTarget(cid, " - 1 Coal");
                            GameServer()->SendChatTarget(cid," ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_CARBONP].m_Amount--;
                            pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount--;
                            pChar->GiveBlock(BLOCK_LUZ, 4);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_LUZ);

                            GameServer()->SendChatTarget(cid,"** You have been added a Torch!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr,"Horn") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_STONE].m_Got || pChar->m_aBlocks[BLOCK_STONE].m_Amount < 20)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: HORN   [1 Unit]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 20 Stone");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_STONE].m_Amount-=20;
                            pChar->GiveBlock(BLOCK_HORNO_OFF, 1);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_HORNO_OFF);

                            GameServer()->SendChatTarget(cid,"** You have been added a Horn!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr,"Bed") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_TRONCO1].m_Got || pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount < 4 || !pChar->m_aBlocks[BLOCK_CUERO].m_Got || pChar->m_aBlocks[BLOCK_CUERO].m_Amount < 2)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: BED   [1 Unit]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 4 Wood [Brown]");
                            GameServer()->SendChatTarget(cid, " - 2 Cow leather");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount-=4;
                            pChar->m_aBlocks[BLOCK_CUERO].m_Amount-=2;
                            pChar->GiveBlock(BLOCK_BED, 1);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_BED);

                            GameServer()->SendChatTarget(cid,"** You have been added a Bed!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Chest") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_TRONCO1].m_Got || pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount < 10)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: CHEST   [1 Unit]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 10 Wood [Brown]");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount-=10;
                            pChar->GiveBlock(BLOCK_INVENTARY, 1);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_INVENTARY);

                            GameServer()->SendChatTarget(cid,"** You have been added a Chest!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Grille") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_PLATAP].m_Got || pChar->m_aBlocks[BLOCK_PLATAP].m_Amount < 6)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: GRILLE   [4 Units]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 6 Iron");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_PLATAP].m_Amount-=6;
                            pChar->GiveBlock(BLOCK_UNDEF45, 4);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_UNDEF45);

                            GameServer()->SendChatTarget(cid,"** You have been added a Grille!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Gold_Block") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_OROP].m_Got || pChar->m_aBlocks[BLOCK_OROP].m_Amount < 10)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: GOLD BLOCK   [1 Unit]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 10 Gold");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_OROP].m_Amount-=10;
                            pChar->GiveBlock(BLOCK_BGOLD, 1);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_BGOLD);

                            GameServer()->SendChatTarget(cid,"** You have been added a Gold Block!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Iron_Fence") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_PLATAP].m_Got || pChar->m_aBlocks[BLOCK_PLATAP].m_Amount < 6)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: IRON FENCE   [4 Units]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 6 Iron");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_PLATAP].m_Amount-=6;
                            pChar->GiveBlock(BLOCK_MBALLA, 4);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_MBALLA);

                            GameServer()->SendChatTarget(cid,"** You have been added a Iron Fence!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Iron_Window") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_PLATAP].m_Got || pChar->m_aBlocks[BLOCK_PLATAP].m_Amount < 6)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: IRON WINDOW   [4 Units]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 6 Iron");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_PLATAP].m_Amount-=6;
                            pChar->GiveBlock(BLOCK_MDOOR, 4);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_MDOOR);

                            GameServer()->SendChatTarget(cid,"** You have been added a Iron Window!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Wood_Window") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_TRONCO1].m_Got || pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount < 6)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: WOOD WINDOW   [4 Units]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 6 Wood [brown]");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount-=6;
                            pChar->GiveBlock(BLOCK_WDOOR, 4);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_WDOOR);

                            GameServer()->SendChatTarget(cid,"** You have been added a Wood Window!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Wood_Fence") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_TRONCO1].m_Got || pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount < 6)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: WOOD WINDOW   [4 Units]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 6 Wood [brown]");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount-=6;
                            pChar->GiveBlock(BLOCK_UNDEF24, 4);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_UNDEF24);

                            GameServer()->SendChatTarget(cid,"** You have been added a Wood Fence!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Dust_A") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_ARENA].m_Got || pChar->m_aBlocks[BLOCK_ARENA].m_Amount < 4)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: DUST A   [1 Unit]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 4 Dust");
                            GameServer()->SendChatTarget(cid, " ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_ARENA].m_Amount-=4;
                            pChar->GiveBlock(BLOCK_TARENA, 1);
                            pChar->SetWeapon(NUM_WEAPONS+BLOCK_TARENA);

                            GameServer()->SendChatTarget(cid,"** You have been added a Dust A!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "GLauncher") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_POLVORA].m_Got || pChar->m_aBlocks[BLOCK_POLVORA].m_Amount < 12 || !pChar->m_aBlocks[BLOCK_TRONCO1].m_Got || pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount < 7 || !pChar->m_aBlocks[BLOCK_PLATAP].m_Got || pChar->m_aBlocks[BLOCK_PLATAP].m_Amount < 4)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: GLAUNCHER   [1 Unit]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 12 Powder");
                            GameServer()->SendChatTarget(cid, " - 7 Wood [Brown]");
                            GameServer()->SendChatTarget(cid, " - 4 Iron");
                            GameServer()->SendChatTarget(cid," ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_POLVORA].m_Amount-=12;
                            pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount-=7;
                            pChar->m_aBlocks[BLOCK_PLATAP].m_Amount-=4;
                            pChar->GiveWeapon(WEAPON_GRENADE, 0);
                            pChar->SetWeapon(WEAPON_GRENADE);

                            GameServer()->SendChatTarget(cid,"** You have been added a Grenade Launcher!");
                        }
                    }

					return false;
				}
				else if (str_comp_nocase(ptr, "Grenade") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (pChar->GetCurrentAmmo(WEAPON_GRENADE) <= 0 || !pChar->m_aBlocks[BLOCK_POLVORA].m_Got || pChar->m_aBlocks[BLOCK_POLVORA].m_Amount < 7 || !pChar->m_aBlocks[BLOCK_PLATAP].m_Got || pChar->m_aBlocks[BLOCK_PLATAP].m_Amount < 2)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: GRENADE   [5 Units]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 1 GLauncher");
                            GameServer()->SendChatTarget(cid, " - 7 Powder");
                            GameServer()->SendChatTarget(cid, " - 2 Iron");
                            GameServer()->SendChatTarget(cid," ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_POLVORA].m_Amount-=7;
                            pChar->m_aBlocks[BLOCK_PLATAP].m_Amount-=2;

                            pChar->GiveWeapon(WEAPON_GRENADE, pChar->GetCurrentAmmo(WEAPON_GRENADE)+5);

                            GameServer()->SendChatTarget(cid,"** You have been added a Grenade!");
                        }
                    }

					return false;
				}
                else if (str_comp_nocase(ptr, "Gun") == 0)
				{
                    CCharacter *pChar = GameServer()->m_apPlayers[cid]->GetCharacter();
                    if (pChar)
                    {
                        if (!pChar->m_aBlocks[BLOCK_PLATAP].m_Got || pChar->m_aBlocks[BLOCK_PLATAP].m_Amount < 12 || !pChar->m_aBlocks[BLOCK_TRONCO1].m_Got || pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount < 4)
                        {
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid," ");
                            GameServer()->SendChatTarget(cid, "- CRAFT: GUN   [1 Unit]");
                            GameServer()->SendChatTarget(cid, "======================================");
                            GameServer()->SendChatTarget(cid, "You need the following items:");
                            GameServer()->SendChatTarget(cid, " ");
                            GameServer()->SendChatTarget(cid, " - 12 Iron");
                            GameServer()->SendChatTarget(cid, " - 4 Wood [Brown]");
                            GameServer()->SendChatTarget(cid," ");
                        }
                        else
                        {
                            pChar->m_aBlocks[BLOCK_TRONCO1].m_Amount-=4;
                            pChar->m_aBlocks[BLOCK_PLATAP].m_Amount-=12;
                            pChar->GiveWeapon(WEAPON_GUN, 0);
                            pChar->SetWeapon(WEAPON_GUN);

                            GameServer()->SendChatTarget(cid,"** You have been added a Gun!");
                        }
                    }

					return false;
				}
				else
                {
					GameServer()->SendChatTarget(cid,"Oops!: Unknown item");
                    return false;
                }
			} while ((ptr = strtok(NULL, " \n\t")) != NULL);
		}
    }
	else
    {
		GameServer()->SendChatTarget(cid,"Oops!: Unknown command");
        return false;
    }

    return true;
}
