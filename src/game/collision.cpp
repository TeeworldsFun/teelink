/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <math.h>
#include <engine/map.h>
#include <engine/kernel.h>

#include <game/mapitems.h>
#include <game/layers.h>
#include <game/collision.h>

#include <game/server/entities/pickup.h> //H-Client
#include <deque>

CCollision::CCollision()
{
	m_pTiles = 0;
	m_Width = 0;
	m_Height = 0;
	m_pLayers = 0;

    m_pMineTeeTiles = 0x0;
    m_pSecBlocks = 0x0;
	m_pFront = 0x0;
}

void CCollision::Init(class CLayers *pLayers)
{
    int sizeMineLayer = -1;

    m_pFront = 0; //H-Client
    m_pMineTeeTiles = 0x0;
    m_pSecBlocks = 0x0;


	m_pLayers = pLayers;
	m_Width = m_pLayers->GameLayer()->m_Width;
	m_Height = m_pLayers->GameLayer()->m_Height;
	m_pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameLayer()->m_Data));
	if (m_pLayers->MineTeeLayer())
	{
        m_pMineTeeTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->MineTeeLayer()->m_Data));
        sizeMineLayer = m_pLayers->MineTeeLayer()->m_Width*m_pLayers->MineTeeLayer()->m_Height;
	}
    m_pSecBlocks = static_cast<int*>(malloc(sizeof(int)*m_Width*m_Height));

	if(m_pLayers->FrontLayer())
		m_pFront = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->FrontLayer()->m_Front));

	for(int i = 0; i < m_Width*m_Height; i++)
	{
		int Index;

        //H-Client: DDRace
        m_pSecBlocks[i]=-1;

		if (m_pFront)
		{
		    Index = m_pFront[i].m_Index;
            if (Index > TILE_END)
                continue;

            switch(Index)
            {
                case TILE_DEATH:
                    m_pFront[i].m_Index = COLFLAG_DEATH;
                    break;
                default:
                    m_pFront[i].m_Index = 0;
            }

            if(Index == TILE_THROUGH || (Index >= TILE_FREEZE && Index <= TILE_UNFREEZE) || (Index >= TILE_BEGIN && Index <= TILE_STOPA))
                m_pFront[i].m_Index = Index;
        }
        //H-Client: MineTee
        if (m_pMineTeeTiles && i<sizeMineLayer)
        {
            Index = m_pMineTeeTiles[i].m_Index;
            if (Index == BLOCK_DIRTYGRASS)
                m_pTiles[i].m_Index = TILE_NOHOOK;
        }
        //

		Index = m_pTiles[i].m_Index;
		if(Index > 128)
			continue;

		switch(Index)
		{
		case TILE_DEATH:
			m_pTiles[i].m_Index = COLFLAG_DEATH;
			break;
		case TILE_SOLID:
			m_pTiles[i].m_Index = COLFLAG_SOLID;
			break;
		case TILE_NOHOOK:
			m_pTiles[i].m_Index = COLFLAG_SOLID|COLFLAG_NOHOOK;
			break;
		default:
			m_pTiles[i].m_Index = 0;
		}

        //H-Client: DDRace Stuff
        if(Index == TILE_THROUGH || (Index >= TILE_FREEZE && Index <= TILE_UNFREEZE) || (Index >= TILE_BEGIN && Index <= TILE_STOPA))
        	m_pTiles[i].m_Index = Index;
	}
}

/*int CCollision::GetTile(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);

	return m_pTiles[Ny*m_Width+Nx].m_Index > 128 ? 0 : m_pTiles[Ny*m_Width+Nx].m_Index;
}*/
//H-Client: DDrace Change
int CCollision::GetTile(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);
	if(!m_pTiles || Ny < 0 || Nx < 0)
		return 0;

	if(m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_SOLID
		|| m_pTiles[Ny*m_Width+Nx].m_Index == (COLFLAG_SOLID|COLFLAG_NOHOOK)
		|| m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_DEATH)
		return m_pTiles[Ny*m_Width+Nx].m_Index;

	return 0;
}
//

bool CCollision::IsTileSolid(int x, int y, bool nocoll)
{
    //H-Client
    if (nocoll && m_pMineTeeTiles)
    {
        if (((GetMineTeeBlockAt(x,y) >= BLOCK_UNDEF48 && GetMineTeeBlockAt(x,y) <= BLOCK_BED) ||
         GetMineTeeBlockAt(x,y) == BLOCK_RSETA || GetMineTeeBlockAt(x,y) == BLOCK_BSETA) ||
         GetMineTeeBlockAt(x,y) == BLOCK_TARTA1 || GetMineTeeBlockAt(x,y) == BLOCK_TARTA2)
        {
            int Nx = clamp(x/32, 0, m_Width-1);
            int Ny = clamp(y/32, 0, m_Height-1);
            Nx *= 32; Ny *= 32;

            if (y >= Ny+16.0f)
                return 1;

            return 0;
        }
        else if (m_pMineTeeTiles && (GetMineTeeBlockAt(x,y) == BLOCK_AZUCAR ||
                                     GetMineTeeBlockAt(x,y) == BLOCK_ROSAR || GetMineTeeBlockAt(x,y) == BLOCK_ROSAY ||
                                     (GetMineTeeBlockAt(x,y) >= BLOCK_SEED1 && GetMineTeeBlockAt(x,y) <= BLOCK_SEED8) ||
                                     GetMineTeeBlockAt(x,y) == BLOCK_SETAR1 || GetMineTeeBlockAt(x,y) == BLOCK_SETAR2))
            return 0;
    }

	return GetTile(x, y)&COLFLAG_SOLID;
}

// TODO: rewrite this smarter!
int CCollision::IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, bool AllowThrough)
{
	float Distance = distance(Pos0, Pos1);
	int End(Distance+1);
	vec2 Last = Pos0;
	//H-Client: DDRace
	int ix = 0, iy = 0; // Temporary position for checking collision
	int dx = 0, dy = 0; // Offset for checking the "through" tile
	if (AllowThrough)
        ThroughOffset(Pos0, Pos1, &dx, &dy);
    //

	for(int i = 0; i < End; i++)
	{
		float a = i/Distance;
		vec2 Pos = mix(Pos0, Pos1, a);
        ix = round(Pos.x);
		iy = round(Pos.y);

		if(CheckPoint(Pos.x, Pos.y, false) && !(AllowThrough && IsThrough(ix + dx, iy + dy)))
		{
			if(pOutCollision)
				*pOutCollision = Pos;
			if(pOutBeforeCollision)
				*pOutBeforeCollision = Last;
			return GetCollisionAt(ix, iy);
		}
		Last = Pos;
	}
	if(pOutCollision)
		*pOutCollision = Pos1;
	if(pOutBeforeCollision)
		*pOutBeforeCollision = Pos1;
	return 0;
}

// TODO: OPT: rewrite this smarter!
void CCollision::MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces, int *pCollide)
{
	if(pBounces)
		*pBounces = 0;

	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;
	if(CheckPoint(Pos + Vel))
	{
		int Affected = 0;
		if(CheckPoint(Pos.x + Vel.x, Pos.y))
		{
			pInoutVel->x *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;

			if (pCollide) *pCollide = 1; //H-Client
		}

		if(CheckPoint(Pos.x, Pos.y + Vel.y))
		{
			pInoutVel->y *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;

			if (pCollide) *pCollide = 1; //H-Client
		}

		if(Affected == 0)
		{
			pInoutVel->x *= -Elasticity;
			pInoutVel->y *= -Elasticity;
		}
	}
	else
	{
		*pInoutPos = Pos + Vel;
	}
}

bool CCollision::TestBox(vec2 Pos, vec2 Size)
{
	Size *= 0.5f;
	if(CheckPoint(Pos.x-Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x-Size.x, Pos.y+Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y+Size.y))
		return true;
	return false;
}

void CCollision::MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity)
{
	// do the move
	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;

	float Distance = length(Vel);
	int Max = (int)Distance;

	if(Distance > 0.00001f)
	{
		//vec2 old_pos = pos;
		float Fraction = 1.0f/(float)(Max+1);
		for(int i = 0; i <= Max; i++)
		{
			//float amount = i/(float)max;
			//if(max == 0)
				//amount = 0;

			vec2 NewPos = Pos + Vel*Fraction; // TODO: this row is not nice

			if(TestBox(vec2(NewPos.x, NewPos.y), Size))
			{
				int Hits = 0;

				if(TestBox(vec2(Pos.x, NewPos.y), Size))
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					Hits++;
				}

				if(TestBox(vec2(NewPos.x, Pos.y), Size))
				{
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
					Hits++;
				}

				// neither of the tests got a collision.
				// this is a real _corner case_!
				if(Hits == 0)
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
				}
			}

			Pos = NewPos;
		}
	}

	*pInoutPos = Pos;
	*pInoutVel = Vel;
}

//H-Client: Ghost & DDRace Stuff
int CCollision::DestroyTile(vec2 Pos)
{
    int LIndex = -1;

    if (!m_pLayers->MineTeeLayer())
        return LIndex;

    Pos = vec2(static_cast<int>(Pos.x/32.0f), static_cast<int>(Pos.y/32.0f));
    if (Pos.x >= m_Width-1 || Pos.x <= 0 || Pos.y >= m_Height-1 || Pos.y <= 0)
        return LIndex;

    //MineTee Layer
    int MTIndex = static_cast<int>(Pos.y*m_pLayers->MineTeeLayer()->m_Width+Pos.x);
    CTile *pMTTiles = (CTile *)m_pLayers->Map()->GetData(m_pLayers->MineTeeLayer()->m_Data);
    LIndex = pMTTiles[MTIndex].m_Index;
    pMTTiles[MTIndex].m_Flags = 0x0;
    pMTTiles[MTIndex].m_Index = 0;

    //GameLayer
    int Index = static_cast<int>(Pos.y*m_Width+Pos.x);
    m_pTiles[Index].m_Flags = 0x0;
    m_pTiles[Index].m_Index = 0;

    //Buffer it
    CNetMsg_Sv_TileChangeExt TileChange;
    TileChange.m_Index = -1;
    TileChange.m_Size = -1;
    TileChange.m_Act = TILE_DESTROY;
    TileChange.m_X = static_cast<int>(Pos.x);
    TileChange.m_Y = static_cast<int>(Pos.y);
    TileChange.m_ITile = 0;
    TileChange.m_State = 0;

    if (!CheckTileChangeBuffer(TileChange))
        m_pLayers->m_BuffNetTileChange.push_back(TileChange);

    return LIndex;
}

void CCollision::CreateTile(vec2 Pos, int ITile, int Type, int State)
{
    if (!m_pLayers->MineTeeLayer() || !m_pLayers->MineTeeBGLayer() || !m_pLayers->MineTeeFGLayer())
        return;

    Pos = vec2(static_cast<int>(Pos.x/32.0f), static_cast<int>(Pos.y/32.0f));
    if (Pos.x >= m_pLayers->MineTeeLayer()->m_Width-1 || Pos.x <= 0 || Pos.y >= m_pLayers->MineTeeLayer()->m_Height-1 || Pos.y <= 0)
        return;

    //MineTee Layer
    if (State == 0)
    {
        int MTIndex = static_cast<int>(Pos.y*m_Width+Pos.x);
        CTile *pMTTiles = (CTile *)m_pLayers->Map()->GetData(m_pLayers->MineTeeLayer()->m_Data);
        pMTTiles[MTIndex].m_Flags = 0x0;
        pMTTiles[MTIndex].m_Index = ITile;
    }
    else if (State == 1)
    {
        int MTIndex = static_cast<int>(Pos.y*m_Width+Pos.x);
        CTile *pMTBGTiles = (CTile *)m_pLayers->Map()->GetData(m_pLayers->MineTeeBGLayer()->m_Data);
        pMTBGTiles[MTIndex].m_Flags = 0x0;
        pMTBGTiles[MTIndex].m_Index = ITile;
    }
    else if (State == 2)
    {
        int MTIndex = static_cast<int>(Pos.y*m_Width+Pos.x);
        CTile *pMTFGTiles = (CTile *)m_pLayers->Map()->GetData(m_pLayers->MineTeeFGLayer()->m_Data);
        pMTFGTiles[MTIndex].m_Flags = 0x0;
        pMTFGTiles[MTIndex].m_Index = ITile;
    }

    //Game Layer
    int Index = Pos.y*m_pLayers->MineTeeLayer()->m_Width+Pos.x;
    m_pTiles[Index].m_Flags = 0x0;
    m_pTiles[Index].m_Index = Type;

    //Buffer it
    CNetMsg_Sv_TileChangeExt TileChange;
    TileChange.m_Size = -1;
    TileChange.m_Index = -1;
    TileChange.m_Act = TILE_CREATE;
    TileChange.m_X = static_cast<int>(Pos.x);
    TileChange.m_Y = static_cast<int>(Pos.y);
    TileChange.m_ITile = ITile;
    if (Type&CCollision::COLFLAG_SOLID)
        TileChange.m_Col = 1;
    else
        TileChange.m_Col = 0;

    TileChange.m_State = State;

    if (!CheckTileChangeBuffer(TileChange))
        m_pLayers->m_BuffNetTileChange.push_back(TileChange);
}

bool CCollision::CheckTileChangeBuffer(CNetMsg_Sv_TileChangeExt tile)
{
    bool delNetTile=false;
    bool found=false;
    std::deque<CNetMsg_Sv_TileChangeExt>::iterator it = m_pLayers->m_BuffNetTileChange.begin();
    while(it != m_pLayers->m_BuffNetTileChange.end())
    {
        CNetMsg_Sv_TileChangeExt *TileExt = static_cast<CNetMsg_Sv_TileChangeExt*>(&(*it));
        if (TileExt->m_X == tile.m_X && TileExt->m_Y == tile.m_Y && TileExt->m_State == tile.m_State)
        {
            m_pLayers->m_BuffNetTileChange.erase(it);

            int index=tile.m_Y*m_Width+tile.m_X;
            if (tile.m_Act == TILE_CREATE && m_pLayers->MineTeeOrigin()[index].m_Index == tile.m_ITile)
                return true;
            else if (tile.m_Act == TILE_DESTROY && m_pLayers->MineTeeOrigin()[index].m_Index == 0)
                return true;

            return false;
        }
        else
            it++;
    }

    return false;
}

int CCollision::GetMineTeeBlockAt(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);
	if(!m_pMineTeeTiles || Ny < 0 || Nx < 0)
		return 0;

    return m_pMineTeeTiles[Ny*m_Width+Nx].m_Index;

	return 0;
}

int CCollision::GetTileIndex(int Index)
{
	if(Index < 0)
		return 0;

	return m_pTiles[Index].m_Index;
}

int CCollision::GetPureMapIndex(vec2 Pos)
{
	int Nx = clamp((int)Pos.x/32, 0, m_Width-1);
	int Ny = clamp((int)Pos.y/32, 0, m_Height-1);

	return Ny*m_Width+Nx;
}
std::list<int> CCollision::GetMapIndices(vec2 PrevPos, vec2 Pos, unsigned MaxIndices)
{
	std::list< int > Indices;
	float d = distance(PrevPos, Pos);
	int End(d + 1);
	if(!d)
	{
		int Nx = clamp((int)Pos.x / 32, 0, m_Width - 1);
		int Ny = clamp((int)Pos.y / 32, 0, m_Height - 1);
		int Index = Ny * m_Width + Nx;

		if(TileExists(Index))
		{
			Indices.push_back(Index);
			return Indices;
		}
		else
			return Indices;
	}
	else
	{
		float a = 0.0f;
		vec2 Tmp = vec2(0, 0);
		int Nx = 0;
		int Ny = 0;
		int Index,LastIndex = 0;
		for(int i = 0; i < End; i++)
		{
			a = i/d;
			Tmp = mix(PrevPos, Pos, a);
			Nx = clamp((int)Tmp.x / 32, 0, m_Width - 1);
			Ny = clamp((int)Tmp.y / 32, 0, m_Height - 1);
			Index = Ny * m_Width + Nx;
			//dbg_msg("lastindex","%d",LastIndex);
			//dbg_msg("index","%d",Index);
			if(TileExists(Index) && LastIndex != Index)
			{
				if(MaxIndices && Indices.size() > MaxIndices)
					return Indices;
				Indices.push_back(Index);
				LastIndex = Index;
				//dbg_msg("pushed","%d",Index);
			}
		}

		return Indices;
	}
}
bool CCollision::TileExists(int Index)
{
	if(Index < 0)
		return false;

	if(m_pTiles[Index].m_Index >= TILE_FREEZE && m_pTiles[Index].m_Index <= TILE_END)
		return true;
	if(m_pFront && m_pFront[Index].m_Index >= TILE_FREEZE && m_pFront[Index].m_Index  <= TILE_END)
		return true;

	return false;
}

int CCollision::IsThrough(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);
	int Index = m_pTiles[Ny*m_Width+Nx].m_Index;
	int Findex = 0;
	if (m_pFront)
		Findex = m_pFront[Ny*m_Width+Nx].m_Index;

    if (Index == TILE_THROUGH)
        return Index;
    if (Findex == TILE_THROUGH)
        return Findex;

	return 0;
}

void ThroughOffset(vec2 Pos0, vec2 Pos1, int *Ox, int *Oy)
{
	float x = Pos0.x - Pos1.x;
	float y = Pos0.y - Pos1.y;
	if (fabs(x) > fabs(y))
	{
		if (x < 0)
		{
			*Ox = -32;
			*Oy = 0;
		}
		else
		{
			*Ox = 32;
			*Oy = 0;
		}
	}
	else
	{
		if (y < 0)
		{
			*Ox = 0;
			*Oy = -32;
		}
		else
		{
			*Ox = 0;
			*Oy = 32;
		}
	}
}
