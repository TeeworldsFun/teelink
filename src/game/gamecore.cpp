/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "gamecore.h"
#include "mapitems.h" // H-Client
#include <engine/shared/config.h> // H-Client

const char *CTuningParams::m_apNames[] =
{
	#define MACRO_TUNING_PARAM(Name,ScriptName,Value) #ScriptName,
	#include "tuning.h"
	#undef MACRO_TUNING_PARAM
};


bool CTuningParams::Set(int Index, float Value)
{
	if(Index < 0 || Index >= Num())
		return false;
	((CTuneParam *)this)[Index] = Value;
	return true;
}

bool CTuningParams::Get(int Index, float *pValue)
{
	if(Index < 0 || Index >= Num())
		return false;
	*pValue = (float)((CTuneParam *)this)[Index];
	return true;
}

bool CTuningParams::Set(const char *pName, float Value)
{
	for(int i = 0; i < Num(); i++)
		if(str_comp_nocase(pName, m_apNames[i]) == 0)
			return Set(i, Value);
	return false;
}

bool CTuningParams::Get(const char *pName, float *pValue)
{
	for(int i = 0; i < Num(); i++)
		if(str_comp_nocase(pName, m_apNames[i]) == 0)
			return Get(i, pValue);

	return false;
}

float HermiteBasis1(float v)
{
	return 2*v*v*v - 3*v*v+1;
}

float VelocityRamp(float Value, float Start, float Range, float Curvature)
{
	if(Value < Start)
		return 1.0f;
	return 1.0f/powf(Curvature, (Value-Start)/Range);
}

void CCharacterCore::Init(CWorldCore *pWorld, CCollision *pCollision)
{
	m_pWorld = pWorld;
	m_pCollision = pCollision;
}

void CCharacterCore::Reset()
{
	m_Pos = vec2(0,0);
	m_Vel = vec2(0,0);
	m_HookPos = vec2(0,0);
	m_HookDir = vec2(0,0);
	m_HookTick = 0;
	m_HookState = HOOK_IDLE;
	m_HookedPlayer = -1;
	m_Jumped = 0;
	m_TriggeredEvents = 0;

	// H-Client: DDNet
	m_Freezes = false;
	m_ActiveWeapon = -1;
	//
}

void CCharacterCore::Tick(bool UseInput)
{
	float PhysSize = 28.0f;
	vec2 PrevPos = m_Pos; // H-Client: DDNet
	m_TriggeredEvents = 0;

	// get ground state
	bool Grounded = false;
	if(m_pCollision->CheckPoint(m_Pos.x+PhysSize/2, m_Pos.y+PhysSize/2+5))
		Grounded = true;
	if(m_pCollision->CheckPoint(m_Pos.x-PhysSize/2, m_Pos.y+PhysSize/2+5))
		Grounded = true;

    // H-Client: DDNet
	bool InTileFreeze = m_pCollision->CheckPointFreeze(m_Pos);
	bool InTileSpeed = m_pCollision->CheckPointSpeedUp(m_Pos);
    //


	vec2 TargetDirection = normalize(vec2(m_Input.m_TargetX, m_Input.m_TargetY));

	m_Vel.y += m_pWorld->m_Tuning.m_Gravity;

	float MaxSpeed = Grounded ? m_pWorld->m_Tuning.m_GroundControlSpeed : m_pWorld->m_Tuning.m_AirControlSpeed;
	float Accel = Grounded ? m_pWorld->m_Tuning.m_GroundControlAccel : m_pWorld->m_Tuning.m_AirControlAccel;
	float Friction = Grounded ? m_pWorld->m_Tuning.m_GroundFriction : m_pWorld->m_Tuning.m_AirFriction;

	// handle input
	if(UseInput)
	{
		m_Direction = m_Input.m_Direction;

		// setup angle
		float a = 0;
		if(m_Input.m_TargetX == 0)
			a = atanf((float)m_Input.m_TargetY);
		else
			a = atanf((float)m_Input.m_TargetY/(float)m_Input.m_TargetX);

		if(m_Input.m_TargetX < 0)
			a = a+PI;

		m_Angle = (int)(a*256.0f);

		// handle jump
		if((g_Config.m_ddrPreventPrediction && m_Input.m_Jump && !m_Freezes && !InTileFreeze) || (!g_Config.m_ddrPreventPrediction && m_Input.m_Jump)) // H-Client: DDNet
		{
			if(!(m_Jumped&1))
			{
				if(Grounded)
				{
					m_TriggeredEvents |= COREEVENT_GROUND_JUMP;
					m_Vel.y = -m_pWorld->m_Tuning.m_GroundJumpImpulse;
					m_Jumped |= 1;
				}
				else if(!(m_Jumped&2))
				{
					m_TriggeredEvents |= COREEVENT_AIR_JUMP;
					m_Vel.y = -m_pWorld->m_Tuning.m_AirJumpImpulse;
					m_Jumped |= 3;
				}
			}
		}
		else
			m_Jumped &= ~1;

		// handle hook
		if((g_Config.m_ddrPreventPrediction && m_Input.m_Hook && !m_Freezes && !InTileFreeze) || (!g_Config.m_ddrPreventPrediction && m_Input.m_Hook)) // H-Client: DDNet
		{
			if(m_HookState == HOOK_IDLE)
			{
				m_HookState = HOOK_FLYING;
				m_HookPos = m_Pos+TargetDirection*PhysSize*1.5f;
				m_HookDir = TargetDirection;
				m_HookedPlayer = -1;
				m_HookTick = 0;
				m_TriggeredEvents |= COREEVENT_HOOK_LAUNCH;
			}
		}
		else
		{
			m_HookedPlayer = -1;
			m_HookState = HOOK_IDLE;
			m_HookPos = m_Pos;
		}
	}

	// add the speed modification according to players wanted direction
	// H-Client: DDNet
	if (g_Config.m_ddrPreventPrediction && (m_Freezes || InTileFreeze))
        m_Vel.x *= Friction;
    else
    {
        if (m_Direction < 0)
            m_Vel.x = SaturatedAdd(-MaxSpeed, MaxSpeed, m_Vel.x, -Accel);
        else if (m_Direction > 0)
            m_Vel.x = SaturatedAdd(-MaxSpeed, MaxSpeed, m_Vel.x, Accel);
        else
            m_Vel.x *= Friction;
    }

	// handle jumping
	// 1 bit = to keep track if a jump has been made on this input
	// 2 bit = to keep track if a air-jump has been made
	if(Grounded)
		m_Jumped &= ~2;

	// do hook
	if(m_HookState == HOOK_IDLE)
	{
		m_HookedPlayer = -1;
		m_HookState = HOOK_IDLE;
		m_HookPos = m_Pos;
	}
	else if(m_HookState >= HOOK_RETRACT_START && m_HookState < HOOK_RETRACT_END)
	{
		m_HookState++;
	}
	else if(m_HookState == HOOK_RETRACT_END)
	{
		m_HookState = HOOK_RETRACTED;
		m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
		m_HookState = HOOK_RETRACTED;
	}
	else if(m_HookState == HOOK_FLYING)
	{
		vec2 NewPos = m_HookPos+m_HookDir*m_pWorld->m_Tuning.m_HookFireSpeed;
		if(distance(m_Pos, NewPos) > m_pWorld->m_Tuning.m_HookLength)
		{
			m_HookState = HOOK_RETRACT_START;
			NewPos = m_Pos + normalize(NewPos-m_Pos) * m_pWorld->m_Tuning.m_HookLength;
		}

		// make sure that the hook doesn't go though the ground
		bool GoingToHitGround = false;
		bool GoingToRetract = false;
		int Hit = m_pCollision->IntersectLine(m_HookPos, NewPos, &NewPos, 0, true); // H-Client: DDNet
		if(Hit)
		{
			if(Hit&CCollision::COLFLAG_NOHOOK)
				GoingToRetract = true;
			else
				GoingToHitGround = true;
		}

		// Check against other players first
		if(m_pWorld && m_pWorld->m_Tuning.m_PlayerHooking)
		{
			float Distance = 0.0f;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				CCharacterCore *pCharCore = m_pWorld->m_apCharacters[i];
				if(!pCharCore || pCharCore == this)
					continue;

				vec2 ClosestPoint = closest_point_on_line(m_HookPos, NewPos, pCharCore->m_Pos);
				if(distance(pCharCore->m_Pos, ClosestPoint) < PhysSize+2.0f)
				{
					if (m_HookedPlayer == -1 || distance(m_HookPos, pCharCore->m_Pos) < Distance)
					{
						m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
						m_HookState = HOOK_GRABBED;
						m_HookedPlayer = i;
						Distance = distance(m_HookPos, pCharCore->m_Pos);
					}
				}
			}
		}

		if(m_HookState == HOOK_FLYING)
		{
			// check against ground
			if(GoingToHitGround)
			{
				m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_GROUND;
				m_HookState = HOOK_GRABBED;
			}
			else if(GoingToRetract)
			{
				m_TriggeredEvents |= COREEVENT_HOOK_HIT_NOHOOK;
				m_HookState = HOOK_RETRACT_START;
			}

			m_HookPos = NewPos;
		}
	}

	if(m_HookState == HOOK_GRABBED)
	{
		if(m_HookedPlayer != -1)
		{
			CCharacterCore *pCharCore = m_pWorld->m_apCharacters[m_HookedPlayer];
			if(pCharCore)
				m_HookPos = pCharCore->m_Pos;
			else
			{
				// release hook
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}

			// keep players hooked for a max of 1.5sec
			//if(Server()->Tick() > hook_tick+(Server()->TickSpeed()*3)/2)
				//release_hooked();
		}

		// don't do this hook rutine when we are hook to a player
		if(m_HookedPlayer == -1 && distance(m_HookPos, m_Pos) > 46.0f)
		{
			vec2 HookVel = normalize(m_HookPos-m_Pos)*m_pWorld->m_Tuning.m_HookDragAccel;
			// the hook as more power to drag you up then down.
			// this makes it easier to get on top of an platform
			if(HookVel.y > 0)
				HookVel.y *= 0.3f;

			// the hook will boost it's power if the player wants to move
			// in that direction. otherwise it will dampen everything abit
			if((HookVel.x < 0 && m_Direction < 0) || (HookVel.x > 0 && m_Direction > 0))
				HookVel.x *= 0.95f;
			else
				HookVel.x *= 0.75f;

			vec2 NewVel = m_Vel+HookVel;

			// check if we are under the legal limit for the hook
			if(length(NewVel) < m_pWorld->m_Tuning.m_HookDragSpeed || length(NewVel) < length(m_Vel))
				m_Vel = NewVel; // no problem. apply

		}

		// release hook (max hook time is 1.25
		m_HookTick++;
		if(m_HookedPlayer != -1 && (m_HookTick > SERVER_TICK_SPEED+SERVER_TICK_SPEED/5 || !m_pWorld->m_apCharacters[m_HookedPlayer]))
		{
			m_HookedPlayer = -1;
			m_HookState = HOOK_RETRACTED;
			m_HookPos = m_Pos;
		}
	}

	if(m_pWorld)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			CCharacterCore *pCharCore = m_pWorld->m_apCharacters[i];
			if(!pCharCore)
				continue;

			//player *p = (player*)ent;
			if(pCharCore == this) // || !(p->flags&FLAG_ALIVE)
				continue; // make sure that we don't nudge our self

			// handle player <-> player collision
			float Distance = distance(m_Pos, pCharCore->m_Pos);
			vec2 Dir = normalize(m_Pos - pCharCore->m_Pos);
			if(m_pWorld->m_Tuning.m_PlayerCollision && Distance < PhysSize*1.25f && Distance > 0.0f)
			{
				float a = (PhysSize*1.45f - Distance);
				float Velocity = 0.5f;

				// make sure that we don't add excess force by checking the
				// direction against the current velocity. if not zero.
				if (length(m_Vel) > 0.0001)
					Velocity = 1-(dot(normalize(m_Vel), Dir)+1)/2;

				m_Vel += Dir*a*(Velocity*0.75f);
				m_Vel *= 0.85f;
			}

			// handle hook influence
			if(m_HookedPlayer == i && m_pWorld->m_Tuning.m_PlayerHooking)
			{
				if(Distance > PhysSize*1.50f) // TODO: fix tweakable variable
				{
					float Accel = m_pWorld->m_Tuning.m_HookDragAccel * (Distance/m_pWorld->m_Tuning.m_HookLength);
					float DragSpeed = m_pWorld->m_Tuning.m_HookDragSpeed;

					// add force to the hooked player
					pCharCore->m_Vel.x = SaturatedAdd(-DragSpeed, DragSpeed, pCharCore->m_Vel.x, Accel*Dir.x*1.5f);
					pCharCore->m_Vel.y = SaturatedAdd(-DragSpeed, DragSpeed, pCharCore->m_Vel.y, Accel*Dir.y*1.5f);

					// add a little bit force to the guy who has the grip
					m_Vel.x = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.x, -Accel*Dir.x*0.25f);
					m_Vel.y = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.y, -Accel*Dir.y*0.25f);
				}
			}
		}

		// SpeedUps Prediction
		if(InTileSpeed)
		{
			int CurrentIndex = m_pCollision->GetPureMapIndex(m_Pos);
			vec2 Direction, MaxVel, TempVel = m_Vel;
			int Force, MaxSpeed = 0;
			float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
			m_pCollision->GetSpeedUp(CurrentIndex, &Direction, &Force, &MaxSpeed);
			if(Force == 255 && MaxSpeed)
			{
				m_Vel = Direction * (MaxSpeed/5);
			}
			else
			{
				if(MaxSpeed > 0 && MaxSpeed < 5) MaxSpeed = 5;
				if(MaxSpeed > 0)
				{
					if(Direction.x > 0.0000001f)
						SpeederAngle = -atan(Direction.y / Direction.x);
					else if(Direction.x < 0.0000001f)
						SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * asin(1.0f);
					else if(Direction.y > 0.0000001f)
						SpeederAngle = asin(1.0f);
					else
						SpeederAngle = asin(-1.0f);

					if(SpeederAngle < 0)
						SpeederAngle = 4.0f * asin(1.0f) + SpeederAngle;

					if(TempVel.x > 0.0000001f)
						TeeAngle = -atan(TempVel.y / TempVel.x);
					else if(TempVel.x < 0.0000001f)
						TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * asin(1.0f);
					else if(TempVel.y > 0.0000001f)
						TeeAngle = asin(1.0f);
					else
						TeeAngle = asin(-1.0f);

					if(TeeAngle < 0)
						TeeAngle = 4.0f * asin(1.0f) + TeeAngle;

					TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

					DiffAngle = SpeederAngle - TeeAngle;
					SpeedLeft = MaxSpeed / 5.0f - cos(DiffAngle) * TeeSpeed;

					if(abs(SpeedLeft) > Force && SpeedLeft > 0.0000001f)
						TempVel += Direction * Force;
					else if(abs(SpeedLeft) > Force)
						TempVel += Direction * -Force;
					else
						TempVel += Direction * SpeedLeft;
				}
				else
					TempVel += Direction * Force;

				m_Vel = TempVel;
			}
		}

		// Only in DDRace (Special Weapons) // FIXME: Use vanilla weapons causes mistakes!
		if (str_find_nocase(m_pWorld->m_aGameType, "ddrace"))
		{
			// jetpack and ninjajetpack prediction
			if(!InTileFreeze && UseInput && (m_Input.m_Fire&1) && (m_ActiveWeapon == WEAPON_GUN || m_ActiveWeapon == WEAPON_NINJA))
				m_Vel += TargetDirection * -1.0f * (m_pWorld->m_Tuning.m_JetpackStrength / 100.0f / 6.11f);
		}
		//

		// Predict Stoppers!
		int TilesIndexGame[5] = { -1, -1, -1, -1, -1 }, TilesIndexFront[5] = { -1, -1, -1, -1, -1 }, TilesIndexSwitch[5] = { -1, -1, -1, -1, -1 };
		int TilesFlagsGame[5] = { -1, -1, -1, -1, -1 }, TilesFlagsFront[5] = { -1, -1, -1, -1, -1 }, TilesFlagsSwitch[5] = { -1, -1, -1, -1, -1 };
		m_pCollision->GetRadTiles(CCollision::TILEMAP_GAME, m_Pos, TilesIndexGame, TilesFlagsGame);
		m_pCollision->GetRadTiles(CCollision::TILEMAP_FRONT, m_Pos, TilesIndexFront, TilesFlagsFront);
		m_pCollision->GetRadTiles(CCollision::TILEMAP_SWITCH, m_Pos, TilesIndexSwitch, TilesFlagsSwitch, m_Team);

		if (((TilesIndexGame[0] == TILE_STOP && TilesFlagsGame[0] == ROTATION_270) ||
		      (TilesIndexGame[1] == TILE_STOP && TilesFlagsGame[1] == ROTATION_270) ||
			  (TilesIndexGame[1] == TILE_STOPS && (TilesFlagsGame[1] == ROTATION_90 || TilesFlagsGame[1] == ROTATION_270)) ||
			  (TilesIndexGame[1] == TILE_STOPA) ||
			  (TilesIndexFront[0] == TILE_STOP && TilesFlagsFront[0] == ROTATION_270) ||
			  (TilesIndexFront[1] == TILE_STOP && TilesFlagsFront[1] == ROTATION_270) ||
			  (TilesIndexFront[1] == TILE_STOPS && (TilesFlagsFront[1] == ROTATION_90 || TilesFlagsFront[1] == ROTATION_270)) ||
			  (TilesIndexFront[1] == TILE_STOPA) ||
			  (TilesIndexSwitch[0] == TILE_STOP && TilesFlagsSwitch[0] == ROTATION_270) ||
			  (TilesIndexSwitch[1] == TILE_STOP && TilesFlagsSwitch[1] == ROTATION_270) ||
			  (TilesIndexSwitch[1] == TILE_STOPS && (TilesFlagsSwitch[1] == ROTATION_90 || TilesFlagsSwitch[1] == ROTATION_270)) ||
			  (TilesIndexSwitch[1] == TILE_STOPA)) && m_Vel.x > 0)
		{
			if((int)m_pCollision->GetPos(TilesIndexGame[1]).x < (int)m_Pos.x)
				m_Pos = PrevPos;

			m_Vel.x = 0;
		}
		if (((TilesIndexGame[0] == TILE_STOP && TilesFlagsGame[0] == ROTATION_90) ||
				(TilesIndexGame[2] == TILE_STOP && TilesFlagsGame[2] == ROTATION_90) ||
				(TilesIndexGame[2] == TILE_STOPS && (TilesFlagsGame[2] == ROTATION_90 || TilesFlagsGame[2] == ROTATION_270)) ||
				(TilesIndexGame[2] == TILE_STOPA) ||
				(TilesIndexFront[0] == TILE_STOP && TilesFlagsFront[0] == ROTATION_90) ||
				(TilesIndexFront[2] == TILE_STOP && TilesFlagsFront[2] == ROTATION_90) ||
				(TilesIndexFront[2] == TILE_STOPS && (TilesFlagsFront[2] == ROTATION_90 || TilesFlagsFront[2] == ROTATION_270)) ||
				(TilesIndexFront[2] == TILE_STOPA) ||
				(TilesIndexSwitch[0] == TILE_STOP && TilesFlagsSwitch[0] == ROTATION_90) ||
				(TilesIndexSwitch[2] == TILE_STOP && TilesFlagsSwitch[2] == ROTATION_90) ||
				(TilesIndexSwitch[2] == TILE_STOPS && (TilesFlagsSwitch[2] == ROTATION_90 || TilesFlagsSwitch[2] == ROTATION_270)) ||
				(TilesIndexSwitch[2] == TILE_STOPA)) && m_Vel.x < 0)
		{
			if((int)m_pCollision->GetPos(TilesIndexGame[2]).x < (int)m_Pos.x)
				m_Pos = PrevPos;

			m_Vel.x = 0;
		}
		if (((TilesIndexGame[0] == TILE_STOP && TilesFlagsGame[0] == ROTATION_180) ||
				(TilesIndexGame[4] == TILE_STOP && TilesFlagsGame[4] == ROTATION_180) ||
				(TilesIndexGame[4] == TILE_STOPS && (TilesFlagsGame[4] == ROTATION_0 || TilesFlagsGame[4] == ROTATION_180)) ||
				(TilesIndexGame[4] == TILE_STOPA) ||
				(TilesIndexFront[0] == TILE_STOP && TilesFlagsFront[0] == ROTATION_180) ||
				(TilesIndexFront[4] == TILE_STOP && TilesFlagsFront[4] == ROTATION_180) ||
				(TilesIndexFront[4] == TILE_STOPS && (TilesFlagsFront[4] == ROTATION_0 || TilesFlagsFront[4] == ROTATION_180)) ||
				(TilesIndexFront[4] == TILE_STOPA) ||
				(TilesIndexSwitch[0] == TILE_STOP && TilesFlagsSwitch[0] == ROTATION_180) ||
				(TilesIndexSwitch[4] == TILE_STOP && TilesFlagsSwitch[4] == ROTATION_180) ||
				(TilesIndexSwitch[4] == TILE_STOPS && (TilesFlagsSwitch[4] == ROTATION_0 || TilesFlagsSwitch[4] == ROTATION_180)) ||
				(TilesIndexSwitch[4] == TILE_STOPA)) && m_Vel.y < 0)
		{
			if((int)m_pCollision->GetPos(TilesIndexGame[4]).y < (int)m_Pos.y)
				m_Pos = PrevPos;

			m_Vel.y = 0;
		}
		if(((TilesIndexGame[0] == TILE_STOP && TilesFlagsGame[0] == ROTATION_0) ||
				(TilesIndexGame[3] == TILE_STOP && TilesFlagsGame[3] == ROTATION_0) ||
				(TilesIndexGame[3] == TILE_STOPS && (TilesFlagsGame[3] == ROTATION_0 || TilesFlagsGame[3] == ROTATION_180)) ||
				(TilesIndexGame[3] == TILE_STOPA) ||
				(TilesIndexFront[0] == TILE_STOP && TilesFlagsFront[0] == ROTATION_0) ||
				(TilesIndexFront[3] == TILE_STOP && TilesFlagsFront[3] == ROTATION_0) ||
				(TilesIndexFront[3] == TILE_STOPS && (TilesFlagsFront[3] == ROTATION_0 || TilesFlagsFront[3] == ROTATION_180)) ||
				(TilesIndexFront[3] == TILE_STOPA) ||
				(TilesIndexSwitch[0] == TILE_STOP && TilesFlagsSwitch[0] == ROTATION_0) ||
				(TilesIndexSwitch[3] == TILE_STOP && TilesFlagsSwitch[3] == ROTATION_0) ||
				(TilesIndexSwitch[3] == TILE_STOPS && (TilesFlagsSwitch[3] == ROTATION_0 || TilesFlagsSwitch[3] == ROTATION_180)) ||
				(TilesIndexSwitch[3] == TILE_STOPA)) && m_Vel.y > 0)
		{
			if((int)m_pCollision->GetPos(TilesIndexGame[3]).y < (int)m_Pos.y)
				m_Pos = PrevPos;

			m_Vel.y = 0;
			m_Jumped = 0;
		}
	}

	// clamp the velocity to something sane
	if(length(m_Vel) > 6000)
		m_Vel = normalize(m_Vel) * 6000;
}

void CCharacterCore::Move()
{
	float RampValue = VelocityRamp(length(m_Vel)*50, m_pWorld->m_Tuning.m_VelrampStart, m_pWorld->m_Tuning.m_VelrampRange, m_pWorld->m_Tuning.m_VelrampCurvature);

	m_Vel.x = m_Vel.x*RampValue;

	vec2 NewPos = m_Pos;
	m_pCollision->MoveBox(&NewPos, &m_Vel, vec2(28.0f, 28.0f), 0);

	m_Vel.x = m_Vel.x*(1.0f/RampValue);

	if(m_pWorld && m_pWorld->m_Tuning.m_PlayerCollision)
	{
		// check player collision
		float Distance = distance(m_Pos, NewPos);
		int End = Distance+1;
		vec2 LastPos = m_Pos;
		for(int i = 0; i < End; i++)
		{
			float a = i/Distance;
			vec2 Pos = mix(m_Pos, NewPos, a);
			for(int p = 0; p < MAX_CLIENTS; p++)
			{
				CCharacterCore *pCharCore = m_pWorld->m_apCharacters[p];
				if(!pCharCore || pCharCore == this)
					continue;
				float D = distance(Pos, pCharCore->m_Pos);
				if(D < 28.0f && D > 0.0f)
				{
					if(a > 0.0f)
						m_Pos = LastPos;
					else if(distance(NewPos, pCharCore->m_Pos) > D)
						m_Pos = NewPos;
					return;
				}
			}
			LastPos = Pos;
		}
	}

	m_Pos = NewPos;
}

void CCharacterCore::Write(CNetObj_CharacterCore *pObjCore)
{
	pObjCore->m_X = round(m_Pos.x);
	pObjCore->m_Y = round(m_Pos.y);

	pObjCore->m_VelX = round(m_Vel.x*256.0f);
	pObjCore->m_VelY = round(m_Vel.y*256.0f);
	pObjCore->m_HookState = m_HookState;
	pObjCore->m_HookTick = m_HookTick;
	pObjCore->m_HookX = round(m_HookPos.x);
	pObjCore->m_HookY = round(m_HookPos.y);
	pObjCore->m_HookDx = round(m_HookDir.x*256.0f);
	pObjCore->m_HookDy = round(m_HookDir.y*256.0f);
	pObjCore->m_HookedPlayer = m_HookedPlayer;
	pObjCore->m_Jumped = m_Jumped;
	pObjCore->m_Direction = m_Direction;
	pObjCore->m_Angle = m_Angle;
}

void CCharacterCore::Read(const CNetObj_CharacterCore *pObjCore)
{
	m_Pos.x = pObjCore->m_X;
	m_Pos.y = pObjCore->m_Y;
	m_Vel.x = pObjCore->m_VelX/256.0f;
	m_Vel.y = pObjCore->m_VelY/256.0f;
	m_HookState = pObjCore->m_HookState;
	m_HookTick = pObjCore->m_HookTick;
	m_HookPos.x = pObjCore->m_HookX;
	m_HookPos.y = pObjCore->m_HookY;
	m_HookDir.x = pObjCore->m_HookDx/256.0f;
	m_HookDir.y = pObjCore->m_HookDy/256.0f;
	m_HookedPlayer = pObjCore->m_HookedPlayer;
	m_Jumped = pObjCore->m_Jumped;
	m_Direction = pObjCore->m_Direction;
	m_Angle = pObjCore->m_Angle;
}

void CCharacterCore::Quantize()
{
	CNetObj_CharacterCore Core;
	Write(&Core);
	Read(&Core);
}

