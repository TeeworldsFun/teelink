/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>
#include <base/math.h>
#include <engine/graphics.h>
#include <engine/shared/config.h> //H-Client
#include <game/client/components/maplayers.h> //H-Client
#include <game/client/components/effects.h> //H-Client
#include <game/generated/client_data.h> //H-Client
#include <game/generated/client_data.h> //H-Client

#include "render.h"

void CRenderTools::RenderEvalEnvelope(CEnvPoint *pPoints, int NumPoints, int Channels, float Time, float *pResult)
{
	if(NumPoints == 0)
	{
		pResult[0] = 0;
		pResult[1] = 0;
		pResult[2] = 0;
		pResult[3] = 0;
		return;
	}

	if(NumPoints == 1)
	{
		pResult[0] = fx2f(pPoints[0].m_aValues[0]);
		pResult[1] = fx2f(pPoints[0].m_aValues[1]);
		pResult[2] = fx2f(pPoints[0].m_aValues[2]);
		pResult[3] = fx2f(pPoints[0].m_aValues[3]);
		return;
	}

	Time = fmod(Time, pPoints[NumPoints-1].m_Time/1000.0f)*1000.0f;
	for(int i = 0; i < NumPoints-1; i++)
	{
		if(Time >= pPoints[i].m_Time && Time <= pPoints[i+1].m_Time)
		{
			float Delta = pPoints[i+1].m_Time-pPoints[i].m_Time;
			float a = (Time-pPoints[i].m_Time)/Delta;


			if(pPoints[i].m_Curvetype == CURVETYPE_SMOOTH)
				a = -2*a*a*a + 3*a*a; // second hermite basis
			else if(pPoints[i].m_Curvetype == CURVETYPE_SLOW)
				a = a*a*a;
			else if(pPoints[i].m_Curvetype == CURVETYPE_FAST)
			{
				a = 1-a;
				a = 1-a*a*a;
			}
			else if (pPoints[i].m_Curvetype == CURVETYPE_STEP)
				a = 0;
			else
			{
				// linear
			}

			for(int c = 0; c < Channels; c++)
			{
				float v0 = fx2f(pPoints[i].m_aValues[c]);
				float v1 = fx2f(pPoints[i+1].m_aValues[c]);
				pResult[c] = v0 + (v1-v0) * a;
			}

			return;
		}
	}

	pResult[0] = fx2f(pPoints[NumPoints-1].m_aValues[0]);
	pResult[1] = fx2f(pPoints[NumPoints-1].m_aValues[1]);
	pResult[2] = fx2f(pPoints[NumPoints-1].m_aValues[2]);
	pResult[3] = fx2f(pPoints[NumPoints-1].m_aValues[3]);
	return;
}


static void Rotate(CPoint *pCenter, CPoint *pPoint, float Rotation)
{
	int x = pPoint->x - pCenter->x;
	int y = pPoint->y - pCenter->y;
	pPoint->x = (int)(x * cosf(Rotation) - y * sinf(Rotation) + pCenter->x);
	pPoint->y = (int)(x * sinf(Rotation) + y * cosf(Rotation) + pCenter->y);
}

void CRenderTools::RenderQuads(CQuad *pQuads, int NumQuads, int RenderFlags, ENVELOPE_EVAL pfnEval, void *pUser)
{
	Graphics()->QuadsBegin();
	float Conv = 1/255.0f;
	for(int i = 0; i < NumQuads; i++)
	{
		CQuad *q = &pQuads[i];

		float r=1, g=1, b=1, a=1;

		if(q->m_ColorEnv >= 0)
		{
			float aChannels[4];
			pfnEval(q->m_ColorEnvOffset/1000.0f, q->m_ColorEnv, aChannels, pUser);
			r = aChannels[0];
			g = aChannels[1];
			b = aChannels[2];
			a = aChannels[3];
		}

		bool Opaque = false;
		if(a < 0.01f || (q->m_aColors[0].a < 0.01f && q->m_aColors[1].a < 0.01f && q->m_aColors[2].a < 0.01f && q->m_aColors[3].a < 0.01f))
			Opaque = true;

		if(Opaque && !(RenderFlags&LAYERRENDERFLAG_OPAQUE))
			continue;
		if(!Opaque && !(RenderFlags&LAYERRENDERFLAG_TRANSPARENT))
			continue;

		Graphics()->QuadsSetSubsetFree(
			fx2f(q->m_aTexcoords[0].x), fx2f(q->m_aTexcoords[0].y),
			fx2f(q->m_aTexcoords[1].x), fx2f(q->m_aTexcoords[1].y),
			fx2f(q->m_aTexcoords[2].x), fx2f(q->m_aTexcoords[2].y),
			fx2f(q->m_aTexcoords[3].x), fx2f(q->m_aTexcoords[3].y)
		);

		float OffsetX = 0;
		float OffsetY = 0;
		float Rot = 0;

		// TODO: fix this
		if(q->m_PosEnv >= 0)
		{
			float aChannels[4];
			pfnEval(q->m_PosEnvOffset/1000.0f, q->m_PosEnv, aChannels, pUser);
			OffsetX = aChannels[0];
			OffsetY = aChannels[1];
			Rot = aChannels[2]/360.0f*pi*2;
		}

		IGraphics::CColorVertex Array[4] = {
			IGraphics::CColorVertex(0, q->m_aColors[0].r*Conv*r, q->m_aColors[0].g*Conv*g, q->m_aColors[0].b*Conv*b, q->m_aColors[0].a*Conv*a),
			IGraphics::CColorVertex(1, q->m_aColors[1].r*Conv*r, q->m_aColors[1].g*Conv*g, q->m_aColors[1].b*Conv*b, q->m_aColors[1].a*Conv*a),
			IGraphics::CColorVertex(2, q->m_aColors[2].r*Conv*r, q->m_aColors[2].g*Conv*g, q->m_aColors[2].b*Conv*b, q->m_aColors[2].a*Conv*a),
			IGraphics::CColorVertex(3, q->m_aColors[3].r*Conv*r, q->m_aColors[3].g*Conv*g, q->m_aColors[3].b*Conv*b, q->m_aColors[3].a*Conv*a)};
		Graphics()->SetColorVertex(Array, 4);

		CPoint *pPoints = q->m_aPoints;

		if(Rot != 0)
		{
			static CPoint aRotated[4];
			aRotated[0] = q->m_aPoints[0];
			aRotated[1] = q->m_aPoints[1];
			aRotated[2] = q->m_aPoints[2];
			aRotated[3] = q->m_aPoints[3];
			pPoints = aRotated;

			Rotate(&q->m_aPoints[4], &aRotated[0], Rot);
			Rotate(&q->m_aPoints[4], &aRotated[1], Rot);
			Rotate(&q->m_aPoints[4], &aRotated[2], Rot);
			Rotate(&q->m_aPoints[4], &aRotated[3], Rot);
		}

		IGraphics::CFreeformItem Freeform(
			fx2f(pPoints[0].x)+OffsetX, fx2f(pPoints[0].y)+OffsetY,
			fx2f(pPoints[1].x)+OffsetX, fx2f(pPoints[1].y)+OffsetY,
			fx2f(pPoints[2].x)+OffsetX, fx2f(pPoints[2].y)+OffsetY,
			fx2f(pPoints[3].x)+OffsetX, fx2f(pPoints[3].y)+OffsetY);
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
	}
	Graphics()->QuadsEnd();
}

void CRenderTools::RenderTilemap(CTile *pGameTiles, CTile *pTiles, int w, int h, float Scale, vec4 Color, int RenderFlags,
									ENVELOPE_EVAL pfnEval, void *pUser, int ColorEnv, int ColorEnvOffset, int TileMineTee, bool Animated, void *pEffects)
{
	//Graphics()->TextureSet(img_get(tmap->image));
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	//Graphics()->MapScreen(screen_x0-50, screen_y0-50, screen_x1+50, screen_y1+50);
	//Graphics()->SetFrameBuffer();

	// calculate the final pixelsize for the tiles
	float TilePixelSize = 1024/32.0f;
	float FinalTileSize = Scale/(ScreenX1-ScreenX0) * Graphics()->ScreenWidth();
	float FinalTilesetScale = FinalTileSize/TilePixelSize;

	float r=1, g=1, b=1, a=1;
	if(ColorEnv >= 0)
	{
		float aChannels[4];
		pfnEval(ColorEnvOffset/1000.0f, ColorEnv, aChannels, pUser);
		r = aChannels[0];
		g = aChannels[1];
		b = aChannels[2];
		a = aChannels[3];
	}

    if (!Animated)
    {
        Graphics()->QuadsBegin();
        Graphics()->SetColor(Color.r*r, Color.g*g, Color.b*b, Color.a*a);
    }

	int StartY = (int)(ScreenY0/Scale)-1;
	int StartX = (int)(ScreenX0/Scale)-1;
	int EndY = (int)(ScreenY1/Scale)+1;
	int EndX = (int)(ScreenX1/Scale)+1;

	// adjust the texture shift according to mipmap level
	float TexSize = 1024.0f;
	float Frac = (1.25f/TexSize) * (1/FinalTilesetScale);
	float Nudge = (0.5f/TexSize) * (1/FinalTilesetScale);

	static float offsetEffX = 0.0f; //H-Client
	static float offsetEffY = 0.0f; //H-Client

	offsetEffX+=0.001f;
	offsetEffY+=0.001f;

    if (offsetEffY >= 1.0f)
        offsetEffY=0.0f;
    if (offsetEffX >= 1.0f)
        offsetEffX=0.0f;

	for(int y = StartY; y < EndY; y++)
		for(int x = StartX; x < EndX; x++)
		{
			int mx = x;
			int my = y;

			if(RenderFlags&TILERENDERFLAG_EXTEND)
			{
				if(mx<0)
					mx = 0;
				if(mx>=w)
					mx = w-1;
				if(my<0)
					my = 0;
				if(my>=h)
					my = h-1;
			}
			else
			{
				if(mx<0)
					continue; // mx = 0;
				if(mx>=w)
					continue; // mx = w-1;
				if(my<0)
					continue; // my = 0;
				if(my>=h)
					continue; // my = h-1;
			}

			int c = mx + my*w;

			unsigned char Index = pTiles[c].m_Index;
			if(Index)
			{
                //H-Client
                if (TileMineTee == 1 && pEffects)
                {
                    CEffects *pEff = static_cast<CEffects*>(pEffects);

                    if (Index == BLOCK_LUZ)
                        pEff->LightFlame(vec2(x*Scale+16.0f, y*Scale));
                    if (my>0)
                    {
                        int tu = mx + (my-1)*w;
                        if (Index == BLOCK_LAVA && pTiles[tu].m_Index == 0)
                        {
                            int rnd = rand()%512;
                            if (rnd == 2)
                                pEff->FireSplit(vec2((x<<5)+16.0f,(y<<5)+16.0f), vec2(0,-1));
                        }
                        else if (Index == BLOCK_HORNO_ON)
                        {
                            int rnd = rand()%215;
                            if (rnd == 2)
                                pEff->SmokeTrail(vec2((x<<5)+16.0f,(y<<5)-6.0f), vec2(0,-5));
                        }
                    }
                }

                if (g_Config.m_ddrShowHiddenWays)
                {
                    if (pGameTiles && !pGameTiles[c].m_Index)
                        Graphics()->SetColor(Color.r, Color.g, Color.b, 0.65f);
                }
                //

				unsigned char Flags = pTiles[c].m_Flags;

				bool Render = false;
				if(Flags&TILEFLAG_OPAQUE)
				{
					if(RenderFlags&LAYERRENDERFLAG_OPAQUE)
						Render = true;
				}
				else
				{
					if(RenderFlags&LAYERRENDERFLAG_TRANSPARENT)
						Render = true;
				}

				if(Render && !Animated)
				{

					int tx = Index%16;
					int ty = Index/16;
					int Px0 = tx*(1024/16);
					int Py0 = ty*(1024/16);
					int Px1 = Px0+(1024/16)-1;
					int Py1 = Py0+(1024/16)-1;

					float x0 = Nudge + Px0/TexSize+Frac;
					float y0 = Nudge + Py0/TexSize+Frac;
					float x1 = Nudge + Px1/TexSize-Frac;
					float y1 = Nudge + Py0/TexSize+Frac;
					float x2 = Nudge + Px1/TexSize-Frac;
					float y2 = Nudge + Py1/TexSize-Frac;
					float x3 = Nudge + Px0/TexSize+Frac;
					float y3 = Nudge + Py1/TexSize-Frac;

					if(Flags&TILEFLAG_VFLIP)
					{
						x0 = x2;
						x1 = x3;
						x2 = x3;
						x3 = x0;
					}

					if(Flags&TILEFLAG_HFLIP)
					{
						y0 = y3;
						y2 = y1;
						y3 = y1;
						y1 = y0;
					}

					if(Flags&TILEFLAG_ROTATE)
					{
						float Tmp = x0;
						x0 = x3;
						x3 = x2;
						x2 = x1;
						x1 = Tmp;
						Tmp = y0;
						y0 = y3;
						y3 = y2;
						y2 = y1;
						y1 = Tmp;
 					}

                    //H-Client
                    if (TileMineTee == 1 && ((Index >= BLOCK_UNDEF82 && Index <= BLOCK_AGUA) || (Index >= BLOCK_UNDEF104 && Index <= BLOCK_LAVA)))
                    {
                        x += pTiles[c].m_Skip;
                        continue;
                    }
                    if (TileMineTee == 2)
                        Graphics()->SetColor(0.35f, 0.35f, 0.35f, Color.a*a);

                    Graphics()->QuadsSetSubsetFree(x0, y0, x1, y1, x2, y2, x3, y3);
					IGraphics::CQuadItem QuadItem(x*Scale, y*Scale, Scale, Scale);
					Graphics()->QuadsDrawTL(&QuadItem, 1);
					Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a); //H-Client
				}
				else if (TileMineTee == 1 && Animated)
				{
                    //H-Client
                    if ((Index < BLOCK_UNDEF82 || Index > BLOCK_AGUA) && (Index < BLOCK_UNDEF104 || Index > BLOCK_LAVA))
                    {
                        x += pTiles[c].m_Skip;
                        continue;
                    }
                    if (Index >= BLOCK_UNDEF82 && Index <= BLOCK_AGUA)
                    {
                        Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MINETEE_FX_WATER].m_Id);
                        Graphics()->QuadsBegin();
                        Graphics()->QuadsSetSubsetFree(0,0+offsetEffY, 0,1+offsetEffY, 1,1+offsetEffY, 1,0+offsetEffY);
                    }
                    else if (Index >= BLOCK_UNDEF104 && Index <= BLOCK_LAVA)
                    {
                        Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MINETEE_FX_LAVA].m_Id);
                        Graphics()->QuadsBegin();
                        Graphics()->QuadsSetSubsetFree(0+offsetEffX,0, 0+offsetEffX,1, 1+offsetEffX,1, 1+offsetEffX,0);
                    }

                    IGraphics::CQuadItem QuadItem(x*Scale, y*Scale, Scale, Scale);
                    if (Index == BLOCK_UNDEF82 || Index == BLOCK_UNDEF104)
                        QuadItem = IGraphics::CQuadItem(x*Scale, y*Scale+(Scale-Scale/4), Scale, Scale/4);
                    else if (Index == BLOCK_UNDEF83 || Index == BLOCK_UNDEF105)
                        QuadItem = IGraphics::CQuadItem(x*Scale, y*Scale+Scale/2, Scale, Scale/2);

                    Graphics()->QuadsDrawTL(&QuadItem, 1);
                    Graphics()->QuadsEnd();
                }
			}

			x += pTiles[c].m_Skip;
		}

    if (!Animated)
        Graphics()->QuadsEnd();

	Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
}


void CRenderTools::UpdateLights(CCollision *pCollision, CTile *pTiles, CTile *pLights, int w, int h, int LightLevel)
{
    CTile *pLightsTemp = static_cast<CTile*>(mem_alloc(sizeof(CTile)*w*h, 1));
    mem_copy(pLightsTemp, pLights, sizeof(CTile)*w*h);

    int Darkness[5] = {0, 154, 170, 186, 202};
    float Scale = 32.0f;
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);

	int StartY = (int)(ScreenY0/Scale)-1;
	int StartX = (int)(ScreenX0/Scale)-1;
	int EndY = (int)(ScreenY1/Scale)+1;
	int EndX = (int)(ScreenX1/Scale)+1;

    StartX = max(StartX-25, 0);
    StartY = max(StartY-25, 0);
    EndX = min(EndX+25, w);
    EndY = min(EndY+25, h);

    //Clear shadows
	for(int y = StartY; y < EndY; y++)
		for(int x = StartX; x < EndX; x++)
		{
			int mx = clamp(x, 0, w-1);
			int my = clamp(y, 0, h-1);
            int c = mx + my * w;

            pLightsTemp[c].m_Index = Darkness[LightLevel];
            pLightsTemp[c].m_Reserved = 0;
        }

    //Shadows
	for(int y = StartY; y < EndY; y++)
		for(int x = StartX; x < EndX; x++)
		{
			int mx = clamp(x, 0, w-1);
			int my = clamp(y, 0, h-1);
            int c = mx + my*w;

            if (pTiles[c].m_Index && pTiles[c].m_Index != BLOCK_LAVA)
            {
                int tmy = my;
                int tc = mx + tmy*w;
                do
                {
                    //if (pTiles[tc].m_Index == 0)
                        pLightsTemp[tc].m_Index = 202;
                    //else
                    //    pLightsTemp[tc].m_Index = 218;

                    tmy++;
                    if (tmy >= h)
                        break;
                    tc = mx + tmy*w;
                } while (pTiles[tc].m_Index == 0);
            }
        }

    //Environment Lights
    if (LightLevel == 0)
    {
        for(int y = StartY; y < EndY; y++)
            for(int x = StartX; x < EndX; x++)
            {
                int mx = clamp(x, 0, w-1);
                int my = clamp(y, 0, h-1);
                int c = mx + my*w;

                if (pTiles[c].m_Index != 0 && pTiles[c].m_Index != BLOCK_LUZ && pTiles[c].m_Index != BLOCK_LAVA)
                {
                    bool light = false;
                    bool rl = false;
                    bool lr = false;
                    bool ud = false;
                    bool du = false;
                    bool d1 = false;
                    bool d2 = false;
                    bool d3 = false;
                    bool d4 = false;

                    int tc = (mx-1) + my*w;
                    tc = (mx-1) + (my-1)*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[tc].m_Index == 0)
                    {
                        d1 = true;
                        light = true;
                    }
                    tc = (mx+1) + (my-1)*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[tc].m_Index == 0)
                    {
                        d2 = true;
                        light = true;
                    }
                    tc = (mx+1) + (my+1)*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[tc].m_Index == 0)
                    {
                        d4 = true;
                        light = true;
                    }
                    tc = (mx-1) + (my+1)*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[tc].m_Index == 0)
                    {
                        d3 = true;
                        light = true;
                    }
                    tc = (mx-1) + my*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[tc].m_Index == 0)
                    {
                        lr = true;
                        light = true;
                    }
                    tc = (mx+1) + my*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[tc].m_Index == 0)
                    {
                        rl = true;
                        light = true;
                    }
                    tc = mx + (my-1)*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[tc].m_Index == 0)
                    {
                        ud = true;
                        light = true;
                    }
                    tc = mx + (my+1)*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[tc].m_Index == 0)
                    {
                        du = true;
                        light = true;
                    }

                    if (light)
                    {
                        pLightsTemp[c].m_Index = 0;

                        int Shadows[4] = {154, 170, 186, 202};

                        for (int i=1;i<=4;i++)
                        {
                            tc = mx + (my+i)*w;

                            if (tc <0 || tc >= w*h)
                                break;

                            if (lr)
                                tc = (mx+i) + my*w;
                            else if (rl)
                                tc = (mx-i) + my*w;

                            if (pTiles[tc].m_Index == 0 || pTiles[tc].m_Index == BLOCK_LUZ || pTiles[tc].m_Index == BLOCK_LAVA || Shadows[i-1] > pLightsTemp[tc].m_Index)
                                continue;
                            pLightsTemp[tc].m_Index = Shadows[i-1];
                        }
                    }
                }

                x += pTiles[c].m_Skip;
            }

        //Diffuse Shadow ->
        for(int y = StartY; y < EndY; y++)
            for(int x = StartX; x < EndX; x++)
            {
                int mx = clamp(x, 0, w-1);
                int my = clamp(y, 0, h-1);
                int c = mx + my*w;

                if (pLightsTemp[c].m_Index != 0)
                {
                    bool diffused = false;

                    int tcs[2] = { (mx-1)+my*w, mx+(my-1)*w };
                    for (int e=0; e<2; e++)
                    {
                        if (tcs[e] < 0 || tcs[e] >= w*h)
                            continue;

                        if (pLightsTemp[tcs[e]].m_Index == 0 && pTiles[c].m_Index == 0 && pTiles[tcs[e]].m_Index == 0)
                        {
                            pLightsTemp[c].m_Reserved = 1;
                            pLightsTemp[c].m_Index = 154;
                            break;
                        }
                        else if (pLightsTemp[tcs[e]].m_Index == 154 && pTiles[tcs[e]].m_Index == 0)
                        {
                            pLightsTemp[c].m_Reserved = 1;
                            pLightsTemp[c].m_Index = 170;
                            break;
                        }
                        else if (pLightsTemp[tcs[e]].m_Index == 170 && pTiles[tcs[e]].m_Index == 0)
                        {
                            pLightsTemp[c].m_Reserved = 1;
                            pLightsTemp[c].m_Index = 186;
                            break;
                        }
                        else if (pLightsTemp[tcs[e]].m_Index == 186 && pTiles[tcs[e]].m_Index == 0)
                        {
                            pLightsTemp[c].m_Reserved = 1;
                            pLightsTemp[c].m_Index = 202;
                            break;
                        }
                    }
                }
            }

        //Diffuse Shadow <-
        for(int y = StartY; y < EndY; y++)
            for(int x = EndX-1; x >= StartX; x--)
            {
                int mx = clamp(x, 0, w-1);
                int my = clamp(y, 0, h-1);
                int c = mx + my*w;

                if (pLightsTemp[c].m_Index != 0)
                {
                    bool diffused = false;

                    int tc = (mx+1)+my*w;
                    if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 0 && pTiles[c].m_Index == 0 && pTiles[tc].m_Index == 0 && (pLightsTemp[c].m_Reserved != 1 || (154 < pLightsTemp[c].m_Index && pLightsTemp[c].m_Reserved == 1)))
                    {
                        pLightsTemp[c].m_Reserved = 1;
                        pLightsTemp[c].m_Index = 154;
                    }
                    else if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 154 && pTiles[tc].m_Index == 0 && (pLightsTemp[c].m_Reserved != 1 || (170 < pLightsTemp[c].m_Index && pLightsTemp[c].m_Reserved == 1)))
                    {
                        pLightsTemp[c].m_Reserved = 1;
                        pLightsTemp[c].m_Index = 170;
                    }
                    else if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 170 && pTiles[tc].m_Index == 0 && (pLightsTemp[c].m_Reserved != 1 || (186 < pLightsTemp[c].m_Index && pLightsTemp[c].m_Reserved == 1)))
                    {
                        pLightsTemp[c].m_Reserved = 1;
                        pLightsTemp[c].m_Index = 186;
                    }
                    else if (tc >= 0 && tc < w*h && pLightsTemp[tc].m_Index == 186 && pTiles[tc].m_Index == 0 && (pLightsTemp[c].m_Reserved != 1 || (202 < pLightsTemp[c].m_Index && pLightsTemp[c].m_Reserved == 1)))
                    {
                        pLightsTemp[c].m_Reserved = 1;
                        pLightsTemp[c].m_Index = 202;
                    }
                }
            }
    }

    //Spot Lights & Lava
    int Shadows[5] = {0, 154, 170, 186, 202};
    static int LightSize = 80;
    static float LightTime = time_get();
    bool CanSizing = false;
    if (time_get() - LightTime > 8.5f*time_freq())
    {
        CanSizing = true;
        LightSize = rand() % (80-79+1) + 79;
        LightTime = time_get();
    }

    for(int y = StartY; y < EndY; y++)
		for(int x = StartX; x < EndX; x++)
		{
			int mx = clamp(x, 0, w-1);
			int my = clamp(y, 0, h-1);
            int c = mx + my*w;

            if (pTiles[c].m_Index == BLOCK_LUZ)
            {
                for (int e=0; e<=LightSize; e++)
                {
                    int index = 4-(e*4)/LightSize;
                    for (int i=(LightSize-e)/2; i>=-((LightSize-e)/2); i--)
                    {

                        for (int o=-((LightSize-e)/2); o<=(LightSize-e)/2; o++)
                        {
                            int tc = clamp(mx+o, 0, w-1) + clamp(my-i, 0, h-1)*w;
                            if (Shadows[index] < pLightsTemp[tc].m_Index)
                                pLightsTemp[tc].m_Index = Shadows[index];
                        }
                    }
                }

                pLightsTemp[c].m_Index = 0;
                x += pTiles[c].m_Skip;
            }
            if (pTiles[c].m_Index == BLOCK_CALABAZA_ON)
            {
                int calabazaLight = 10;
                for (int e=0; e<=calabazaLight; e++)
                {
                    int index = 4-(e*4)/calabazaLight;
                    for (int i=(calabazaLight-e)/2; i>=-((calabazaLight-e)/2); i--)
                    {

                        for (int o=-((calabazaLight-e)/2); o<=(calabazaLight-e)/2; o++)
                        {
                            int tc = clamp(mx+o, 0, w-1) + clamp(my-i, 0, h-1)*w;
                            if (Shadows[index] < pLightsTemp[tc].m_Index)
                                pLightsTemp[tc].m_Index = Shadows[index];
                        }
                    }
                }

                pLightsTemp[c].m_Index = 0;
                x += pTiles[c].m_Skip;
            }
            else if (pTiles[c].m_Index >= BLOCK_UNDEF104 && pTiles[c].m_Index <= BLOCK_LAVA)
                pLightsTemp[c].m_Index = 0;
            else if (pTiles[c].m_Index == BLOCK_HORNO_ON)
                pLightsTemp[c].m_Index = 0;
		}

    mem_copy(pLights, pLightsTemp, sizeof(CTile)*w*h);
    mem_free(pLightsTemp);
}

void CRenderTools::RenderTile(int Index, vec2 Pos, float Scale, float Alpha, float Rot)
{
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);

	// calculate the final pixelsize for the tiles
	float TilePixelSize = 1024/32.0f;
	float FinalTileSize = Scale/(ScreenX1-ScreenX0) * Graphics()->ScreenWidth();
	float FinalTilesetScale = FinalTileSize/TilePixelSize;


	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);

	int StartY = (int)(ScreenY0/Scale)-1;
	int StartX = (int)(ScreenX0/Scale)-1;
	int EndY = (int)(ScreenY1/Scale)+1;
	int EndX = (int)(ScreenX1/Scale)+1;

	// adjust the texture shift according to mipmap level
	float TexSize = 1024.0f;
	float Frac = (1.25f/TexSize) * (1/FinalTilesetScale);
	float Nudge = (0.5f/TexSize) * (1/FinalTilesetScale);

    int tx = Index%16;
    int ty = Index/16;
    int Px0 = tx*(1024/16);
    int Py0 = ty*(1024/16);
    int Px1 = Px0+(1024/16)-1;
    int Py1 = Py0+(1024/16)-1;

    float x0 = Nudge + Px0/TexSize+Frac;
    float y0 = Nudge + Py0/TexSize+Frac;
    float x1 = Nudge + Px1/TexSize-Frac;
    float y1 = Nudge + Py0/TexSize+Frac;
    float x2 = Nudge + Px1/TexSize-Frac;
    float y2 = Nudge + Py1/TexSize-Frac;
    float x3 = Nudge + Px0/TexSize+Frac;
    float y3 = Nudge + Py1/TexSize-Frac;

    Graphics()->QuadsSetSubsetFree(x0, y0, x1, y1, x2, y2, x3, y3);
    IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Scale, Scale);
    Graphics()->QuadsSetRotation(Rot);
    Graphics()->QuadsDrawTL(&QuadItem, 1);

	Graphics()->QuadsEnd();
	Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
}
