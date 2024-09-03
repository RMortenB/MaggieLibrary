#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

/*****************************************************************************/

int mipOffset_table[] =
{
	0,									// 512 * 512
	512 * 256,							// 256 * 256
	512 * 256 + 256 * 128,				// 128 * 128
	512 * 256 + 256 * 128 + 128 * 64	// 64 * 64
};

/*****************************************************************************/

ULONG mipTexOffsets[] =
{
	0,									// 512 * 512
	1 << 15,							// 256 * 256
	1 << 16,							// 128 * 128
	1 << 17								// 64 * 64
};

/*****************************************************************************/

int texSizeIndex[] =
{
	9,	// 512 * 512
	8,	// 256 * 256
	7,	// 128 * 128
	6	// 64 * 64
};


/*****************************************************************************/

int GetMipmapIndex(int texSize)
{
	return texSizeIndex[texSize];
}

/*****************************************************************************/

int mag_abs(int val)
{
	return val < 0 ? -val : val;
}

int GetMipMapLevel(int dUUuu, int dVVvv)
{

	ULONG maxdUV = mag_abs(mag_abs(dUUuu) > mag_abs(dVVvv) ? dUUuu : dVVvv);
	int mipLevel = maxdUV >> 15;
	if(mipLevel > 3)
		mipLevel = 3;
	return mipLevel;
}

/*****************************************************************************/

int GetMipMapOffset(int mipLevel)
{
	return mipOffset_table[mipLevel];
}

/*****************************************************************************/

ULONG GetUVOffset(int mipLevel)
{
	return 1 << (mipLevel + 1);
}

/*****************************************************************************/

ULONG mipColours[] =
{
	0x0000ff00,
	0x00ff0000,
	0x000000ff,
	0x00ffff00,
};


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
	UWORD drawMode = 0;
	if(mode & MAG_DRAWMODE_BILINEAR)
	{
		drawMode |=  0x0001;
	}
	if(mode & MAG_DRAWMODE_DEPTHBUFFER)
	{
		drawMode |= 0x0002;
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
	maggieRegs.mode = drawMode;
	maggieRegs.modulo = modulo;
	maggieRegs.lightRGBA = lib->colour;
}

/*****************************************************************************/

struct DXTBlock
{
	UWORD colour0;
	UWORD colour1;
	ULONG indices;
};

/*****************************************************************************/
UWORD BSwap16(UWORD val)
{
	return (val >> 8) | (val << 8);
}

/*****************************************************************************/

ULONG BSwap32(ULONG val)
{
	return (val >> 24) | ((val >> 8) & 0x0000ff00) | ((val << 8) & 0x00ff0000) | (val << 24);
}

/*****************************************************************************/

ULONG ReadDXT1Texture(magTexture *txtr, ULONG UUuu, ULONG VVvv, int Ii)
{
	int indxU = UUuu >> ((8-txtr->texSize) + 16);
	int indxV = VVvv >> ((8-txtr->texSize) + 16);

	int blockU = indxU >> 2;
	int blockV = indxV >> 2;

	indxU &= 3;
	indxV &= 3;

	int blockRes = 1 << ((int)txtr->texSize - 2);
	int blockMask = blockRes - 1;

	blockU &= blockMask;
	blockV &= blockMask;

	struct DXTBlock *block = (struct DXTBlock *)GetTextureData(txtr);
	struct DXTBlock *currentBlock = &block[blockU + blockV * blockRes];

	ULONG indices = BSwap32(currentBlock->indices);
	UWORD colour0 = BSwap16(currentBlock->colour0);
	UWORD colour1 = BSwap16(currentBlock->colour1);
	int shift = (indxU + indxV * 4) << 1;
	ULONG index = (indices >> shift) & 3;
	ULONG colour = 0;
	if(colour0 > colour1)
	{
		switch(index)
		{
			case 0:	
			{
				int r0 = (((colour0 & 0xf800) >> 8) * Ii) >> 8;
				int g0 = (((colour0 & 0x07e0) >> 3) * Ii) >> 8;
				int b0 = (((colour0 & 0x001f) << 3) * Ii) >> 8;
				return (r0 << 16) | (g0 << 8) | b0;
			} break;
			case 1:
			{
				int r1 = (((colour1 & 0xf800) >> 8) * Ii) >> 8;
				int g1 = (((colour1 & 0x07e0) >> 3) * Ii) >> 8;
				int b1 = (((colour1 & 0x001f) << 3) * Ii) >> 8;
				return (r1 << 16) | (g1 << 8) | b1;
			} break;
			case 2:
			{	int r0 = (colour0 & 0xf800) >> 8;
				int g0 = (colour0 & 0x07e0) >> 3;
				int b0 = (colour0 & 0x001f) << 3;
				int r1 = (colour1 & 0xf800) >> 8;
				int g1 = (colour1 & 0x07e0) >> 3;
				int b1 = (colour1 & 0x001f) << 3;
				int r = ((r0 * 10 + r1 * 6) / 16 * Ii) >> 8;
				int g = ((g0 * 10 + g1 * 6) / 16 * Ii) >> 8;
				int b = ((b0 * 10 + b1 * 6) / 16 * Ii) >> 8;
				return (r << 16) | (g << 8) | b;
			} break;
			case 3:
			{
				int r0 = (colour0 & 0xf800) >> 8;
				int g0 = (colour0 & 0x07e0) >> 3;
				int b0 = (colour0 & 0x001f) << 3;
				int r1 = (colour1 & 0xf800) >> 8;
				int g1 = (colour1 & 0x07e0) >> 3;
				int b1 = (colour1 & 0x001f) << 3;
				int r = (((r0 * 6 + r1 * 10) / 16) * Ii) >> 8;
				int g = (((g0 * 6 + g1 * 10) / 16) * Ii) >> 8;
				int b = (((b0 * 6 + b1 * 10) / 16) * Ii) >> 8;
				return (r << 16) | (g << 8) | b;
			} break;
		}
	}
	else
	{
		switch(index)
		{
			case 0:	
			{
				int r0 = (((colour0 & 0xf800) >> 8) * Ii) >> 8;
				int g0 = (((colour0 & 0x07e0) >> 3) * Ii) >> 8;
				int b0 = (((colour0 & 0x001f) << 3) * Ii) >> 8;
				return (r0 << 16) | (g0 << 8) | b0;
			} break;
			case 1:
			{
				int r1 = (((colour1 & 0xf800) >> 8) * Ii) >> 8;
				int g1 = (((colour1 & 0x07e0) >> 3) * Ii) >> 8;
				int b1 = (((colour1 & 0x001f) << 3) * Ii) >> 8;
				return (r1 << 16) | (g1 << 8) | b1;
			} break;
			case 2:
			{
				int r0 = (colour0 & 0xf800) >> 8;
				int g0 = (colour0 & 0x07e0) >> 3;
				int b0 = (colour0 & 0x001f) << 3;
				int r1 = (colour1 & 0xf800) >> 8;
				int g1 = (colour1 & 0x07e0) >> 3;
				int b1 = (colour1 & 0x001f) << 3;
				int r = ((r0 + r1) / 2 * Ii) >> 8;
				int g = ((g0 + g1) / 2 * Ii) >> 8;
				int b = ((b0 + b1) / 2 * Ii) >> 8;
				return (r << 16) | (g << 8) | b;
			} break;
			case 3:
			{
				return 0;
			} break;
		}
	}
}


/*****************************************************************************/

void DrawHardwareSpanZBuffered(APTR dest, APTR zDest, int len, ULONG ZZzz, ULONG UUuu, ULONG VVvv, UWORD Ii, LONG dZZzz, int dUUuu, int dVVvv, UWORD dIi, APTR txtrData)
{
	int mipLevel = GetMipMapLevel(dUUuu, dVVvv);
	int mipOffset = GetMipMapOffset(mipLevel);
	int uvOffset = GetUVOffset(mipLevel);

#if 0
	ULONG *colPtr = (ULONG *)dest;
	UWORD *zPtr = (UWORD *)zDest;

	magTexture *txtr = lib->textures[lib->txtrIndex];
	for(int i = 0; i < len; ++i)
	{
		ULONG z = (ZZzz >> 16);
		if(*zPtr > z)
		{
			ULONG colour = ReadDXT1Texture(txtr, UUuu, VVvv, Ii >> 8);
			*zPtr = z;
			*colPtr = colour;
		}
		colPtr++;
		zPtr++;
		UUuu += dUUuu;
		VVvv += dVVvv;
		ZZzz += dZZzz;
		Ii += dIi;
	}
#else
//	maggieRegs.texture = txtrData + mipOffset;
//	maggieRegs.texSize = GetMipmapIndex(mipLevel);

	maggieRegs.pixDest = dest;
	maggieRegs.depthDest = zDest;
	maggieRegs.depthStart = ZZzz;
	maggieRegs.depthDelta = dZZzz;
	maggieRegs.uCoord = UUuu;// + uvOffset;
	maggieRegs.vCoord = VVvv;// + uvOffset;
	maggieRegs.uDelta = dUUuu;
	maggieRegs.vDelta = dVVvv;
	maggieRegs.light = Ii;
	maggieRegs.lightDelta = dIi;
	maggieRegs.startLength = len;
#endif
}

/*****************************************************************************/
static void DrawHardwareSpan(APTR dest, int len, ULONG UUuu, ULONG VVvv, UWORD Ii, ULONG dUUuu, ULONG dVVvv, UWORD dIi)
{
	int mipLevel = GetMipMapLevel(dUUuu, dVVvv);
	int mipOffset = GetMipMapOffset(mipLevel);
	int uvOffset = GetUVOffset(mipLevel);

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

void DrawSpansHW32ZBuffer(int ymin, int ymax, MaggieBase *lib)
{
	APTR txtrData = GetTextureData(lib->textures[lib->txtrIndex]);
	UWORD txtrSize = GetTexSizeIndex(lib->textures[lib->txtrIndex]);
	SetupHW(lib);
	SetTexture(txtrData, txtrSize);

	magEdgePos *edges = &lib->magEdge[ymin];
	int ylen = ymax - ymin;
	int modulo = lib->xres;
	ULONG *pixels = ((ULONG *)lib->screen) + ymin * lib->xres;
	UWORD *zbuffer = lib->depthBuffer + ymin * lib->xres;

	int scissorLeft = lib->scissor.x0;
	int scissorRight = lib->scissor.x1;

	for(int i = 0; i < ylen; ++i)
	{
		int x0 = edges[i].xPosLeft;
		int x1 = edges[i].xPosRight;

		ULONG *dstColPtr = pixels + x0;
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

				DrawHardwareSpanZBuffered(dstColPtr, dstZPtr, PIXEL_RUN, (ULONG)zPos, uStart, vStart, (LONG)iStart, (LONG)zDDA, dUUuu, dVVvv, dIi, lib);

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

			DrawHardwareSpanZBuffered(dstColPtr, dstZPtr, len, (ULONG)zPos, uStart, vStart, (LONG)iStart, (LONG)zDDA, dUUuu, dVVvv, dIi, lib);
		}
	}
}

/*****************************************************************************/

void DrawSpansHW32(int ymin, int ymax, MaggieBase *lib)
{
	APTR txtrData = GetTextureData(lib->textures[lib->txtrIndex]);
	UWORD txtrSize = GetTexSizeIndex(lib->textures[lib->txtrIndex]);
	SetTexture(txtrData, txtrSize);
	SetupHW(lib);

	magEdgePos *edges = &lib->magEdge[ymin];
	int ylen = ymax - ymin;
	int modulo = lib->xres;
	ULONG *pixels = ((ULONG *)lib->screen) + ymin * lib->xres;

	int scissorLeft = lib->scissor.x0;
	int scissorRight = lib->scissor.x1;

	for(int i = 0; i < ylen; ++i)
	{
		int x0 = edges[i].xPosLeft;
		int x1 = edges[i].xPosRight;

		ULONG *dstColPtr = pixels + x0;

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

/*****************************************************************************/
