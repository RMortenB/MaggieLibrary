#include "maggie_internal.h"

typedef struct
{
	UWORD col0;
	UWORD col1;
	ULONG pixels;
} DXTBlock;

/*****************************************************************************/

#define MinVal(a,b) (((a) < (b)) ? (a) : (b))
#define MaxVal(a,b) (((a) > (b)) ? (a) : (b))

/*****************************************************************************/

static UWORD RGBTo16Bit(ULONG rgb)
{
	return ((rgb >> 8) & 0xf800) | ((rgb >> 5) & 0x07e0) | ((rgb >> 3) & 0x001f);
}

/*****************************************************************************/

static ULONG BSwap32(ULONG val)
{
	return (val >> 24) | ((val >> 8) & 0x0000ff00) | ((val << 8) & 0x00ff0000) | (val << 24);
}

/*****************************************************************************/

static UWORD BSwap16(UWORD val)
{
	return (val >> 8) | (val << 8);
}

/*****************************************************************************/

static int Sqr(int a)
{
	return a * a;
}

/*****************************************************************************/

static float QuantizeBlock3(DXTBlock *block, UBYTE *src, int width, int pixelSize, int rVec, int gVec, int bVec, int rOrigo, int gOrigo, int bOrigo, float ooLenSq)
{
	float error = 0.0f;

	int rCols[3];
	int gCols[3];
	int bCols[3];

	rCols[0] = ((block->col0 >> 8) & 0xf8);
	gCols[0] = ((block->col0 >> 3) & 0xfc);
	bCols[0] = ((block->col0 << 3) & 0xf8);
	rCols[1] = ((block->col1 >> 8) & 0xf8);
	gCols[1] = ((block->col1 >> 3) & 0xfc);
	bCols[1] = ((block->col1 << 3) & 0xf8);
	rCols[2] = (rCols[0] + rCols[1]) / 2;
	gCols[2] = (gCols[0] + gCols[1]) / 2;
	bCols[2] = (bCols[0] + bCols[1]) / 2;

	block->pixels = 0;

	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			int r = src[(i * width + j) * pixelSize + 0];
			int g = src[(i * width + j) * pixelSize + 1];
			int b = src[(i * width + j) * pixelSize + 2];

			float dotProd = ((r - rOrigo) * rVec + (g - gOrigo) * gVec + (b - bOrigo) * bVec) * ooLenSq;
			int pixVal = 0;

			if(dotProd < (1.0f / 3.0f))
			{
				pixVal = 1;
			}
			else if(dotProd < (2.0f / 3.0f))
			{
				pixVal = 2;
			}
			error += Sqr(r - rCols[pixVal]) + Sqr(g - gCols[pixVal]) + Sqr(b - bCols[pixVal]);
			block->pixels |= pixVal << (((i) * 4 + j) * 2);
		}
	}
	return error;
}

/*****************************************************************************/

static float QuantizeBlock4(DXTBlock *block, UBYTE *src, int width, int pixelSize, int rVec, int gVec, int bVec, int rOrigo, int gOrigo, int bOrigo, float ooLenSq)
{
	float error = 0.0f;

	float rCols[4];
	float gCols[4];
	float bCols[4];

	rCols[0] = ((block->col0 >> 8) & 0xf8);
	gCols[0] = ((block->col0 >> 3) & 0xfc);
	bCols[0] = ((block->col0 << 3) & 0xf8);
	rCols[1] = ((block->col1 >> 8) & 0xf8);
	gCols[1] = ((block->col1 >> 3) & 0xfc);
	bCols[1] = ((block->col1 << 3) & 0xf8);
	rCols[2] = (rCols[0] * 10.0f + rCols[1] * 6.0f) / 16.0f;
	gCols[2] = (gCols[0] * 10.0f + gCols[1] * 6.0f) / 16.0f;
	bCols[2] = (bCols[0] * 10.0f + bCols[1] * 6.0f) / 16.0f;
	rCols[3] = (rCols[0] * 6.0f + rCols[1] * 10.0f) / 16.0f;
	gCols[3] = (gCols[0] * 6.0f + gCols[1] * 10.0f) / 16.0f;
	bCols[3] = (bCols[0] * 6.0f + bCols[1] * 10.0f) / 16.0f;

	block->pixels = 0;

	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			int r = src[(i * width + j) * pixelSize + 0];
			int g = src[(i * width + j) * pixelSize + 1];
			int b = src[(i * width + j) * pixelSize + 2];

			float dotProd = ((r - rOrigo) * rVec + (g - gOrigo) * gVec + (b - bOrigo) * bVec) * ooLenSq;
			int pixVal = 0;

			if(dotProd < (3.0f / 16.0f))
			{
				pixVal = 1;
			}
			else if(dotProd < (1.0f / 2.0f))
			{
				pixVal = 3;
			}
			else if(dotProd < (13.0f / 16.0f))
			{
				pixVal = 2;
			}
			error += Sqr(r - rCols[pixVal]) + Sqr(g - gCols[pixVal]) + Sqr(b - bCols[pixVal]);
			block->pixels |= pixVal << (((i) * 4 + j) * 2);
		}
	}
	return error;
}

/*****************************************************************************/

void CompressRGB(UBYTE *dst, UBYTE *src, int width, int height, int pixelSize, int quality, MaggieBase *lib)
{
	DXTBlock *block = (DXTBlock *)dst;
	for(int y = 0; y < height; y += 4)
	{
		for(int x = 0; x < width; x += 4)
		{
			int bMin = 255;
			int gMin = 255;
			int rMin = 255;
			int bMax = 0;
			int gMax = 0;
			int rMax = 0;
			for(int i = 0; i < 4; ++i)
			{
				for(int j = 0; j < 4; ++j)
				{
					int r = src[((y + i) * width + x + j) * pixelSize + 0];
					int g = src[((y + i) * width + x + j) * pixelSize + 1];
					int b = src[((y + i) * width + x + j) * pixelSize + 2];
					rMin = MinVal(rMin, r);
					gMin = MinVal(gMin, g);
					bMin = MinVal(bMin, b);
					rMax = MaxVal(rMax, r);
					gMax = MaxVal(gMax, g);
					bMax = MaxVal(bMax, b);
				}
			}

			int rVec = rMax - rMin;
			int gVec = gMax - gMin;
			int bVec = bMax - bMin;
			float lenSq = bVec * bVec + gVec * gVec + rVec * rVec;

			block->col0 = RGBTo16Bit((rMax << 16) | (gMax << 8) | bMax);
			block->col1 = RGBTo16Bit((rMin << 16) | (gMin << 8) | bMin);
			block->pixels = 0;
			int blk4 = 1;
			if(lenSq > 0.0f)
			{
				float lowestError;
				float ooLenSq = 1.0f / lenSq;

				lowestError = QuantizeBlock4(block, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, gVec, bVec, rMin, gMin, bMin, ooLenSq);
				if(quality)
				{
					float error;
					DXTBlock testBlk;
					testBlk.col0 = RGBTo16Bit((rMax << 16) | (gMax << 8) | bMax);
					testBlk.col1 = RGBTo16Bit((rMin << 16) | (gMin << 8) | bMin);
					testBlk.pixels = 0;
					error = QuantizeBlock3(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, gVec, bVec, rMin, gMin, bMin, ooLenSq);
					if(lowestError > error)
					{
						lowestError = error;
						*block = testBlk;
						blk4 = 0;
					}

					testBlk.col0 = RGBTo16Bit((rMax << 16) | (gMax << 8) | bMin);
					testBlk.col1 = RGBTo16Bit((rMin << 16) | (gMin << 8) | bMax);
					error = QuantizeBlock4(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, gVec, -bVec, rMin, gMin, bMax, ooLenSq);
					if(lowestError > error)
					{
						lowestError = error;
						*block = testBlk;
						blk4 = 1;
					}
					error = QuantizeBlock3(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, gVec, -bVec, rMin, gMin, bMax, ooLenSq);
					if(lowestError > error)
					{
						lowestError = error;
						*block = testBlk;
						blk4 = 0;
					}

					testBlk.col0 = RGBTo16Bit((rMax << 16) | (gMin << 8) | bMax);
					testBlk.col1 = RGBTo16Bit((rMin << 16) | (gMax << 8) | bMin);
					error = QuantizeBlock4(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, -gVec, bVec, rMin, gMax, bMin, ooLenSq);
					if(lowestError > error)
					{
						lowestError = error;
						*block = testBlk;
						blk4 = 1;
					}
					error = QuantizeBlock3(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, -gVec, bVec, rMin, gMax, bMin, ooLenSq);
					if(lowestError > error)
					{
						lowestError = error;
						*block = testBlk;
						blk4 = 0;
					}

					testBlk.col0 = RGBTo16Bit((rMax << 16) | (gMin << 8) | bMin);
					testBlk.col1 = RGBTo16Bit((rMin << 16) | (gMax << 8) | bMax);
					error = QuantizeBlock4(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, -gVec, -bVec, rMin, gMax, bMax, ooLenSq);
					if(lowestError > error)
					{
						lowestError = error;
						*block = testBlk;
						blk4 = 1;
					}
					error = QuantizeBlock3(&testBlk, &src[(y * width + x) * pixelSize], width, pixelSize, rVec, -gVec, -bVec, rMin, gMax, bMax, ooLenSq);
					if(lowestError > error)
					{
						lowestError = error;
						*block = testBlk;
						blk4 = 0;
					}
				}
				if(blk4)
				{
					if(block->col0 < block->col1)
					{
						UWORD tmp = block->col0;
						block->col0 = block->col1;
						block->col1 = tmp;
						block->pixels ^= 0x55555555;
					}
				}
				else
				{
					if(block->col0 > block->col1)
					{
						UWORD tmp = block->col0;
						block->col0 = block->col1;
						block->col1 = tmp;
						block->pixels = ((~(block->pixels >> 1)) & 0x55555555) ^ block->pixels;
					}
				}
			}
			block->pixels = BSwap32(block->pixels);
			block->col0 = BSwap16(block->col0);
			block->col1 = BSwap16(block->col1);
			block++;
		}
	}
}

/*****************************************************************************/

void DeCompressDXT1(UBYTE *dst, UBYTE *src, int width, int height, MaggieBase *lib)
{
	DXTBlock *block = (DXTBlock *)src;
	for(int y = 0; y < height; y += 4)
	{
		for(int x = 0; x < width; x += 4)
		{
			UWORD col0 = BSwap16(block->col0);
			UWORD col1 = BSwap16(block->col1);
			ULONG pixels = BSwap32(block->pixels);
			int rCols[4];
			int gCols[4];
			int bCols[4];
			int aCols[4];
			if(col0 > col1)
			{
				int r0 = (col0 >> 11) & 0x1f;
				int g0 = (col0 >> 5) & 0x3f;
				int b0 = col0 & 0x1f;
				int r1 = (col1 >> 11) & 0x1f;
				int g1 = (col1 >> 5) & 0x3f;
				int b1 = col1 & 0x1f;
				int rVec0 = (r0 << 3) | (r0 >> 2);
				int gVec0 = (g0 << 2) | (g0 >> 4);
				int bVec0 = (b0 << 3) | (b0 >> 2);
				int rVec1 = (r1 << 3) | (r1 >> 2);
				int gVec1 = (g1 << 2) | (g1 >> 4);
				int bVec1 = (b1 << 3) | (b1 >> 2);
				rCols[0] = rVec0;
				gCols[0] = gVec0;
				bCols[0] = bVec0;
				aCols[0] = 0xff;
				rCols[1] = rVec1;
				gCols[1] = gVec1;
				bCols[1] = bVec1;
				aCols[1] = 0xff;
				rCols[2] = (10.0f * rVec0 + rVec1 * 6.0f) / 16.0f;
				gCols[2] = (10.0f * gVec0 + gVec1 * 6.0f) / 16.0f;
				bCols[2] = (10.0f * bVec0 + bVec1 * 6.0f) / 16.0f;
				aCols[2] = 0xff;
				rCols[3] = (6.0f * rVec0 + rVec1 * 10.0f) / 16.0f;
				gCols[3] = (6.0f * gVec0 + gVec1 * 10.0f) / 16.0f;
				bCols[3] = (6.0f * bVec0 + bVec1 * 10.0f) / 16.0f;
				aCols[3] = 0xff;
			}
			else
			{
				int r0 = (col0 >> 11) & 0x1f;
				int g0 = (col0 >> 5) & 0x3f;
				int b0 = col0 & 0x1f;
				int r1 = (col1 >> 11) & 0x1f;
				int g1 = (col1 >> 5) & 0x3f;
				int b1 = col1 & 0x1f;
				int rVec0 = (r0 << 3) | (r0 >> 2);
				int gVec0 = (g0 << 2) | (g0 >> 4);
				int bVec0 = (b0 << 3) | (b0 >> 2);
				int rVec1 = (r1 << 3) | (r1 >> 2);
				int gVec1 = (g1 << 2) | (g1 >> 4);
				int bVec1 = (b1 << 3) | (b1 >> 2);
				rCols[0] = rVec0;
				gCols[0] = gVec0;
				bCols[0] = bVec0;
				aCols[0] = 0xff;
				rCols[1] = rVec1;
				gCols[1] = gVec1;
				bCols[1] = bVec1;
				aCols[1] = 0xff;
				rCols[2] = (rVec0 + rVec1) / 2;
				gCols[2] = (gVec0 + gVec1) / 2;
				bCols[2] = (bVec0 + bVec1) / 2;
				aCols[2] = 0xff;
				rCols[3] = 0x00;
				gCols[3] = 0x00;
				bCols[3] = 0x00;
				aCols[3] = 0x00;
			}
			for(int i = 0; i < 4; ++i)
			{
				for(int j = 0; j < 4; ++j)
				{
					int pixVal = (pixels >> (((i) * 4 + j) * 2)) & 3;
					dst[((y + i) * width + x + j) * 4 + 0] = rCols[pixVal];
					dst[((y + i) * width + x + j) * 4 + 1] = gCols[pixVal];
					dst[((y + i) * width + x + j) * 4 + 2] = bCols[pixVal];
					dst[((y + i) * width + x + j) * 4 + 3] = aCols[pixVal];
				}
			}
			block++;
		}
	}
}




/*****************************************************************************/


