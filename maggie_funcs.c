#include "maggie_internal.h"
#include "maggie_debug.h"
#include <proto/graphics.h>
#include <string.h>

/*****************************************************************************/

VertexBufferMemory *CreateVertexBufferMemory(UWORD nVerts, MaggieBase *lib)
{
	ULONG memSize = sizeof(VertexBufferMemory) +
					sizeof(vec3) * nVerts +
					sizeof(MaggieNormal) * nVerts +
					sizeof(struct MaggieTexCoord) * nVerts +
					sizeof(ULONG) * nVerts +
					sizeof(UBYTE) * nVerts +
					sizeof(struct MaggieTransVertex) * nVerts;

	struct ExecBase *SysBase = lib->sysBase;

	BYTE *ptr = (BYTE *)AllocMem(memSize, MEMF_ANY | MEMF_CLEAR);

	VertexBufferMemory *vbMem = (VertexBufferMemory *)ptr;
	ptr += sizeof(VertexBufferMemory);
	vbMem->positions = (vec3 *)ptr;
	ptr += sizeof(vec3) * nVerts;
	vbMem->normals = (MaggieNormal *)ptr;
	ptr += sizeof(MaggieNormal) * nVerts;
	vbMem->uvs = (struct MaggieTexCoord *)ptr;
	ptr += sizeof(vec3) * nVerts;
	vbMem->colours = (ULONG *)ptr;
	ptr += sizeof(ULONG) * nVerts;
	vbMem->clipCodes = (UBYTE *)ptr;
	ptr += sizeof(UBYTE) * nVerts;
	vbMem->transVerts = (struct MaggieTransVertex *)ptr;

	vbMem->nVerts = nVerts;
	vbMem->memSize = memSize;

	memset(vbMem->colours, ~0, sizeof(ULONG) * nVerts);


	return vbMem;
}

/*****************************************************************************/

void FreeVertexBufferMemory(VertexBufferMemory *mem, MaggieBase *lib)
{
	struct ExecBase *SysBase = lib->sysBase;
	FreeMem(mem, mem->memSize);
}

/*****************************************************************************/

void magSetScreenMemory(REG(a0, APTR *pixels), REG(d0, UWORD xres), REG(d1, UWORD yres), REG(a6, MaggieBase *lib))
{
	lib->xres = xres;
	lib->yres = yres;
	lib->screen = pixels;

	lib->scissor.x0 = 0;
	lib->scissor.y0 = 0;
	lib->scissor.x1 = xres;
	lib->scissor.y1 = yres;

	lib->dirtyMatrix = 1;
}

/*****************************************************************************/

void magSetTexture(REG(d0, UWORD unit), REG(d1, UWORD txtr), REG(a6, MaggieBase *lib))
{
	lib->txtrIndex = txtr;
}

/*****************************************************************************/

void magSetDrawMode(REG(d0, UWORD mode), REG(a6, MaggieBase *lib))
{
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
/*****************************************************************************/

ULONG ColourTo16Bit(ULONG colour)
{
	return ((colour >> 8) & 0xf800) | ((colour >> 1) & 0x07e0)  | ((colour >> 3) & 0x001f);
}

/*****************************************************************************/

// UWORD GetVBNumVerts(ULONG *mem)
// {
// 	return mem[0];
// }

/*****************************************************************************/

// struct MaggieVertex *GetVBVertices(ULONG *mem)
// {
// 	return (struct MaggieVertex *)&mem[2];
// }

/*****************************************************************************/

// UBYTE *GetVBClipCodes(ULONG *mem)
// {
// 	return ((UBYTE *)&mem[2]) + sizeof(struct MaggieVertex) * GetVBNumVerts(mem);
// }

/*****************************************************************************/

// struct MaggieTransVertex *GetVBTransVertices(ULONG *mem)
// {
// 	return (struct MaggieTransVertex *)(((UBYTE *)&mem[2]) + (sizeof(struct MaggieVertex) + 1) * GetVBNumVerts(mem));
// }

/*****************************************************************************/

UWORD magAllocateVertexBuffer(REG(d0, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	UWORD vBuffer = 0xffff;

	struct ExecBase *SysBase = lib->sysBase;

	ObtainSemaphore(&lib->lock);

	for(int i = 0; i < MAX_VERTEX_BUFFERS; ++i)
	{
		if(!lib->vertexBuffers[i])
		{
			vBuffer = i;
			lib->vertexBuffers[i] = (VertexBufferMemory *)1;
			break;
		}
	}

	ReleaseSemaphore(&lib->lock);

	if(vBuffer == 0xffff)
		return 0xffff;

	lib->vertexBuffers[vBuffer] = CreateVertexBufferMemory(nVerts, lib);
	return vBuffer;
}

/*****************************************************************************/

vec3 *GetVertexPositions(UWORD vBuffer, MaggieBase *lib)
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return NULL;

	return mem->positions;
}

/*****************************************************************************/

MaggieNormal *GetVertexNormals(UWORD vBuffer, MaggieBase *lib)
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return NULL;

	return mem->normals;
}

/*****************************************************************************/

struct MaggieTexCoord *GetVertexTexCoords(UWORD vBuffer, MaggieBase *lib)
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return NULL;

	return mem->uvs;
}

/*****************************************************************************/

ULONG *GetVertexColours(UWORD vBuffer, MaggieBase *lib)
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return NULL;

	return mem->colours;
}

/*****************************************************************************/

void magUploadVertexPositions(REG(d0, UWORD vBuffer), REG(a0, vec3 *pos), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return;
	if(startVtx + nVerts > mem->nVerts)
	{
		nVerts = mem->nVerts - startVtx;
	}
	if(nVerts <= 0)
		return;
	memcpy(mem->positions + startVtx, pos, sizeof(vec3) * nVerts);
}

/*****************************************************************************/

void magUploadVertexNormals(REG(d0, UWORD vBuffer), REG(a0, vec3 *norm), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return;
	if(startVtx + nVerts > mem->nVerts)
	{
		nVerts = mem->nVerts - startVtx;
	}
	if(nVerts <= 0)
		return;

	for(int i = 0; i < nVerts; ++i)
	{
		mem->normals[startVtx + i].x = (UWORD)(norm[i].x * 256.0f);
		mem->normals[startVtx + i].y = (UWORD)(norm[i].y * 256.0f);
		mem->normals[startVtx + i].z = (UWORD)(norm[i].z * 256.0f);
	}
}

/*****************************************************************************/

void magUploadVertexTexCoords2(REG(d0, UWORD vBuffer), REG(a0, vec2 *uvs), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return;
	if(startVtx + nVerts > mem->nVerts)
	{
		nVerts = mem->nVerts - startVtx;
	}
	if(nVerts <= 0)
		return;

	for(int i = 0; i < nVerts; ++i)
	{
		mem->uvs[startVtx + i].u = uvs[i].x * 65536.0f * 256.0f;
		mem->uvs[startVtx + i].v = uvs[i].y * 65536.0f * 256.0f;
		mem->uvs[startVtx + i].w = 65536.0f * 256.0f;
	}

}

/*****************************************************************************/

void magUploadVertexTexCoords3(REG(d0, UWORD vBuffer), REG(a0, vec3 *uvs), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return;
	if(startVtx + nVerts > mem->nVerts)
	{
		nVerts = mem->nVerts - startVtx;
	}
	if(nVerts <= 0)
		return;

	for(int i = 0; i < nVerts; ++i)
	{
		mem->uvs[startVtx + i].u = uvs[i].x * 65536.0f * 256.0f;
		mem->uvs[startVtx + i].v = uvs[i].y * 65536.0f * 256.0f;
		mem->uvs[startVtx + i].w = uvs[i].z * 65536.0f * 256.0f;
	}
}

/*****************************************************************************/

void magUploadVertexColours(REG(d0, UWORD vBuffer), REG(a0, ULONG *colours), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	VertexBufferMemory *mem = lib->vertexBuffers[vBuffer];
	if(!mem)
		return;
	if(startVtx + nVerts > mem->nVerts)
	{
		nVerts = mem->nVerts - startVtx;
	}
	if(nVerts <= 0)
		return;

	for(int i = 0; i < nVerts; ++i)
	{
		UWORD gray = RGBToGrayScale(colours[i]);
		mem->colours[startVtx + i] = gray | (gray << 8);
		mem->transVerts[startVtx + i].colour = gray | (gray << 8);
	}
}

/*****************************************************************************/

static MaggieNormal CompressNormal(vec3 *normal)
{
	MaggieNormal n;
	n.x = (WORD)(normal->x * 256.0f);
	n.y = (WORD)(normal->y * 256.0f);
	n.z = (WORD)(normal->z * 256.0f);
	return n;
}

void magUploadVertexBuffer(REG(d0, UWORD vBuffer), REG(a0, struct MaggieVertex *vtx), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	VertexBufferMemory *vbMem = lib->vertexBuffers[vBuffer];
	if(!vbMem)
		return;

	if(startVtx + nVerts > vbMem->nVerts)
	{
		nVerts = vbMem->nVerts - startVtx;
	}

	for(int i = 0; i < nVerts; ++i)
	{
		vbMem->positions[startVtx + i] = vtx[i].pos;
		vbMem->normals[startVtx + i] = CompressNormal(&vtx[i].normal);
		vbMem->uvs[startVtx + i].u = vtx[i].tex[0].u * 256.0f * 65536.0f;
		vbMem->uvs[startVtx + i].v = vtx[i].tex[0].v * 256.0f * 65536.0f;
		vbMem->uvs[startVtx + i].w = vtx[i].tex[0].w;
		UWORD gray = RGBToGrayScale(vtx[i].colour);
		vbMem->colours[startVtx + i] = gray | (gray << 8);
		vbMem->transVerts[startVtx + i].colour = gray | (gray << 8);

		for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
		{
			vbMem->transVerts[startVtx + i].tex[j].u = vtx[i].tex[j].u * 256.0f * 65536.0f;
			vbMem->transVerts[startVtx + i].tex[j].v = vtx[i].tex[j].v * 256.0f * 65536.0f;
			vbMem->transVerts[startVtx + i].tex[j].w = vtx[i].tex[j].w;
		}
	}
}

/*****************************************************************************/

void magFreeVertexBuffer(REG(d0, UWORD vBuffer), REG(a6, MaggieBase *lib))
{
	if(vBuffer >= MAX_VERTEX_BUFFERS)
		return;

	VertexBufferMemory *vbMem = lib->vertexBuffers[vBuffer];
	if(!vbMem)
		return;

	lib->vertexBuffers[vBuffer] = NULL;

	struct ExecBase *SysBase = lib->sysBase;
	FreeMem(vbMem, vbMem->memSize);
}

/*****************************************************************************/

UWORD GetIBNumIndices(ULONG *mem)
{
	return mem[0];
}

/*****************************************************************************/

UWORD *GetIBIndices(ULONG *mem)
{
	return (UWORD *)(&mem[2]);
}

/*****************************************************************************/

UWORD magAllocateIndexBuffer(REG(d0, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	UWORD iBuffer = 0xffff;

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

	if(iBuffer == 0xffff)
		return 0xffff;

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
void setRoundingModeRZ();
// Library Lock..
void magBeginScene(REG(a6, MaggieBase *lib))
{
	struct ExecBase *SysBase = lib->sysBase;
	ObtainSemaphore(&lib->lock);

	setRoundingModeRZ();

	for(int i = 0; i < MAG_MAX_LIGHTS; ++i)
	{
		lib->lights[i].type = MAG_LIGHT_OFF;
	}
	DebugReset();
	lib->colour = 0x00ffffff;
#if PROFILE
	memset(&lib->profile, 0, sizeof(lib->profile));
	// Snapshot after the memset, which would otherwise zero it, and before the frame timer starts so the four register reads land outside the measured window.
	lib->profile.magStart.texReads = maggiePerf.texReads;
	lib->profile.magStart.depthReads = maggiePerf.depthReads;
	lib->profile.magStart.screenReads = maggiePerf.screenReads;
	lib->profile.magStart.pixels = maggiePerf.pixels;
	lib->profile.frame = GetClocks();
#endif
}

/*****************************************************************************/

void magEndScene(REG(a6, MaggieBase *lib))
{
	struct ExecBase *SysBase = lib->sysBase;
#if PROFILE
	lib->profile.frame = GetClocks() - lib->profile.frame;

	lib->profile.magDelta.texReads = maggiePerf.texReads - lib->profile.magStart.texReads;
	lib->profile.magDelta.depthReads = maggiePerf.depthReads - lib->profile.magStart.depthReads;
	lib->profile.magDelta.screenReads = maggiePerf.screenReads - lib->profile.magStart.screenReads;
	lib->profile.magDelta.pixels = maggiePerf.pixels - lib->profile.magStart.pixels;

	if(lib->profile.frame)
	{
		// `draw` is the whole magDraw* window and already contains `spans` (and trans/texgen/light), so the number worth looking at is the difference: Spans is the rasterizer, Setup is the CPU cost of feeding it.
		ULONG spans = lib->profile.spans;
		ULONG setup = (lib->profile.draw > spans) ? lib->profile.draw - spans : 0;

		TextOut(lib, "Frame  : %d", lib->profile.frame);
		TextOut(lib, "Clear  : %d - %d%%", lib->profile.clear, lib->profile.clear * 100 / lib->profile.frame);
		TextOut(lib, "Spans  : %d - %d%%", spans, spans * 100 / lib->profile.frame);
		TextOut(lib, "Setup  : %d - %d%%", setup, setup * 100 / lib->profile.frame);
		TextOut(lib, "  Trans : %d - %d%%", lib->profile.trans, lib->profile.trans * 100 / lib->profile.frame);
		TextOut(lib, "  TexGen: %d - %d%%", lib->profile.texgen, lib->profile.texgen * 100 / lib->profile.frame);
		TextOut(lib, "  Light : %d - %d%%", lib->profile.light, lib->profile.light * 100 / lib->profile.frame);
		TextOut(lib, "Draw   : %d - %d%% (Setup+Spans)", lib->profile.draw, lib->profile.draw * 100 / lib->profile.frame);
		if(lib->profile.prims)
		{
			TextOut(lib, "Prims  : %d", lib->profile.prims);
			TextOut(lib, "per prim: setup %d, spans %d", setup / lib->profile.prims, spans / lib->profile.prims);
		}
		TextOut(lib, "ClipOut: %d", lib->profile.clipOut);
		TextOut(lib, "ClipIn : %d", lib->profile.clipIn);
		TextOut(lib, "ClipPar: %d", lib->profile.clipPartial);
		// Per primitive, not per batch: Guarded is what the guard band saved, so Guarded / (ClipPrm + Guarded) is the fraction of straddling primitives that stayed on the fast path.
		TextOut(lib, "ClipPrm: %d", lib->profile.clipPrims);
		TextOut(lib, "Guarded: %d", lib->profile.guardPrims);

		// Maggie's own counters, in memory fetches. Per started pixel is the figure to compare between projects: it normalises out how much geometry a frame had,
		// leaving the cost of feeding one pixel. Texture is the one the layout moves - a wide, short cache footprint gets no reuse from the previous scanline when
		// the texture-space step runs down v, so this number rises with how diagonal the geometry is. Depth and screen should stay flat by comparison.
		ULONG magPixels = lib->profile.magDelta.pixels;
		if(magPixels)
		{
			TextOut(lib, "MagPix : %d", magPixels);
			TextOut(lib, "MagTex : %d - %f/pix", lib->profile.magDelta.texReads, (double)lib->profile.magDelta.texReads / (double)magPixels);
			TextOut(lib, "MagZ   : %d - %f/pix", lib->profile.magDelta.depthReads, (double)lib->profile.magDelta.depthReads / (double)magPixels);
			TextOut(lib, "MagDst : %d - %f/pix", lib->profile.magDelta.screenReads, (double)lib->profile.magDelta.screenReads / (double)magPixels);
		}
		else if(lib->profile.magDelta.texReads | lib->profile.magDelta.depthReads | lib->profile.magDelta.screenReads)
		{
			// Fetches but no pixel count: the pixel counter is dead, so nothing can be normalised. Show the raw fetches rather than silently dividing by zero.
			TextOut(lib, "MagTex : %d (no pixel count)", lib->profile.magDelta.texReads);
			TextOut(lib, "MagZ   : %d", lib->profile.magDelta.depthReads);
			TextOut(lib, "MagDst : %d", lib->profile.magDelta.screenReads);
		}
		else
		{
			TextOut(lib, "MagCnt : idle - $dff300 not wired?");
		}
#if PROFILE_EDGES
		// Only meaningful in isolation: the timers that produce these also inflate Setup above, since DrawEdge nearly doubles in size.
		TextOut(lib, "Lines  : %d - %d%%", lib->profile.lines, lib->profile.lines * 100 / lib->profile.frame);
		if(lib->profile.nLinePixels)
			TextOut(lib, "Line time per pixel %d (%d)", lib->profile.lines / lib->profile.nLinePixels, lib->profile.nLinePixels);
#endif
	}
#endif
	ReleaseSemaphore(&lib->lock);
}

/*****************************************************************************/
/*****************************************************************************/

UWORD GetUserVertexBuffer(MaggieBase *lib)
{
	if(lib->upVertexBuffer == 0xffff)
	{
		lib->upVertexBuffer = magAllocateVertexBuffer(65535, lib);
	}
	return lib->upVertexBuffer;
}

/*****************************************************************************/

UWORD GetUserIndexBuffer(MaggieBase *lib)
{
	if(lib->upIndexBuffer == 0xffff)
	{
		lib->upIndexBuffer = magAllocateVertexBuffer(65535, lib);
	}
	return lib->upVertexBuffer;
}

/*****************************************************************************/

// Immediate mode, or "slow mode"..
void magBegin(REG(a6, MaggieBase *lib))
{
	lib->nIModeVtx = 0;
	if(lib->immModeVtx == 0xffff)
	{
		lib->immModeVtx = magAllocateVertexBuffer(IMM_MODE_MAGGIE_VERTS, lib);
	}
}

/*****************************************************************************/

void magEnd(REG(a6, MaggieBase *lib))
{
	if(lib->nIModeVtx)
	{
		FlushImmediateMode(lib);
	}
}

/*****************************************************************************/

void magVertex(REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(a6, MaggieBase *lib))
{
	if(lib->nIModeVtx >= IMM_MODE_MAGGIE_VERTS)
	{
		FlushImmediateMode(lib);
	}
	if(lib->immModeVtx == 0xffff)
		return;
	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->immModeVtx];
	vbMem->positions[lib->nIModeVtx].x = x;
	vbMem->positions[lib->nIModeVtx].y = y;
	vbMem->positions[lib->nIModeVtx].z = z;
	vbMem->normals[lib->nIModeVtx] = CompressNormal(&lib->ImmVtx.normal);
	vbMem->uvs[lib->nIModeVtx].u = lib->ImmVtx.tex[0].u * 256.0f * 65536.0f;
	vbMem->uvs[lib->nIModeVtx].v = lib->ImmVtx.tex[0].v * 256.0f * 65536.0f;
	vbMem->colours[lib->nIModeVtx] = RGBToGrayScale(lib->ImmVtx.colour);
	lib->nIModeVtx++;
}

/*****************************************************************************/

void magNormal(REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(a6, MaggieBase *lib))
{
	lib->ImmVtx.normal.x = x;
	lib->ImmVtx.normal.y = y;
	lib->ImmVtx.normal.z = z;
}

/*****************************************************************************/

void magTexCoord(REG(d0, UWORD texReg), REG(fp0, float u), REG(fp1, float v), REG(a6, MaggieBase *lib))
{
	lib->ImmVtx.tex[texReg].u = u;
	lib->ImmVtx.tex[texReg].v = v;
	lib->ImmVtx.tex[texReg].w = 1.0f;
}

/*****************************************************************************/

void magTexCoord3(REG(d0, UWORD texReg), REG(fp0, float u), REG(fp1, float v), REG(fp2, float w), REG(a6, MaggieBase *lib))
{
	lib->ImmVtx.tex[texReg].u = u;
	lib->ImmVtx.tex[texReg].v = v;
	lib->ImmVtx.tex[texReg].w = w;
}

/*****************************************************************************/

void magColour(REG(d0, ULONG col), REG(a6, MaggieBase *lib))
{
	lib->ImmVtx.colour = col;
}

/*****************************************************************************/

void magClear(REG(d0, UWORD buffers), REG(a6, MaggieBase *lib))
{
#if PROFILE
	ULONG startTime = GetClocks();
#endif

	struct ExecBase *SysBase = lib->sysBase;
	if(buffers & MAG_CLEAR_COLOUR)
	{
		if(lib->drawMode & MAG_DRAWMODE_32BIT)
		{
			int size = (lib->xres * lib->yres * 4) & ~15;
			magFastClear(lib->screen, size, lib->clearColour);
		}
		else
		{
			int size = (lib->xres * lib->yres * 2) & ~15;
			ULONG colour = ColourTo16Bit(lib->clearColour);
			colour |= colour << 16;
			magFastClear(lib->screen, size, colour);
		}
	}
	if(buffers & MAG_CLEAR_DEPTH)
	{
		if(lib->depthBuffer)
		{
			int size = lib->xres * lib->yres * 2;
			size = (size + 15) & ~15;
			magFastClear(lib->depthBuffer, size, lib->clearDepth | (lib->clearDepth << 16));
		}
	}
#if PROFILE
	lib->profile.clear += GetClocks() - startTime;
#endif
}

/*****************************************************************************/

void magClearColour(REG(d0, ULONG colour), REG(a6, MaggieBase *lib))
{
	lib->clearColour = colour;
}

/*****************************************************************************/

void magClearDepth(REG(d0, UWORD depth), REG(a6, MaggieBase *lib))
{
	lib->clearDepth = depth;
}

/*****************************************************************************/
