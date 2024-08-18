#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

/*****************************************************************************/

static void DrawSpanZBuffer16(UWORD *destCol, UWORD *zBuffer, int len, ULONG Zz, LONG Ii, LONG dZz, LONG dIi, UWORD col)
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
			*destCol = col;//ti * 0x010101;
		}
		destCol++;
		zBuffer++;
		Ii += dIi;
		Zz += dZz;
	}
}

/*****************************************************************************/

void DrawSpansSW16ZBuffer(int ymin, int ymax, MaggieBase *lib)
{
	magEdgePos *edges = &lib->magEdge[ymin];
	int ylen = ymax - ymin;
	int modulo = lib->xres;
	UWORD *pixels = ((UWORD *)lib->screen) + ymin * lib->xres;
	UWORD *zbuffer = lib->depthBuffer + ymin * lib->xres;

	int r = ((lib->colour & 0x00f80000) >> 8);
	int g = ((lib->colour & 0x0000fc00) >> 5);
	int b = ((lib->colour & 0x000000f8) >> 3);

	UWORD col = r | g | b;

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

		int runLength = x1 - x0;
		float xFracStart = edges[i].xPosLeft - x0;
		float preStep = 1.0f - xFracStart;

		float xLen = edges[i].xPosRight - edges[i].xPosLeft;
		float zLen = edges[i].zowRight - edges[i].zowLeft;
		float iLen = edges[i].iowRight - edges[i].iowLeft;

	    float ooXLength = 1.0f / (edges[i].xPosRight - edges[i].xPosLeft);

		float zDDA = zLen * ooXLength;
		float iDDA = iLen * ooXLength;

		ULONG zPos = edges[i].zowLeft + preStep * zDDA;
		LONG iPos = edges[i].iowLeft + preStep * iDDA;

		if(x0 < scissorLeft)
		{
			int diff = scissorLeft - x0;
			iPos += iDDA * diff;
			zPos += zDDA * diff;
			dstColPtr += diff;
			dstZPtr += diff;
			runLength -= diff;
			if(runLength <= 0)
				continue;
		}
		if(x1 > scissorRight)
		{
			runLength -= x1 - scissorRight;
		}

		DrawSpanZBuffer16(dstColPtr, dstZPtr, runLength, zPos, iPos, zDDA, iDDA, col);
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

void DrawSpansSW16(int ymin, int ymax, MaggieBase *lib)
{
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

		int runLength = x1 - x0;
		float xFracStart = edges[i].xPosLeft - x0;
		float preStep = 1.0f - xFracStart;

		float iLen = edges[i].iowRight - edges[i].iowLeft;

		float iDDA = iLen / (edges[i].xPosRight - edges[i].xPosLeft);

		LONG iPos = edges[i].iowLeft + preStep * iDDA;

		if(x0 < scissorLeft)
		{
			int diff = scissorLeft - x0;
			iPos += iDDA * diff;
			dstColPtr += diff;
			runLength -= diff;
			if(runLength <= 0)
				continue;
		}
		if(x1 > scissorRight)
		{
			runLength -= x1 - scissorRight;
			if(runLength <= 0)
				continue;
		}

		DrawSpan16(dstColPtr, runLength, iPos, iDDA);
	}
}

/*****************************************************************************/
