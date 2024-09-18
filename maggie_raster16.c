#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

/*****************************************************************************/

static void DrawHardwareSpanZBuffered(APTR dest, APTR zDest, int len, ULONG ZZzz, ULONG UUuu, ULONG VVvv, UWORD Ii, ULONG dZZzz, ULONG dUUuu, ULONG dVVvv, UWORD dIi)
{
	maggieRegs.depthDest = zDest;
	maggieRegs.depthStart = ZZzz;
	maggieRegs.depthDelta = dZZzz;
	maggieRegs.pixDest = dest;
	maggieRegs.uCoord = UUuu;
	maggieRegs.vCoord = VVvv;
	maggieRegs.light = Ii;
	maggieRegs.uDelta = dUUuu;
	maggieRegs.vDelta = dVVvv;
	maggieRegs.lightDelta = dIi;
	maggieRegs.startLength = len;
}

/*****************************************************************************/

static void DrawHardwareSpan(APTR dest, int len, ULONG UUuu, ULONG VVvv, UWORD Ii, ULONG dUUuu, ULONG dVVvv, UWORD dIi)
{
	maggieRegs.pixDest = dest;
	maggieRegs.uCoord = UUuu;
	maggieRegs.vCoord = VVvv;
	maggieRegs.light = Ii;
	maggieRegs.uDelta = dUUuu;
	maggieRegs.vDelta = dVVvv;
	maggieRegs.lightDelta = dIi;
	maggieRegs.startLength = len;
}

/*****************************************************************************/

static void SetTexture(APTR txtrData, UWORD size)
{
	maggieRegs.texSize = size;
	maggieRegs.texture = txtrData;
}

/*****************************************************************************/

static void SetupHW(MaggieBase *lib)
{
	UWORD mode = lib->drawMode;
	UWORD drawMode = 0x0004;
	if(mode & MAG_DRAWMODE_BILINEAR)
	{
		drawMode |=  0x0001;
	}
	if(mode & MAG_DRAWMODE_DEPTHBUFFER)
	{
		drawMode |= 0x0002;
	}
	UWORD modulo = 2;
	maggieRegs.mode = drawMode;
	maggieRegs.modulo = modulo;
	maggieRegs.lightRGBA = lib->colour;
}

/*****************************************************************************/

void DrawSpansHW16ZBuffer(int ymin, int ymax, MaggieBase *lib)
{
	APTR txtrData = GetTextureData(lib->textures[lib->txtrIndex]);
	UWORD txtrSize = GetTexSizeIndex(lib->textures[lib->txtrIndex]);
	SetupHW(lib);
	SetTexture(txtrData, txtrSize);

	magEdgePos *edges = &lib->magEdge[ymin];
	int ylen = ymax - ymin;
	int modulo = lib->xres;
	UWORD *pixels = ((UWORD *)lib->screen) + ymin * lib->xres;
	UWORD *zbuffer = lib->depthBuffer + ymin * lib->xres;

	int scissorLeft = lib->scissor.x0;
	int scissorRight = lib->scissor.x1;

	for(int i = 0; i < ylen; ++i)
	{
		int x0 = edges[i].xPosLeft;
		int x1 = edges[i].xPosRight;

		UWORD *dstColPtr = pixels + x0;
		UWORD *dstZPtr =  &zbuffer[x0];

		pixels += modulo;
		zbuffer += modulo;

		if(x0 >= x1)
			continue;

		int len = x1 - x0;
		float xFracStart = edges[i].xPosLeft - x0;
		float preStep = 1.0f - xFracStart;

		float ooXLength = 1.0f / (edges[i].xPosRight - edges[i].xPosLeft);

		float zDDA = (edges[i].zowRight - edges[i].zowLeft) * ooXLength;
		float wDDA = (edges[i].oowRight - edges[i].oowLeft) * ooXLength;
		float uDDA = (edges[i].uowRight - edges[i].uowLeft) * ooXLength;
		float vDDA = (edges[i].vowRight - edges[i].vowLeft) * ooXLength;
		float iDDA = (edges[i].iowRight - edges[i].iowLeft) * ooXLength;

		float zPos = edges[i].zowLeft + preStep * zDDA;
		float wPos = edges[i].oowLeft + preStep * wDDA;
		float uPos = edges[i].uowLeft + preStep * uDDA;
		float vPos = edges[i].vowLeft + preStep * vDDA;
		float iPos = edges[i].iowLeft + preStep * iDDA;

		if(x0 < scissorLeft)
		{
			int diff = scissorLeft - x0;
			zPos += zDDA * diff;
			wPos += wDDA * diff;
			uPos += uDDA * diff;
			vPos += vDDA * diff;
			iPos += iDDA * diff;
			dstColPtr += diff;
			dstZPtr += diff;
			len -= diff;
			if(len <= 0)
				continue;
		}
		if(x1 > scissorRight)
		{
			len -= x1 - scissorRight;
		}

		if(len > PIXEL_RUN)
		{
			float zDDAFullRun = zDDA * PIXEL_RUN;
			float wDDAFullRun = wDDA * PIXEL_RUN;
			float uDDAFullRun = uDDA * PIXEL_RUN;
			float vDDAFullRun = vDDA * PIXEL_RUN;
			float iDDAFullRun = iDDA * PIXEL_RUN;

			float w = 1.0 / wPos;
			LONG uStart = uPos * w;
			LONG vStart = vPos * w;
			float iStart = iPos;
			float ooLen = 1.0f / PIXEL_RUN;

			while(len >= PIXEL_RUN)
			{
				wPos += wDDAFullRun;
				uPos += uDDAFullRun;
				vPos += vDDAFullRun;
				iPos += iDDAFullRun;

				w = 1.0 / wPos;

				float iEnd = iPos;

				LONG uEnd = (uPos * w);
				LONG vEnd = (vPos * w);

				LONG dUUuu = (LONG)((uEnd - uStart) * ooLen);
				LONG dVVvv = (LONG)((vEnd - vStart) * ooLen);
				LONG dIi = (LONG)((iEnd - iStart) * ooLen);

				DrawHardwareSpanZBuffered(dstColPtr, dstZPtr, PIXEL_RUN, (ULONG)zPos, uStart, vStart, (LONG)iStart, (LONG)zDDA, dUUuu, dVVvv, dIi);

				dstColPtr += PIXEL_RUN;
				dstZPtr += PIXEL_RUN;
				zPos += zDDAFullRun;
				uStart = uEnd;
				vStart = vEnd;
				iStart = iEnd;
				len -= PIXEL_RUN;
			}
		}
		if(len > 0)
		{
			float w = 1.0 / wPos;
			LONG uStart = uPos * w;
			LONG vStart = vPos * w;
			float iStart = iPos;
			float ooLen = 1.0f / len;

			wPos += wDDA * len;
			uPos += uDDA * len;
			vPos += vDDA * len;
			iPos += iDDA * len;

			w = 1.0 / wPos;

			float iEnd = iPos;

			LONG uEnd = uPos * w;
			LONG vEnd = vPos * w;

			LONG dUUuu = (LONG)((uEnd - uStart) * ooLen);
			LONG dVVvv = (LONG)((vEnd - vStart) * ooLen);
			LONG dIi = (LONG)((iEnd - iStart) * ooLen);

			DrawHardwareSpanZBuffered(dstColPtr, dstZPtr, len, (ULONG)zPos, uStart, vStart, (LONG)iStart, (LONG)zDDA, dUUuu, dVVvv, dIi);
		}
	}
}

/*****************************************************************************/

void DrawSpansHW16(int ymin, int ymax, MaggieBase *lib)
{
	APTR txtrData = GetTextureData(lib->textures[lib->txtrIndex]);
	UWORD txtrSize = GetTexSizeIndex(lib->textures[lib->txtrIndex]);
	SetTexture(txtrData, txtrSize);
	SetupHW(lib);

	magEdgePos *edges = &lib->magEdge[ymin];
	int ylen = ymax - ymin;
	int modulo = lib->xres;
	UWORD *pixels = ((UWORD *)lib->screen) + ymin * lib->xres;

	int scissorLeft = lib->scissor.x0;
	int scissorRight = lib->scissor.x1;

	for(int i = 0; i < ylen; ++i)
	{
		int x0 = edges[i].xPosLeft;
		int x1 = edges[i].xPosRight;

		UWORD *dstColPtr = pixels + x0;

		pixels += modulo;

		if(x0 >= x1)
			continue;

		int len = x1 - x0;
		float xFracStart = edges[i].xPosLeft - x0;
		float preStep = 1.0f - xFracStart;

		float ooXLength = 1.0f / (edges[i].xPosRight - edges[i].xPosLeft);

		float wDDA = (edges[i].oowRight - edges[i].oowLeft) * ooXLength;
		float uDDA = (edges[i].uowRight - edges[i].uowLeft) * ooXLength;
		float vDDA = (edges[i].vowRight - edges[i].vowLeft) * ooXLength;
		float iDDA = (edges[i].iowRight - edges[i].iowLeft) * ooXLength;

		float wPos = edges[i].oowLeft + preStep * wDDA;
		float uPos = edges[i].uowLeft + preStep * uDDA;
		float vPos = edges[i].vowLeft + preStep * vDDA;
		float iPos = edges[i].iowLeft + preStep * iDDA;

		if(x0 < scissorLeft)
		{
			int diff = scissorLeft - x0;
			wPos += wDDA * diff;
			uPos += uDDA * diff;
			vPos += vDDA * diff;
			iPos += iDDA * diff;
			dstColPtr += diff;
			len -= diff;
			if(len <= 0)
				continue;
		}
		if(x1 > scissorRight)
		{
			len -= x1 - scissorRight;
		}

		if(len > PIXEL_RUN)
		{
			float wDDAFullRun = wDDA * PIXEL_RUN;
			float uDDAFullRun = uDDA * PIXEL_RUN;
			float vDDAFullRun = vDDA * PIXEL_RUN;
			float iDDAFullRun = iDDA * PIXEL_RUN;

			float w = 1.0 / wPos;
			LONG uStart = uPos * w;
			LONG vStart = vPos * w;
			float iStart = iPos;
			float ooLen = 1.0f / PIXEL_RUN;

			while(len >= PIXEL_RUN)
			{
				wPos += wDDAFullRun;
				uPos += uDDAFullRun;
				vPos += vDDAFullRun;
				iPos += iDDAFullRun;

				w = 1.0 / wPos;

				float iEnd = iPos;

				LONG uEnd = uPos * w;
				LONG vEnd = vPos * w;

				LONG dUUuu = (LONG)((uEnd - uStart) * ooLen);
				LONG dVVvv = (LONG)((vEnd - vStart) * ooLen);
				LONG dIi = (LONG)((iEnd - iStart) * ooLen);

				DrawHardwareSpan(dstColPtr, PIXEL_RUN, uStart, vStart, (LONG)iStart, dUUuu, dVVvv, dIi);

				dstColPtr += PIXEL_RUN;
				uStart = uEnd;
				vStart = vEnd;
				iStart = iEnd;
				len -= PIXEL_RUN;
			}
		}
		if(len > 0)
		{
			float w = 1.0 / wPos;
			LONG uStart = uPos * w;
			LONG vStart = vPos * w;
			float iStart = iPos;
			float ooLen = 1.0f / len;

			wPos += wDDA * len;
			uPos += uDDA * len;
			vPos += vDDA * len;
			iPos += iDDA * len;

			w = 1.0 / wPos;

			float iEnd = iPos;

			LONG uEnd = uPos * w;
			LONG vEnd = vPos * w;

			LONG dUUuu = (LONG)((uEnd - uStart) * ooLen);
			LONG dVVvv = (LONG)((vEnd - vStart) * ooLen);
			LONG dIi = (LONG)((iEnd - iStart) * ooLen);

			DrawHardwareSpan(dstColPtr, len, uStart, vStart, (LONG)iStart, dUUuu, dVVvv, dIi);
		}
	}
}
