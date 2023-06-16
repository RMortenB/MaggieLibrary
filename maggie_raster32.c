#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

/*****************************************************************************/

static void DrawHardwareSpanZBuffered(	APTR dest, APTR zDest,
										int len,
										ULONG ZZzz, ULONG UUuu, ULONG VVvv, UWORD Ii,
										LONG dZZzz, ULONG dUUuu, ULONG dVVvv, UWORD dIi)
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
		// len--;
		// if(len <= 0)
		// 	continue;
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

		float preRatioDiff = (preStep0 - preStep1) / xLen;
	    float corrFactor = (1.0f - preRatioDiff) * oolen;

		float zDDA = zLen * corrFactor;
		float wDDA = wLen * corrFactor;
		float uDDA = uLen * corrFactor;
		float vDDA = vLen * corrFactor;
		float iDDA = iLen * corrFactor;

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

static void DrawHardwareSpan(	APTR dest,
								int len,
								ULONG UUuu, ULONG VVvv, UWORD Ii,
								ULONG dUUuu, ULONG dVVvv, UWORD dIi)
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

		float preRatioDiff = (preStep0 - preStep1) / xLen;
	    float corrFactor = (1.0f - preRatioDiff) * oolen;

		float wDDA = wLen * corrFactor;
		float uDDA = uLen * corrFactor;
		float vDDA = vLen * corrFactor;
		float iDDA = iLen * corrFactor;

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
		{
			runLength = len;
		}

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
