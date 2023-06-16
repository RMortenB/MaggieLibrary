#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

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

void DrawSpansSW16ZBuffer(UWORD * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo)
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

		int runLength = x1 - x0;
		float oolen = 1.0f / runLength;
		float xFrac0 = left[i].xPos - x0;
		float xFrac1 = right[i].xPos - x1;
		float preStep0 = 1.0f - xFrac0;
		float preStep1 = 1.0f - xFrac1;

		float xLen = right[i].xPos - left[i].xPos;
		float zLen = right[i].zow - left[i].zow;
		float iLen = right[i].iow - left[i].iow;

		float preRatioDiff = (preStep0 - preStep1) / xLen;
	    float corrFactor = (1.0f - preRatioDiff) * oolen;

		float zDDA = zLen * corrFactor;
		float iDDA = iLen * corrFactor;

		ULONG zPos = left[i].zow + preStep0 * zDDA;
		LONG iPos = left[i].iow + preStep0 * iDDA;

		DrawSpanZBuffer16(dstColPtr, dstZPtr, runLength, zPos, iPos, zDDA, iDDA);
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

void DrawSpansSW16(UWORD * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;

		UWORD *dstColPtr = pixels + x0;

		pixels += modulo;

		if(x0 >= x1)
			continue;

		int runLength = x1 - x0;
		float oolen = 1.0f / runLength;
		float xFrac0 = left[i].xPos - x0;
		float xFrac1 = right[i].xPos - x1;
		float preStep0 = 1.0f - xFrac0;
		float preStep1 = 1.0f - xFrac1;

		float xLen = right[i].xPos - left[i].xPos;
		float iLen = right[i].iow - left[i].iow;

		float preRatioDiff = (preStep0 - preStep1) / xLen;
	    float corrFactor = (1.0f - preRatioDiff) * oolen;

		float iDDA = iLen * corrFactor;

		LONG iPos = left[i].iow + preStep0 * iDDA;

		DrawSpan16(dstColPtr, runLength, iPos, iDDA);
	}
}

/*****************************************************************************/
