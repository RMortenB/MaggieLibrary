#include "maggie_internal.h"

ULONG GetTextureMipMapSize(UWORD texSize)
{
	switch(texSize)
	{
		case 9 : return 512*512 / 2;
		case 8 : return 256*256 / 2;
		case 7 : return 128*128 / 2;
		case 6 : return 64*64 / 2;
	}
	return 0;
}

/*****************************************************************************/

ULONG GetTexturePixelWidth(UWORD texSize)
{
	switch(texSize)
	{
		case 9 : return 512;
		case 8 : return 256;
		case 7 : return 128;
		case 6 : return 64;
	}
	return 0;
}

/*****************************************************************************/

ULONG GetTexturePixelHeight(UWORD texSize)
{
	switch(texSize)
	{
		case 9 : return 512;
		case 8 : return 256;
		case 7 : return 128;
		case 6 : return 64;
	}
	return 0;
}
/*****************************************************************************/

ULONG GetTextureSize(UWORD texSize)
{
	switch(texSize)
	{
		case 9 : return (512*512 + 256*256 + 128*128 + 64*64) / 2;
		case 8 : return (256*256 + 128*128 + 64*64) / 2;
		case 7 : return (128*128 + 64*64) / 2;
		case 6 : return (64*64) / 2;
	}
	return 0;
}

/*****************************************************************************/

ULONG GetTextureMipMapOffset(UWORD topLevel, UWORD mipmap)
{
	if(topLevel < mipmap)
		return 0;
	return GetTextureSize(topLevel) & ~GetTextureSize(mipmap);
}

/*****************************************************************************/

APTR GetTextureData(ULONG *mem)
{
	return &mem[2];
}

/*****************************************************************************/

UWORD magAllocateTexture(REG(d0, UWORD size), REG(a6, MaggieBase *lib))
{
	UWORD txtr = ~0;

	struct ExecBase *SysBase = lib->sysBase;

	ObtainSemaphore(&lib->lock);

	for(int i = 0; i < MAX_TEXTURES; ++i)
	{
		if(!lib->textures[i])
		{
			txtr = i;
			lib->textures[i] = (ULONG *)1;
			break;
		}
	}

	ReleaseSemaphore(&lib->lock);

	if(txtr == ~0)
	{
		return 0xffff;
	}

	int txtrMemSize = GetTextureSize(size) + sizeof(ULONG) * 2;

	ULONG *mem = (ULONG *)AllocMem(txtrMemSize, MEMF_ANY | MEMF_CLEAR);
	if(!mem)
	{
		ObtainSemaphore(&lib->lock);
		lib->textures[txtr] = NULL;
		ReleaseSemaphore(&lib->lock);
		return 0xffff;
	}
	mem[0] = size;
	mem[1] = txtrMemSize;
	lib->textures[txtr] = mem;

	return txtr;
}

/*****************************************************************************/

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

UWORD RGBTo16Bit(ULONG rgb)
{
	return ((rgb >> 8) & 0xf800) | ((rgb >> 5) & 0x07e0) | ((rgb >> 3) & 0x001f);
}

/*****************************************************************************/

ULONG BSwap32(ULONG val)
{
	return (val >> 24) | ((val >> 8) & 0x0000ff00) | ((val << 8) & 0x00ff0000) | (val << 24);
}

/*****************************************************************************/

UWORD BSwap16(UWORD val)
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

void magUploadTexture(REG(d0, UWORD txtr), REG(d1, UWORD mipmap), REG(a0, APTR data), REG(d2, UWORD format), REG(a6, MaggieBase *lib))
{
	ULONG *mem = lib->textures[txtr];
	if(!mem)
		return;

	if(mipmap > mem[0])
		return;
	if(mipmap < 6)
		return;

	ULONG mipmapSize = GetTextureMipMapSize(mipmap);
	ULONG mipmapOffset = GetTextureMipMapOffset(mem[0], mipmap);

	UBYTE *dst = ((UBYTE *)&mem[2]) + mipmapOffset;
	UBYTE *src = (UBYTE *)data;

	switch(format & MAG_TEXFMT_MASK)
	{
		case MAG_TEXFMT_DXT1 :
		{
			for(int i = 0; i < mipmapSize; ++i)
			{
				dst[i] = src[i];
			}
		} break;
		case MAG_TEXFMT_RGB :
		{
			CompressRGB(dst, src, GetTexturePixelWidth(mipmap), GetTexturePixelHeight(mipmap), 3, format & MAG_TEXCOMP_HQ, lib);
		} break;
		case MAG_TEXFMT_RGBA :
		{
			CompressRGB(dst, src, GetTexturePixelWidth(mipmap), GetTexturePixelHeight(mipmap), 4, format & MAG_TEXCOMP_HQ, lib);
		} break;
	}
}

/*****************************************************************************/

void magFreeTexture(REG(d0, UWORD txtr), REG(a6, MaggieBase *lib))
{
	if(txtr >= MAX_TEXTURES)
		return;

	ULONG *mem = lib->textures[txtr];

	if(!mem)
		return;

	struct ExecBase *SysBase = lib->sysBase;

	ULONG size = mem[1];
	FreeMem(mem, size);

	lib->textures[txtr] = NULL;
}

/*****************************************************************************/
