#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

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

void DrawSpansSW32ZBuffer(ULONG * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo)
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

		float zPos = left[i].zow + preStep0 * zDDA;
		float iPos = left[i].iow + preStep0 * iDDA;

		DrawSpanZBuffer32(dstColPtr, dstZPtr, runLength, (ULONG)zPos, (LONG)iPos, zDDA, iDDA);
	}
}

/*****************************************************************************/

static unsigned int seed = 2234234;
unsigned int rndNum()
{
	seed = seed * 1015871 + 1023499;
	return seed;
}

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

void DrawSpansSW32(ULONG * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo)
{
	for(int i = 0; i < ylen; ++i)
	{
		int x0 = left[i].xPos;
		int x1 = right[i].xPos;

		ULONG *dstColPtr = pixels + x0;

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

		float iPos = left[i].iow + preStep0 * iDDA;

		DrawSpan32(dstColPtr, runLength, (LONG)iPos, iDDA);
	}
}

/*****************************************************************************/
/*

ULONG MortonCode(ULONG x)
{
    x = (x | (x << 8)) & 0x00ff00ff;
    x = (x | (x << 4)) & 0x0f0f0f0f;
    x = (x | (x << 2)) & 0x33333333;
    x = (x | (x << 1)) & 0x55555555;

    return x;
}

void DrawSpanZBufferedTxtr32(ULONG *destCol, UWORD *zBuffer, ULONG *texture, ULONG txtrMask,
											int len,
											ULONG ZZzz, ULONG UUuu, ULONG VVvv, UWORD Ii,
											LONG dZZzz, ULONG dUUuu, ULONG dVVvv, UWORD dIi)
{
    UUuu = (UUuu >> 8) & 0xffff;
    VVvv = (VVvv >> 8) & 0xffff;
    dUUuu = (dUUuu >> 8) & 0xffff;
    dVVvv = (dVVvv >> 8) & 0xffff;

	UUuu = MortonCode(UUuu) | 0xaaaaaaaa;
	VVvv = (MortonCode(VVvv) << 1) | 0x55555555;
	dUUuu = MortonCode(dUUuu);
	dVVvv = MortonCode(dVVvv) << 1;

	for(int i = 0; i < len; ++i)
	{
		UWORD z = (ZZzz >> 16);
		if(*zBuffer > z)
		{
			int ti = Ii >> 8;
			ULONG col = texture[((UUuu & VVvv) >> 16) & txtrMask];
			ULONG colRB = (col & 0xff00ff) * ti >> 8;
			ULONG colG = (col & 0x00ff00) * ti >> 8;

			*zBuffer = z;
			*destCol = colRB | colG;
		}
		destCol++;
		zBuffer++;
		UUuu = (UUuu + dUUuu) | 0xaaaaaaaa;
		VVvv = (VVvv + dVVvv) | 0x55555555;
		Ii += dIi;
		ZZzz += dZZzz;
	}
}

*/
/*****************************************************************************/
