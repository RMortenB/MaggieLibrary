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

	for(int i = 0; i < MAX_INDEX_BUFFERS; ++i)
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
		return ~0;
	}

	int txtrMemSize = GetTextureSize(size) + sizeof(ULONG) * 2;

	ULONG *mem = (ULONG *)AllocMem(txtrMemSize, MEMF_ANY | MEMF_CLEAR);
	mem[0] = size;
	mem[1] = txtrMemSize;
	lib->textures[txtr] = mem;

	return txtr;
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

	switch(format)
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
			//CompressRGB(dst, src, mipmapSize);
		} break;
		case MAG_TEXFMT_RGBA :
		{
			//CompressRGBA(dst, src, mipmapSize);
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
