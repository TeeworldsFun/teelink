#include "trunk.h"

CTrunk::CTrunk(CGameWorld *pWorld, int Type)
: CEntity(pWorld, CGameWorld::ENTTYPE_TRUNK)
{
    m_pItems = 0x0;
    m_MaxItems = 0;

    if (Type == TRUNK_SMALL)
        m_MaxItems = 20;
    else if (Type == TRUNK_BIG)
        m_MaxItems = 40;

    m_pItems = static_cast<TrunkItem*>(mem_alloc(sizeof(TrunkItem)*m_MaxItems, 1));

    Reset();
}

void CTrunk::Destroy()
{
    mem_free(m_pItems);
}

void CTrunk::Reset()
{
    for (int i=0; i<m_MaxItems; i++)
    {
        m_pItems[i].m_Index = NUM_WEAPONS+NUM_BLOCKS;
        m_pItems[i].m_Amount = 0;
    }
}

void CTrunk::FillInfo(CNetObj_Trunk *pTrunk)
{
    pTrunk->m_Item1 = m_pItems[0].m_Index;
	pTrunk->m_Ammo1 = m_pItems[0].m_Amount;
    pTrunk->m_Item2 = m_pItems[1].m_Index;
	pTrunk->m_Ammo2 = m_pItems[1].m_Amount;
    pTrunk->m_Item3 = m_pItems[2].m_Index;
	pTrunk->m_Ammo3 = m_pItems[2].m_Amount;
    pTrunk->m_Item4 = m_pItems[3].m_Index;
	pTrunk->m_Ammo4 = m_pItems[3].m_Amount;
    pTrunk->m_Item5 = m_pItems[4].m_Index;
	pTrunk->m_Ammo5 = m_pItems[4].m_Amount;
    pTrunk->m_Item6 = m_pItems[5].m_Index;
	pTrunk->m_Ammo6 = m_pItems[5].m_Amount;
    pTrunk->m_Item7 = m_pItems[6].m_Index;
	pTrunk->m_Ammo7 = m_pItems[6].m_Amount;
    pTrunk->m_Item8 = m_pItems[7].m_Index;
	pTrunk->m_Ammo8 = m_pItems[7].m_Amount;
    pTrunk->m_Item9 = m_pItems[8].m_Index;
	pTrunk->m_Ammo9 = m_pItems[8].m_Amount;
    pTrunk->m_Item10 = m_pItems[9].m_Index;
	pTrunk->m_Ammo10 = m_pItems[9].m_Amount;
    pTrunk->m_Item11 = m_pItems[10].m_Index;
	pTrunk->m_Ammo11 = m_pItems[10].m_Amount;
    pTrunk->m_Item12 = m_pItems[11].m_Index;
	pTrunk->m_Ammo12 = m_pItems[11].m_Amount;
    pTrunk->m_Item13 = m_pItems[12].m_Index;
	pTrunk->m_Ammo13 = m_pItems[12].m_Amount;
    pTrunk->m_Item14 = m_pItems[13].m_Index;
	pTrunk->m_Ammo14 = m_pItems[13].m_Amount;
    pTrunk->m_Item15 = m_pItems[14].m_Index;
	pTrunk->m_Ammo15 = m_pItems[14].m_Amount;
    pTrunk->m_Item16 = m_pItems[15].m_Index;
	pTrunk->m_Ammo16 = m_pItems[15].m_Amount;
    pTrunk->m_Item17 = m_pItems[16].m_Index;
	pTrunk->m_Ammo17 = m_pItems[16].m_Amount;
    pTrunk->m_Item18 = m_pItems[17].m_Index;
	pTrunk->m_Ammo18 = m_pItems[17].m_Amount;
    pTrunk->m_Item19 = m_pItems[18].m_Index;
	pTrunk->m_Ammo19 = m_pItems[18].m_Amount;
    pTrunk->m_Item20 = m_pItems[19].m_Index;
	pTrunk->m_Ammo20 = m_pItems[19].m_Amount;
}

/*TrunkItem CTrunk::PutItem(int Index, int Amount, int Pos)
{
    if (Pos < 0 || Pos > ((m_Type==TRUNK_SMALL)?20:40))
        return -1;

    TrunkItem CurrentItem;

    if (m_pItems[Pos].m_Index < NUM_WEAPONS+NUM_BLOCKS)
        CurrentItem = m_pItems[Pos];

    m_pItems[Pos].m_Index = Index;
    m_pItems[Pos].m_Amount = Amount;

    return CurrentItem;
}

TrunkItem CTrunk::DelItem(int Pos)
{
    if (Pos < 0 || Pos > ((m_Type==TRUNK_SMALL)?20:40))
        return -1;

    TrunkItem Item;

    if (m_pItems[Pos].m_Index < NUM_WEAPONS+NUM_BLOCKS)
        Item = m_pItems[Pos];

    m_pItems[Pos].m_Index = NUM_WEAPONS+NUM_BLOCKS;
    m_pItems[Pos].m_Amount = 0;

    return Item;
}*/
