/* maggie_static.c - static-link context lifecycle for maggie.library. */
/* A statically linked host has no RTF_AUTOINIT, so this is a plain-C create/destroy pair doing the same work as maggieInit()/maggieExpunge() in maggie.c, minus the OS library bookkeeping. */
/* MaggieBase is used unchanged, embedded struct Library and all, so the asm-hardcoded field offsets stay valid. */

#include <proto/exec.h>
#include <exec/memory.h>

#include "maggie_internal.h"

/*****************************************************************************/

void setRoundingModeRZ();

/*****************************************************************************/

struct Library *magCreateContext(void)
{
	struct ExecBase *SysBase = *(struct ExecBase **)4UL;	/* AbsExecBase */

	MaggieBase *lib = AllocMem(sizeof(MaggieBase), MEMF_PUBLIC | MEMF_CLEAR);
	if(!lib)
		return NULL;

	setRoundingModeRZ();

	/* Not a real OS library node, but keep it tidy in case anything peeks. */
	lib->lib.lib_Node.ln_Type = NT_LIBRARY;
	lib->lib.lib_Node.ln_Name = (char *)"maggie.library";
	lib->lib.lib_Version = 4;
	lib->lib.lib_Revision = 0;

	lib->sysBase = SysBase;
	lib->segList = 0;

	lib->gfxBase = (struct GfxBase *)OpenLibrary((STRPTR)"graphics.library", 0UL);
	if(!lib->gfxBase)
	{
		FreeMem(lib, sizeof(MaggieBase));
		return NULL;
	}

	lib->initialised = 1;
	lib->hasMaggie = 1;

	InitSemaphore(&lib->lock);

	lib->immModeVtx = 0xffff;
	lib->nIModeVtx = 0;

	lib->upVertexBuffer = 0xffff;
	lib->upIndexBuffer = 0xffff;

	lib->clearColour = 0x00000000;
	lib->clearDepth = 0xffff;

	/* AllocMem(MEMF_CLEAR) already NULLed the handle tables and dummyTextureData, so only the non-zero state needs setting. */
	for(int i = 0; i < MAG_MAX_LIGHTS; ++i)
	{
		lib->lights[i].type = MAG_LIGHT_OFF;
	}

	mat4_identity(&lib->worldMatrix);
	mat4_identity(&lib->viewMatrix);
	mat4_identity(&lib->perspectiveMatrix);
	mat4_identity(&lib->modelViewProj);
	mat4_identity(&lib->modelView);

	lib->dirtyMatrix = 0;

	lib->depthBuffer = AllocMem(MAGGIE_MAX_XRES * MAGGIE_MAX_YRES * sizeof(UWORD), MEMF_ANY | MEMF_CLEAR);
	if(!lib->depthBuffer)
	{
		CloseLibrary((struct Library *)lib->gfxBase);
		FreeMem(lib, sizeof(MaggieBase));
		return NULL;
	}

	return (struct Library *)lib;
}

/*****************************************************************************/

void magDeleteContext(struct Library *base)
{
	MaggieBase *lib = (MaggieBase *)base;
	if(!lib)
		return;

	struct ExecBase *SysBase = lib->sysBase;

	FreeMem(lib->depthBuffer, MAGGIE_MAX_XRES * MAGGIE_MAX_YRES * sizeof(UWORD));

	for(int i = 0; i < MAX_VERTEX_BUFFERS; ++i)
	{
		VertexBufferMemory *vbMem = lib->vertexBuffers[i];
		if(vbMem)
		{
			FreeMem(vbMem, vbMem->memSize);
		}
	}

	for(int i = 0; i < MAX_INDEX_BUFFERS; ++i)
	{
		ULONG *mem = lib->indexBuffers[i];
		if(mem)
		{
			FreeMem(mem, mem[1]);
		}
	}

	for(int i = 0; i < MAX_TEXTURES; ++i)
	{
		magTexture *texture = lib->textures[i];
		if(texture)
		{
			FreeMem(texture->allocPtr, texture->memSize);
		}
	}

	if(lib->dummyTextureData)
	{
		FreeMem(lib->dummyTextureData, 16 * 16 * 8);
	}

	CloseLibrary((struct Library *)lib->gfxBase);

	FreeMem(lib, sizeof(MaggieBase));
}
