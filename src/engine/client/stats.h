/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_STATS_H
#define ENGINE_CLIENT_STATS_H

struct CStatistics
{
	CStatistics()
	{
		Reset();
	}

	void Reset()
	{
		m_ClientRuns = 0;
		m_ClientTimeRun = 0;
		m_Kills = 0;
		m_Deaths = 0;
		m_TotalDamage = 0;
		m_ServerJoins = 0;
		m_ChatMessagesSent = 0;
		m_ChatMessagesReceived = 0;
		mem_zero(m_BestScore, sizeof(m_BestScore));
		m_WeaponChanges = 0;
		m_ShootsHammer = 0;
		m_ShootsGun = 0;
		m_ShootsShotgun = 0;
		m_ShootsGrenades = 0;
		m_ShootsRifle = 0;
	}

	unsigned long m_ClientRuns;
	int64 m_ClientTimeRun;
	unsigned long m_Kills;
	unsigned long m_Deaths;
	unsigned long m_TotalDamage;
	unsigned long m_ServerJoins;
	unsigned long m_ChatMessagesSent;
	unsigned long m_ChatMessagesReceived;
	unsigned long m_BestScore[3];
	unsigned long m_WeaponChanges;
	unsigned long m_ShootsHammer;
	unsigned long m_ShootsGun;
	unsigned long m_ShootsShotgun;
	unsigned long m_ShootsGrenades;
	unsigned long m_ShootsRifle;
};
//

extern CStatistics g_Stats;

#endif
