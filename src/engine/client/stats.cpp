/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/stats.h>
#include <engine/storage.h>
#include "stats.h"

CStatistics g_Stats;

class CStats : public IStats
{
	IStorage *m_pStorage;

public:

	CStats()
	{
		m_pStorage = 0x0;
	}

	virtual void Init()
	{
		m_pStorage = Kernel()->RequestInterface<IStorage>();
		Reset();

		if(!m_pStorage)
			return;

		IOHANDLE StatsFile = m_pStorage->OpenFile("stats.dat", IOFLAG_READ, IStorage::TYPE_SAVE);
		if(!StatsFile)
			return;

		io_read(StatsFile, &g_Stats, sizeof(CStatistics));
		io_close(StatsFile);
	}

	virtual void Save()
	{
		if(!m_pStorage)
			return;

		IOHANDLE StatsFile = m_pStorage->OpenFile("stats.dat", IOFLAG_WRITE, IStorage::TYPE_SAVE);
		if(!StatsFile)
			return;

		io_write(StatsFile, &g_Stats, sizeof(CStatistics));
		io_close(StatsFile);
	}

	virtual void Reset()
	{
		g_Stats.Reset();
	}
};

IStats *CreateStats() { return new CStats; }
