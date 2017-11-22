/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <base/vmath.h>
// H-Client: DDNet
#include <engine/shared/protocol.h>
#include <map>
#include <vector>
#include <list> // Ghost
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
	class CDoorTile *m_pDoor;
	class CSwitchTile *m_pSwitch;
	struct SSwitchers
	{
		bool m_Status[MAX_CLIENTS];
		bool m_Initial;
		int m_EndTick[MAX_CLIENTS];
		int m_Type[MAX_CLIENTS];
	} *m_pSwitchers;
	std::map<int, std::vector<vec2> > m_TeleOuts;
	std::size_t m_NumSwitchers;
	bool IsTileFreeze(int x, int y);
	int IsSpeedUp(int Index);
	void InitTeleports();
	void InitSwitchers();
	//

public:
	enum
	{
		COLFLAG_SOLID=1,
		COLFLAG_DEATH=2,
		COLFLAG_NOHOOK=4,

		//H-Client: DDNet
		COLFLAG_FREEZE=8, // H-Client: DDNet
		COLFLAG_TELE=32, // H-Client: DDNet

		TILEMAP_GAME=0,
		TILEMAP_FRONT,
		TILEMAP_TELE,
		TILEMAP_SPEEDUP,
		TILEMAP_SWITCH
	};

	CCollision();
	~CCollision();	// H-Client

	void Init(class CLayers *pLayers);
	bool CheckPoint(float x, float y, bool nocoll=true) { return IsTileSolid(round(x), round(y)); }
	bool CheckPoint(const vec2 &Pos, bool nocoll=true) { return CheckPoint(Pos.x, Pos.y); }
	int GetCollisionAt(float x, float y) { return GetTile(round(x), round(y)); }
	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }
	int IntersectLine(const vec2 &Pos0, const vec2 &Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision);
	void MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces, int *pCollide = 0);
	void MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, const vec2 &Size, float Elasticity);
	bool TestBox(const vec2 &Pos, const vec2 &Size);

	//H-Client: DDRace
	bool CheckPointFreeze(float x, float y) { return IsTileFreeze(round(x), round(y)); }
	bool CheckPointFreeze(const vec2 &Pos) { return CheckPointFreeze(Pos.x, Pos.y); }
	bool CheckPointSpeedUp(const vec2 &Pos) { return IsSpeedUp(GetPureMapIndex(Pos)); }

	int GetFTileIndex(int Index);
	int GetFTileFlags(int Index);

	int GetTileIndex(int Index);
	int GetTileFlags(int Index);

	bool IsThrough(int x, int y, int xoff, int yoff, const vec2 &pos0, const vec2 &pos1);
    int GetPureMapIndex(const vec2 &Pos);
    vec2 GetPos(int Index);
    void GetSpeedUp(int Index, vec2 *Dir, int *Force, int *MaxSpeed);
    bool GetRadTiles(int tilemap, const vec2 &pos, int *index, int *flags, int team = 0);
    int IsTeleportHook(int Index);
    bool IsHookBlocker(const vec2 &tilePos, vec2 pos0, vec2 pos1);
    int IsTeleport(int Index);
    int IntersectLineTeleHook(const vec2 &Pos0, const vec2 &Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, int *pTeleNr);
    std::map<int, std::vector<vec2> > *GetTeleOuts() { return &m_TeleOuts; }
    std::list<int> GetMapIndices(const vec2 &PrevPos, const vec2 &Pos, unsigned MaxIndices = 0); // Ghost
    bool TileExists(int Index); // Ghost

    void CreateTile(const vec2 &pos, int group, int layer, int index, int flags); // Android
};

void ThroughOffset(vec2 Pos0, vec2 Pos1, int *Ox, int *Oy); //H-Client: DDRace

#endif
