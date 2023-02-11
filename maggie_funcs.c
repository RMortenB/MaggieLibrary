#include "maggie_internal.h"
#include "maggie_debug.h"
#include <proto/graphics.h>
#include <string.h>

void magSetScreenMemory(REG(a0, APTR *pixels), REG(d0, UWORD xres), REG(d1, UWORD yres), REG(a6, MaggieBase *lib))
{
	lib->xres = xres;
	lib->yres = yres;
	lib->screen = pixels;
}

/*****************************************************************************/

void magSetTexture(REG(d0, UWORD unit), REG(d1, UWORD txtr), REG(a6, MaggieBase *lib))
{
	lib->txtrIndex = txtr;
}

/*****************************************************************************/

void magSetDrawMode(REG(d0, UWORD mode), REG(a6, MaggieBase *lib))
{
	if((mode & MAG_DRAWMODE_DEPTHBUFFER) & !lib->depthBuffer)
	{
		struct ExecBase *SysBase = lib->sysBase;
		lib->depthBuffer = AllocMem(MAGGIE_MAX_XRES * MAGGIE_MAX_YRES * sizeof(UWORD), MEMF_ANY | MEMF_CLEAR);

	}
	lib->drawMode = mode;
}

/*****************************************************************************/

void magSetRGB(REG(d0, ULONG rgb), REG(a6, MaggieBase *lib))
{
	lib->colour = rgb;
}

/*****************************************************************************/

UWORD *magGetDepthBuffer(REG(a6, MaggieBase *lib))
{
	return lib->depthBuffer;
}

/*****************************************************************************/

void magSetVertexBuffer(REG(d0, WORD vBuffer), REG(a6, MaggieBase *lib))
{
	lib->vBuffer = vBuffer;
}

/*****************************************************************************/

void magSetIndexBuffer(REG(d0, WORD iBuffer), REG(a6, MaggieBase *lib))
{
	lib->iBuffer = iBuffer;
}

/*****************************************************************************/

UWORD GetVBNumVerts(ULONG *mem)
{
	return mem[0];
}

/*****************************************************************************/

struct MaggieVertex *GetVBVertices(ULONG *mem)
{
	return (struct MaggieVertex *)&mem[2];
}

/*****************************************************************************/

UBYTE *GetVBClipCodes(ULONG *mem)
{
	return ((UBYTE *)&mem[2]) + sizeof(struct MaggieVertex) * GetVBNumVerts(mem);
}

/*****************************************************************************/

struct MaggieTransVertex *GetVBTransVertices(ULONG *mem)
{
	return (struct MaggieTransVertex *)(((UBYTE *)&mem[2]) + (sizeof(struct MaggieVertex) + 1) * GetVBNumVerts(mem));
}

/*****************************************************************************/

UWORD magAllocateVertexBuffer(REG(d0, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	UWORD vBuffer = ~0;

	struct ExecBase *SysBase = lib->sysBase;

	ObtainSemaphore(&lib->lock);

	for(int i = 0; i < MAX_VERTEX_BUFFERS; ++i)
	{
		if(!lib->vertexBuffers[i])
		{
			vBuffer = i;
			lib->vertexBuffers[i] = (ULONG *)1;
			break;
		}
	}

	ReleaseSemaphore(&lib->lock);

	if(vBuffer == ~0)
		return ~0;

	ULONG *mem = (ULONG *)AllocMem(sizeof(struct MaggieVertex) * nVerts + sizeof(struct MaggieTransVertex) * nVerts + sizeof(ULONG) * 2, MEMF_ANY | MEMF_CLEAR);
	mem[0] = nVerts;
	mem[1] = sizeof(struct MaggieVertex) * nVerts + sizeof(ULONG) * 2;
	lib->vertexBuffers[vBuffer] = mem;

	return vBuffer;
}

/*****************************************************************************/

void magUploadVertexBuffer(REG(d0, UWORD vBuffer), REG(a0, struct MaggieVertex *vtx), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	ULONG *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return;

	if(nVerts > GetVBNumVerts(mem))
		nVerts = GetVBNumVerts(mem);
	if(startVtx + nVerts > GetVBNumVerts(mem))
	{
		nVerts = GetVBNumVerts(mem) - startVtx;
	}

	struct MaggieVertex *dst = (struct MaggieVertex *)&mem[2];
	for(int i = 0; i < nVerts; ++i)
	{
		dst[i] = vtx[startVtx + i];
	}
}

/*****************************************************************************/

void magFreeVertexBuffer(REG(d0, UWORD vBuffer), REG(a6, MaggieBase *lib))
{
	if(vBuffer >= MAX_VERTEX_BUFFERS)
		return;

	ULONG *mem = lib->vertexBuffers[vBuffer];

	if(!mem)
		return;

	ULONG size = mem[1];

	struct ExecBase *SysBase = lib->sysBase;

	FreeMem(mem, size);

	lib->vertexBuffers[vBuffer] = NULL;
}

/*****************************************************************************/

UWORD GetIBNumIndices(ULONG *mem)
{
	return mem[0];
}

UWORD *GetIBIndices(ULONG *mem)
{
	return (UWORD *)(&mem[2]);
}

/*****************************************************************************/

UWORD magAllocateIndexBuffer(REG(d0, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	UWORD iBuffer = ~0;

	struct ExecBase *SysBase = lib->sysBase;

	ObtainSemaphore(&lib->lock);

	for(int i = 0; i < MAX_INDEX_BUFFERS; ++i)
	{
		if(!lib->indexBuffers[i])
		{
			iBuffer = i;
			lib->indexBuffers[i] = (ULONG *)1;
			break;
		}
	}

	ReleaseSemaphore(&lib->lock);

	if(iBuffer == ~0)
		return ~0;

	ULONG *mem = (ULONG *)AllocMem(sizeof(UWORD) * nIndx + sizeof(ULONG) * 2, MEMF_ANY | MEMF_CLEAR);
	mem[0] = nIndx;
	mem[1] = sizeof(UWORD) * nIndx + sizeof(ULONG) * 2;
	lib->indexBuffers[iBuffer] = mem;

	return iBuffer;
}

/*****************************************************************************/

void magUploadIndexBuffer(REG(d0, UWORD iBuffer), REG(a0, UWORD *indx), REG(d1, UWORD startIndx), REG(d2, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	ULONG *mem = lib->indexBuffers[iBuffer];
	if(!mem)
		return;

	if(nIndx > mem[0])
		nIndx = mem[0];

	if(startIndx + nIndx > mem[0])
	{
		nIndx = mem[0] - startIndx;
	}

	UWORD *dst = (UWORD *)&mem[2];
	for(int i = 0; i < nIndx; ++i)
	{
		dst[i] = indx[startIndx + i];
	}
}

/*****************************************************************************/

void magFreeIndexBuffer(REG(d0, UWORD iBuffer), REG(a6, MaggieBase *lib))
{
	if(iBuffer >= MAX_INDEX_BUFFERS)
		return;

	ULONG *mem = lib->indexBuffers[iBuffer];

	if(!mem)
		return;

	struct ExecBase *SysBase = lib->sysBase;

	ULONG size = mem[1];
	FreeMem(mem, size);

	lib->indexBuffers[iBuffer] = NULL;
}

/*****************************************************************************/

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

	ULONG mipmapSize = GetTextureMipMapSize(mem[0]);
	ULONG mipmapOffset = GetTextureMipMapOffset(mem[0], mipmap);

	UBYTE *dst = ((UBYTE *)&mem[2]) + mipmapOffset;
	UBYTE *src = (UBYTE *)data;

	for(int i = 0; i < mipmapSize; ++i)
	{
		dst[i] = src[i];
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

// Library Lock..
void magBeginScene(REG(a6, MaggieBase *lib))
{
	struct ExecBase *SysBase = lib->sysBase;
	ObtainSemaphore(&lib->lock);
	DebugReset();
	memset(lib->depthBuffer, 0xff, sizeof(unsigned short) * lib->xres * lib->yres);
}

/*****************************************************************************/

void magEndScene(REG(a6, MaggieBase *lib))
{
	struct ExecBase *SysBase = lib->sysBase;
	ReleaseSemaphore(&lib->lock);
}

/*****************************************************************************/
/*****************************************************************************/

// Immediate mode, or "slow mode"..
void magBegin(REG(a6, MaggieBase *lib))
{
	lib->nIModeVtx = 0;
	if(lib->immModeVtx == 0xffff)
	{
		lib->immModeVtx = magAllocateVertexBuffer(IMM_MODE_MAGGIE_VERTS, lib);
	}
	struct GfxBase *GfxBase = lib->gfxBase;
	OwnBlitter();
	WaitBlit();
}

/*****************************************************************************/

void magEnd(REG(a6, MaggieBase *lib))
{
	if(lib->nIModeVtx)
	{
		FlushImmediateMode(lib);
	}
	struct GfxBase *GfxBase = lib->gfxBase;
	DisownBlitter();
}

/*****************************************************************************/

void magVertex(REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(fp3, float w), REG(a6, MaggieBase *lib))
{
	if(lib->nIModeVtx == IMM_MODE_MAGGIE_VERTS)
	{
		FlushImmediateMode(lib);
	}
	if(lib->immModeVtx == 0xffff)
		return;
	struct MaggieVertex *vtx = (struct MaggieVertex *)&(lib->vertexBuffers[lib->immModeVtx][2]);
	vtx[lib->nIModeVtx].pos.x = x;
	vtx[lib->nIModeVtx].pos.y = y;
	vtx[lib->nIModeVtx].pos.z = z;
	for(int i = 0; i < MAGGIE_MAX_TEXCOORDS; ++i)
	{
		vtx[lib->nIModeVtx].tex[i].u = 0.0f;
		vtx[lib->nIModeVtx].tex[i].v = 0.0f;
		vtx[lib->nIModeVtx].tex[i].w = 1.0f;
	}
	vtx[lib->nIModeVtx].rgba = ~0;

	lib->nIModeVtx++;
}

/*****************************************************************************/

void magTexCoord(REG(d0, UWORD texReg), REG(fp0, float u), REG(fp1, float v), REG(a6, MaggieBase *lib))
{
	struct MaggieVertex *vtx = (struct MaggieVertex *)&(lib->vertexBuffers[lib->immModeVtx][2]);
	vtx->tex[texReg].u = u * 256.0f * 65536.0f;
	vtx->tex[texReg].v = v * 256.0f * 65536.0f;
}

/*****************************************************************************/

void magTexCoord3(REG(d0, UWORD texReg), REG(fp0, float u), REG(fp1, float v), REG(fp2, float w), REG(a6, MaggieBase *lib))
{
	struct MaggieVertex *vtx = (struct MaggieVertex *)&(lib->vertexBuffers[lib->immModeVtx][2]);
	vtx[lib->nIModeVtx].tex[texReg].u = u * 256.0f * 65536.0f;
	vtx[lib->nIModeVtx].tex[texReg].v = v * 256.0f * 65536.0f;
	vtx[lib->nIModeVtx].tex[texReg].w = w;
}

/*****************************************************************************/

void magColour(REG(d0, ULONG col), REG(a6, MaggieBase *lib))
{
	struct MaggieVertex *vtx = (struct MaggieVertex *)&(lib->vertexBuffers[lib->immModeVtx][2]);
	vtx[lib->nIModeVtx].rgba = (col & 0xff00) | ((col >> 8) & 0xff);
}

/*****************************************************************************/
