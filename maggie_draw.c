#include "maggie_internal.h"
#include "maggie_debug.h"
#include <proto/graphics.h>
#include <float.h>

/*****************************************************************************/
void DrawPolygon1Pass(const magGradients *gradients, struct MaggieTransVertex *vtx, int nVerts, MaggieBase *lib);
float getBestDistance3(magGradients *res, const struct MaggieTransVertex *vtx);
float getBestDistance(magGradients *res, const struct MaggieTransVertex *vtx, int nVerts);

// The asm reaches into MaggieBase at hardcoded offsets (MB_* in raster/raster_structs.i), so a layout change must fail the build here rather than corrupt a field at runtime.
_Static_assert(__builtin_offsetof(MaggieBase, gradients) == 35032, "MB_gradients in raster/raster_structs.i is stale");
_Static_assert(__builtin_offsetof(MaggieBase, scissor.y0) == 460, "MB_scissorY0 in raster/raster_structs.i is stale");
_Static_assert(__builtin_offsetof(MaggieBase, scissor.y1) == 468, "MB_scissorY1 in raster/raster_structs.i is stale");
_Static_assert(__builtin_offsetof(MaggieBase, cullSign) == 158314, "MB_cullSign in raster/raster_structs.i is stale");
_Static_assert(__builtin_offsetof(MaggieBase, primMinY) == 158318, "MB_primMinY in raster/raster_structs.i is stale");
_Static_assert(__builtin_offsetof(MaggieBase, primMaxY) == 158322, "MB_primMaxY in raster/raster_structs.i is stale");

// The edge table's layout is equally hardcoded: STRUCTURE EPos in raster/raster_structs.i, written by the edge walkers and read by the span renderers.
_Static_assert(sizeof(magEdgePos) == 32, "EPos_Size in raster/raster_structs.i is stale");
_Static_assert(__builtin_offsetof(magEdgePos, xPosLeft) == 0, "EPos_xPosLeft is stale");
_Static_assert(__builtin_offsetof(magEdgePos, xPosRight) == 4, "EPos_xPosRight is stale");
_Static_assert(__builtin_offsetof(magEdgePos, iLeft) == 8, "EPos_iLeft is stale");
_Static_assert(__builtin_offsetof(magEdgePos, uLeft) == 12, "EPos_uLeft is stale");
_Static_assert(__builtin_offsetof(magEdgePos, vLeft) == 16, "EPos_vLeft is stale");
_Static_assert(__builtin_offsetof(magEdgePos, zLeft) == 20, "EPos_zLeft is stale");
_Static_assert(__builtin_offsetof(magEdgePos, oowLeft) == 24, "EPos_oowLeft is stale");
_Static_assert(__builtin_offsetof(magEdgePos, iRight) == 28, "EPos_iRight is stale");
/*****************************************************************************/

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
	if((mode & MAG_DRAWMODE_BLEND_MASK) == MAG_DRAWMODE_BLEND_ADD)
	{
		drawMode |= 0x0040;
	}
	if((mode & MAG_DRAWMODE_BLEND_MASK) == MAG_DRAWMODE_BLEND_MUL)
	{
		drawMode |= 0x0080;
	}
	if(mode & MAG_DRAWMODE_DEPTH_MASK)
	{
		drawMode |= 0x0008;
	}
	UWORD modulo = 4;
	if(!(mode & MAG_DRAWMODE_32BIT))
	{
		drawMode |= 0x0004;
		modulo = 2;
	}
	maggieRegs.mode = drawMode;
	maggieRegs.modulo = modulo;

	// Zero the second-order deltas once per batch: registers hold their value, and the library has otherwise never set them. Left uninitialised they add a stale
	// per-pixel term to u and v, which would move the deltas the chipset actually uses away from the ones the raster wrote.
	maggieRegs.uDeltaDelta = 0;
	maggieRegs.vDeltaDelta = 0;
	maggieRegs.lightRGBA = lib->colour;

	APTR txtrData = GetTextureData(lib->textures[lib->txtrIndex]);
	UWORD txtrSize = GetTexSizeIndex(lib->textures[lib->txtrIndex]);

	maggieRegs.texture = txtrData;

	if((txtrSize != 5) && (mode & MAG_DRAWMODE_MIPMAP))
	{
		maggieRegs.texSize = txtrSize | 0x0010;
	}
	else
	{
		maggieRegs.texSize = txtrSize;
	}
}

/*****************************************************************************/

// A batch with no Maggie or no bound texture rasterizes nothing, so reject it before transforming/lighting/clipping anything.

static int CanDraw(MaggieBase *lib)
{
	if((!lib->hasMaggie) || (lib->txtrIndex == 0xffff))
		return 0;
	return lib->textures[lib->txtrIndex] != NULL;
}

/*****************************************************************************/

// Hoists everything that depends only on drawMode / the bound texture out of the per-primitive and per-edge path. Called once per magDraw* call.

static void BeginDrawBatch(MaggieBase *lib)
{
	SetupHW(lib);

	UWORD mode = lib->drawMode;

	// Pick the pair of edge walkers this batch needs: the left one drops oow when the mapping is affine and z when there is no depth buffer, and the right one carries iRight only for a polygon source, the only case where the …Poly span renderers are reachable.
	magDrawLineFunc left;
	magDrawLineFunc right;

	if(mode & MAG_DRAWMODE_AFFINE_MAPPING)
	{
		left = (mode & MAG_DRAWMODE_DEPTHBUFFER) ? DrawEdgeLeftZ : DrawEdgeLeft;
	}
	else
	{
		left = (mode & MAG_DRAWMODE_DEPTHBUFFER) ? DrawEdgeLeftWZ : DrawEdgeLeftW;
	}

	right = lib->sourceIsPoly ? DrawEdgeRightI : DrawEdgeRight;

	if(mode & MAG_DRAWMODE_CULL_CCW)
	{
		lib->edgeFuncDown = right;
		lib->edgeFuncUp = left;
		lib->cullSign = -1.0f;
	}
	else
	{
		lib->edgeFuncDown = left;
		lib->edgeFuncUp = right;
		lib->cullSign = 1.0f;
	}

	SelectScanFunctions(lib);
}

/*****************************************************************************/

#define CLIPPED_OUT		0
#define CLIPPED_IN		1
#define CLIPPED_PARTIAL 2

/*****************************************************************************/
/*****************************************************************************/

// The per-polygon gradient maths lives in asm: maggie_setuptri.s for triangles, maggie_setuppoly.s for n-gons, which also carry the sign and precision notes.

/*****************************************************************************/

static UBYTE ClipCode(const vec4 *v)
{
	UBYTE code = 0;

	if(-v->w >= v->x) code |= 0x01;
	if( v->w <= v->x) code |= 0x02;
	if(-v->w >= v->y) code |= 0x04;
	if( v->w <= v->y) code |= 0x08;
	if( 0.0f >= v->z) code |= 0x10;
	if( v->w <= v->z) code |= 0x20;

	return code;
}

/*****************************************************************************/

// The two z bits of a clip code. Neither may be relaxed: NormaliseClippedVertexBuffer divides by w, which the near plane is what keeps positive, and a z past the far plane
// overflows the span renderers' fixed-point depth conversion (fsub.s #$4f000000 then fmove.l) rather than saturating.
#define CLIP_NEARFAR	0x30

// Guard band, in units of w. A primitive that leaves the frustum only sideways needs no geometric clip: the span renderers clamp each scanline's x to the scissor and
// pre-step the attributes from the clamped start, and the setup clamps the y span while DrawEdge walks only the rows that survive it. The band exists to bound how far out
// that may go - xPosLeft reaches the raster through fmove.l, and MaggieSetupTri's denom loses relative precision as the coordinates grow, worst on slivers.
#define GUARD_BAND	2.0f

// Valid only once CLIP_NEARFAR is clear for the whole primitive, which is what makes w positive. A w that is somehow still negative fails both compares and takes the clipper.
static int InsideGuardBand(const vec4 *v)
{
	float guard = GUARD_BAND * v->w;

	return (v->x >= -guard) && (v->x <= guard) && (v->y >= -guard) && (v->y <= guard);
}

/*****************************************************************************/

// Replaces "any clip code set -> clip it" with "any clip code the guard band cannot absorb -> clip it". Called only for primitives that already have a code set, so the
// extra compares stay off the path everything wholly inside takes.

static int NeedsClipping(const struct MaggieTransVertex *vtx, int nVerts, int anyCode, MaggieBase *lib)
{
	int needed = 1;

	if(!(anyCode & CLIP_NEARFAR))
	{
		needed = 0;
		for(int i = 0; i < nVerts; ++i)
		{
			if(!InsideGuardBand(&vtx[i].pos))
			{
				needed = 1;
				break;
			}
		}
	}
#if PROFILE
	if(needed)
		lib->profile.clipPrims++;
	else
		lib->profile.guardPrims++;
#endif
	return needed;
}

/*****************************************************************************/

static int ComputeClipCodes(UBYTE *clipCodes, struct MaggieTransVertex *vtx, UWORD nVerts)
{
	UBYTE out = ~0;
	UBYTE in = 0;
	for(int i = 0; i < nVerts; ++i)
	{
		clipCodes[i] = ClipCode(&vtx[i].pos);
		out &= clipCodes[i];
		in |= clipCodes[i];
	}
	if(out)
	{
		return CLIPPED_OUT;
	}
	if(!in)
	{
		return CLIPPED_IN;
	}
	return CLIPPED_PARTIAL;
}

/*****************************************************************************/

static void NormaliseVertexBuffer(struct MaggieTransVertex *vtx, int nVerts, UBYTE *clipCodes, MaggieBase *lib)
{
	float offsetScaleX = (lib->xres + 0.5f) * 0.5f;
	float offsetScaleY = (lib->yres + 0.5f) * 0.5f;

	if(lib->drawMode & MAG_DRAWMODE_AFFINE_MAPPING)
	{
		for(int i = 0; i < nVerts; ++i)
		{
			if(clipCodes[i])
				continue;

			float oow = 1.0f / vtx[i].pos.w;

			vtx[i].pos.x = offsetScaleX * (vtx[i].pos.x * oow + 1.0f);
			vtx[i].pos.y = offsetScaleY * (vtx[i].pos.y * oow + 1.0f);
			vtx[i].pos.z = vtx[i].pos.z * oow * 4294901760.0f;	// 65536.0f * 65535.0f
			vtx[i].pos.w = oow;
		}
	}
	else
	{
		for(int i = 0; i < nVerts; ++i)
		{
			if(clipCodes[i])
				continue;

			float oow = 1.0f / vtx[i].pos.w;

			vtx[i].pos.x = offsetScaleX * (vtx[i].pos.x * oow + 1.0f);
			vtx[i].pos.y = offsetScaleY * (vtx[i].pos.y * oow + 1.0f);
			vtx[i].pos.z = vtx[i].pos.z * oow * 4294901760.0f;	// 65536.0f * 65535.0f
			vtx[i].pos.w = oow;
			for(int j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
			{
				vtx[i].tex[j].u = vtx[i].tex[j].u * oow;
				vtx[i].tex[j].v = vtx[i].tex[j].v * oow;
			}
		}
	}
}

/*****************************************************************************/

static void NormaliseClippedVertexBuffer(struct MaggieTransVertex *vtx, int nVerts, MaggieBase *lib)
{
	float offsetScaleX = (lib->xres + 0.5f) * 0.5f;
	float offsetScaleY = (lib->yres + 0.5f) * 0.5f;

	if(lib->drawMode & MAG_DRAWMODE_AFFINE_MAPPING)
	{
		for(int i = 0; i < nVerts; ++i)
		{
			float oow = 1.0f / vtx[i].pos.w;

			vtx[i].pos.x = offsetScaleX * (vtx[i].pos.x * oow + 1.0f);
			vtx[i].pos.y = offsetScaleY * (vtx[i].pos.y * oow + 1.0f);
			vtx[i].pos.z = vtx[i].pos.z * oow * 4294901760.0f;	// 65536.0f * 65535.0f
			vtx[i].pos.w = oow;
		}
	}
	else
	{
		for(int i = 0; i < nVerts; ++i)
		{
			float oow = 1.0f / vtx[i].pos.w;

			vtx[i].pos.x = offsetScaleX * (vtx[i].pos.x * oow + 1.0f);
			vtx[i].pos.y = offsetScaleY * (vtx[i].pos.y * oow + 1.0f);
			vtx[i].pos.z = vtx[i].pos.z * oow * 4294901760.0f;	// 65536.0f * 65535.0f
			vtx[i].pos.w = oow;
			for(int j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
			{
				vtx[i].tex[j].u = vtx[i].tex[j].u * oow;
				vtx[i].tex[j].v = vtx[i].tex[j].v * oow;
			}
		}
	}
}

/*****************************************************************************/

// Cull, screen-y span and the five attribute gradients all live in MaggieSetupTri (maggie_setuptri.s). 3 verts are always planar in intensity, so scanFunc stays scanFuncFlat.

static void DrawTriangle(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, struct MaggieTransVertex *vtx2, MaggieBase *lib)
{
	if(!MaggieSetupTri(vtx0, vtx1, vtx2, lib))
		return;

	int miny = lib->primMinY;

	DrawEdge(vtx0, vtx1, miny, lib);
	DrawEdge(vtx1, vtx2, miny, lib);
	DrawEdge(vtx2, vtx0, miny, lib);
	DrawSpans(miny, lib->primMaxY, lib);
}
/*****************************************************************************/

// Cull, screen-y span and the five attribute gradients all live in MaggieSetupPoly (maggie_setuppoly.s).

static void DrawPolygon(struct MaggieTransVertex *vtx, int nVerts, MaggieBase *lib)
{
	if(nVerts < 3)
		return;

	if(!MaggieSetupPoly(vtx, NULL, nVerts, lib))
		return;

	lib->scanFunc = (nVerts > 3) ? lib->scanFuncPoly : lib->scanFuncFlat;

	int miny = lib->primMinY;
	int prev = nVerts - 1;
	for(int i = 0; i < nVerts; ++i)
	{
		DrawEdge(&vtx[prev], &vtx[i], miny, lib);
		prev = i;
	}
	DrawSpans(miny, lib->primMaxY, lib);
}

/*****************************************************************************/

static void DrawIndexedPolygon(struct MaggieTransVertex *vtx, UWORD *indx, int nIndx, MaggieBase *lib)
{
	if(nIndx < 3)
		return;

	if(!MaggieSetupPoly(vtx, indx, nIndx, lib))
		return;

	lib->scanFunc = (nIndx > 3) ? lib->scanFuncPoly : lib->scanFuncFlat;

	int miny = lib->primMinY;
	int prev = nIndx - 1;
	for(int i = 0; i < nIndx; ++i)
	{
		DrawEdge(&vtx[indx[prev]], &vtx[indx[i]], miny, lib);
		prev = i;
	}
	DrawSpans(miny, lib->primMaxY, lib);
}

/*****************************************************************************/

static struct MaggieVertex vtxBufferUP[65536];
static struct MaggieSpriteVertex spriteBufferUP[65536];
static struct MaggieTransVertex transVtxBufferUP[65536];
static UBYTE transClipCodesUP[65536];
static struct MaggieTransVertex clippedPoly[MAG_MAX_POLYSIZE + 8];

/*****************************************************************************/

void magDrawTrianglesUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	UWORD vBuffer = GetUserVertexBuffer(lib);

	magUploadVertexBuffer(vBuffer, vtx, 0, nVerts, lib);

	UWORD oldVBuffer = lib->vBuffer;

	magSetVertexBuffer(vBuffer, lib);

	magDrawTriangles(0, nVerts, lib);

	magSetVertexBuffer(oldVBuffer, lib);
}

/*****************************************************************************/

void magDrawIndexedTrianglesUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a1, UWORD *indx), REG(d1, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	UWORD vBuffer = GetUserVertexBuffer(lib);
	UWORD iBuffer = GetUserIndexBuffer(lib);

	magUploadVertexBuffer(vBuffer, vtx, 0, nVerts, lib);
	magUploadIndexBuffer(iBuffer, indx, 0, nIndx, lib);

	UWORD oldVBuffer = lib->vBuffer;
	UWORD oldIBuffer = lib->iBuffer;

	magSetVertexBuffer(vBuffer, lib);
	magSetIndexBuffer(iBuffer, lib);

	magDrawIndexedTriangles(0, nVerts, 0, nIndx, lib);

	magSetVertexBuffer(oldVBuffer, lib);
	magSetIndexBuffer(oldIBuffer, lib);
}

/*****************************************************************************/

void magDrawIndexedPolygonsUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a1, UWORD *indx), REG(d1, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	UWORD vBuffer = GetUserVertexBuffer(lib);
	UWORD iBuffer = GetUserIndexBuffer(lib);

	magUploadVertexBuffer(vBuffer, vtx, 0, nVerts, lib);
	magUploadIndexBuffer(iBuffer, indx, 0, nIndx, lib);

	UWORD oldVBuffer = lib->vBuffer;
	UWORD oldIBuffer = lib->iBuffer;

	magSetVertexBuffer(vBuffer, lib);
	magSetIndexBuffer(iBuffer, lib);

	magDrawIndexedPolygons(0, nVerts, 0, nIndx, lib);

	magSetVertexBuffer(oldVBuffer, lib);
	magSetIndexBuffer(oldIBuffer, lib);
}

/*****************************************************************************/

void FlushImmediateMode(MaggieBase *lib)
{
	if(lib->immModeVtx == 0xffff)
		return;
	if(lib->nIModeVtx >= 3)
	{
		UWORD oldVBuffer = lib->vBuffer;
		lib->vBuffer = lib->immModeVtx;
		VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];

		for(int i = 0; i < lib->nIModeVtx; ++i)
		{
			vbMem->transVerts[i].colour = (vbMem->colours[i] << 8) | vbMem->colours[i];
		}

		magDrawTriangles(0, lib->nIModeVtx, lib);

		lib->vBuffer = oldVBuffer;
	}
	lib->nIModeVtx = 0;
}

/*****************************************************************************/

void magDrawTriangles(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(a6, MaggieBase *lib))
{
#if PROFILE
	ULONG drawStart = GetClocks();
#endif
	if(!CanDraw(lib))
		return;

	lib->sourceIsPoly = 0;
	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];

	TransformVertexPositions(vbMem->transVerts, vbMem->positions + startVtx, nVerts, lib);

	int clipRes = ComputeClipCodes(vbMem->clipCodes, vbMem->transVerts, nVerts);
#if PROFILE
	if(clipRes == CLIPPED_OUT)
		lib->profile.clipOut++;
	else if(clipRes == CLIPPED_IN)
		lib->profile.clipIn++;
	else if(clipRes == CLIPPED_PARTIAL)
		lib->profile.clipPartial++;
#endif
	if(clipRes == CLIPPED_OUT)
		return;

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
	{
		LightBuffer(vbMem, startVtx, nVerts, lib);
	}

	TexGenBuffer(vbMem, startVtx, nVerts, lib);

	BeginDrawBatch(lib);

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(vbMem->transVerts, nVerts, lib);

		for(int i = 0; i < nVerts; i += 3)
		{
			DrawTriangle(&vbMem->transVerts[i + 0], &vbMem->transVerts[i + 1], &vbMem->transVerts[i + 2], lib);
		}
	}
	else if(clipRes == CLIPPED_PARTIAL)
	{
		for(int i = 0; i < nVerts; i += 3)
		{
			if(vbMem->clipCodes[i + 0] & vbMem->clipCodes[i + 1] & vbMem->clipCodes[i + 2])
				continue;

			int anyCode = vbMem->clipCodes[i + 0] | vbMem->clipCodes[i + 1] | vbMem->clipCodes[i + 2];

			if(anyCode && NeedsClipping(&vbMem->transVerts[i], 3, anyCode, lib))
			{
				clippedPoly[0] = vbMem->transVerts[i + 0];
				clippedPoly[1] = vbMem->transVerts[i + 1];
				clippedPoly[2] = vbMem->transVerts[i + 2];
				int nClippedVerts = ClipPolygon(clippedPoly, 3);
				if(nClippedVerts > 2)
				{
					NormaliseClippedVertexBuffer(clippedPoly, nClippedVerts, lib);
					DrawPolygon(clippedPoly, nClippedVerts, lib);
				}
			}
			else
			{
				// Also the guard band's path: these are non-indexed verts, so normalising in place cannot be seen by another triangle.
				NormaliseClippedVertexBuffer(&vbMem->transVerts[i], 3, lib);
				DrawTriangle(&vbMem->transVerts[i + 0], &vbMem->transVerts[i + 1], &vbMem->transVerts[i + 2], lib);
			}
		}
	}
#if PROFILE
	lib->profile.draw += GetClocks() - drawStart;
#endif
}

/*****************************************************************************/
static struct MaggieTransVertex pass1vtx[1024];

/*****************************************************************************/

static void DrawSpan(float leftPos, float rightPos, float leftWW, float leftUU, float leftVV, float leftZZ, float leftII, const magGradients *gradients, MaggieBase *lib)
{

}

void magDrawIndexedTriangles(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(d2, UWORD startIndx), REG(d3, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	if(!CanDraw(lib))
		return;

	lib->sourceIsPoly = 0;
#if PROFILE
	ULONG drawStart = GetClocks();
#endif
	UWORD *indexBuffer = GetIBIndices(lib->indexBuffers[lib->iBuffer]) + startIndx;
	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];
	TransformVertexPositions(&vbMem->transVerts[startVtx], &vbMem->positions[startVtx], nVerts, lib);

	int clipRes = ComputeClipCodes(&vbMem->clipCodes[startVtx], &vbMem->transVerts[startVtx], nVerts);
#if PROFILE
	if(clipRes == CLIPPED_OUT)
		lib->profile.clipOut++;
	else if(clipRes == CLIPPED_IN)
		lib->profile.clipIn++;
	else if(clipRes == CLIPPED_PARTIAL)
		lib->profile.clipPartial++;
#endif

	if(clipRes == CLIPPED_OUT)
	{
#if PROFILE
		lib->profile.draw += GetClocks() - drawStart;
#endif
		return;
	}

	// Lighting and texgen run after the clip test so a fully off-screen batch pays for neither.
	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
	{
		LightBuffer(vbMem, startVtx, nVerts, lib);
	}

	TexGenBuffer(vbMem, startVtx, nVerts, lib);

	BeginDrawBatch(lib);

	// Hoisted: DrawTriangle takes lib, so the compiler assumes the call may have rewritten vbMem->transVerts and reloads it every iteration.
	struct MaggieTransVertex *transVerts = vbMem->transVerts;

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(&transVerts[startVtx], nVerts, lib);
		for(int i = 0; i < nIndx; i += 3)
		{
			int i0 = indexBuffer[i + 0];
			int i1 = indexBuffer[i + 1];
			int i2 = indexBuffer[i + 2];
#if 1
# if PROFILE
//			ULONG distStart = GetClocks();
# endif
			DrawTriangle(&transVerts[i0], &transVerts[i1], &transVerts[i2], lib);
# if PROFILE
//			lib->profile.draw += GetClocks() - distStart;
# endif
#else
			for(int j = 0; j < 3; ++j)
			{
				pass1vtx[j] = transVtx[indexBuffer[i + j]];
			}
# if PROFILE
			ULONG distStart = GetClocks();
# endif
			magGradients gradients;
			if(getBestDistance3(&gradients, pass1vtx) > 0.0f)
			{
				DrawPolygon1Pass(&gradients, pass1vtx, 3, lib);
			}
# if PROFILE
			lib->profile.draw += GetClocks() - distStart;
# endif
#endif
		}
	}
	if(clipRes == CLIPPED_PARTIAL)
	{
		for(int i = 0; i < nIndx; i += 3)
		{
			int i0 = indexBuffer[i + 0];
			int i1 = indexBuffer[i + 1];
			int i2 = indexBuffer[i + 2];

			if(!(vbMem->clipCodes[i0] & vbMem->clipCodes[i1] & vbMem->clipCodes[i2]))
			{
				clippedPoly[0] = transVerts[i0];
				clippedPoly[1] = transVerts[i1];
				clippedPoly[2] = transVerts[i2];
				int anyCode = vbMem->clipCodes[i0] | vbMem->clipCodes[i1] | vbMem->clipCodes[i2];
				if(anyCode && NeedsClipping(clippedPoly, 3, anyCode, lib))
				{
					int nClippedVerts = ClipPolygon(clippedPoly, 3);
					if(nClippedVerts > 2)
					{
						NormaliseClippedVertexBuffer(clippedPoly, nClippedVerts, lib);
						DrawPolygon(clippedPoly, nClippedVerts, lib);
					}
				}
				else
				{
					NormaliseClippedVertexBuffer(clippedPoly, 3, lib);
					DrawTriangle(&clippedPoly[0], &clippedPoly[1], &clippedPoly[2], lib);
				}
			}
		}
	}
#if PROFILE
	lib->profile.draw += GetClocks() - drawStart;
#endif
}

/*****************************************************************************/

void magDrawIndexedPolygons(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(d2, UWORD startIndx), REG(d3, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	if(!CanDraw(lib))
		return;

	lib->sourceIsPoly = 1;
#if PROFILE
	ULONG drawStart = GetClocks();
#endif
	UWORD *indexBuffer = GetIBIndices(lib->indexBuffers[lib->iBuffer]) + startIndx;

	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];

	struct MaggieTransVertex *transVtx = vbMem->transVerts;
	UBYTE *clipCodes = vbMem->clipCodes;

	TransformVertexPositions(&vbMem->transVerts[startVtx], &vbMem->positions[startVtx], nVerts, lib);

	int clipRes = ComputeClipCodes(&clipCodes[startVtx], &vbMem->transVerts[startVtx], nVerts);
#if PROFILE
	if(clipRes == CLIPPED_OUT)
		lib->profile.clipOut++;
	else if(clipRes == CLIPPED_IN)
		lib->profile.clipIn++;
	else if(clipRes == CLIPPED_PARTIAL)
		lib->profile.clipPartial++;
#endif

	if(clipRes == CLIPPED_OUT)
	{
#if PROFILE
		lib->profile.draw += GetClocks() - drawStart;
#endif
		return;
	}

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
	{
		LightBuffer(vbMem, startVtx, nVerts, lib);
	}

	TexGenBuffer(vbMem, startVtx, nVerts, lib);

	BeginDrawBatch(lib);

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(&transVtx[startVtx], nVerts, lib);
		int indxPos = 0;
		while(indxPos < nIndx)
		{
			int nPolyVerts = 0;
			for(int i = indxPos; i < nIndx; ++i)
			{
				if(indexBuffer[i] == 0xffff)
					break;

				nPolyVerts++;
			}
			if(nPolyVerts >= 3)
			{
				DrawIndexedPolygon(transVtx, &indexBuffer[indxPos], nPolyVerts, lib);
			}
			indxPos += nPolyVerts + 1;
		}
	}
	if(clipRes == CLIPPED_PARTIAL)
	{
		int indxPos = 0;

		while(indxPos < nIndx)
		{
			int clippedAll = ~0;
			int clippedAny = 0;
			int nPolyVerts = 0;
			for(int i = indxPos; i < nIndx; ++i)
			{
				if(indexBuffer[i] == 0xffff)
				{
					break;
				}
				nPolyVerts++;
				clippedAll &= clipCodes[indexBuffer[i]];
				clippedAny |= clipCodes[indexBuffer[i]];
			}
			if(clippedAll || (nPolyVerts < 3))
			{
				indxPos += nPolyVerts + 1;
				continue;
			}
			for(int i = 0; i < nPolyVerts; ++i)
			{
				clippedPoly[i] = transVtx[indexBuffer[i + indxPos]];
			}
			indxPos += nPolyVerts + 1;
			// One tail for both: the guard band's path is the old clippedAny == 0 path, which already knew nPolyVerts >= 3.
			if(clippedAny && NeedsClipping(clippedPoly, nPolyVerts, clippedAny, lib))
			{
				nPolyVerts = ClipPolygon(clippedPoly, nPolyVerts);
			}
			if(nPolyVerts > 2)
			{
				NormaliseClippedVertexBuffer(clippedPoly, nPolyVerts, lib);
				DrawPolygon(clippedPoly, nPolyVerts, lib);
			}
		}
	}
#if PROFILE
	lib->profile.draw += GetClocks() - drawStart;
#endif
}

/*****************************************************************************/

void magDrawLinearSpan(REG(a0, struct SpanPosition *start), REG(a1, struct SpanPosition *end), REG(a6, MaggieBase *lib))
{
	SetupHW(lib);
}

/*****************************************************************************/

void magDrawSpan(REG(a0, struct MaggieClippedVertex *start), REG(a1, struct MaggieClippedVertex *end), REG(a6, MaggieBase *lib))
{
	SetupHW(lib);
}

/*****************************************************************************/

static void ExpandSpriteBuffer(struct MaggieTransVertex *dest, struct MaggieSpriteVertex *srcVtx, int nSprites, float spriteSize, MaggieBase *lib)
{
	for(int i = 0; i < nSprites; ++i)
	{
		vec3 p0 = srcVtx[i].pos; p0.x += spriteSize; p0.y += spriteSize;
		vec3 p1 = srcVtx[i].pos; p1.x += spriteSize; p1.y -= spriteSize;
		vec3 p2 = srcVtx[i].pos; p2.x -= spriteSize; p2.y -= spriteSize;
		vec3 p3 = srcVtx[i].pos; p3.x -= spriteSize; p3.y += spriteSize;
		vec3_tformh(&dest[i * 4 + 0].pos, &lib->perspectiveMatrix, &p0, 1.0f);
		dest[i * 4 + 0].tex[0].u = 0.0f * 256.0f * 65536.0f;
		dest[i * 4 + 0].tex[0].v = 0.0f * 256.0f * 65536.0f;
		dest[i * 4 + 0].tex[0].w = 1.0f;
		dest[i * 4 + 0].colour = srcVtx[i].colour;
		vec3_tformh(&dest[i * 4 + 1].pos, &lib->perspectiveMatrix, &p1, 1.0f);
		dest[i * 4 + 1].tex[0].u = 0.0f * 256.0f * 65536.0f;
		dest[i * 4 + 1].tex[0].v = 1.0f * 256.0f * 65536.0f;
		dest[i * 4 + 1].tex[0].w = 1.0f;
		dest[i * 4 + 1].colour = srcVtx[i].colour;
		vec3_tformh(&dest[i * 4 + 2].pos, &lib->perspectiveMatrix, &p2, 1.0f);
		dest[i * 4 + 2].tex[0].u = 1.0f * 256.0f * 65536.0f;
		dest[i * 4 + 2].tex[0].v = 1.0f * 256.0f * 65536.0f;
		dest[i * 4 + 2].tex[0].w = 1.0f;
		dest[i * 4 + 2].colour = srcVtx[i].colour;
		vec3_tformh(&dest[i * 4 + 3].pos, &lib->perspectiveMatrix, &p3, 1.0f);
		dest[i * 4 + 3].tex[0].u = 1.0f * 256.0f * 65536.0f;
		dest[i * 4 + 3].tex[0].v = 0.0f * 256.0f * 65536.0f;
		dest[i * 4 + 3].tex[0].w = 1.0f;
		dest[i * 4 + 3].colour = srcVtx[i].colour;
	}
}

/*****************************************************************************/

void magDrawSprites(REG(d0, UWORD startVtx), REG(d1, UWORD nSprites), REG(fp0, float spriteSize), REG(a6, MaggieBase *lib))
{
	if(!CanDraw(lib))
		return;

#if PROFILE
	ULONG drawStart = GetClocks();
#endif
	lib->sourceIsPoly = 1;			// sprite quads: intensity may be non-planar -> safe per-scanline path
	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];
	TransformToSpriteBuffer(spriteBufferUP, &vbMem->positions[startVtx], nSprites, lib);
	ExpandSpriteBuffer(transVtxBufferUP, spriteBufferUP, nSprites, spriteSize * 0.5f, lib);
	int clipRes = ComputeClipCodes(transClipCodesUP, transVtxBufferUP, nSprites * 4);
#if PROFILE
	if(clipRes == CLIPPED_OUT)
		lib->profile.clipOut++;
	else if(clipRes == CLIPPED_IN)
		lib->profile.clipIn++;
	else if(clipRes == CLIPPED_PARTIAL)
		lib->profile.clipPartial++;
#endif

	if(clipRes == CLIPPED_OUT)
	{
		return;
	}
	BeginDrawBatch(lib);

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(transVtxBufferUP, nSprites * 4, lib);
		for(int i = 0; i < nSprites; ++i)
		{
			DrawPolygon(&transVtxBufferUP[i * 4], 4, lib);
		}
	}
	else
	{
		for(int i = 0; i < nSprites; ++i)
		{
			clippedPoly[0] = transVtxBufferUP[i * 4 + 0];
			clippedPoly[1] = transVtxBufferUP[i * 4 + 1];
			clippedPoly[2] = transVtxBufferUP[i * 4 + 2];
			clippedPoly[3] = transVtxBufferUP[i * 4 + 3];
			int nClippedVerts = ClipPolygon(clippedPoly, 4);
			if(nClippedVerts > 2)
			{
				NormaliseClippedVertexBuffer(clippedPoly, nClippedVerts, lib);
				DrawPolygon(clippedPoly, nClippedVerts, lib);
			}
		}
	}
#if PROFILE
	lib->profile.draw += GetClocks() - drawStart;
#endif
}

/*****************************************************************************/

void magDrawSpritesUP(REG(a0, struct MaggieSpriteVertex *vtx), REG(d0, UWORD nSprites), REG(fp0, float spriteSize), REG(a6, MaggieBase *lib))
{
	if(!CanDraw(lib))
		return;

#if PROFILE
	ULONG drawStart = GetClocks();
#endif
	lib->sourceIsPoly = 0;			// sprite quads: always flat.
	TransformSpriteBuffer(spriteBufferUP, vtx, nSprites, lib);
	ExpandSpriteBuffer(transVtxBufferUP, spriteBufferUP, nSprites, spriteSize * 0.5f, lib);
	int clipRes = ComputeClipCodes(transClipCodesUP, transVtxBufferUP, nSprites * 4);
#if PROFILE
	if(clipRes == CLIPPED_OUT)
		lib->profile.clipOut++;
	else if(clipRes == CLIPPED_IN)
		lib->profile.clipIn++;
	else if(clipRes == CLIPPED_PARTIAL)
		lib->profile.clipPartial++;
#endif

	if(clipRes == CLIPPED_OUT)
	{
		return;
	}
	BeginDrawBatch(lib);

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(transVtxBufferUP, nSprites * 4, lib);
		for(int i = 0; i < nSprites; ++i)
		{
			DrawPolygon(&transVtxBufferUP[i * 4], 4, lib);
		}
	}
	else
	{
		for(int i = 0; i < nSprites; ++i)
		{
			clippedPoly[0] = transVtxBufferUP[i * 4 + 0];
			clippedPoly[1] = transVtxBufferUP[i * 4 + 1];
			clippedPoly[2] = transVtxBufferUP[i * 4 + 2];
			clippedPoly[3] = transVtxBufferUP[i * 4 + 3];
			int nClippedVerts = ClipPolygon(clippedPoly, 4);
			if(nClippedVerts > 2)
			{
				NormaliseClippedVertexBuffer(clippedPoly, nClippedVerts, lib);
				DrawPolygon(clippedPoly, nClippedVerts, lib);
			}
		}
	}
#if PROFILE
	lib->profile.draw += GetClocks() - drawStart;
#endif
}

/*****************************************************************************/
