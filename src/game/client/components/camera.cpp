/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>

#include "camera.h"
#include "controls.h"

CCamera::CCamera()
{
	m_CamType = CAMTYPE_UNDEFINED;
	m_TimeEffect = 0;
}

void CCamera::OnRender()
{
	//vec2 center;
	m_Zoom = 1.0f;

	// update camera center
	if (Client()->State() == IClient::STATE_OFFLINE)
	{
        m_Center = vec2(32.0f*20.0f, 32.0f*15.0f);
        m_Zoom = 2.25f;
	}
	else if(m_pClient->m_Snap.m_SpecInfo.m_Active && !m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
	{
		if(m_CamType != CAMTYPE_SPEC)
		{
			m_pClient->m_pControls->m_MousePos = m_PrevCenter;
			m_pClient->m_pControls->ClampMousePos();
			m_CamType = CAMTYPE_SPEC;
		}
		m_Center = m_pClient->m_pControls->m_MousePos;
	}
	else
	{
		if(m_CamType != CAMTYPE_PLAYER)
		{
			m_pClient->m_pControls->ClampMousePos();
			m_CamType = CAMTYPE_PLAYER;
		}

		static vec2 CameraOffset(0, 0);
		static vec2 MaxVel(0, 0);
		vec2 PredictedCameraOffset(0, 0);

		float l = length(m_pClient->m_pControls->m_MousePos);
		if(l > 0.0001f) // make sure that this isn't 0
		{
		    if (g_Config.m_hcDynamicCamera)
		    {
		        float OffsetFactor = 5.0f;
		        int LocalCID = m_pClient->m_Snap.m_LocalClientID;
                CNetObj_Character CurChar = m_pClient->m_Snap.m_aCharacters[LocalCID].m_Cur;
                CNetObj_Character PrevChar = m_pClient->m_Snap.m_aCharacters[LocalCID].m_Prev;
                vec2 Vel = mix(vec2(PrevChar.m_VelX/256.0f, PrevChar.m_VelY/256.0f), vec2(CurChar.m_VelX/256.0f, CurChar.m_VelY/256.0f), Client()->IntraGameTick());

                if (Vel.x > MaxVel.x)
                    MaxVel.x = Vel.x;
                if (Vel.y > MaxVel.y)
                    MaxVel.y = Vel.y;

                //TODO: Vel.x return 0.02 when tee state without move
                if (Vel.x > -1.0f && Vel.x < 1.0f)
                {
                    Vel.x = 0.0f;
                    MaxVel.x = 0.0f;
                }
                if (Vel.y > -1.0f && Vel.y < 1.0f)
                {
                    Vel.y = 0.0f;
                    MaxVel.y = 0.0f;
                }


                if (Vel.x > -60.0f || MaxVel.x < 60.0f )
                    PredictedCameraOffset.x = MaxVel.x*8.0f;
                if (MaxVel.y > -60.0f || MaxVel.y < 60.0f)
                    PredictedCameraOffset.y = MaxVel.y*8.0f;

                //if (Client()->GameTick() - m_TimeEffect > Client()->GameTickSpeed()*0.5f)
                //{
                    if (PredictedCameraOffset.x > CameraOffset.x)
                        CameraOffset.x+=OffsetFactor;
                    else if (PredictedCameraOffset.x < CameraOffset.x)
                        CameraOffset.x-=OffsetFactor;

                    /*if (PredictedCameraOffset.y > CameraOffset.y)
                        CameraOffset.y+=OffsetFactor;
                    else if (PredictedCameraOffset.y < CameraOffset.y)
                        CameraOffset.y-=OffsetFactor;*/
                //}

                if (CameraOffset != PredictedCameraOffset)
                    m_TimeEffect = Client()->GameTick();

                //dbg_msg("h-client", "CHARACTER: %.2f -- %.2f ||| %.2f -- %.2f", CameraOffset.x, CameraOffset.y , PredictedCameraOffset.x, PredictedCameraOffset.y);
		    }
		    else
		    {
		        CameraOffset = vec2(0, 0);
                float DeadZone = g_Config.m_ClMouseDeadzone;
                float FollowFactor = g_Config.m_ClMouseFollowfactor/100.0f;
                float OffsetAmount = max(l-DeadZone, 0.0f) * FollowFactor;

                CameraOffset = normalize(m_pClient->m_pControls->m_MousePos)*OffsetAmount;
		    }
		}

		if(m_pClient->m_Snap.m_SpecInfo.m_Active)
			m_Center = m_pClient->m_Snap.m_SpecInfo.m_Position + CameraOffset;
		else
			m_Center = m_pClient->m_LocalCharacterPos + CameraOffset;
	}

	m_PrevCenter = m_Center;
}
