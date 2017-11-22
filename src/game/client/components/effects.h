/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_EFFECTS_H
#define GAME_CLIENT_COMPONENTS_EFFECTS_H
#include <game/client/component.h>

class CEffects : public CComponent
{
	bool m_Add50hz;
	bool m_Add100hz;
public:
	CEffects();

	virtual void OnRender();

	void BulletTrail(vec2 Pos, const vec4 &color); // H-Client: Color
	void SmokeTrail(vec2 Pos, vec2 Vel, const vec4 &color); // H-Client: Color
	void SkidTrail(vec2 Pos, vec2 Vel);
	void Explosion(vec2 Pos);
	void HammerHit(vec2 Pos);
	void AirJump(vec2 Pos);
	void DamageIndicator(vec2 Pos, vec2 Dir);
	void PlayerSpawn(vec2 Pos);
	void PlayerDeath(vec2 Pos, int ClientID);
	void PowerupShine(vec2 Pos, vec2 Size);

	// H-Client
	void LaserTrail(vec2 Pos, vec2 Vel, vec4 color);
	void Blood(vec2 Pos, vec2 Dir, int Type, int ClientID = -1);
	void Unfreeze(vec2 Pos, vec2 Dir, float alpha);
    //

	void Update();
};
#endif
