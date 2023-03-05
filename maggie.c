#include <proto/exec.h>
#include <exec/resident.h>

#include "maggie_internal.h"


int cliReturn()
{
	return -1;
}

const struct Resident romTag;

static APTR maggieInit(int segList __asm("a0"), MaggieBase *lib __asm("d0"), struct ExecBase *sysBase __asm("a6"));

static LONG maggieExpunge(MaggieBase *lib __asm("a6"))
{
	lib->lib.lib_Flags |= LIBF_DELEXP;
	if(lib->lib.lib_OpenCnt)
		return 0;

	struct ExecBase *SysBase = lib->sysBase;

	Remove(&lib->lib.lib_Node);

	FreeMem(lib->depthBuffer, MAGGIE_MAX_XRES * MAGGIE_MAX_YRES * sizeof(UWORD));

	for(int i = 0; i < MAX_VERTEX_BUFFERS; ++i)
	{
		ULONG *mem = lib->vertexBuffers[i];
		if(mem)
		{
			FreeMem(mem, mem[1]);
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
		ULONG *mem = lib->textures[i];
		if(mem)
		{
			FreeMem(mem, mem[1]);
		}
	}

	CloseLibrary((struct Library *)lib->gfxBase);

	ULONG segList = lib->segList;
	FreeMem(((char *)lib) - lib->lib.lib_NegSize, lib->lib.lib_NegSize + lib->lib.lib_PosSize);

	return segList;
}

static MaggieBase *maggieOpen(MaggieBase *lib __asm("a6"))
{
	lib->lib.lib_OpenCnt++;
	lib->lib.lib_Flags &= ~LIBF_DELEXP;

	return lib;
}

static LONG maggieClose(MaggieBase *lib __asm("a6"))
{
	lib->lib.lib_OpenCnt--;
	LONG ret = 0;
	if(!lib->lib.lib_OpenCnt)
	{
		if(lib->lib.lib_Flags & LIBF_DELEXP)
		{
			ret = maggieExpunge(lib);
		}
	}
	return ret;
}

/*****************************************************************************/

static APTR functionTable[] =
{
	maggieOpen, maggieClose, maggieExpunge, NULL,
	magSetScreenMemory,
	magSetTexture,
	magSetDrawMode,
	magSetRGB,
	magGetDepthBuffer,
	magSetWorldMatrix,
	magSetViewMatrix,
	magSetPerspectiveMatrix,
	magDrawTrianglesUP,
	magDrawIndexedTrianglesUP,
	magDrawIndexedPolygonsUP,
	magSetVertexBuffer,
	magSetIndexBuffer,
	magDrawTriangles,
	magDrawIndexedTriangles,
	magDrawIndexedPolygons,
	magDrawLinearSpan,
	magDrawSpan,
	magAllocateVertexBuffer,
	magUploadVertexBuffer,
	magFreeVertexBuffer,
	magAllocateIndexBuffer,
	magUploadIndexBuffer,
	magFreeIndexBuffer,
	magAllocateTexture,
	magUploadTexture,
	magFreeTexture,
	magBeginScene,
	magEndScene,
	magBegin,
	magEnd,
	magVertex,
	magNormal,
	magTexCoord,
	magTexCoord3,
	magColour,
	magClear,
	magSetLightType,
	magSetLightPosition,
	magSetLightDirection,
	magSetLightCone,
	magSetLightAttenuation,
	magSetLightColour,
	(APTR)-1
};

static const char libName[] = "maggie.library";
static const char libId[] = "maggie 1.00 (20.1.2023)";

static APTR maggieInit(int segList __asm("a0"), MaggieBase *lib __asm("d0"), struct ExecBase *sysBase __asm("a6"))
{
	lib->lib.lib_Node.ln_Type = NT_LIBRARY;
	lib->lib.lib_Node.ln_Name = (char *)libName;
	lib->lib.lib_Flags = LIBF_CHANGED | LIBF_SUMUSED;
	lib->lib.lib_Version = 1;
	lib->lib.lib_Revision = 0;
	lib->lib.lib_IdString = (char *)libId;

	lib->segList = segList;
	lib->sysBase = sysBase;

	struct ExecBase *SysBase = lib->sysBase;

	lib->gfxBase = (struct GfxBase *)OpenLibrary((STRPTR)"graphics.library", 0UL);
	lib->initialised = 1;

	InitSemaphore(&lib->lock);

	lib->immModeVtx = 0xffff;
	lib->nIModeVtx = 0;

	for(int i = 0; i < MAX_VERTEX_BUFFERS; ++i)
	{
		lib->vertexBuffers[i] = NULL;
	}
	for(int i = 0; i < MAX_INDEX_BUFFERS; ++i)
	{
		lib->indexBuffers[i] = NULL;
	}
	for(int i = 0; i < MAX_TEXTURES; ++i)
	{
		lib->textures[i] = NULL;
	}
	for(int i = 0; i < MAG_MAX_LIGHTS; ++i)
	{
		lib->lights[i].type = MAG_LIGHT_OFF;
	}

	mat4_identity(&lib->worldMatrix);
	mat4_identity(&lib->viewMatrix);
	mat4_identity(&lib->perspectiveMatrix);
	mat4_identity(&lib->modelviewProj);
	lib->dirtyMatrix = 0;

	lib->depthBuffer = AllocMem(MAGGIE_MAX_XRES * MAGGIE_MAX_YRES * sizeof(UWORD), MEMF_ANY | MEMF_CLEAR);

	return lib;
}

static ULONG initTable[] =
{
	sizeof(MaggieBase),
	(ULONG)functionTable,
	(ULONG)NULL,
	(ULONG)maggieInit
};

const struct Resident romTag =
{
	RTC_MATCHWORD,
	(APTR)&romTag,
	(APTR)&romTag + 1,
	RTF_AUTOINIT,
	1,
	NT_LIBRARY,
	0,
	(APTR)libName,
	(APTR)libId,
	initTable
};
