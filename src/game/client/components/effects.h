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

	void BulletTrail(const vec2 &Pos, const vec4 &color); // H-Client: Color
	void SmokeTrail(const vec2 &Pos, const vec2 &Vel, const vec4 &color); // H-Client: Color
	void SkidTrail(const vec2 &Pos, const vec2 &Vel);
	void Explosion(const vec2 &Pos);
	void HammerHit(const vec2 &Pos);
	void AirJump(const vec2 &Pos);
	void DamageIndicator(const vec2 &Pos, const vec2 &Dir);
	void PlayerSpawn(const vec2 &Pos);
	void PlayerDeath(const vec2 &Pos, int ClientID);
	void PowerupShine(const vec2 &Pos, const vec2 &Size);

	// H-Client
	void LaserTrail(const vec2 &Pos, const vec2 &Vel, const vec4 &color);
	void Blood(const vec2 &Pos, const vec2 &Dir, int Type, int ClientID = -1);
	void Unfreeze(const vec2 &Pos, const vec2 &Dir, const vec4 &Color, float alpha);
	void ExplosionDebris(const vec2 &Pos);
    //

	void Update();
};
#endif
