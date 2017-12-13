/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <base/math.h>

#include <math.h>
#include <engine/demo.h>
#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/serverbrowser.h> //H-Client
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>

#include <game/client/components/flow.h>
#include <game/client/components/skins.h>
#include <game/client/components/effects.h>
#include <game/client/components/sounds.h>
#include <game/client/components/controls.h>

#include "players.h"


// H-Client
void CPlayers::OnMapLoad()
{
	CComponent::OnMapLoad();

    m_RenderFreezeInfo.m_TextureDamage = -1;
    m_RenderFreezeInfo.m_Texture = -1;
    m_RenderFreezeInfo.m_Size = 64.0f;
    const int fSkin = m_pClient->m_pSkins->Find("x_freeze");
    if (fSkin != -1)
    {
    	m_RenderFreezeInfo.m_Texture = m_pClient->m_pSkins->Get(fSkin)->m_OrgTexture;
    	m_RenderFreezeInfo.m_ColorBody = vec4(1,1,1,1);
    	m_RenderFreezeInfo.m_ColorFeet = vec4(1,1,1,1);
    	m_RenderFreezeInfo.m_ColorHand = vec4(1,1,1,1);

    }
}
//

void CPlayers::RenderHand(CTeeRenderInfo *pInfo, const vec2 &CenterPos, vec2 Dir, float AngleOffset, const vec2 &PostRotOffset)
{
	// for drawing hand
	//const skin *s = skin_get(skin_id);

	const float BaseSize = 10.0f;
	//dir = normalize(hook_pos-pos);

	vec2 HandPos = CenterPos + Dir;
	float Angle = GetAngle(Dir);
	if (Dir.x < 0)
		Angle -= AngleOffset;
	else
		Angle += AngleOffset;

	vec2 DirX = Dir;
	vec2 DirY(-Dir.y,Dir.x);

	if (Dir.x < 0)
		DirY = -DirY;

	HandPos += DirX * PostRotOffset.x;
	HandPos += DirY * PostRotOffset.y;

	//Graphics()->TextureSet(data->m_aImages[IMAGE_CHAR_DEFAULT].id);
	Graphics()->TextureSet(pInfo->m_Texture);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(pInfo->m_ColorHand.r, pInfo->m_ColorHand.g, pInfo->m_ColorHand.b, pInfo->m_ColorHand.a); // H-Client

	// two passes
	for (int i = 0; i < 2; i++)
	{
		const bool OutLine = i == 0;

		RenderTools()->SelectSprite(OutLine?SPRITE_TEE_HAND_OUTLINE:SPRITE_TEE_HAND, 0, 0, 0);
		Graphics()->QuadsSetRotation(Angle);
		IGraphics::CQuadItem QuadItem(HandPos.x, HandPos.y, 2*BaseSize, 2*BaseSize);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}

	Graphics()->QuadsSetRotation(0);
	Graphics()->QuadsEnd();
}

inline float NormalizeAngular(float f)
{
	return fmod(f+PI*2, PI*2);
}

inline float AngularMixDirection (float Src, float Dst) { return sinf(Dst-Src) >0?1:-1; }
inline float AngularDistance(float Src, float Dst) { return asinf(sinf(Dst-Src)); }

inline float AngularApproach(float Src, float Dst, float Amount)
{
	const float d = AngularMixDirection (Src, Dst);
	const float n = Src + Amount*d;
	if(AngularMixDirection (n, Dst) != d)
		return Dst;
	return n;
}

void CPlayers::RenderHook(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPrevInfo,
	const CNetObj_PlayerInfo *pPlayerInfo
	)
{
	CNetObj_Character Prev;
	CNetObj_Character Player;
	Prev = *pPrevChar;
	Player = *pPlayerChar;

	CNetObj_PlayerInfo pInfo = *pPlayerInfo;
	CTeeRenderInfo RenderInfo = m_aRenderInfo[pInfo.m_ClientID];

	float IntraTick = Client()->IntraGameTick();

	// set size
	RenderInfo.m_Size = 64.0f;


	// use preditect players if needed
	if(pInfo.m_Local && g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(!m_pClient->m_Snap.m_pLocalCharacter || (m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)))
		{
		}
		else
		{
			// apply predicted results
			m_pClient->m_PredictedChar.Write(&Player);
			m_pClient->m_PredictedPrevChar.Write(&Prev);
			IntraTick = Client()->PredIntraGameTick();
		}
	}

	const vec2 Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Player.m_X, Player.m_Y), IntraTick);

	// draw hook
	if (Prev.m_HookState>0 && Player.m_HookState>0)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		//Graphics()->QuadsBegin();

		vec2 HookPos;

		if(pPlayerChar->m_HookedPlayer != -1)
		{
			if(m_pClient->m_Snap.m_pLocalInfo && pPlayerChar->m_HookedPlayer == m_pClient->m_Snap.m_pLocalInfo->m_ClientID)
			{
				if(Client()->State() == IClient::STATE_DEMOPLAYBACK) // only use prediction if needed
					HookPos = vec2(m_pClient->m_LocalCharacterPos.x, m_pClient->m_LocalCharacterPos.y);
				else
					HookPos = mix(vec2(m_pClient->m_PredictedPrevChar.m_Pos.x, m_pClient->m_PredictedPrevChar.m_Pos.y),
						vec2(m_pClient->m_PredictedChar.m_Pos.x, m_pClient->m_PredictedChar.m_Pos.y), Client()->PredIntraGameTick());
			}
			else if(pInfo.m_Local)
			{
				HookPos = mix(vec2(m_pClient->m_Snap.m_aCharacters[pPlayerChar->m_HookedPlayer].m_Prev.m_X,
					m_pClient->m_Snap.m_aCharacters[pPlayerChar->m_HookedPlayer].m_Prev.m_Y),
					vec2(m_pClient->m_Snap.m_aCharacters[pPlayerChar->m_HookedPlayer].m_Cur.m_X,
					m_pClient->m_Snap.m_aCharacters[pPlayerChar->m_HookedPlayer].m_Cur.m_Y),
					Client()->IntraGameTick());
			}
			else
				HookPos = mix(vec2(pPrevChar->m_HookX, pPrevChar->m_HookY), vec2(pPlayerChar->m_HookX, pPlayerChar->m_HookY), Client()->IntraGameTick());
		}
		else
			HookPos = mix(vec2(Prev.m_HookX, Prev.m_HookY), vec2(Player.m_HookX, Player.m_HookY), IntraTick);

		const float d = distance(Position, HookPos);
		const vec2 Dir = normalize(Position-HookPos);

		Graphics()->QuadsSetRotation(GetAngle(Dir)+PI);

		// H-Client
        const float DropHookTime = SERVER_TICK_SPEED+SERVER_TICK_SPEED/5;
        if (pPlayerChar->m_HookedPlayer != -1 && pPlayerChar->m_HookTick <= DropHookTime)
        {
            const float restRGB = pPlayerChar->m_HookTick / DropHookTime;
            Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f-restRGB);
        }
        //

		// render head
		RenderTools()->SelectSprite(SPRITE_HOOK_HEAD);
		IGraphics::CQuadItem QuadItem(HookPos.x, HookPos.y, 24,16);
		Graphics()->QuadsDraw(&QuadItem, 1);

		// render chain
		RenderTools()->SelectSprite(SPRITE_HOOK_CHAIN);
		IGraphics::CQuadItem Array[1024];
		int i = 0;
		for(float f = 24; f < d && i < 1024; f += 24, i++)
		{
			vec2 p = HookPos + Dir*f;
			Array[i] = IGraphics::CQuadItem(p.x, p.y,24,16);
		}

		Graphics()->QuadsDraw(Array, i);
		Graphics()->QuadsSetRotation(0);
		Graphics()->QuadsEnd();

		RenderHand(&RenderInfo, Position, normalize(HookPos-Position), -PI/2, vec2(20, 0));
	}
}

void CPlayers::RenderPlayer(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPrevInfo,
	const CNetObj_PlayerInfo *pPlayerInfo,
	int flags
	)
{
	CNetObj_Character Prev;
	CNetObj_Character Player;
	Prev = *pPrevChar;
	Player = *pPlayerChar;

	const CNetObj_PlayerInfo &PlayerInfo = *pPlayerInfo;
	CTeeRenderInfo RenderInfo = m_aRenderInfo[PlayerInfo.m_ClientID];

    //H-Client
	if (PlayerInfo.m_Local && Player.m_Health < 10 && Player.m_Health != 0)
	{
	    char skinName[25];
	    int damage = 10-Player.m_Health;
	    if (Client()->IsServerType("infection") && str_find_nocase(m_pClient->m_aClients[PlayerInfo.m_ClientID].m_aClan, "zomb"))
            damage = 8;

        str_format(skinName, sizeof(skinName), "x_damage%i", damage);
        const int damageSkin = m_pClient->m_pSkins->Find(skinName);
        if (damageSkin != -1)
            RenderInfo.m_TextureDamage = m_pClient->m_pSkins->Get(damageSkin)->m_OrgTexture;
    }
    //

	bool NewTick = m_pClient->m_NewTick;
	float IntraTick = Client()->IntraGameTick();

	// set size
	RenderInfo.m_Size = 64.0f;

	// use preditect players if needed
	if(g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(m_pClient->m_Snap.m_pLocalCharacter && (PlayerInfo.m_Local || (flags&PLAYERFLAG_ANTIPING)) && !(m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)))
		{
			// apply predicted results
			m_pClient->m_aClients[PlayerInfo.m_ClientID].m_Predicted.Write(&Player);
			m_pClient->m_aClients[PlayerInfo.m_ClientID].m_PrevPredicted.Write(&Prev);
			IntraTick = Client()->PredIntraGameTick();
		}
	}

	float Angle = mix((float)Prev.m_Angle, (float)Player.m_Angle, IntraTick)/256.0f;

	//float angle = 0;

	if(PlayerInfo.m_Local && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		// just use the direct input if it's local player we are rendering
		Angle = GetAngle(m_pClient->m_pControls->m_MousePos);
	}
	else
	{
		/*
		float mixspeed = Client()->FrameTime()*2.5f;
		if(player.attacktick != prev.attacktick) // shooting boosts the mixing speed
			mixspeed *= 15.0f;

		// move the delta on a constant speed on a x^2 curve
		float current = g_GameClient.m_aClients[info.cid].angle;
		float target = player.angle/256.0f;
		float delta = angular_distance(current, target);
		float sign = delta < 0 ? -1 : 1;
		float new_delta = delta - 2*mixspeed*sqrt(delta*sign)*sign + mixspeed*mixspeed;

		// make sure that it doesn't vibrate when it's still
		if(fabs(delta) < 2/256.0f)
			angle = target;
		else
			angle = angular_approach(current, target, fabs(delta-new_delta));

		g_GameClient.m_aClients[info.cid].angle = angle;*/
	}

	// use preditect players if needed
	if(g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(m_pClient->m_Snap.m_pLocalCharacter && (PlayerInfo.m_Local || (flags&PLAYERFLAG_ANTIPING)) && !(m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)))
		{
			// apply predicted results
			m_pClient->m_aClients[PlayerInfo.m_ClientID].m_Predicted.Write(&Player);
			m_pClient->m_aClients[PlayerInfo.m_ClientID].m_PrevPredicted.Write(&Prev);
			IntraTick = Client()->PredIntraGameTick();
			NewTick = m_pClient->m_NewPredictedTick;
		}
	}

	const vec2 Direction = GetDirection((int)(Angle*256.0f));
	const vec2 Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Player.m_X, Player.m_Y), IntraTick);
	const vec2 Vel = mix(vec2(Prev.m_VelX/256.0f, Prev.m_VelY/256.0f), vec2(Player.m_VelX/256.0f, Player.m_VelY/256.0f), IntraTick);

	m_pClient->m_pFlow->Add(Position, Vel*100.0f, 10.0f);

	RenderInfo.m_GotAirJump = Player.m_Jumped&2?0:1;


	// detect events
	if(NewTick)
	{
		// detect air jump
		if(!RenderInfo.m_GotAirJump && !(Prev.m_Jumped&2))
			m_pClient->m_pEffects->AirJump(Position);
	}

	const bool Stationary = Player.m_VelX <= 1 && Player.m_VelX >= -1;
	const bool InAir = !Collision()->CheckPoint(Player.m_X, Player.m_Y+16);
	const bool WantOtherDir = (Player.m_Direction == -1 && Vel.x > 0) || (Player.m_Direction == 1 && Vel.x < 0);

	// evaluate animation
	const float WalkTime = fmod(absolute(Position.x), 100.0f)/100.0f;
	CAnimState State;
	State.Set(&g_pData->m_aAnimations[ANIM_BASE], 0);

	if(InAir)
		State.Add(&g_pData->m_aAnimations[ANIM_INAIR], 0, 1.0f); // TODO: some sort of time here
	else if(Stationary)
		State.Add(&g_pData->m_aAnimations[ANIM_IDLE], 0, 1.0f); // TODO: some sort of time here
	else if(!WantOtherDir)
		State.Add(&g_pData->m_aAnimations[ANIM_WALK], WalkTime, 1.0f);

	static float s_LastGameTickTime = Client()->GameTickTime();
	if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
		s_LastGameTickTime = Client()->GameTickTime();
	if (Player.m_Weapon == WEAPON_HAMMER)
	{
		const float ct = (Client()->PrevGameTick()-Player.m_AttackTick)/(float)SERVER_TICK_SPEED + s_LastGameTickTime;
		State.Add(&g_pData->m_aAnimations[ANIM_HAMMER_SWING], clamp(ct*5.0f,0.0f,1.0f), 1.0f);
	}
	if (Player.m_Weapon == WEAPON_NINJA)
	{
		const float ct = (Client()->PrevGameTick()-Player.m_AttackTick)/(float)SERVER_TICK_SPEED + s_LastGameTickTime;
		State.Add(&g_pData->m_aAnimations[ANIM_NINJA_SWING], clamp(ct*2.0f,0.0f,1.0f), 1.0f);
	}

	// do skidding
	if(!InAir && WantOtherDir && length(Vel*50) > 500.0f)
	{
		static int64 SkidSoundTime = 0;
		if(time_get()-SkidSoundTime > time_freq()/10)
		{
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_PLAYER_SKID, 0.25f, Position);
			SkidSoundTime = time_get();
		}

		m_pClient->m_pEffects->SkidTrail(
			Position+vec2(-Player.m_Direction*6,12),
			vec2(-Player.m_Direction*100*length(Vel),-50)
		);
	}

	// H-Client: Draw AIM Line
    if (Player.m_PlayerFlags&PLAYERFLAG_AIM)
    {
        Graphics()->TextureSet(-1);

        static const float PhysSize = 28.0f;
        static const float RealPhysSize = PhysSize * 1.5f;
        vec2 orgPos, toPos, ExDirection = Direction;
        int Hit = 0;

        if (pPlayerInfo->m_Local && Client()->State() != IClient::STATE_DEMOPLAYBACK)
            ExDirection = normalize(vec2(m_pClient->m_pControls->m_InputData.m_TargetX, m_pClient->m_pControls->m_InputData.m_TargetY));

        orgPos = Position+ExDirection*RealPhysSize;
        toPos = orgPos + ExDirection*(m_pClient->m_Tuning.m_HookLength - PhysSize*2 - 1.5f);

        Graphics()->LinesBegin();

            Graphics()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);

            // Collide with walls?
            int teleNr = 0;
            Hit = Collision()->IntersectLineTeleHook(orgPos, toPos, &toPos, 0, &teleNr);
            if (Hit && !(Hit&CCollision::COLFLAG_NOHOOK)) // Hookable Tile
            	Graphics()->SetColor(0.5f, 0.9f, 0.62f, 1.0f);

            // Collide with characters?
            if (m_pClient->IntersectCharacter(orgPos, toPos, &toPos, pPlayerInfo->m_ClientID) != -1)
                Graphics()->SetColor(1.0f, 1.0f, 0.0f, 1.0f);

            // Don't draw 0 length lines
            if (distance(orgPos, toPos) > 0.0f)
            {
                IGraphics::CLineItem LineItem(Position.x, Position.y, toPos.x, toPos.y);
                Graphics()->LinesDraw(&LineItem, 1);
            }

        Graphics()->LinesEnd();
    }
    //


	// draw gun
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(State.GetAttach()->m_Angle*PI*2+Angle);

		// normal weapons
		const int iw = clamp(Player.m_Weapon, 0, NUM_WEAPONS-1);
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody, Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);

		float Recoil = 0.0f;
		vec2 p;
		if (Player.m_Weapon == WEAPON_HAMMER)
		{
			// Static position for hammer
			p = Position + vec2(State.GetAttach()->m_X, State.GetAttach()->m_Y);
			p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;
			// if attack is under way, bash stuffs
			if(Direction.x < 0)
			{
				Graphics()->QuadsSetRotation(-PI/2-State.GetAttach()->m_Angle*PI*2);
				p.x -= g_pData->m_Weapons.m_aId[iw].m_Offsetx;
			}
			else
			{
				Graphics()->QuadsSetRotation(-PI/2+State.GetAttach()->m_Angle*PI*2);
			}
			RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
		}
		else if (Player.m_Weapon == WEAPON_NINJA)
		{
			p = Position;
			p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;

			if(Direction.x < 0)
			{
				Graphics()->QuadsSetRotation(-PI/2-State.GetAttach()->m_Angle*PI*2);
				p.x -= g_pData->m_Weapons.m_aId[iw].m_Offsetx;
				m_pClient->m_pEffects->PowerupShine(p+vec2(32,0), vec2(32,12));
			}
			else
			{
				Graphics()->QuadsSetRotation(-PI/2+State.GetAttach()->m_Angle*PI*2);
				m_pClient->m_pEffects->PowerupShine(p-vec2(32,0), vec2(32,12));
			}
			RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);

			// HADOKEN
			if ((Client()->GameTick()-Player.m_AttackTick) <= (SERVER_TICK_SPEED / 6) && g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles)
			{
				int IteX = rand() % g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles;
				static int s_LastIteX = IteX;
				if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
				{
					const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
					if(pInfo->m_Paused)
						IteX = s_LastIteX;
					else
						s_LastIteX = IteX;
				}
				else
				{
					if(m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
						IteX = s_LastIteX;
					else
						s_LastIteX = IteX;
				}
				if(g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX])
				{
					const vec2 Dir = normalize(vec2(pPlayerChar->m_X,pPlayerChar->m_Y) - vec2(pPrevChar->m_X, pPrevChar->m_Y));
					Graphics()->QuadsSetRotation(GetAngle(Dir));
					//float offsety = -data->weapons[iw].muzzleoffsety;
					RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX], 0);
					const vec2 DirY(-Dir.y,Dir.x);
					p = Position;
					const float OffsetX = g_pData->m_Weapons.m_aId[iw].m_Muzzleoffsetx;
					p -= Dir * OffsetX;
					RenderTools()->DrawSprite(p.x, p.y, 160.0f);
				}
			}
		}
		else
		{
			// TODO: should be an animation
			Recoil = 0;
			static float s_LastIntraTick = IntraTick;
			if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
				s_LastIntraTick = IntraTick;

			const float a = (Client()->GameTick()-Player.m_AttackTick+s_LastIntraTick)/5.0f;
			if(a < 1)
				Recoil = sinf(a*PI);
			p = Position + Direction * g_pData->m_Weapons.m_aId[iw].m_Offsetx - Direction*Recoil*10.0f;
			p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;
			RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
		}

		if (Player.m_Weapon == WEAPON_GUN || Player.m_Weapon == WEAPON_SHOTGUN)
		{
			// check if we're firing stuff
			if(g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles)//prev.attackticks)
			{
				float Alpha = 0.0f;
				const int Phase1Tick = (Client()->GameTick() - Player.m_AttackTick);
				if (Phase1Tick < (g_pData->m_Weapons.m_aId[iw].m_Muzzleduration + 3))
				{
					float t = ((((float)Phase1Tick) + IntraTick)/(float)g_pData->m_Weapons.m_aId[iw].m_Muzzleduration);
					Alpha = mix(2.0f, 0.0f, min(1.0f,max(0.0f,t)));
				}

				int IteX = rand() % g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles;
				static int s_LastIteX = IteX;
				if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
				{
					const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
					if(pInfo->m_Paused)
						IteX = s_LastIteX;
					else
						s_LastIteX = IteX;
				}
				else
				{
					if(m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
						IteX = s_LastIteX;
					else
						s_LastIteX = IteX;
				}
				if (Alpha > 0.0f && g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX])
				{
					float OffsetY = -g_pData->m_Weapons.m_aId[iw].m_Muzzleoffsety;
					RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX], Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
					if(Direction.x < 0)
						OffsetY = -OffsetY;

					const vec2 DirY(-Direction.y,Direction.x);
					const vec2 MuzzlePos = p + Direction * g_pData->m_Weapons.m_aId[iw].m_Muzzleoffsetx + DirY * OffsetY;

					RenderTools()->DrawSprite(MuzzlePos.x, MuzzlePos.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
				}
			}
		}
		Graphics()->QuadsEnd();

		switch (Player.m_Weapon)
		{
			case WEAPON_GUN: RenderHand(&RenderInfo, p, Direction, -3*PI/4, vec2(-15, 4)); break;
			case WEAPON_SHOTGUN: RenderHand(&RenderInfo, p, Direction, -PI/2, vec2(-5, 4)); break;
			case WEAPON_GRENADE: RenderHand(&RenderInfo, p, Direction, -PI/2, vec2(-4, 7)); break;
		}

	}

	// render the "shadow" tee
	if(PlayerInfo.m_Local && g_Config.m_Debug)
	{
		const vec2 GhostPosition = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), Client()->IntraGameTick());
		CTeeRenderInfo Ghost = RenderInfo;
		Ghost.m_ColorBody.a = 0.5f;
		Ghost.m_ColorFeet.a = 0.5f;
		RenderTools()->RenderTee(&State, &Ghost, Player.m_Emote, Direction, GhostPosition); // render ghost
	}

	RenderInfo.m_Size = 64.0f; // force some settings
	RenderInfo.m_ColorBody.a = 1.0f;
	RenderInfo.m_ColorFeet.a = 1.0f;

    //H-Client: DDRace Stuff
	if (Client()->IsServerType("ddrace"))
	{
        // Tee Direction Info
        if (g_Config.m_ddrShowTeeDirection && !PlayerInfo.m_Local)
        {
            if (Player.m_Direction != 0)
            {
                Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
                Graphics()->QuadsBegin();
                IGraphics::CQuadItem QuadItem(Position.x-15, Position.y - 70, 22, 22);
                if (Player.m_Direction == -1)
                    Graphics()->QuadsSetRotation(GetAngle(vec2(1,0))+PI);
                Graphics()->QuadsDraw(&QuadItem, 1);
                Graphics()->QuadsEnd();
            }
            if (Player.m_Jumped&1)
            {
                Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
                Graphics()->QuadsBegin();
                IGraphics::CQuadItem QuadItem(Position.x+15, Position.y - 70, 22, 22);
                Graphics()->QuadsSetRotation(GetAngle(vec2(0,1))+PI);
                Graphics()->QuadsDraw(&QuadItem, 1);
                Graphics()->QuadsEnd();
            }
        }
	}

	// H-Client: Render Tee (Is Freezed?)
	m_RenderFreezeInfo.m_GotAirJump = RenderInfo.m_GotAirJump;

	if (m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Freezed)
	{
		if (m_RenderFreezeInfo.m_Texture != -1 && Client()->GameTick() - m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_TimerFreeze > Client()->GameTickSpeed() * 0.5f && m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha < 1.0f)
		{
			m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha = min(m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha+0.01f, 1.0f);
			m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_TimerFreeze = Client()->GameTick();
		}

		if (m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha < 1.0f)
		{
			RenderInfo.m_ColorBody.a = RenderInfo.m_ColorFeet.a = 1.0f - m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha;
			RenderTools()->RenderTee(&State, &RenderInfo, Player.m_Emote, Direction, Position);
		}

		if (m_RenderFreezeInfo.m_Texture != -1)
		{
			m_RenderFreezeInfo.m_ColorBody.a = m_RenderFreezeInfo.m_ColorFeet.a = m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha;
			RenderTools()->RenderTee(&State, &m_RenderFreezeInfo, Player.m_Emote, Direction, Position);
		}
	}
	else
	{
		if (Client()->GameTick() - m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_TimerFreeze > Client()->GameTickSpeed() * 0.12f && m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha > 0.0f)
		{
			m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha = max(0.0f, m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha-0.01f);
			m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_TimerFreeze = Client()->GameTick();
		}

		RenderInfo.m_ColorBody.a = RenderInfo.m_ColorFeet.a = 1.0f - m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha;
		RenderTools()->RenderTee(&State, &RenderInfo, Player.m_Emote, Direction, Position);

		if (m_RenderFreezeInfo.m_Texture != -1 && m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha > 0.0f)
		{
			m_RenderFreezeInfo.m_ColorBody.a = m_RenderFreezeInfo.m_ColorFeet.a = m_pClient->m_aClients[PlayerInfo.m_ClientID].m_FreezedState.m_Alpha;
			RenderTools()->RenderTee(&State, &m_RenderFreezeInfo, Player.m_Emote, Direction, Position);
		}
	}
	//

	//H-Client: Gore
    if (g_Config.m_hcGoreStyle && !Client()->IsServerType("ddrace") && Prev.m_Emote == EMOTE_NORMAL && Player.m_Emote == EMOTE_PAIN)
        m_pClient->m_pEffects->Blood(Position, Direction, 0);
    //

	if(Player.m_PlayerFlags&PLAYERFLAG_CHATTING)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_DOTDOT);
		IGraphics::CQuadItem QuadItem(Position.x + 24, Position.y - 40, 64,64);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}
	else if(Player.m_PlayerFlags&PLAYERFLAG_IN_MENU) // H-Client
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_ZZZ);
		IGraphics::CQuadItem QuadItem(Position.x + 24, Position.y - 40, 64,64);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}

	if (m_pClient->m_aClients[PlayerInfo.m_ClientID].m_EmoticonStart != -1 && m_pClient->m_aClients[PlayerInfo.m_ClientID].m_EmoticonStart + 2 * Client()->GameTickSpeed() > Client()->GameTick())
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
		Graphics()->QuadsBegin();

		const int SinceStart = Client()->GameTick() - m_pClient->m_aClients[PlayerInfo.m_ClientID].m_EmoticonStart;
		const int FromEnd = m_pClient->m_aClients[PlayerInfo.m_ClientID].m_EmoticonStart + 2 * Client()->GameTickSpeed() - Client()->GameTick();

		float a = 1;

		if (FromEnd < Client()->GameTickSpeed() / 5)
			a = FromEnd / (Client()->GameTickSpeed() / 5.0);

		float h = 1;
		if (SinceStart < Client()->GameTickSpeed() / 10)
			h = SinceStart / (Client()->GameTickSpeed() / 10.0);

		float Wiggle = 0;
		if (SinceStart < Client()->GameTickSpeed() / 5)
			Wiggle = SinceStart / (Client()->GameTickSpeed() / 5.0);

		const float WiggleAngle = sinf(5*Wiggle);

		Graphics()->QuadsSetRotation(PI/6*WiggleAngle);

		Graphics()->SetColor(1.0f,1.0f,1.0f,a);
		// client_datas::emoticon is an offset from the first emoticon
		RenderTools()->SelectSprite(SPRITE_OOP + m_pClient->m_aClients[PlayerInfo.m_ClientID].m_Emoticon);
		IGraphics::CQuadItem QuadItem(Position.x, Position.y - 23 - 32*h, 64, 64*h);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}
}

// H-Client: TDTW: Anti-Ping
void CPlayers::OnRender()
{
	// update RenderInfo for ninja
	bool IsTeamplay = false;
	if(m_pClient->m_Snap.m_pGameInfoObj)
		IsTeamplay = (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS) != 0;
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		m_aRenderInfo[i] = m_pClient->m_aClients[i].m_RenderInfo;
		if(m_pClient->m_Snap.m_aCharacters[i].m_Cur.m_Weapon == WEAPON_NINJA)
		{
			// change the skin for the player to the ninja
			const int Skin = m_pClient->m_pSkins->Find("x_ninja");
			if(Skin != -1)
			{
				if(IsTeamplay)
					m_aRenderInfo[i].m_Texture = m_pClient->m_pSkins->Get(Skin)->m_ColorTexture;
				else
				{
					m_aRenderInfo[i].m_Texture = m_pClient->m_pSkins->Get(Skin)->m_OrgTexture;
					m_aRenderInfo[i].m_ColorBody = vec4(1,1,1,1);
					m_aRenderInfo[i].m_ColorFeet = vec4(1,1,1,1);
				}
			}
		}
	}

	// render other players in two passes, first pass we render the other, second pass we render our self
	for(int p = 0; p < 4; ++p)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			// only render active characters
			if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
				continue;

			const void *pPrevInfo = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_PLAYERINFO, i);
			const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);

			if(pPrevInfo && pInfo)
			{
				//
				const bool Local = ((const CNetObj_PlayerInfo *)pInfo)->m_Local !=0;
				if((p % 2) == 0 && Local) continue;
				if((p % 2) == 1 && !Local) continue;

				CNetObj_Character PrevChar = m_pClient->m_Snap.m_aCharacters[i].m_Prev;
				CNetObj_Character CurChar = m_pClient->m_Snap.m_aCharacters[i].m_Cur;

				if(p<2)
				{
					RenderHook(
							&PrevChar,
							&CurChar,
							(const CNetObj_PlayerInfo *)pPrevInfo,
							(const CNetObj_PlayerInfo *)pInfo
						);
				}
				else
				{
					if (Local)
					{
						RenderPlayer(
								&PrevChar,
								&CurChar,
								(const CNetObj_PlayerInfo *)pPrevInfo,
								(const CNetObj_PlayerInfo *)pInfo
							);
					}
					else
					{
						if (g_Config.m_AntiPing)
							RenderPlayer(
									&PrevChar,
									&CurChar,
									(const CNetObj_PlayerInfo *)pPrevInfo,
									(const CNetObj_PlayerInfo *)pInfo,
									PLAYERFLAG_ANTIPING
								);
						else
							RenderPlayer(
									&PrevChar,
									&CurChar,
									(const CNetObj_PlayerInfo *)pPrevInfo,
									(const CNetObj_PlayerInfo *)pInfo
								);
					}
				}
			}
		}
	}
}
//
