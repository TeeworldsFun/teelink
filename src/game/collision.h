/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <base/math.h>
#include <base/vmath.h>
#include <game/generated/protocol.h>
#include <list>

class CCollision
{
	class CTile *m_pTiles;
	class CTile *m_pMineTeeTiles; //H-Client
	class CTile *m_pFront;
	int m_Width;
	int m_Height;
	class CLayers *m_pLayers;

	bool IsTileSolid(int x, int y, bool nocoll);
	int GetTile(int x, int y);

	bool CheckTileChangeBuffer(CNetMsg_Sv_TileChangeExt tile); //H-Client

public:
    int *m_pSecBlocks; //H-Client

	enum
	{
		COLFLAG_SOLID=1,
		COLFLAG_DEATH=2,
		COLFLAG_NOHOOK=4,
		//H-Client: DDRace
		COLFLAG_THROUGH=16
	};

	CCollision();
	void Init(class CLayers *pLayers);
	bool CheckPoint(float x, float y, bool nocoll=true) { return IsTileSolid(round(x), round(y), nocoll); }
	bool CheckPoint(vec2 Pos, bool nocoll=true) { return CheckPoint(Pos.x, Pos.y, nocoll); }
	int GetCollisionAt(float x, float y) { return GetTile(round(x), round(y)); }
	int GetWidth() { return m_Width; };
	int GetHeight() { return m_Height; };
	int IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, bool AllowThrough = false);
	void MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces, int *collide = 0x0);
	void MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity);
	bool TestBox(vec2 Pos, vec2 Size);

	//H-Client: Ghost Stuff & DDRace Stuff
	int DestroyTile(vec2 Pos);
	void CreateTile(vec2 Pos, int ITile, int Type = CCollision::COLFLAG_SOLID, int State = 0);
	int GetTileIndex(int index);
	int GetPureMapIndex(vec2 Pos);
    std::list<int> GetMapIndices(vec2 PrevPos, vec2 Pos, unsigned MaxIndices = 0);
    bool TileExists(int Index);
	bool TileExistsNext(int Index);
	int IsThrough(int x, int y);
	int GetMineTeeBlockAt(int x, int y);
};

void ThroughOffset(vec2 Pos0, vec2 Pos1, int *Ox, int *Oy); //H-Client: DDRace
#endif
