/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/serverbrowser.h> // H-Client
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/animstate.h> //H-Client
#include "emoticon.h"
#include <cstdio>

CEmoticon::CEmoticon()
{
	OnReset();
}

void CEmoticon::ConKeyEmoticon(IConsole::IResult *pResult, void *pUserData)
{
	CEmoticon *pSelf = (CEmoticon *)pUserData;
	if(!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
		pSelf->m_Active = pResult->GetInteger(0) != 0;
}

void CEmoticon::ConEmote(IConsole::IResult *pResult, void *pUserData)
{
	((CEmoticon *)pUserData)->Emote(pResult->GetInteger(0));
}

void CEmoticon::OnConsoleInit()
{
	Console()->Register("+emote", "", CFGFLAG_CLIENT, ConKeyEmoticon, this, "Open emote selector");
	Console()->Register("emote", "i", CFGFLAG_CLIENT, ConEmote, this, "Use emote");
}

void CEmoticon::OnReset()
{
	m_WasActive = false;
	m_Active = false;
	m_SelectedEmote = -1;
	m_SelectedEyes = -1; // H-Client
}

void CEmoticon::OnRelease()
{
	m_Active = false;
}

void CEmoticon::OnMessage(int MsgType, void *pRawMsg)
{
}

bool CEmoticon::OnMouseMove(float x, float y)
{
	if(!m_Active)
		return false;

	UI()->ConvertMouseMove(&x, &y);
	m_SelectorMouse += vec2(x,y);
	return true;
}

void CEmoticon::DrawCircle(float x, float y, float r, int Segments)
{
	IGraphics::CFreeformItem Array[32];
	int NumItems = 0;
	float FSegments = (float)Segments;
	for(int i = 0; i < Segments; i+=2)
	{
		float a1 = i/FSegments * 2*pi;
		float a2 = (i+1)/FSegments * 2*pi;
		float a3 = (i+2)/FSegments * 2*pi;
		float Ca1 = cosf(a1);
		float Ca2 = cosf(a2);
		float Ca3 = cosf(a3);
		float Sa1 = sinf(a1);
		float Sa2 = sinf(a2);
		float Sa3 = sinf(a3);

		Array[NumItems++] = IGraphics::CFreeformItem(
			x, y,
			x+Ca1*r, y+Sa1*r,
			x+Ca3*r, y+Sa3*r,
			x+Ca2*r, y+Sa2*r);
		if(NumItems == 32)
		{
			m_pClient->Graphics()->QuadsDrawFreeform(Array, 32);
			NumItems = 0;
		}
	}
	if(NumItems)
		m_pClient->Graphics()->QuadsDrawFreeform(Array, NumItems);
}


void CEmoticon::OnRender()
{
    CServerInfo SInfo;
    Client()->GetServerInfo(&SInfo);

	if(!m_Active)
	{
		if(m_WasActive)
		{
            if (m_SelectedEmote != -1)
                Emote(m_SelectedEmote);
            else if (str_find_nocase(SInfo.m_aGameType, "ddrace") && m_SelectedEyes != -1)
                Eyes(m_SelectedEyes);
        }
		m_WasActive = false;
		return;
	}

	if(m_pClient->m_Snap.m_SpecInfo.m_Active)
	{
		m_Active = false;
		m_WasActive = false;
		return;
	}

	m_WasActive = true;

	if (length(m_SelectorMouse) > 170.0f)
		m_SelectorMouse = normalize(m_SelectorMouse) * 170.0f;

	float SelectedAngle = GetAngle(m_SelectorMouse) + 2*pi/24;
	if (SelectedAngle < 0)
		SelectedAngle += 2*pi;

    float mouselen = length(m_SelectorMouse);
	if (mouselen > 110.0f)
	{
        m_SelectedEyes = -1;
		m_SelectedEmote = (int)(SelectedAngle / (2*pi) * NUM_EMOTICONS);
    } else if (str_find_nocase(SInfo.m_aGameType, "ddrace") && mouselen > 50.0f && mouselen < 110.0f) // H-Client
    {
        m_SelectedEmote = -1;
		m_SelectedEyes = (int)(SelectedAngle / (2*pi) * NUM_EMOTES);
    } else
    {
        m_SelectedEyes = -1;
        m_SelectedEmote = -1;
    }

	CUIRect Screen = *UI()->Screen();

	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	Graphics()->BlendNormal();

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.3f);
	DrawCircle(Screen.w/2, Screen.h/2, 190.0f, 64);
	Graphics()->QuadsEnd();

    // H-Client
    if (str_find_nocase(SInfo.m_aGameType, "ddrace"))
    {
        Graphics()->TextureSet(-1);
        Graphics()->QuadsBegin();
        Graphics()->SetColor(60,60,60,0.3f);
        DrawCircle(Screen.w/2, Screen.h/2, 110.0f, 64);
        Graphics()->QuadsEnd();
	}
	//

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
	Graphics()->QuadsBegin();

	for (int i = 0; i < NUM_EMOTICONS; i++)
	{
		float Angle = 2*pi*i/NUM_EMOTICONS;
		if (Angle > pi)
			Angle -= 2*pi;

		bool Selected = m_SelectedEmote == i;

		float Size = Selected ? 80.0f : 50.0f;

		float NudgeX = 150.0f * cosf(Angle);
		float NudgeY = 150.0f * sinf(Angle);
		RenderTools()->SelectSprite(SPRITE_OOP + i);
		IGraphics::CQuadItem QuadItem(Screen.w/2 + NudgeX, Screen.h/2 + NudgeY, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
    Graphics()->QuadsEnd();

    if (str_find_nocase(SInfo.m_aGameType, "ddrace"))
    {
        for (int i = 0; i < NUM_EMOTES; i++)
        {
            float Angle = 2*pi*i/NUM_EMOTES;
            if (Angle > pi)
                Angle -= 2*pi;

            bool Selected = m_SelectedEyes == i;

            float Size = Selected ? 80.0f : 50.0f;

            float NudgeX = 80.0f * cosf(Angle);
            float NudgeY = 80.0f * sinf(Angle);

            CTeeRenderInfo teeRenderInfo = m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID].m_RenderInfo;
            teeRenderInfo.m_Size = Size;

            Graphics()->TextureSet(teeRenderInfo.m_Texture);
            Graphics()->QuadsBegin();
            Graphics()->SetColor(teeRenderInfo.m_ColorBody.r, teeRenderInfo.m_ColorBody.g, teeRenderInfo.m_ColorBody.b, teeRenderInfo.m_ColorBody.a);

            switch (i)
            {
                case EMOTE_PAIN:
                    RenderTools()->SelectSprite(SPRITE_TEE_EYE_PAIN, 0, 0, 0);
                    break;
                case EMOTE_HAPPY:
                    RenderTools()->SelectSprite(SPRITE_TEE_EYE_HAPPY, 0, 0, 0);
                    break;
                case EMOTE_SURPRISE:
                    RenderTools()->SelectSprite(SPRITE_TEE_EYE_SURPRISE, 0, 0, 0);
                    break;
                case EMOTE_ANGRY:
                    RenderTools()->SelectSprite(SPRITE_TEE_EYE_ANGRY, 0, 0, 0);
                    break;
                default:
                    RenderTools()->SelectSprite(SPRITE_TEE_EYE_NORMAL, 0, 0, 0);
                    break;
            }

            vec2 Direction = vec2(-1,0);
            float BaseSize = teeRenderInfo.m_Size;
            float EyeScale = BaseSize*0.40f;
            float h = i == EMOTE_BLINK ? BaseSize*0.15f : EyeScale;
            float EyeSeparation = (0.075f - 0.010f*absolute(Direction.x))*BaseSize;
            vec2 Offset = vec2(Direction.x*0.125f, -0.05f+Direction.y*0.10f)*BaseSize;
            vec2 BodyPos = vec2(Screen.w/2 + NudgeX, Screen.h/2 + NudgeY);
            IGraphics::CQuadItem Array[2] = {
                IGraphics::CQuadItem(BodyPos.x-EyeSeparation+Offset.x, BodyPos.y+Offset.y, EyeScale, h),
                IGraphics::CQuadItem(BodyPos.x+EyeSeparation+Offset.x, BodyPos.y+Offset.y, -EyeScale, h)};
            Graphics()->QuadsDraw(Array, 2);

            Graphics()->QuadsEnd();
        }
	}

	//Graphics()->QuadsEnd();

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);
	IGraphics::CQuadItem QuadItem(m_SelectorMouse.x+Screen.w/2,m_SelectorMouse.y+Screen.h/2,24,24);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}

void CEmoticon::Emote(int Emoticon)
{
	CNetMsg_Cl_Emoticon Msg;
	Msg.m_Emoticon = Emoticon;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

// H-Client
void CEmoticon::Eyes(int SelEyes)
{
    const char *emoteNames[] = { "normal", "pain", "happy", "surprise", "angry", "blink" };
    char cmdEmote[128];

    int time = 3;
    sscanf(g_Config.m_hcEyesSelectorTime, "%d", &time);
    str_format(cmdEmote, sizeof(cmdEmote), "/emote %s %d", emoteNames[SelEyes], time);
	CNetMsg_Cl_Say Msg;
	Msg.m_Team = 0;
	Msg.m_pMessage = cmdEmote;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}
//
