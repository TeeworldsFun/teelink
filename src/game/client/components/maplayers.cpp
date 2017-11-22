/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/demo.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>
#include <game/client/render.h>

#include <game/client/components/camera.h>
#include <game/client/components/mapimages.h>


#include "maplayers.h"

CMapLayers::CMapLayers(int t)
{
	m_Type = t;
	m_pLayers = 0;
	m_CurrentLocalTick = 0;
	m_LastLocalTick = 0;
	m_EnvelopeUpdate = false;
}

void CMapLayers::OnInit()
{
	m_pLayers = Layers();
}

void CMapLayers::EnvelopeUpdate()
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		m_CurrentLocalTick = pInfo->m_CurrentTick;
		m_LastLocalTick = pInfo->m_CurrentTick;
		m_EnvelopeUpdate = true;
	}
}


void CMapLayers::MapScreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup)
{
	//H-Client
	if (Graphics()->ShowInfoKills())
        Graphics()->MapScreen(0.0f, 0.0f, Collision()->GetWidth()*32, Collision()->GetHeight()*32);
	else if (Graphics()->Thumbnail())
	{
		// FIXME: Don't use 'magic' numbers..
		Graphics()->MapScreen(0.0f, 0.0f, Collision()->GetWidth()*32*7, Collision()->GetHeight()*32*6);
	}
    else
    {
        float Points[4];

        RenderTools()->MapscreenToWorld(CenterX, CenterY, pGroup->m_ParallaxX/100.0f, pGroup->m_ParallaxY/100.0f,
            pGroup->m_OffsetX, pGroup->m_OffsetY, Graphics()->ScreenAspect(), 1.0f, Points);
        Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
    }
}

void CMapLayers::EnvelopeEval(float TimeOffset, int Env, float *pChannels, void *pUser)
{
	CMapLayers *pThis = (CMapLayers *)pUser;
	pChannels[0] = 0;
	pChannels[1] = 0;
	pChannels[2] = 0;
	pChannels[3] = 0;

	CEnvPoint *pPoints = 0;

	{
		int Start, Num;
		pThis->m_pLayers->Map()->GetType(MAPITEMTYPE_ENVPOINTS, &Start, &Num);
		if(Num)
			pPoints = (CEnvPoint *)pThis->m_pLayers->Map()->GetItem(Start, 0, 0);
	}

	int Start, Num;
	pThis->m_pLayers->Map()->GetType(MAPITEMTYPE_ENVELOPE, &Start, &Num);

	if(Env >= Num)
		return;

	CMapItemEnvelope *pItem = (CMapItemEnvelope *)pThis->m_pLayers->Map()->GetItem(Start+Env, 0, 0);

	static float s_Time = 0.0f;
	static float s_LastLocalTime = pThis->Client()->LocalTime();
	if(pThis->Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = pThis->DemoPlayer()->BaseInfo();

		if(!pInfo->m_Paused || pThis->m_EnvelopeUpdate)
		{
			if(pThis->m_CurrentLocalTick != pInfo->m_CurrentTick)
			{
				pThis->m_LastLocalTick = pThis->m_CurrentLocalTick;
				pThis->m_CurrentLocalTick = pInfo->m_CurrentTick;
			}

			s_Time = mix(pThis->m_LastLocalTick / (float)pThis->Client()->GameTickSpeed(),
						pThis->m_CurrentLocalTick / (float)pThis->Client()->GameTickSpeed(),
						pThis->Client()->IntraGameTick());
		}

		pThis->RenderTools()->RenderEvalEnvelope(pPoints+pItem->m_StartPoint, pItem->m_NumPoints, 4, s_Time+TimeOffset, pChannels);
	}
	else
	{
		if(pThis->m_pClient->m_Snap.m_pGameInfoObj && !(pThis->m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
		{
			if(pItem->m_Version < 2 || pItem->m_Synchronized)
			{
				s_Time = mix((pThis->Client()->PrevGameTick()-pThis->m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick) / (float)pThis->Client()->GameTickSpeed(),
							(pThis->Client()->GameTick()-pThis->m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick) / (float)pThis->Client()->GameTickSpeed(),
							pThis->Client()->IntraGameTick());
			}
			else
				s_Time += pThis->Client()->LocalTime()-s_LastLocalTime;
		}
		pThis->RenderTools()->RenderEvalEnvelope(pPoints+pItem->m_StartPoint, pItem->m_NumPoints, 4, s_Time+TimeOffset, pChannels);
		s_LastLocalTime = pThis->Client()->LocalTime();
	}
}

void CMapLayers::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	CUIRect Screen;
	Graphics()->GetScreen(&Screen.x, &Screen.y, &Screen.w, &Screen.h);

	vec2 Center = m_pClient->m_pCamera->m_Center;
	//float center_x = gameclient.camera->center.x;
	//float center_y = gameclient.camera->center.y;

    // H-Client
    CMapItemLayerTilemap *pGTMap = m_pLayers->GameLayer();
    CMapItemLayerTilemap *pFTMap = m_pLayers->FrontLayer();
    CTile *pGameTiles = (CTile *)m_pLayers->Map()->GetData(pGTMap->m_Data);
    CTile *pFrontTiles = 0x0;
    if (pFTMap)
        pFrontTiles = (CTile *)m_pLayers->Map()->GetData(pFTMap->m_Data);

    CServerInfo Info;
    Client()->GetServerInfo(&Info);
    //

	bool PassedGameLayer = false;
	bool RenderBkg = true;

	for(int g = 0; g < m_pLayers->NumGroups(); g++)
	{
		CMapItemGroup *pGroup = m_pLayers->GetGroup(g);
		if (!pGroup)
            continue;

		if(!g_Config.m_GfxNoclip && pGroup->m_Version >= 2 && pGroup->m_UseClipping)
		{
			// set clipping
			float Points[4];
			MapScreenToGroup(Center.x, Center.y, m_pLayers->GameGroup());
			Graphics()->GetScreen(&Points[0], &Points[1], &Points[2], &Points[3]);
			float x0 = (pGroup->m_ClipX - Points[0]) / (Points[2]-Points[0]);
			float y0 = (pGroup->m_ClipY - Points[1]) / (Points[3]-Points[1]);
			float x1 = ((pGroup->m_ClipX+pGroup->m_ClipW) - Points[0]) / (Points[2]-Points[0]);
			float y1 = ((pGroup->m_ClipY+pGroup->m_ClipH) - Points[1]) / (Points[3]-Points[1]);

			Graphics()->ClipEnable((int)(x0*Graphics()->ScreenWidth()), (int)(y0*Graphics()->ScreenHeight()),
				(int)((x1-x0)*Graphics()->ScreenWidth()), (int)((y1-y0)*Graphics()->ScreenHeight()));
		}

		MapScreenToGroup(Center.x, Center.y, pGroup);

		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			CMapItemLayer *pLayer = m_pLayers->GetLayer(pGroup->m_StartLayer+l);
			bool Render = false;
			bool IsGameLayer = false;

			if(pLayer == (CMapItemLayer*)m_pLayers->GameLayer())
			{
				IsGameLayer = true;
				PassedGameLayer = 1;
			}

			// skip rendering if detail layers if not wanted
			if((pLayer->m_Flags&LAYERFLAG_DETAIL) && !g_Config.m_GfxHighDetail && !IsGameLayer)
				continue;

			if(m_Type == -1)
				Render = true;
			else if(m_Type == 0)
			{
				if(PassedGameLayer)
					return;
				Render = true;
			}
			else
			{
				if(PassedGameLayer && !IsGameLayer)
					Render = true;
			}

			if(Render && pLayer->m_Type == LAYERTYPE_TILES && Input()->KeyPressed(KEY_LCTRL) && Input()->KeyPressed(KEY_LSHIFT) && Input()->KeyDown(KEY_KP0))
			{
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				CTile *pTiles = (CTile *)m_pLayers->Map()->GetData(pTMap->m_Data);
				CServerInfo CurrentServerInfo;
				Client()->GetServerInfo(&CurrentServerInfo);
				char aFilename[256];
				str_format(aFilename, sizeof(aFilename), "dumps/tilelayer_dump_%s-%d-%d-%dx%d.txt", CurrentServerInfo.m_aMap, g, l, pTMap->m_Width, pTMap->m_Height);
				IOHANDLE File = Storage()->OpenFile(aFilename, IOFLAG_WRITE, IStorage::TYPE_SAVE);
				if(File)
				{
					for(int y = 0; y < pTMap->m_Height; y++)
					{
						for(int x = 0; x < pTMap->m_Width; x++)
							io_write(File, &(pTiles[y*pTMap->m_Width + x].m_Index), sizeof(pTiles[y*pTMap->m_Width + x].m_Index));
						io_write_newline(File);
					}
					io_close(File);
				}
			}

			if(Render && !IsGameLayer)
			{
				//layershot_begin();

				if(pLayer->m_Type == LAYERTYPE_TILES)
				{
					CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
					if(pTMap->m_Image == -1)
						Graphics()->TextureSet(-1);
					else
						Graphics()->TextureSet(m_pClient->m_pMapimages->Get(pTMap->m_Image));

					CTile *pTiles = (CTile *)m_pLayers->Map()->GetData(pTMap->m_Data);
					Graphics()->BlendNone();
					vec4 Color = vec4(pTMap->m_Color.r/255.0f, pTMap->m_Color.g/255.0f, pTMap->m_Color.b/255.0f, pTMap->m_Color.a/255.0f);

                    // H-Client
                    if (pGameTiles && g_Config.m_ddrShowHiddenWays && str_find_nocase(Info.m_aGameType, "race"))
                    {
                        Graphics()->BlendNone();
                        // FIXME: 100000000000000 parameters for the method not its a good implementation uh?
                        RenderTools()->RenderTilemap(pFrontTiles, (pFTMap)?pFTMap->m_Width:-1, (pFTMap)?pFTMap->m_Height:-1, pGameTiles, pGTMap->m_Width, pGTMap->m_Height, pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND|LAYERRENDERFLAG_OPAQUE,
                                                         EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);
                        Graphics()->BlendNormal();
                        RenderTools()->RenderTilemap(pFrontTiles, (pFTMap)?pFTMap->m_Width:-1, (pFTMap)?pFTMap->m_Height:-1, pGameTiles, pGTMap->m_Width, pGTMap->m_Height, pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND|LAYERRENDERFLAG_TRANSPARENT,
                                                        EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);
                    }
                    else
                    {
                        Graphics()->BlendNone();
                        RenderTools()->RenderTilemap(0x0, -1, -1, 0x0, -1, -1, pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND|LAYERRENDERFLAG_OPAQUE,
                                                        EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);
                        Graphics()->BlendNormal();
                        RenderTools()->RenderTilemap(0x0, -1, -1, 0x0, -1, -1, pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND|LAYERRENDERFLAG_TRANSPARENT,
                                                        EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);
                    }
				}
				else if(pLayer->m_Type == LAYERTYPE_QUADS)
				{
				    // H-Client
				    if (Graphics()->ShowInfoKills() && RenderBkg)
                    {
                        Graphics()->ShowInfoKills(false);
                        MapScreenToGroup(Center.x, Center.y, pGroup);

                        CQuad *pQuad = NewQuad(0, 0);
                        const int Width = 800000;
                        const int Height = 600000;
                        pQuad->m_aPoints[0].x = pQuad->m_aPoints[2].x = -Width;
                        pQuad->m_aPoints[1].x = pQuad->m_aPoints[3].x = Width;
                        pQuad->m_aPoints[0].y = pQuad->m_aPoints[1].y = -Height;
                        pQuad->m_aPoints[2].y = pQuad->m_aPoints[3].y = Height;
                        pQuad->m_aPoints[4].x = pQuad->m_aPoints[4].y = 0;
                        pQuad->m_aColors[0].r = pQuad->m_aColors[1].r = 94;
                        pQuad->m_aColors[0].g = pQuad->m_aColors[1].g = 132;
                        pQuad->m_aColors[0].b = pQuad->m_aColors[1].b = 174;
                        pQuad->m_aColors[2].r = pQuad->m_aColors[3].r = 204;
                        pQuad->m_aColors[2].g = pQuad->m_aColors[3].g = 232;
                        pQuad->m_aColors[2].b = pQuad->m_aColors[3].b = 255;

                        Graphics()->TextureSet(-1);
                        Graphics()->BlendNone();
                        RenderTools()->RenderQuads(pQuad, 1, LAYERRENDERFLAG_OPAQUE, EnvelopeEval, this);
                        Graphics()->BlendNormal();
                        RenderTools()->RenderQuads(pQuad, 1, LAYERRENDERFLAG_TRANSPARENT, EnvelopeEval, this);

                        Graphics()->ShowInfoKills(true);
                        MapScreenToGroup(Center.x, Center.y, pGroup);
                        RenderBkg = false;
                    }
                    //

					CMapItemLayerQuads *pQLayer = (CMapItemLayerQuads *)pLayer;
					if(pQLayer->m_Image == -1)
						Graphics()->TextureSet(-1);
					else
						Graphics()->TextureSet(m_pClient->m_pMapimages->Get(pQLayer->m_Image));

					CQuad *pQuads = (CQuad *)m_pLayers->Map()->GetDataSwapped(pQLayer->m_Data);

					Graphics()->BlendNone();
					RenderTools()->RenderQuads(pQuads, pQLayer->m_NumQuads, LAYERRENDERFLAG_OPAQUE, EnvelopeEval, this);
					Graphics()->BlendNormal();
					RenderTools()->RenderQuads(pQuads, pQLayer->m_NumQuads, LAYERRENDERFLAG_TRANSPARENT, EnvelopeEval, this);
				}

				//layershot_end();
			}
		}
		if(!g_Config.m_GfxNoclip)
			Graphics()->ClipDisable();
	}

// render screen sizes
	if(m_pClient->Graphics()->ShowInfoKills())
	{
		Graphics()->TextureSet(-1);
		Graphics()->LinesBegin();

		float aLastPoints[4];
		float Start = 1.0f; //9.0f/16.0f;
		float End = 16.0f/9.0f;
		const int NumSteps = 20;
		for(int i = 0; i <= NumSteps; i++)
		{
			float aPoints[4];
			float Aspect = Start + (End-Start)*(i/(float)NumSteps);

			RenderTools()->MapscreenToWorld(
				m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y,
				1.0f, 1.0f, 0.0f, 0.0f, Aspect, 1.0f, aPoints);

			if(i == 0)
			{
				IGraphics::CLineItem Array[2] = {
					IGraphics::CLineItem(aPoints[0], aPoints[1], aPoints[2], aPoints[1]),
					IGraphics::CLineItem(aPoints[0], aPoints[3], aPoints[2], aPoints[3])};
				Graphics()->LinesDraw(Array, 2);
			}

			if(i != 0)
			{
				IGraphics::CLineItem Array[4] = {
					IGraphics::CLineItem(aPoints[0], aPoints[1], aLastPoints[0], aLastPoints[1]),
					IGraphics::CLineItem(aPoints[2], aPoints[1], aLastPoints[2], aLastPoints[1]),
					IGraphics::CLineItem(aPoints[0], aPoints[3], aLastPoints[0], aLastPoints[3]),
					IGraphics::CLineItem(aPoints[2], aPoints[3], aLastPoints[2], aLastPoints[3])};
				Graphics()->LinesDraw(Array, 4);
			}

			if(i == NumSteps)
			{
				IGraphics::CLineItem Array[2] = {
					IGraphics::CLineItem(aPoints[0], aPoints[1], aPoints[0], aPoints[3]),
					IGraphics::CLineItem(aPoints[2], aPoints[1], aPoints[2], aPoints[3])};
				Graphics()->LinesDraw(Array, 2);
			}

			mem_copy(aLastPoints, aPoints, sizeof(aPoints));
		}

		if(1)
		{
			Graphics()->SetColor(1,0,0,1);
			for(int i = 0; i < 2; i++)
			{
				float aPoints[4];
				float aAspects[] = {4.0f/3.0f, 16.0f/10.0f, 5.0f/4.0f, 16.0f/9.0f};
				float Aspect = aAspects[i];

				RenderTools()->MapscreenToWorld(
					m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y,
					1.0f, 1.0f, 0.0f, 0.0f, Aspect, 1.0f, aPoints);

				CUIRect r;
				r.x = aPoints[0];
				r.y = aPoints[1];
				r.w = aPoints[2]-aPoints[0];
				r.h = aPoints[3]-aPoints[1];

				IGraphics::CLineItem Array[4] = {
					IGraphics::CLineItem(r.x, r.y, r.x+r.w, r.y),
					IGraphics::CLineItem(r.x+r.w, r.y, r.x+r.w, r.y+r.h),
					IGraphics::CLineItem(r.x+r.w, r.y+r.h, r.x, r.y+r.h),
					IGraphics::CLineItem(r.x, r.y+r.h, r.x, r.y)};
				Graphics()->LinesDraw(Array, 4);
				Graphics()->SetColor(0,1,0,1);
			}
		}

		Graphics()->LinesEnd();
	}

	if(!g_Config.m_GfxNoclip)
		Graphics()->ClipDisable();

	// reset the screen like it was before
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
}

CQuad *CMapLayers::NewQuad(int _x, int _y)
{
	CQuad *q = new CQuad();

	q->m_PosEnv = -1;
	q->m_ColorEnv = -1;
	q->m_PosEnvOffset = 0;
	q->m_ColorEnvOffset = 0;
	int x = _x, y = _y;
	q->m_aPoints[0].x = x;
	q->m_aPoints[0].y = y;
	q->m_aPoints[1].x = x+64;
	q->m_aPoints[1].y = y;
	q->m_aPoints[2].x = x;
	q->m_aPoints[2].y = y+64;
	q->m_aPoints[3].x = x+64;
	q->m_aPoints[3].y = y+64;

	q->m_aPoints[4].x = x+32; // pivot
	q->m_aPoints[4].y = y+32;

	for(int i = 0; i < 5; i++)
	{
		q->m_aPoints[i].x <<= 10;
		q->m_aPoints[i].y <<= 10;
	}


	q->m_aTexcoords[0].x = 0;
	q->m_aTexcoords[0].y = 0;

	q->m_aTexcoords[1].x = 1<<10;
	q->m_aTexcoords[1].y = 0;

	q->m_aTexcoords[2].x = 0;
	q->m_aTexcoords[2].y = 1<<10;

	q->m_aTexcoords[3].x = 1<<10;
	q->m_aTexcoords[3].y = 1<<10;

	q->m_aColors[0].r = 255; q->m_aColors[0].g = 255; q->m_aColors[0].b = 255; q->m_aColors[0].a = 255;
	q->m_aColors[1].r = 255; q->m_aColors[1].g = 255; q->m_aColors[1].b = 255; q->m_aColors[1].a = 255;
	q->m_aColors[2].r = 255; q->m_aColors[2].g = 255; q->m_aColors[2].b = 255; q->m_aColors[2].a = 255;
	q->m_aColors[3].r = 255; q->m_aColors[3].g = 255; q->m_aColors[3].b = 255; q->m_aColors[3].a = 255;

	return q;
}
