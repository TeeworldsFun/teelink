/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <base/vmath.h>
// H-Client: DDNet
#include <map>
#include <vector>
//

class CCollision
{
	class CTile *m_pTiles;
	int m_Width;
	int m_Height;
	class CLayers *m_pLayers;

	bool IsTileSolid(int x, int y);
	int GetTile(int x, int y);

	// H-Client: DDNet
    class CTile *m_pFront;
	class CTeleTile *m_pTele;
	class CSpeedUpTile *m_pSpeedUp;
	std::map<int, std::vector<vec2> > m_TeleOuts;
	bool IsTileFreeze(int x, int y);
	int IsSpeedUp(int Index);
	void InitTeleports();
	//

public:
	enum
	{
		COLFLAG_SOLID=1,
		COLFLAG_DEATH=2,
		COLFLAG_NOHOOK=4,
		COLFLAG_FREEZE=8, // H-Client: DDNet

		COLFLAG_TELE=32, // H-Client: DDNet
	};

	CCollision();
	void Init(class CLayers *pLayers);
	bool CheckPoint(float x, float y, bool nocoll=true) { return IsTileSolid(round(x), round(y)); }
	bool CheckPoint(vec2 Pos, bool nocoll=true) { return CheckPoint(Pos.x, Pos.y); }
	int GetCollisionAt(float x, float y) { return GetTile(round(x), round(y)); }
	int GetWidth() { return m_Width; };
	int GetHeight() { return m_Height; };
	int IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, bool AllowThrough = false);
	void MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces, int *pCollide = 0);
	void MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity);
	bool TestBox(vec2 Pos, vec2 Size);

	//H-Client: DDRace
	bool CheckPointFreeze(float x, float y) { return IsTileFreeze(round(x), round(y)); }
	bool CheckPointFreeze(vec2 Pos) { return CheckPointFreeze(Pos.x, Pos.y); }
	bool CheckPointSpeedUp(vec2 Pos) { return IsSpeedUp(GetPureMapIndex(Pos)); }

	int IsThrough(int x, int y);
	int GetTileIndex(int Index);
    int GetPureMapIndex(vec2 Pos);
    void GetSpeedUp(int Index, vec2 *Dir, int *Force, int *MaxSpeed);
    int IsTeleportHook(int Index);
    int IsTeleport(int Index);
    int IntersectLineTeleHook(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, int *pTeleNr, bool AllowThrough);
    std::map<int, std::vector<vec2> > *GetTeleOuts() { return &m_TeleOuts; }

    void CreateTile(vec2 pos, int group, int layer, int index, int flags); // Android
};

void ThroughOffset(vec2 Pos0, vec2 Pos1, int *Ox, int *Oy); //H-Client: DDRace

#endif
