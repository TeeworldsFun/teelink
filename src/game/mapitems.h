/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_MAPITEMS_H
#define GAME_MAPITEMS_H

// layer types
enum
{
	LAYERTYPE_INVALID=0,
	LAYERTYPE_GAME,
	LAYERTYPE_TILES,
	LAYERTYPE_QUADS,

	MAPITEMTYPE_VERSION=0,
	MAPITEMTYPE_INFO,
	MAPITEMTYPE_IMAGE,
	MAPITEMTYPE_ENVELOPE,
	MAPITEMTYPE_GROUP,
	MAPITEMTYPE_LAYER,
	MAPITEMTYPE_ENVPOINTS,


	CURVETYPE_STEP=0,
	CURVETYPE_LINEAR,
	CURVETYPE_SLOW,
	CURVETYPE_FAST,
	CURVETYPE_SMOOTH,
	NUM_CURVETYPES,

	// game layer tiles
	ENTITY_NULL=0,
	ENTITY_SPAWN,
	ENTITY_SPAWN_RED,
	ENTITY_SPAWN_BLUE,
	ENTITY_FLAGSTAND_RED,
	ENTITY_FLAGSTAND_BLUE,
	ENTITY_ARMOR_1,
	ENTITY_HEALTH_1,
	ENTITY_WEAPON_SHOTGUN,
	ENTITY_WEAPON_GRENADE,
	ENTITY_POWERUP_NINJA,
	ENTITY_WEAPON_RIFLE,
	NUM_ENTITIES,

	TILE_AIR=0,
	TILE_SOLID,
	TILE_DEATH,
	TILE_NOHOOK,
    TILE_NOLASER,
	TILE_THROUGH_CUT,
	TILE_THROUGH,
	TILE_JUMP,
	TILE_FREEZE = 9,
	TILE_TELEINEVIL,
	TILE_UNFREEZE,
	TILE_DFREEZE,
	TILE_DUNFREEZE,
	TILE_TELEINWEAPON,
	TILE_TELEINHOOK,
	TILE_ENDLESS_HOOK_ON=17,
	TILE_ENDLEES_HOOK_OFF,

	//Switches
	TILE_SWITCHTIMEDOPEN = 22,
	TILE_SWITCHTIMEDCLOSE,
	TILE_SWITCHOPEN,
	TILE_SWITCHCLOSE,
	TILE_TELEIN,
	TILE_TELEOUT,
	TILE_BOOST,
	TILE_TELECHECK,
	TILE_TELECHECKOUT,
	TILE_TELECHECKIN,
	TILE_REFILL_JUMPS = 32,
	TILE_BEGIN,
	TILE_END,
	TILE_STOP = 60,
	TILE_STOPS,
	TILE_STOPA,
	TILE_TELECHECKINEVIL = 63,
	TILE_CP = 64,
	TILE_CP_F,
	TILE_THROUGH_ALL,
	TILE_THROUGH_DIR,
	TILE_TUNE1,
	TILE_OLDLASER = 71,
	TILE_NPC,
	TILE_EHOOK,
	TILE_NOHIT,
	TILE_NPH,
	TILE_PENALTY = 79,
	TILE_NPC_END = 88,
	TILE_SUPER_END,
	TILE_JETPACK_END,
	TILE_NPH_END,
	TILE_BONUS = 95,
	TILE_NPC_START = 104,
	TILE_SUPER_START,
	TILE_JETPACK_START,
	TILE_NPH_START,
	//End of higher tiles
	//Layers
	LAYER_GAME=0,
	LAYER_FRONT,
	LAYER_TELE,
	LAYER_SPEEDUP,
	LAYER_SWITCH,
	NUM_LAYERS,

	TILEFLAG_VFLIP=1,
	TILEFLAG_HFLIP=2,
	TILEFLAG_OPAQUE=4,
	TILEFLAG_ROTATE=8,

	// H-Client: DDNet
	ROTATION_0 = 0,
	ROTATION_90 = TILEFLAG_ROTATE,
	ROTATION_180 = (TILEFLAG_VFLIP|TILEFLAG_HFLIP),
	ROTATION_270 = (TILEFLAG_VFLIP|TILEFLAG_HFLIP|TILEFLAG_ROTATE),
	//

	LAYERFLAG_DETAIL=1,
	TILESLAYERFLAG_GAME=1,
	TILESLAYERFLAG_TELE=2,
	TILESLAYERFLAG_SPEEDUP=4,
	TILESLAYERFLAG_FRONT=8,
	TILESLAYERFLAG_SWITCH=16,

	ENTITY_OFFSET=255-16*4,
};

struct CPoint
{
	int x, y; // 22.10 fixed point
};

struct CColor
{
	int r, g, b, a;
};

struct CQuad
{
	CPoint m_aPoints[5];
	CColor m_aColors[4];
	CPoint m_aTexcoords[4];

	int m_PosEnv;
	int m_PosEnvOffset;

	int m_ColorEnv;
	int m_ColorEnvOffset;
};

class CTile
{
public:
	unsigned char m_Index;
	unsigned char m_Flags;
	unsigned char m_Skip;
	unsigned char m_Reserved;
};

struct CMapItemInfo
{
	int m_Version;
	int m_Author;
	int m_MapVersion;
	int m_Credits;
	int m_License;
} ;

struct CMapItemImage_v1
{
	int m_Version;
	int m_Width;
	int m_Height;
	int m_External;
	int m_ImageName;
	int m_ImageData;
} ;

struct CMapItemImage : public CMapItemImage_v1
{
	enum { CURRENT_VERSION=2 };
	int m_Format;
};

struct CMapItemGroup_v1
{
	int m_Version;
	int m_OffsetX;
	int m_OffsetY;
	int m_ParallaxX;
	int m_ParallaxY;

	int m_StartLayer;
	int m_NumLayers;
} ;


struct CMapItemGroup : public CMapItemGroup_v1
{
	enum { CURRENT_VERSION=3 };

	int m_UseClipping;
	int m_ClipX;
	int m_ClipY;
	int m_ClipW;
	int m_ClipH;

	int m_aName[3];
} ;

struct CMapItemLayer
{
	int m_Version;
	int m_Type;
	int m_Flags;
} ;

struct CMapItemLayerTilemap
{
	CMapItemLayer m_Layer;
	int m_Version;

	int m_Width;
	int m_Height;
	int m_Flags;

	CColor m_Color;
	int m_ColorEnv;
	int m_ColorEnvOffset;

	int m_Image;
	int m_Data;

	int m_aName[3];

	// DDRace

	int m_Tele;
	int m_SpeedUp;
	int m_Front;
	int m_Switch;
} ;

struct CMapItemLayerQuads
{
	CMapItemLayer m_Layer;
	int m_Version;

	int m_NumQuads;
	int m_Data;
	int m_Image;

	int m_aName[3];
} ;

struct CMapItemVersion
{
    enum { CURRENT_VERSION=1 };
	int m_Version;
} ;

struct CEnvPoint
{
	int m_Time; // in ms
	int m_Curvetype;
	int m_aValues[4]; // 1-4 depending on envelope (22.10 fixed point)

	bool operator<(const CEnvPoint &Other) { return m_Time < Other.m_Time; }
} ;

struct CMapItemEnvelope_v1
{
	int m_Version;
	int m_Channels;
	int m_StartPoint;
	int m_NumPoints;
	int m_aName[8];
} ;

struct CMapItemEnvelope : public CMapItemEnvelope_v1
{
	enum { CURRENT_VERSION=2 };
	int m_Synchronized;
};

// H-Client: DDNet
class CTeleTile
{
public:
	unsigned char m_Number;
	unsigned char m_Type;
};

class CSpeedUpTile
{
public:
	unsigned char m_Force;
	unsigned char m_MaxSpeed;
	unsigned char m_Type;
	short m_Angle;
};

class CDoorTile
{
public:
	unsigned char m_Index;
	unsigned char m_Flags;
	int m_Number;
};

class CSwitchTile
{
public:
	unsigned char m_Number;
	unsigned char m_Type;
	unsigned char m_Flags;
	unsigned char m_Delay;
};
//

#endif
