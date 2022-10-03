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
}

void CCollision::Init(class CLayers *pLayers)
{
	m_pLayers = pLayers;
	m_Width = m_pLayers->GameLayer()->m_Width;
	m_Height = m_pLayers->GameLayer()->m_Height;
	m_pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameLayer()->m_Data));

	for(int i = 0; i < m_Width*m_Height; i++)
	{
		int Index = m_pTiles[i].m_Index;
		m_pTiles[i].m_Index = 0;

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
		}
	}
}

int CCollision::GetTile(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);

	return m_pTiles[Ny*m_Width+Nx].m_Index > 128 ? 0 : m_pTiles[Ny*m_Width+Nx].m_Index;
}

bool CCollision::IsTileSolid(int x, int y)
{
	return GetTile(x, y)&COLFLAG_SOLID;
}


// TODO: rewrite this smarter!
int CCollision::IntersectLine(const vec2 &Pos0, const vec2 &Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision)
{
	float Distance = distance(Pos0, Pos1);
	int End(Distance+1);
	float a = 0.0f;
	vec2 Last = Pos0;
	int ix = 0, iy = 0; // Temporary position for checking collision
	for(int i = 0; i <= End; i++)
	{
		a = i/(float)End;
		const vec2 Pos = mix(Pos0, Pos1, a);
		ix = round(Pos.x);
		iy = round(Pos.y);

		if(CheckPoint(ix, iy))
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
	if (pCollide)
		*pCollide = 0;

	const vec2 &Pos = *pInoutPos;
	const vec2 &Vel = *pInoutVel;
	if(CheckPoint(Pos + Vel))
	{
		int Affected = 0;
		if(CheckPoint(Pos.x + Vel.x, Pos.y))
		{
			pInoutVel->x *= -Elasticity;
			if(pBounces)
				++(*pBounces);
			++Affected;

			if (pCollide) *pCollide = 1; //H-Client
		}

		if(CheckPoint(Pos.x, Pos.y + Vel.y))
		{
			pInoutVel->y *= -Elasticity;
			if(pBounces)
				++(*pBounces);
			++Affected;

			if (pCollide) *pCollide = 2; //H-Client
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

bool CCollision::TestBox(const vec2 &Pos, const vec2 &Size)
{
	const vec2 tmpSize = Size*0.5f;
	if(CheckPoint(Pos.x-tmpSize.x, Pos.y-tmpSize.y) ||
			CheckPoint(Pos.x+tmpSize.x, Pos.y-tmpSize.y) ||
			CheckPoint(Pos.x-tmpSize.x, Pos.y+tmpSize.y) ||
			CheckPoint(Pos.x+tmpSize.x, Pos.y+tmpSize.y))
		return true;
	return false;
}

void CCollision::MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, const vec2 &Size, float Elasticity)
{
	// do the move
	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;

	const float Distance = length(Vel);
	const int Max = (int)Distance;

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
					++Hits;
				}

				if(TestBox(vec2(NewPos.x, Pos.y), Size))
				{
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
					++Hits;
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

bool CCollision::IsTileFreeze(int x, int y)
{
	return GetTile(x, y)&COLFLAG_FREEZE;
}

std::list<int> CCollision::GetMapIndices(const vec2 &PrevPos, const vec2 &Pos, unsigned MaxIndices)
{
	std::list< int > Indices;
	const float d = distance(PrevPos, Pos);
	const int End(d + 1);
	if(!d)
	{
		const int Nx = clamp((int)Pos.x / 32, 0, m_Width - 1);
		const int Ny = clamp((int)Pos.y / 32, 0, m_Height - 1);
		const int Index = Ny * m_Width + Nx;

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
		int Index, LastIndex = 0;
		for(int i = 0; i < End; i++)
		{
			a = i/d;
			Tmp = mix(PrevPos, Pos, a);
			Nx = clamp((int)Tmp.x / 32, 0, m_Width - 1);
			Ny = clamp((int)Tmp.y / 32, 0, m_Height - 1);
			Index = Ny * m_Width + Nx;
			if(TileExists(Index) && LastIndex != Index)
			{
				if(MaxIndices && Indices.size() > MaxIndices)
					return Indices;
				Indices.push_back(Index);
				LastIndex = Index;
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

	return false;
}

// Android Mapper
void CCollision::CreateTile(const vec2 &pos, int group, int layer, int index, int flags)
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

