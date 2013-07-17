/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/keys.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <game/client/gameclient.h>

#include "motd.h"

void CMotd::Clear()
{
	m_ServerMotdTime = 0;
}

bool CMotd::IsActive()
{
	return time_get() < m_ServerMotdTime;
}

void CMotd::OnStateChange(int NewState, int OldState)
{
	if(OldState == IClient::STATE_ONLINE || OldState == IClient::STATE_OFFLINE)
		Clear();
}

void CMotd::OnRender()
{
    //H-Client: IF Change
	if(!IsActive() || Graphics()->Tumbtail())
		return;

	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width, Height);

    CUIRect rBox, rTitle;
    rBox.h = 800.0f;
    rBox.w = 650.0f;
    rBox.x = Width/2 - rBox.w/2;
    rBox.y = 150.0f;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	RenderTools()->DrawUIRect(&rBox, vec4(0,0,0,0.5f), CUI::CORNER_T, 15.0f);
	rBox.HSplitTop(50.0f, &rTitle, &rBox);
	RenderTools()->DrawUIRect(&rTitle, vec4(1,1,1,0.5f), CUI::CORNER_T, 15.0f);
	TextRender()->Text(0, rTitle.x+5.0f, rTitle.y+5.0f, 32.0f, "MoTD", rBox.w-10.0f);

	TextRender()->Text(0, rBox.x+40.0f, rBox.y+40.0f, 22.0f, m_aServerMotd, (int)(rBox.w-80.0f));
}

void CMotd::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(MsgType == NETMSGTYPE_SV_MOTD)
	{
		CNetMsg_Sv_Motd *pMsg = (CNetMsg_Sv_Motd *)pRawMsg;

		// process escaping
		str_copy(m_aServerMotd, pMsg->m_pMessage, sizeof(m_aServerMotd));
		for(int i = 0; m_aServerMotd[i]; i++)
		{
			if(m_aServerMotd[i] == '\\')
			{
				if(m_aServerMotd[i+1] == 'n')
				{
					m_aServerMotd[i] = ' ';
					m_aServerMotd[i+1] = '\n';
					i++;
				}
			}
		}

		if(m_aServerMotd[0] && g_Config.m_ClMotdTime)
			m_ServerMotdTime = time_get()+time_freq()*g_Config.m_ClMotdTime;
		else
			m_ServerMotdTime = 0;
	}
}

bool CMotd::OnInput(IInput::CEvent Event)
{
	if(IsActive() && Event.m_Flags&IInput::FLAG_PRESS && Event.m_Key == KEY_ESCAPE)
	{
		Clear();
		return true;
	}
	return false;
}

