#include "maggie_internal.h"

/*****************************************************************************/

ULONG GetTextureMipMapSize(UWORD format, UWORD texSize)
{
	if(format == MAG_TEXFMT_DXT1)
	{
		switch(texSize)
		{
			case 9 : return 512*512 / 2;
			case 8 : return 256*256 / 2;
			case 7 : return 128*128 / 2;
			case 6 : return 64*64 / 2;
		}
	}
	else if(format == MAG_TEXFMT_RGBA)
	{
		switch(texSize)
		{
			case 9 : return 512*512 * 4;
			case 8 : return 256*256 * 4;
			case 7 : return 128*128 * 4;
			case 6 : return 64*64 * 4;
		}
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

ULONG GetTextureSize(UWORD format, UWORD texSize)
{
	if(format == MAG_TEXFMT_DXT1)
	{
		switch(texSize)
		{
			case 9 : return (512*512 + 256*256 + 128*128 + 64*64) / 2;
			case 8 : return (256*256 + 128*128 + 64*64) / 2;
			case 7 : return (128*128 + 64*64) / 2;
			case 6 : return (64*64) / 2;
		}
	}
	else if(format == MAG_TEXFMT_RGBA)
	{
		switch(texSize)
		{
			case 9 : return (512*512 + 256*256 + 128*128 + 64*64) * 4;
			case 8 : return (256*256 + 128*128 + 64*64) * 4;
			case 7 : return (128*128 + 64*64) * 4;
			case 6 : return (64*64) * 4;
		}
	}
	return 0;
}

/*****************************************************************************/

ULONG GetTextureMipMapOffset(UWORD format, UWORD topLevel, UWORD mipmap)
{
	if(topLevel < mipmap)
		return 0;
	return GetTextureSize(format, topLevel) - GetTextureSize(format, mipmap);
}

/*****************************************************************************/

APTR GetTextureData(magTexture *txtr)
{
	return txtr->data;
}

/*****************************************************************************/

int GetTexSizeIndex(magTexture *txtr)
{
	return txtr->texSize;
}

/*****************************************************************************/

UWORD magAllocateTexture(REG(d0, UWORD size), REG(a6, MaggieBase *lib))
{
	UWORD txtr = 0xffff;

	struct ExecBase *SysBase = lib->sysBase;

	ObtainSemaphore(&lib->lock);

	for(int i = 0; i < MAX_TEXTURES; ++i)
	{
		if(!lib->textures[i])
		{
			txtr = i;
			lib->textures[i] = (magTexture *)1;
			break;
		}
	}

	ReleaseSemaphore(&lib->lock);

	if(txtr == 0xffff)
	{
		return 0xffff;
	}

	int format;
	if(lib->hasMaggie)
	 	format = MAG_TEXFMT_DXT1;
	else
	 	format = MAG_TEXFMT_RGBA;

	int txtrMemSize = GetTextureSize(format, size) + sizeof(magTexture);
	UBYTE *mem = (UBYTE *)AllocMem(txtrMemSize, MEMF_ANY | MEMF_CLEAR);

	if(!mem)
	{
		lib->textures[txtr] = NULL;
		return 0xffff;
	}

	magTexture *texture = (magTexture *)mem;
	texture->texSize = size;
	texture->mipMaps = 0;
	texture->allocPtr = mem;
	texture->format = MAG_TEXFMT_DXT1;
	texture->memSize = txtrMemSize;
	lib->textures[txtr] = texture;

	return txtr;
}

/*****************************************************************************/

void magUploadTexture(REG(d0, UWORD txtr), REG(d1, UWORD mipmap), REG(a0, APTR data), REG(d2, UWORD format), REG(a6, MaggieBase *lib))
{
	magTexture *texture = lib->textures[txtr];
	if(!texture)
		return;

	if(mipmap > texture->texSize)
		return;
	if(mipmap < 6)
		return;

	ULONG mipmapSize = GetTextureMipMapSize(texture->format, mipmap);
	ULONG mipmapOffset = GetTextureMipMapOffset(texture->format, texture->texSize, mipmap);

	UBYTE *dst = GetTextureData(texture);// + mipmapOffset;
	UBYTE *src = (UBYTE *)data;
	if(lib->hasMaggie)
	{
		switch(format & MAG_TEXFMT_MASK)
		{
			case MAG_TEXFMT_DXT1 :
			{
				 UBYTE tmp[8] = { 0xff, 0xff, 0x00, 0x00, 0x69, 0x96, 0x69, 0x96 };
				for(ULONG i = 0; i < mipmapSize; ++i)
				{
#if 0
					dst[i] = tmp[i & 0x07];
#else
					dst[i] = src[i];
#endif
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
	else
	{
		switch(format & MAG_TEXFMT_MASK)
		{
			case MAG_TEXFMT_DXT1 :
			{
				DeCompressDXT1(dst, src, GetTexturePixelWidth(mipmap), GetTexturePixelHeight(mipmap), lib);
			} break;
			case MAG_TEXFMT_RGB :
			{
				int srcPos = 0;
				int dstPos = 0;
				int nPixels = GetTexturePixelWidth(mipmap) * GetTexturePixelHeight(mipmap);
				for(int i = 0; i < nPixels; ++i)
				{
					dst[dstPos++] = src[srcPos++];
					dst[dstPos++] = src[srcPos++];
					dst[dstPos++] = src[srcPos++];
					dst[dstPos++] = 255;
				}
			} break;
			case MAG_TEXFMT_RGBA :
			{
				for(int i = 0; i < mipmapSize; ++i)
				{
					dst[i] = src[i];
				}
			} break;
		}
	}
}

/*****************************************************************************/

void magFreeTexture(REG(d0, UWORD txtr), REG(a6, MaggieBase *lib))
{
	if(txtr >= MAX_TEXTURES)
		return;

	magTexture *texture = lib->textures[txtr];

	if(!texture)
		return;

	struct ExecBase *SysBase = lib->sysBase;

	FreeMem(texture->allocPtr, texture->memSize);

	lib->textures[txtr] = NULL;
}

/*****************************************************************************/
