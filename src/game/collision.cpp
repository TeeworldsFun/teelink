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

CCollision::CCollision()
{
	m_pTiles = 0;
	m_Width = 0;
	m_Height = 0;
	m_pLayers = 0;

	// H-Client: DDNet
	m_pFront = 0x0;
	m_pTele = 0x0;
	m_pSpeedUp = 0x0;
	m_pDoor = 0x0;
	m_pSwitchers = 0x0;
	m_NumSwitchers = 0;
	//
}
CCollision::~CCollision()
{
	if (m_pDoor)
		delete m_pDoor; // H-Client
}

void CCollision::Init(class CLayers *pLayers)
{
	// H-Client: DDNet
    m_pFront = 0x0;
    m_pTele = 0x0;
    m_pSpeedUp = 0x0;
    m_pDoor = 0x0;
    m_pSwitchers = 0x0;
    m_NumSwitchers = 0;
    //

	m_pLayers = pLayers;
	m_Width = m_pLayers->GameLayer()->m_Width;
	m_Height = m_pLayers->GameLayer()->m_Height;
	m_pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameLayer()->m_Data));

	// H-Client: DDNet
	if(m_pLayers->FrontLayer())
		m_pFront = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->FrontLayer()->m_Front));
	if(m_pLayers->TeleLayer())
	{
		unsigned int Size = m_pLayers->Map()->GetUncompressedDataSize(m_pLayers->TeleLayer()->m_Tele);
		if (Size >= m_Width*m_Height*sizeof(CTeleTile))
			m_pTele = static_cast<CTeleTile *>(m_pLayers->Map()->GetData(m_pLayers->TeleLayer()->m_Tele));
	}
	if(m_pLayers->SpeedUpLayer())
	{
		unsigned int Size = m_pLayers->Map()->GetUncompressedDataSize(m_pLayers->SpeedUpLayer()->m_SpeedUp);
		if (Size >= m_Width*m_Height*sizeof(CSpeedUpTile))
			m_pSpeedUp = static_cast<CSpeedUpTile *>(m_pLayers->Map()->GetData(m_pLayers->SpeedUpLayer()->m_SpeedUp));
	}
	if(m_pLayers->SwitchLayer())
	{
		unsigned int Size = m_pLayers->Map()->GetUncompressedDataSize(m_pLayers->SwitchLayer()->m_Switch);
		if (Size >= m_Width*m_Height*sizeof(CSwitchTile))
			m_pSwitch = static_cast<CSwitchTile *>(m_pLayers->Map()->GetData(m_pLayers->SwitchLayer()->m_Switch));

		if (m_pDoor)
			delete m_pDoor;

		m_pDoor = new CDoorTile[m_Width*m_Height];
		mem_zero(m_pDoor, m_Width * m_Height * sizeof(CDoorTile));
	}

	for(int i = 0; i < m_Width*m_Height; i++)
	{
		int Index;

		if(m_pSwitch)
		{
			if(m_pSwitch[i].m_Number > m_NumSwitchers)
				m_NumSwitchers = m_pSwitch[i].m_Number;

			m_pDoor[i].m_Number = m_pSwitch[i].m_Number;

			Index = m_pSwitch[i].m_Type;
			if(Index <= TILE_NPH_START)
				m_pSwitch[i].m_Type = (Index >= TILE_JUMP && Index <= TILE_BONUS)?Index:0;
		}
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
                case TILE_FREEZE:
                    m_pFront[i].m_Index = COLFLAG_FREEZE;
                    break;
                default:
                    m_pFront[i].m_Index = 0;
            }

            if(Index == TILE_THROUGH || (Index > TILE_FREEZE && Index <= TILE_UNFREEZE) || (Index >= TILE_BEGIN && Index <= TILE_STOPA))
                m_pFront[i].m_Index = Index;
        }

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
		case TILE_FREEZE:
			m_pTiles[i].m_Index = COLFLAG_FREEZE;
			break;
		default:
			m_pTiles[i].m_Index = 0;
		}

        //H-Client: DDRace Stuff
        if(Index == TILE_THROUGH || (Index > TILE_FREEZE && Index <= TILE_UNFREEZE) || (Index >= TILE_BEGIN && Index <= TILE_STOPA))
        	m_pTiles[i].m_Index = Index;
	}

	InitTeleports(); // H-Client: DDNet
	InitSwitchers(); // H-Client: DDNet
}

// H-Client: DDNet
void CCollision::InitTeleports()
{
    if (!m_pLayers->TeleLayer())
        return;

    int Width = m_pLayers->TeleLayer()->m_Width;
    int Height = m_pLayers->TeleLayer()->m_Height;

    for (int i = 0; i < Width * Height; i++)
    {
        if (m_pTele[i].m_Number > 0 && m_pTele[i].m_Type == TILE_TELEOUT)
            m_TeleOuts[m_pTele[i].m_Number - 1].push_back(vec2(i % Width * 32 + 16, i / Width * 32 + 16));
    }
}
void CCollision::InitSwitchers()
{
	if(m_NumSwitchers)
	{
		m_pSwitchers = new SSwitchers[m_NumSwitchers+1];

		for (int i = 0; i < m_NumSwitchers+1; ++i)
		{
			for (int j = 0; j < MAX_CLIENTS; ++j)
			{
				m_pSwitchers[i].m_Status[j] = true;
				m_pSwitchers[i].m_EndTick[j] = 0;
				m_pSwitchers[i].m_Type[j] = 0;
			}
		}
	}
}
//

int CCollision::GetTile(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);
	if(!m_pTiles || Ny < 0 || Nx < 0)
		return 0;

	if(m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_SOLID
		|| m_pTiles[Ny*m_Width+Nx].m_Index == (COLFLAG_SOLID|COLFLAG_NOHOOK)
		|| m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_DEATH
		|| m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_FREEZE)
		return m_pTiles[Ny*m_Width+Nx].m_Index;

	return 0;
}

bool CCollision::IsTileSolid(int x, int y)
{
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

		if(CheckPoint(Pos.x, Pos.y) && !(AllowThrough && IsThrough(ix + dx, iy + dy)))
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

int CCollision::IntersectLineTeleHook(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, int *pTeleNr, bool AllowThrough)
{
    float Distance = distance(Pos0, Pos1);
    int End(Distance+1);
    vec2 Last = Pos0;
    int ix = 0, iy = 0; // Temporary position for checking collision
    int dx = 0, dy = 0; // Offset for checking the "through" tile

    if (AllowThrough)
        ThroughOffset(Pos0, Pos1, &dx, &dy);

    for(int i = 0; i < End; i++)
    {
        float a = i/(float)End;
        vec2 Pos = mix(Pos0, Pos1, a);
        ix = round(Pos.x);
        iy = round(Pos.y);
        int Nx = clamp(ix/32, 0, m_Width-1);
        int Ny = clamp(iy/32, 0, m_Height-1);

        *pTeleNr = IsTeleportHook(Ny*m_Width+Nx);

        if(*pTeleNr)
        {
            if(pOutCollision)
                *pOutCollision = Pos;
            if(pOutBeforeCollision)
                *pOutBeforeCollision = Last;

            return COLFLAG_TELE;
        }
        if((CheckPoint(ix, iy) && !(AllowThrough && IsThrough(ix + dx, iy + dy))))
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

int CCollision::IsTeleportHook(int Index)
{
    if(Index < 0 || !m_pTele)
        return 0;

    if(m_pTele[Index].m_Type == TILE_TELEINHOOK)
        return m_pTele[Index].m_Number;

    return 0;
}

int CCollision::IsTeleport(int Index)
{
	if(Index < 0 || !m_pTele)
		return 0;

	if(m_pTele[Index].m_Type == TILE_TELEIN)
		return m_pTele[Index].m_Number;

	return 0;
}

bool CCollision::IsTileFreeze(int x, int y)
{
	return GetTile(x, y)&COLFLAG_FREEZE;
}

int CCollision::IsSpeedUp(int Index)
{
	if(!m_pSpeedUp || Index < 0 || 	Index >= m_pLayers->SpeedUpLayer()->m_Width*m_pLayers->SpeedUpLayer()->m_Height)
		return 0;

	return m_pSpeedUp[Index].m_Force;
}

void CCollision::GetSpeedUp(int Index, vec2 *Dir, int *Force, int *MaxSpeed)
{
	if(Index < 0 || !m_pSpeedUp)
		return;
	float Angle = m_pSpeedUp[Index].m_Angle * (pi / 180.0f);
	*Force = m_pSpeedUp[Index].m_Force;
	*Dir = vec2(cos(Angle), sin(Angle));
	if(MaxSpeed)
		*MaxSpeed = m_pSpeedUp[Index].m_MaxSpeed;
}

bool CCollision::GetRadTiles(int tilemap, vec2 pos, int *index, int *flags, int team)
{
	static const float sProximityRadius = 14.0f; // FIXME: Perhaps best TILE_SIZE/2?
	static const float sOffset = 4.0f;

	int tmpIndex[] = {
			GetPureMapIndex(pos), // Current
			GetPureMapIndex(vec2(pos.x + sProximityRadius + sOffset, pos.y)), // Left
			GetPureMapIndex(vec2(pos.x - sProximityRadius - sOffset, pos.y)), // Right
			GetPureMapIndex(vec2(pos.x, pos.y + sProximityRadius + sOffset)), // Top
			GetPureMapIndex(vec2(pos.x, pos.y - sProximityRadius - sOffset)) // Bottom
	};

	if (tilemap == TILEMAP_SWITCH && m_pSwitchers)
	{
		for (int i=0; i<5; i++)
		{
			index[i] = m_pSwitchers[m_pDoor[tmpIndex[i]].m_Number].m_Status[team]?m_pDoor[tmpIndex[i]].m_Index:0;
			flags[i] = m_pSwitchers[m_pDoor[tmpIndex[i]].m_Number].m_Status[team]?m_pDoor[tmpIndex[i]].m_Flags:0;
		}
	}
	else if (tilemap == TILEMAP_GAME && m_pTiles)
	{
		for (int i=0; i<5; i++)
		{
			index[i] = m_pTiles[tmpIndex[i]].m_Index;
			flags[i] = m_pTiles[tmpIndex[i]].m_Flags;
		}
	}
	else if (tilemap == TILEMAP_FRONT && m_pFront)
	{
		for (int i=0; i<5; i++)
		{
			index[i] = m_pFront[tmpIndex[i]].m_Index;
			flags[i] = m_pFront[tmpIndex[i]].m_Flags;
		}
	}
	else
		return false;

	return true;
}

vec2 CCollision::GetPos(int Index)
{
	if(Index < 0)
		return vec2(0,0);

	int x = Index%m_Width;
	int y = Index/m_Width;
	return vec2(x*32+16, y*32+16);
}

// Android Mapper
void CCollision::CreateTile(vec2 pos, int group, int layer, int index, int flags)
{
    CMapItemGroup *pGroup = m_pLayers->GetGroup(group);
    CMapItemLayer *pLayer = m_pLayers->GetLayer(pGroup->m_StartLayer+layer);
    if (pLayer->m_Type != LAYERTYPE_TILES) // protect against the dark side people
        return;

    CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);
    int tpos = (int)pos.y*pTilemap->m_Width+(int)pos.x;
    if (tpos < 0 || tpos >= pTilemap->m_Width*pTilemap->m_Height) // protect against the dark side people
        return;

    if (pTilemap != m_pLayers->GameLayer())
    {
        CTile *pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(pTilemap->m_Data));
        pTiles[tpos].m_Flags = flags;
        pTiles[tpos].m_Index = index;
    }
    else
    {
        m_pTiles[tpos].m_Index = index;
        m_pTiles[tpos].m_Flags = flags;

        switch(index)
        {
        case TILE_DEATH:
            m_pTiles[tpos].m_Index = COLFLAG_DEATH;
            break;
        case TILE_SOLID:
            m_pTiles[tpos].m_Index = COLFLAG_SOLID;
            break;
        case TILE_NOHOOK:
            m_pTiles[tpos].m_Index = COLFLAG_SOLID|COLFLAG_NOHOOK;
            break;
        default:
            if(index <= 128)
                m_pTiles[tpos].m_Index = 0;
        }
    }
}

