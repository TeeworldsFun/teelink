#ifndef GAME_SERVER_ENTITIES_TRUNK_H
#define GAME_SERVER_ENTITIES_TRUNK_H

#include <game/server/entity.h>
#include <game/generated/server_data.h>
#include <game/generated/protocol.h>

#include <game/gamecore.h>

enum
{
    TRUNK_SMALL=0,
    TRUNK_BIG,
};

class CTrunk : public CEntity
{
private:
    int m_MaxItems;
    int m_Type;

public:
    CTrunk(CGameWorld *pWorld, int Type);

	virtual void Reset();
	virtual void Destroy();
	//virtual void Snap(int SnappingClient);

    struct TrunkItem
    {
        TrunkItem()
        {
            m_Index = NUM_WEAPONS+NUM_BLOCKS;
            m_Amount = 0;
        }

        int m_Index;
        int m_Amount;
    };

    TrunkItem *m_pItems;

    int GetType() const { return m_Type; }
    void FillInfo(CNetObj_Trunk *pTrunk);

    //TrunkItem PutItem(int Index, int Amount, int Pos);
    //TrunkItem DelItem(int Pos);
};

#endif
