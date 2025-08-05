#include "maggie_internal.h"
#include "maggie_debug.h"
#include <proto/graphics.h>
#include <float.h>

/*****************************************************************************/
void DrawPolygon1Pass(const magGradients *gradients, struct MaggieTransVertex *vtx, int nVerts, MaggieBase *lib);
float getBestDistance3(magGradients *res, const struct MaggieTransVertex *vtx);
float getBestDistance(magGradients *res, const struct MaggieTransVertex *vtx, int nVerts);
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

/*****************************************************************************/

#define CLIPPED_OUT		0
#define CLIPPED_IN		1
#define CLIPPED_PARTIAL 2

/*****************************************************************************/
/*****************************************************************************/

static float TriangleArea(vec4 *p0, vec4 *p1, vec4 *p2)
{
	float x0 = p1->x - p0->x;
	float y0 = p1->y - p0->y;
	float x1 = p2->x - p0->x;
	float y1 = p2->y - p0->y;

	return x0 * y1 - x1 * y0;
}

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

int ComputeClipCodes(UBYTE *clipCodes, struct MaggieTransVertex *vtx, UWORD nVerts)
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
		return CLIPPED_OUT;
	if(!in)
		return CLIPPED_IN;
	return CLIPPED_PARTIAL;
}

/*****************************************************************************/

void NormaliseVertexBuffer(struct MaggieTransVertex *vtx, int nVerts, UBYTE *clipCodes, MaggieBase *lib)
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
			vtx[i].pos.z = vtx[i].pos.z * oow * 65536.0f * 65535.0f;
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
			vtx[i].pos.z = vtx[i].pos.z * oow * 65536.0f * 65535.0f;
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

void NormaliseClippedVertexBuffer(struct MaggieTransVertex *vtx, int nVerts, MaggieBase *lib)
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
			vtx[i].pos.z = vtx[i].pos.z * oow * 65536.0f * 65535.0f;
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
			vtx[i].pos.z = vtx[i].pos.z * oow * 65536.0f * 65535.0f;
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

int my_abs(int v)
{
	if(v < 0)
		return -v;
	return v;
}

/*****************************************************************************/

void DrawScreenLine(MaggieBase *lib, int x0, int y0, int x1, int y1)
{
	int dx = my_abs(x1 - x0);
	int dy = my_abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;
	int e2;
	ULONG *screen = (ULONG *)lib->screen;
	for(;;)
	{
		if((x0 > lib->scissor.x0) && (x0 < lib->scissor.x1) && (y0 > lib->scissor.y0) && (y0 < lib->scissor.y1))
			screen[y0 * lib->xres + x0] = 0xffffff;

		if((x0 == x1) && (y0 == y1))
			break;

		e2 = 2 * err;
		if(e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if(e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

/*****************************************************************************/

void DrawTriangle(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, struct MaggieTransVertex *vtx2, MaggieBase *lib)
{
	float area = TriangleArea(&vtx0->pos, &vtx1->pos, &vtx2->pos);

	if(lib->drawMode & MAG_DRAWMODE_CULL_CCW)
		area = -area;
	if(area > 0.0f)
		return;

	if(lib->frameCounter & 1)
	{
		DrawScreenLine(lib, vtx0->pos.x, vtx0->pos.y, vtx1->pos.x, vtx1->pos.y);
		DrawScreenLine(lib, vtx1->pos.x, vtx1->pos.y, vtx2->pos.x, vtx2->pos.y);
		DrawScreenLine(lib, vtx2->pos.x, vtx2->pos.y, vtx0->pos.x, vtx0->pos.y);
		return;
	}

	int miny = vtx0->pos.y;
	if(miny > vtx1->pos.y)
		miny = vtx1->pos.y;
	if(miny > vtx2->pos.y)
		miny = vtx2->pos.y;

	int maxy = vtx0->pos.y;
	if(maxy < vtx1->pos.y)
		maxy = vtx1->pos.y;
	if(maxy < vtx2->pos.y)
		maxy = vtx2->pos.y;

	DrawEdge(vtx0, vtx1, miny, maxy, lib);
	DrawEdge(vtx1, vtx2, miny, maxy, lib);
	DrawEdge(vtx2, vtx0, miny, maxy, lib);
	DrawSpans(miny, maxy, lib);
}

/*****************************************************************************/

void DrawPolygon(struct MaggieTransVertex *vtx, int nVerts, MaggieBase *lib)
{
	if(nVerts < 3)
		return;

	float area = TriangleArea(&vtx[0].pos, &vtx[1].pos, &vtx[2].pos);
	for(int i = 2; i < nVerts - 1; ++i)
	{
		area += TriangleArea(&vtx[0].pos, &vtx[i].pos, &vtx[i + 1].pos);
	}

	if(lib->drawMode & MAG_DRAWMODE_CULL_CCW)
		area = -area;
	if(area >= 0.0f)
		return;

	if(lib->frameCounter & 1)
	{
		for(int i = 0; i < nVerts; ++i)
		{
			DrawScreenLine(lib, vtx[i].pos.x, vtx[i].pos.y, vtx[(i + 1) % nVerts].pos.x, vtx[(i + 1) % nVerts].pos.y);
		}
		return;
	}
	int miny = vtx[0].pos.y;
	int maxy = vtx[0].pos.y;
	for(int i = 1; i < nVerts; ++i)
	{
		if(miny > vtx[i].pos.y)
		{
			miny = vtx[i].pos.y;
		}
		if(maxy < vtx[i].pos.y)
		{
			maxy = vtx[i].pos.y;
		}
	}
	int prev = nVerts - 1;
	for(int i = 0; i < nVerts; ++i)
	{
		DrawEdge(&vtx[prev], &vtx[i], miny, maxy, lib);	
		prev = i;
	}
	DrawSpans(miny, maxy, lib);
}

/*****************************************************************************/

void DrawIndexedPolygon(struct MaggieTransVertex *vtx, UWORD *indx, int nIndx, MaggieBase *lib)
{
	if(nIndx < 3)
		return;

	float area = TriangleArea(&vtx[indx[0]].pos, &vtx[indx[1]].pos, &vtx[indx[2]].pos);
	for(int i = 2; i < nIndx - 1; ++i)
	{
		area += TriangleArea(&vtx[indx[0]].pos, &vtx[indx[i]].pos, &vtx[indx[i + 1]].pos);
	}
	if(lib->drawMode & MAG_DRAWMODE_CULL_CCW)
		area = -area;
	if(area >= 0.0f)
		return;

	if(lib->frameCounter & 1)
	{
		for(int i = 0; i < nIndx; ++i)
		{
			DrawScreenLine(lib, vtx[indx[i]].pos.x, vtx[indx[i]].pos.y, vtx[indx[(i + 1) % nIndx]].pos.x, vtx[indx[(i + 1) % nIndx]].pos.y);
		}
		return;
	}
	int miny = vtx[indx[0]].pos.y;
	int maxy = vtx[indx[0]].pos.y;

	for(int i = 1; i < nIndx; ++i)
	{
		if(miny > vtx[indx[i]].pos.y)
		{
			miny = vtx[indx[i]].pos.y;
		}
		if(maxy < vtx[indx[i]].pos.y)
		{
			maxy = vtx[indx[i]].pos.y;
		}
	}

	if(maxy == miny)
		return;

	int prev = nIndx - 1;
	for(int i = 0; i < nIndx; ++i)
	{
		DrawEdge(&vtx[indx[prev]], &vtx[indx[i]], miny, maxy, lib);
		prev = i;
	}
	DrawSpans(miny, maxy, lib);
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

		magDrawTriangles(0, lib->nIModeVtx, lib);

		lib->vBuffer = oldVBuffer;
	}
	lib->nIModeVtx = 0;
}

/*****************************************************************************/

void magDrawTriangles(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];

	TransformVertexPositions(vbMem->transVerts, vbMem->positions + startVtx, nVerts, lib);

	int clipRes = ComputeClipCodes(vbMem->clipCodes, vbMem->transVerts, nVerts);

	if(clipRes == CLIPPED_OUT)
		return;

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
	{
		LightBuffer(vbMem, startVtx, nVerts, lib);
	}

	TexGenBuffer(vbMem, startVtx, nVerts, lib);

	struct GfxBase *GfxBase = lib->gfxBase;
	OwnBlitter();

	SetupHW(lib);

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
			if(vbMem->clipCodes[i + 0] | vbMem->clipCodes[i + 1] | vbMem->clipCodes[i + 2])
			{
				if(!(vbMem->clipCodes[i + 0] & vbMem->clipCodes[i + 1] & vbMem->clipCodes[i + 2]))
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
			}
			else
			{
				NormaliseClippedVertexBuffer(&vbMem->transVerts[i], 3, lib);
				DrawTriangle(&vbMem->transVerts[i + 0], &vbMem->transVerts[i + 1], &vbMem->transVerts[i + 2], lib);
			}
		}
	}
	DisownBlitter();
}

/*****************************************************************************/
static struct MaggieTransVertex pass1vtx[1024];

/*****************************************************************************/

void DrawSpan(float leftPos, float rightPos, float leftWW, float leftUU, float leftVV, float leftZZ, float leftII, const magGradients *gradients, MaggieBase *lib)
{

}

void magDrawIndexedTriangles(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(d2, UWORD startIndx), REG(d3, UWORD nIndx), REG(a6, MaggieBase *lib))
{
#if PROFILE
	ULONG drawStart = GetClocks();
#endif
	UWORD *indexBuffer = GetIBIndices(lib->indexBuffers[lib->iBuffer]) + startIndx;
	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];
	TransformVertexPositions(&vbMem->transVerts[startVtx], &vbMem->positions[startVtx], nVerts, lib);

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
	{
		LightBuffer(vbMem, startVtx, nVerts, lib);
	}

	TexGenBuffer(vbMem, startVtx, nVerts, lib);

	int clipRes = ComputeClipCodes(&vbMem->clipCodes[startVtx], &vbMem->transVerts[startVtx], nVerts);

	if(clipRes == CLIPPED_OUT)
	{
#if PROFILE
		lib->profile.draw += GetClocks() - drawStart;
#endif
		return;
	}

	struct GfxBase *GfxBase = lib->gfxBase;
	OwnBlitter();

	SetupHW(lib);

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(&vbMem->transVerts[startVtx], nVerts, lib);
		for(int i = 0; i < nIndx; i += 3)
		{
			int i0 = indexBuffer[i + 0];
			int i1 = indexBuffer[i + 1];
			int i2 = indexBuffer[i + 2];
#if 1
# if PROFILE
//			ULONG distStart = GetClocks();
# endif
			DrawTriangle(&vbMem->transVerts[i0], &vbMem->transVerts[i1], &vbMem->transVerts[i2], lib);
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
				clippedPoly[0] = vbMem->transVerts[i0];
				clippedPoly[1] = vbMem->transVerts[i1];
				clippedPoly[2] = vbMem->transVerts[i2];
				if(vbMem->clipCodes[i0] | vbMem->clipCodes[i1] | vbMem->clipCodes[i2])
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
	DisownBlitter();
#if PROFILE
//	lib->profile.draw += GetClocks() - drawStart;
#endif
}

/*****************************************************************************/

void magDrawIndexedPolygons(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(d2, UWORD startIndx), REG(d3, UWORD nIndx), REG(a6, MaggieBase *lib))
{
#if PROFILE
	ULONG drawStart = GetClocks();
#endif
	UWORD *indexBuffer = GetIBIndices(lib->indexBuffers[lib->iBuffer]) + startIndx;

	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];

	struct MaggieTransVertex *transVtx = vbMem->transVerts;
	UBYTE *clipCodes = vbMem->clipCodes;

	TransformVertexPositions(&vbMem->transVerts[startVtx], &vbMem->positions[startVtx], nVerts, lib);

	int clipRes = ComputeClipCodes(&clipCodes[startVtx], &vbMem->transVerts[startVtx], nVerts);

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

	if(lib->txtrIndex != 0xffff)
	{
		TexGenBuffer(vbMem, startVtx, nVerts, lib);
	}

	struct GfxBase *GfxBase = lib->gfxBase;

	OwnBlitter();

	SetupHW(lib);

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(&vbMem->transVerts[startVtx], nVerts, lib);
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
				DrawIndexedPolygon(vbMem->transVerts, &indexBuffer[indxPos], nPolyVerts, lib);
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
				clippedPoly[i] = vbMem->transVerts[indexBuffer[i + indxPos]];
			}
			indxPos += nPolyVerts + 1;
			if(clippedAny)
			{
				nPolyVerts = ClipPolygon(clippedPoly, nPolyVerts);
				if(nPolyVerts > 2)
				{
					NormaliseClippedVertexBuffer(clippedPoly, nPolyVerts, lib);
					DrawPolygon(clippedPoly, nPolyVerts, lib);
				}
			}
			else
			{
				NormaliseClippedVertexBuffer(clippedPoly, nPolyVerts, lib);
				DrawPolygon(clippedPoly, nPolyVerts, lib);
			}
		}
	}
	DisownBlitter();
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

void ExpandSpriteBuffer(struct MaggieTransVertex *dest, struct MaggieSpriteVertex *srcVtx, int nSprites, float spriteSize, MaggieBase *lib)
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
	VertexBufferMemory *vbMem = lib->vertexBuffers[lib->vBuffer];
	TransformToSpriteBuffer(spriteBufferUP, &vbMem->positions[startVtx], nSprites, lib);
	ExpandSpriteBuffer(transVtxBufferUP, spriteBufferUP, nSprites, spriteSize * 0.5f, lib);
	int clipRes = ComputeClipCodes(transClipCodesUP, transVtxBufferUP, nSprites * 4);

	if(clipRes == CLIPPED_OUT)
	{
		return;
	}
	struct GfxBase *GfxBase = lib->gfxBase;

	OwnBlitter();

	SetupHW(lib);

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
	DisownBlitter();
}

/*****************************************************************************/

void magDrawSpritesUP(REG(a0, struct MaggieSpriteVertex *vtx), REG(d0, UWORD nSprites), REG(fp0, float spriteSize), REG(a6, MaggieBase *lib))
{
	TransformSpriteBuffer(spriteBufferUP, vtx, nSprites, lib);
	ExpandSpriteBuffer(transVtxBufferUP, spriteBufferUP, nSprites, spriteSize * 0.5f, lib);
	int clipRes = ComputeClipCodes(transClipCodesUP, transVtxBufferUP, nSprites * 4);

	if(clipRes == CLIPPED_OUT)
	{
		return;
	}
	struct GfxBase *GfxBase = lib->gfxBase;

	OwnBlitter();

	SetupHW(lib);

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
	DisownBlitter();
}

/*****************************************************************************/
