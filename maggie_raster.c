#include "maggie_internal.h"
#include "maggie_vertex.h"

/*****************************************************************************/

#define PIXEL_RUN 16

/*****************************************************************************/

static void SetTexture(APTR txtrData, UWORD size)
{
	maggieRegs->texture = txtrData;
	maggieRegs->texSize = size;
}

/*****************************************************************************/

static void SetDepth(APTR depth)
{
	maggieRegs->depthDest = depth;
	maggieRegs->depthStart = 0;
	maggieRegs->depthDelta = 0;
}

/*****************************************************************************/

static void SetupHWZBuffer(UWORD mode)
{
	UWORD drawMode = 0x0002;
	if(mode & MAG_DRAWMODE_BILINEAR)
	{
		drawMode |=  0x0001;
	}
	UWORD modulo = 2;
	if(mode & MAG_DRAWMODE_32BIT)
	{
		modulo = 4;
	}
	else
	{
		drawMode |= 0x0004;
	}
	maggieRegs->mode = drawMode;
	maggieRegs->modulo = modulo;
	maggieRegs->lightRGBA = 0x00ffffff;
}

/*****************************************************************************/

static void SetupHW(UWORD mode)
{
	UWORD drawMode = 0x0002;
	if(mode & MAG_DRAWMODE_BILINEAR)
	{
		drawMode |=  0x0001;
	}
	UWORD modulo = 2;
	if(mode & MAG_DRAWMODE_32BIT)
	{
		modulo = 4;
	}
	else
	{
		drawMode |= 0x0004;
	}
	maggieRegs->mode = drawMode;
	maggieRegs->modulo = modulo;
	maggieRegs->lightRGBA = 0x00ffffff;
}

/*****************************************************************************/

static void DrawHardwareSpanZBuffered(	APTR dest, APTR zDest,
										int len,
										ULONG ZZzz,
										ULONG UUuu, ULONG VVvv,
										UWORD Ii,
										LONG dZZzz,
										ULONG dUUuu, ULONG dVVvv,
										UWORD dIi)
{
	maggieRegs->depthDest = zDest;
	maggieRegs->depthStart = ZZzz;
	maggieRegs->depthDelta = dZZzz;
	maggieRegs->pixDest = dest;
	maggieRegs->uCoord = UUuu;
	maggieRegs->vCoord = VVvv;
	maggieRegs->light = Ii;
	maggieRegs->uDelta = dUUuu;
	maggieRegs->vDelta = dVVvv;
	maggieRegs->lightDelta = dIi;
	maggieRegs->startLength = len;
}

/*****************************************************************************/

static void DrawHardwareSpan(	APTR dest,
								int len,
								ULONG UUuu, ULONG VVvv,
								UWORD Ii,
								ULONG dUUuu, ULONG dVVvv,
								UWORD dIi)
{
	maggieRegs->pixDest = dest;
	maggieRegs->uCoord = UUuu;
	maggieRegs->vCoord = VVvv;
	maggieRegs->light = Ii;
	maggieRegs->uDelta = dUUuu;
	maggieRegs->vDelta = dVVvv;
	maggieRegs->lightDelta = dIi;
	maggieRegs->startLength = len;
}

/*****************************************************************************/

static void DrawSpanZBuffer16(UWORD *destCol, UWORD *zBuffer, int len, ULONG Zz, LONG Ii, LONG dZz, LONG dIi)
{
	for(int i = 0; i < len; ++i)
	{
		UWORD z = (Zz >> 16);
		if(*zBuffer > z)
		{
			int ti = Ii >> 8;
			if(ti > 255)
				ti = 255;
			*zBuffer = z;
			*destCol = ~0;//ti * 0x010101;
		}
		destCol++;
		zBuffer++;
		Ii += dIi;
		Zz += dZz;
	}
}

/*****************************************************************************/

static void DrawSpanZBuffer32(ULONG *destCol, UWORD *zBuffer, int len, ULONG Zz, LONG Ii, LONG dZz, LONG dIi)
{
	for(int i = 0; i < len; ++i)
	{
		UWORD z = (Zz >> 16);
		if(*zBuffer > z)
		{
			int ti = Ii >> 8;
			if(ti > 255)
				ti = 255;
			*zBuffer = z;
			*destCol = ti * 0x010101;
		}
		destCol++;
		zBuffer++;
		Ii += dIi;
		Zz += dZz;
	}
}

/*****************************************************************************/

static void DrawSpan16(UWORD *destCol, int len, LONG Ii, LONG dIi)
{
	for(int i = 0; i < len; ++i)
	{
		int ti = Ii >> 8;
		if(ti > 255)
			ti = 255;
		*destCol = ~0;//ti * 0x010101;
		destCol++;
		Ii += dIi;
	}
}

/*****************************************************************************/

static void DrawSpan32(ULONG *destCol, int len, LONG Ii, LONG dIi)
{
	for(int i = 0; i < len; ++i)
	{
		int ti = Ii >> 8;
		if(ti > 255)
			ti = 255;
		*destCol = ti * 0x010101;
		destCol++;
		Ii += dIi;
	}
}

/*****************************************************************************/
/*****************************************************************************/

void DrawSpansHW32ZBuffer(ULONG * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, int modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;

		ULONG *dstColPtr = pixels + x0;
		UWORD *dstZPtr =  &zbuffer[x0];

		pixels += modulo;
		zbuffer += modulo;

		if(x0 >= x1)
			continue;

		int len = x1 - x0;
		float oolen = 1.0f / len;
		float xFrac0 = left[i].xPos - x0;
		float xFrac1 = right[i].xPos - x1;
		float preStep0 = 1.0f - xFrac0;
		float preStep1 = 1.0f - xFrac1;

		float xLen = right[i].xPos - left[i].xPos;
		float zLen = right[i].zow - left[i].zow;
		float wLen = right[i].oow - left[i].oow;
		float uLen = right[i].uow - left[i].uow;
		float vLen = right[i].vow - left[i].vow;
		float iLen = right[i].iow - left[i].iow;

		float preRatio0 = preStep0 / xLen;
		float preRatio1 = preStep1 / xLen;
		float preRatioDiff = preRatio0 - preRatio1;

		float zDDA = (zLen - zLen * preRatioDiff) * oolen;
		float wDDA = (wLen - wLen * preRatioDiff) * oolen;
		float uDDA = (uLen - uLen * preRatioDiff) * oolen;
		float vDDA = (vLen - vLen * preRatioDiff) * oolen;
		float iDDA = (iLen - iLen * preRatioDiff) * oolen;

		float zPos = left[i].zow + preStep0 * zDDA;
		float wPos = left[i].oow + preStep0 * wDDA;
		float uPos = left[i].uow + preStep0 * uDDA;
		float vPos = left[i].vow + preStep0 * vDDA;
		float iPos = left[i].iow + preStep0 * iDDA;

		zDDA *= PIXEL_RUN;
		wDDA *= PIXEL_RUN;
		uDDA *= PIXEL_RUN;
		vDDA *= PIXEL_RUN;
		iDDA *= PIXEL_RUN;

		float w = 1.0 / wPos;
		LONG uStart = uPos * w;
		LONG vStart = vPos * w;
		float zStart = zPos;
		float iStart = iPos;
//		int nRuns = len / PIXEL_RUN;
		float ooLen = 1.0f / PIXEL_RUN;

		int runLength = PIXEL_RUN;
		if(runLength > len)
		{
			runLength = len;
		}

		while(len > 0)
		{
			wPos += wDDA;
			uPos += uDDA;
			vPos += vDDA;
			zPos += zDDA;
			iPos += iDDA;

			w = 1.0 / wPos;

			float zEnd = zPos;
			float iEnd = iPos;

			LONG uEnd = uPos * w;
			LONG vEnd = vPos * w;

			if(len <= (PIXEL_RUN * 3 / 2))
			{
				runLength = len;
			}

			LONG dUUuu = (LONG)((uEnd - uStart) * ooLen);
			LONG dVVvv = (LONG)((vEnd - vStart) * ooLen);
			LONG dZz = (LONG)((zEnd - zStart) * ooLen);
			LONG dIi = (LONG)((iEnd - iStart) * ooLen);

			DrawHardwareSpanZBuffered(dstColPtr, dstZPtr, runLength, (ULONG)zStart, uStart, vStart, (LONG)iStart, dZz, dUUuu, dVVvv, dIi);

			dstColPtr += runLength;
			dstZPtr += runLength;
			uStart = uEnd;
			vStart = vEnd;
			zStart = zEnd;
			iStart = iEnd;
			len -= runLength;
		}
	}
}

/*****************************************************************************/

void DrawSpansHW16ZBuffer(UWORD * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, int modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;

		UWORD *dstColPtr = pixels + x0;
		UWORD *dstZPtr =  &zbuffer[x0];

		pixels += modulo;
		zbuffer += modulo;

		if(x0 >= x1)
			continue;

		int len = x1 - x0;
		float oolen = 1.0f / len;
		float xFrac0 = left[i].xPos - x0;
		float xFrac1 = right[i].xPos - x1;
		float preStep0 = 1.0f - xFrac0;
		float preStep1 = 1.0f - xFrac1;

		float xLen = right[i].xPos - left[i].xPos;
		float zLen = right[i].zow - left[i].zow;
		float wLen = right[i].oow - left[i].oow;
		float uLen = right[i].uow - left[i].uow;
		float vLen = right[i].vow - left[i].vow;
		float iLen = right[i].iow - left[i].iow;

		float preRatio0 = preStep0 / xLen;
		float preRatio1 = preStep1 / xLen;
		float preRatioDiff = preRatio0 - preRatio1;

		float zPreStep = zLen * preRatioDiff;
		float wPreStep = wLen * preRatioDiff;
		float uPreStep = uLen * preRatioDiff;
		float vPreStep = vLen * preRatioDiff;
		float iPreStep = iLen * preRatioDiff;

		float zDDA = (zLen - zPreStep) * oolen;
		float wDDA = (wLen - wPreStep) * oolen;
		float uDDA = (uLen - uPreStep) * oolen;
		float vDDA = (vLen - vPreStep) * oolen;
		float iDDA = (iLen - iPreStep) * oolen;

		float zPos = left[i].zow + preStep0 * zDDA;
		float wPos = left[i].oow + preStep0 * wDDA;
		float uPos = left[i].uow + preStep0 * uDDA;
		float vPos = left[i].vow + preStep0 * vDDA;
		float iPos = left[i].iow + preStep0 * iDDA;

		zDDA *= PIXEL_RUN;
		wDDA *= PIXEL_RUN;
		uDDA *= PIXEL_RUN;
		vDDA *= PIXEL_RUN;
		iDDA *= PIXEL_RUN;

		float w = 1.0 / wPos;
		LONG uStart = uPos * w;
		LONG vStart = vPos * w;
		float zStart = zPos;
		float iStart = iPos;
//		int nRuns = len / PIXEL_RUN;
		float ooLen = 1.0f / PIXEL_RUN;

		int runLength = PIXEL_RUN;
		if(runLength > len)
			runLength = len;
		while(len > 0)
		{
			wPos += wDDA;
			uPos += uDDA;
			vPos += vDDA;
			zPos += zDDA;
			iPos += iDDA;

			w = 1.0 / wPos;

			float zEnd = zPos;
			float iEnd = iPos;

			LONG uEnd = uPos * w;
			LONG vEnd = vPos * w;

			if(len <= (PIXEL_RUN * 3 / 2))
			{
				runLength = len;
			}

			LONG dUUuu = (LONG)((uEnd - uStart) * ooLen);
			LONG dVVvv = (LONG)((vEnd - vStart) * ooLen);
			LONG dZz = (LONG)((zEnd - zStart) * ooLen);
			LONG dIi = (LONG)((iEnd - iStart) * ooLen);

			DrawHardwareSpanZBuffered(dstColPtr, dstZPtr, runLength, (ULONG)zStart, uStart, vStart, (LONG)iStart, dZz, dUUuu, dVVvv, dIi);

			dstColPtr += runLength;
			dstZPtr += runLength;
			uStart = uEnd;
			vStart = vEnd;
			zStart = zEnd;
			iStart = iEnd;
			len -= runLength;
		}
	}
}

/*****************************************************************************/

void DrawSpansHW32(ULONG * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, int modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;

		ULONG *dstColPtr = pixels + x0;

		pixels += modulo;

		if(x0 >= x1)
			continue;

		int len = x1 - x0;
		float oolen = 1.0f / len;
		float xFrac0 = left[i].xPos - x0;
		float xFrac1 = right[i].xPos - x1;
		float preStep0 = 1.0f - xFrac0;
		float preStep1 = 1.0f - xFrac1;

		float xLen = right[i].xPos - left[i].xPos;
		float wLen = right[i].oow - left[i].oow;
		float uLen = right[i].uow - left[i].uow;
		float vLen = right[i].vow - left[i].vow;
		float iLen = right[i].iow - left[i].iow;

		float preRatio0 = preStep0 / xLen;
		float preRatio1 = preStep1 / xLen;
		float preRatioDiff = preRatio0 - preRatio1;

		float wPreStep = wLen * preRatioDiff;
		float uPreStep = uLen * preRatioDiff;
		float vPreStep = vLen * preRatioDiff;
		float iPreStep = iLen * preRatioDiff;

		float wDDA = (wLen - wPreStep) * oolen;
		float uDDA = (uLen - uPreStep) * oolen;
		float vDDA = (vLen - vPreStep) * oolen;
		float iDDA = (iLen - iPreStep) * oolen;

		float wPos = left[i].oow + preStep0 * wDDA;
		float uPos = left[i].uow + preStep0 * uDDA;
		float vPos = left[i].vow + preStep0 * vDDA;
		float iPos = left[i].iow + preStep0 * iDDA;

		wDDA *= PIXEL_RUN;
		uDDA *= PIXEL_RUN;
		vDDA *= PIXEL_RUN;
		iDDA *= PIXEL_RUN;

		float w = 1.0 / wPos;
		LONG uStart = uPos * w;
		LONG vStart = vPos * w;
		float iStart = iPos;
		float ooLen = 1.0f / PIXEL_RUN;

		int runLength = PIXEL_RUN;
		if(runLength > len)
			runLength = len;
		while(len > 0)
		{
			wPos += wDDA;
			uPos += uDDA;
			vPos += vDDA;
			iPos += iDDA;

			w = 1.0 / wPos;

			float iEnd = iPos;

			LONG uEnd = uPos * w;
			LONG vEnd = vPos * w;

			if(len <= (PIXEL_RUN * 3 / 2))
			{
				runLength = len;
			}

			LONG dUUuu = (LONG)((uEnd - uStart) * ooLen);
			LONG dVVvv = (LONG)((vEnd - vStart) * ooLen);
			LONG dIi = (LONG)((iEnd - iStart) * ooLen);

			DrawHardwareSpan(dstColPtr, runLength, uStart, vStart, (LONG)iStart, dUUuu, dVVvv, dIi);

			dstColPtr += runLength;
			uStart = uEnd;
			vStart = vEnd;
			iStart = iEnd;
			len -= runLength;
		}
	}
}

/*****************************************************************************/

void DrawSpansHW16(UWORD * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, int modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;

		UWORD *dstColPtr = pixels + x0;

		pixels += modulo;

		if(x0 >= x1)
			continue;

		int len = x1 - x0;
		float oolen = 1.0f / len;
		float xFrac0 = left[i].xPos - x0;
		float xFrac1 = right[i].xPos - x1;
		float preStep0 = 1.0f - xFrac0;
		float preStep1 = 1.0f - xFrac1;

		float xLen = right[i].xPos - left[i].xPos;
		float wLen = right[i].oow - left[i].oow;
		float uLen = right[i].uow - left[i].uow;
		float vLen = right[i].vow - left[i].vow;
		float iLen = right[i].iow - left[i].iow;

		float preRatio0 = preStep0 / xLen;
		float preRatio1 = preStep1 / xLen;
		float preRatioDiff = preRatio0 - preRatio1;

		float wPreStep = wLen * preRatioDiff;
		float uPreStep = uLen * preRatioDiff;
		float vPreStep = vLen * preRatioDiff;
		float iPreStep = iLen * preRatioDiff;

		float wDDA = (wLen - wPreStep) * oolen;
		float uDDA = (uLen - uPreStep) * oolen;
		float vDDA = (vLen - vPreStep) * oolen;
		float iDDA = (iLen - iPreStep) * oolen;

		float wPos = left[i].oow + preStep0 * wDDA;
		float uPos = left[i].uow + preStep0 * uDDA;
		float vPos = left[i].vow + preStep0 * vDDA;
		float iPos = left[i].iow + preStep0 * iDDA;

		wDDA *= PIXEL_RUN;
		uDDA *= PIXEL_RUN;
		vDDA *= PIXEL_RUN;
		iDDA *= PIXEL_RUN;

		float w = 1.0 / wPos;
		LONG uStart = uPos * w;
		LONG vStart = vPos * w;
		float iStart = iPos;
		float ooLen = 1.0f / PIXEL_RUN;

		int runLength = PIXEL_RUN;
		if(runLength > len)
			runLength = len;
		while(len > 0)
		{
			wPos += wDDA;
			uPos += uDDA;
			vPos += vDDA;
			iPos += iDDA;

			w = 1.0 / wPos;

			float iEnd = iPos;

			LONG uEnd = uPos * w;
			LONG vEnd = vPos * w;

			if(len <= (PIXEL_RUN * 3 / 2))
			{
				runLength = len;
			}

			LONG dUUuu = (LONG)((uEnd - uStart) * ooLen);
			LONG dVVvv = (LONG)((vEnd - vStart) * ooLen);
			LONG dIi = (LONG)((iEnd - iStart) * ooLen);

			DrawHardwareSpan(dstColPtr, runLength, uStart, vStart, (LONG)iStart, dUUuu, dVVvv, dIi);

			dstColPtr += runLength;
			uStart = uEnd;
			vStart = vEnd;
			iStart = iEnd;
			len -= runLength;
		}
	}
}

/*****************************************************************************/

void DrawSpansSW32ZBuffer(ULONG * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;
		int len = x1 - x0;
		float ooLen = 1.0f / len;

		ULONG *dstColPtr = &pixels[x0];
		UWORD *dstZPtr =  (UWORD *)&zbuffer[x0];

		pixels += modulo;
		zbuffer += modulo;

		if(x0 == x1)
			continue;

		if(x0 < 0)
			x0 = 0;
		if(x1 >= modulo)
			x1 = modulo - 1;

		ULONG zPos = left[i].zow;
		LONG iPos = left[i].iow;
		LONG zDDA = (right[i].zow - left[i].zow) * ooLen;
		LONG iDDA = (int)((right[i].iow - left[i].iow) * ooLen);

		DrawSpanZBuffer32(dstColPtr, dstZPtr, len, zPos, iPos, zDDA, iDDA);
	}
}

/*****************************************************************************/

void DrawSpansSW16ZBuffer(UWORD * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;
		int len = x1 - x0;
		float ooLen = 1.0f / len;

		UWORD *dstColPtr = &pixels[x0];
		UWORD *dstZPtr =  (UWORD *)&zbuffer[x0];

		pixels += modulo;
		zbuffer += modulo;

		if(x0 == x1)
			continue;

		if(x0 < 0)
			x0 = 0;
		if(x1 >= modulo)
			x1 = modulo - 1;

		ULONG zPos = left[i].zow;
		int iPos = (int)left[i].iow;
		LONG zDDA = (right[i].zow - left[i].zow) * ooLen;
		int iDDA = (int)((right[i].iow - left[i].iow) * ooLen);

		DrawSpanZBuffer16(dstColPtr, dstZPtr, len, zPos, iPos, zDDA, iDDA);
	}
}

/*****************************************************************************/

void DrawSpansSW16(UWORD * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;
		int len = x1 - x0;

		UWORD *dstColPtr = &pixels[x0];

		pixels += modulo;

		if(x0 == x1)
			continue;

		if(x0 < 0)
			x0 = 0;
		if(x1 >= modulo)
			x1 = modulo - 1;

		int iPos = (int)left[i].iow;
		int iDDA = (int)((right[i].iow - left[i].iow) / len);

		DrawSpan16(dstColPtr, len, iPos, iDDA);
	}
}

/*****************************************************************************/

void DrawSpansSW32(ULONG * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;
		int len = x1 - x0;

		ULONG *dstColPtr = &pixels[x0];

		pixels += modulo;

		if(x0 == x1)
			continue;

		if(x0 < 0)
			x0 = 0;
		if(x1 >= modulo)
			x1 = modulo - 1;

		int iPos = (int)left[i].iow;
		int iDDA = (int)((right[i].iow - left[i].iow) / len);

		DrawSpan32(dstColPtr, len, iPos, iDDA);
	}
}

/*****************************************************************************/

void DrawSpans(int miny, int maxy, MaggieBase *lib)
{
	magEdgePos *left = &lib->magLeftEdge[miny];
	magEdgePos *right = &lib->magRightEdge[miny];
	UWORD *depth = lib->depthBuffer + miny * lib->xres;

	if(lib->drawMode & MAG_DRAWMODE_DEPTHBUFFER)
	{
		if((lib->txtrIndex == 0xffff) || !lib->textures[lib->txtrIndex])
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				ULONG *pixels = ((ULONG *)lib->screen) + miny * lib->xres;
				DrawSpansSW32ZBuffer(pixels, depth, left, right, maxy - miny, lib->xres);
			}
			else
			{
				UWORD *pixels = ((UWORD *)lib->screen) + miny * lib->xres;
				DrawSpansSW16ZBuffer(pixels, depth, left, right, maxy - miny, lib->xres);
			}
		}
		else
		{
			APTR txtrData = GetTextureData(lib->textures[lib->txtrIndex]);
			UWORD txtrSize = lib->textures[lib->txtrIndex][0];
			SetTexture(txtrData, txtrSize);
			SetupHWZBuffer(lib->drawMode);
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				ULONG *pixels = ((ULONG *)lib->screen) + miny * lib->xres;
				DrawSpansHW32ZBuffer(pixels, depth, left, right, maxy - miny, lib->xres);
			}
			else
			{
				UWORD *pixels = ((UWORD *)lib->screen) + miny * lib->xres;
				DrawSpansHW16ZBuffer(pixels, depth, left, right, maxy - miny, lib->xres);
			}
		}
	}
	else
	{
		if((lib->txtrIndex == 0xffff) || !lib->textures[lib->txtrIndex])
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				ULONG *pixels = ((ULONG *)lib->screen) + miny * lib->xres;
				DrawSpansSW32(pixels, left, right, maxy - miny, lib->xres);
			}
			else
			{
				UWORD *pixels = ((UWORD *)lib->screen) + miny * lib->xres;
				DrawSpansSW16(pixels, left, right, maxy - miny, lib->xres);
			}
		}
		else
		{
			APTR txtrData = GetTextureData(lib->textures[lib->txtrIndex]);
			UWORD txtrSize = lib->textures[lib->txtrIndex][0];
			SetTexture(txtrData, txtrSize);
			SetupHW(lib->drawMode);
			SetDepth(lib->depthBuffer);
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				ULONG *pixels = ((ULONG *)lib->screen) + miny * lib->xres;
				DrawSpansHW32(pixels, left, right, maxy - miny, lib->xres);
			}
			else
			{
				UWORD *pixels = ((UWORD *)lib->screen) + miny * lib->xres;
				DrawSpansHW16(pixels, left, right, maxy - miny, lib->xres);
			}
		}
	}
}
