#include "maggie_internal.h"
#include "maggie_debug.h"
#include <proto/graphics.h>
#include <float.h>

/*****************************************************************************/
/*****************************************************************************/

#define CLIPPED_OUT		0
#define CLIPPED_IN		1
#define CLIPPED_PARTIAL 2

/*****************************************************************************/
/*****************************************************************************/

float TriangleArea(vec4 *p0, vec4 *p1, vec4 *p2)
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
	float offsetScaleX = (lib->xres + 0.5) * 0.5f;
	float offsetScaleY = (lib->yres + 0.5) * 0.5f;
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

/*****************************************************************************/

void NormaliseClippedVertexBuffer(struct MaggieTransVertex *vtx, int nVerts, MaggieBase *lib)
{
	float offsetScaleX = (lib->xres + 0.5) * 0.5f;
	float offsetScaleY = (lib->yres + 0.5) * 0.5f;
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

/*****************************************************************************/

void DrawTriangle(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, struct MaggieTransVertex *vtx2, MaggieBase *lib)
{
	float area = TriangleArea(&vtx0->pos, &vtx1->pos, &vtx2->pos);

	if(lib->drawMode & MAG_DRAWMODE_CULL_CCW)
		area = -area;
	if(area > 0.0f)
		return;

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

	DrawEdge(vtx0, vtx1, lib);
	DrawEdge(vtx1, vtx2, lib);
	DrawEdge(vtx2, vtx0, lib);
	DrawSpans(miny, maxy, lib);
}

/*****************************************************************************/

void DrawPolygon(struct MaggieTransVertex *vtx, int nVerts, MaggieBase *lib)
{
	if(nVerts < 3)
		return;
	float area = TriangleArea(&vtx[0].pos, &vtx[1].pos, &vtx[2].pos);

	if(lib->drawMode & MAG_DRAWMODE_CULL_CCW)
		area = -area;
	if(area > 0.0f)
		return;

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
		DrawEdge(&vtx[prev], &vtx[i], lib);	
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
	if(lib->drawMode & MAG_DRAWMODE_CULL_CCW)
		area = -area;
	if(area >= 0.0f)
		return;

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
	int prev = nIndx - 1;
	for(int i = 0; i < nIndx; ++i)
	{
		DrawEdge(&vtx[indx[prev]], &vtx[indx[i]], lib);	
		prev = i;
	}
	DrawSpans(miny, maxy, lib);
}

/*****************************************************************************/

static struct MaggieVertex vtxBufferUP[65536];
static struct MaggieTransVertex transVtxBufferUP[65536];
static UBYTE transClipCodesUP[65536];
static struct MaggieTransVertex clippedPoly[MAG_MAX_POLYSIZE + 8];

/*****************************************************************************/

void magDrawTrianglesUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a6, MaggieBase *lib))
{
	for(int i = 0; i < nVerts; ++i)
	{
		vtxBufferUP[i] = vtx[i];
	}

	PrepareVertexBuffer(vtxBufferUP, nVerts);

	TransformVertexBuffer(transVtxBufferUP, vtxBufferUP, nVerts, lib);

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
		LightBuffer(lib, transVtxBufferUP, vtxBufferUP, nVerts);

	int clipRes = ComputeClipCodes(transClipCodesUP, transVtxBufferUP, nVerts);

	if(clipRes == CLIPPED_OUT)
		return;

	struct GfxBase *GfxBase = lib->gfxBase;

	OwnBlitter();
	WaitBlit();

	if(clipRes == CLIPPED_IN)
	{
		NormaliseVertexBuffer(transVtxBufferUP, nVerts, transClipCodesUP, lib);

		for(int i = 0; i < nVerts; i += 3)
		{
			DrawTriangle(&transVtxBufferUP[i + 0], &transVtxBufferUP[i + 1], &transVtxBufferUP[i + 2], lib);
		}
	}
	else if(clipRes == CLIPPED_PARTIAL)
	{
		for(int i = 0; i < nVerts; i += 3)
		{
			if(transClipCodesUP[i + 0] | transClipCodesUP[i + 1] | transClipCodesUP[i + 2])
			{
				if(!(transClipCodesUP[i + 0] & transClipCodesUP[i + 1] & transClipCodesUP[i + 2]))
				{
					clippedPoly[0] = transVtxBufferUP[i + 0];
					clippedPoly[1] = transVtxBufferUP[i + 1];
					clippedPoly[2] = transVtxBufferUP[i + 2];
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
				NormaliseClippedVertexBuffer(&transVtxBufferUP[i], 3, lib);
				DrawTriangle(&transVtxBufferUP[i + 0], &transVtxBufferUP[i + 1], &transVtxBufferUP[i + 2], lib);
			}
		}
	}
	DisownBlitter();
}

/*****************************************************************************/

void magDrawIndexedTrianglesUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a1, UWORD *indx), REG(d1, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	for(int i = 0; i < nVerts; ++i)
	{
		vtxBufferUP[i] = vtx[i];
	}

	PrepareVertexBuffer(vtxBufferUP, nVerts);

	TransformVertexBuffer(transVtxBufferUP, vtxBufferUP, nVerts, lib);

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
		LightBuffer(lib, transVtxBufferUP, vtxBufferUP, nVerts);

	int clipRes = ComputeClipCodes(transClipCodesUP, transVtxBufferUP, nVerts);

	if(clipRes == CLIPPED_OUT)
		return;

	struct GfxBase *GfxBase = lib->gfxBase;

	OwnBlitter();
	WaitBlit();

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(transVtxBufferUP, nVerts, lib);
		for(int i = 0; i < nIndx; i += 3)
		{
			int i0 = indx[i + 0];
			int i1 = indx[i + 1];
			int i2 = indx[i + 2];
			DrawTriangle(&transVtxBufferUP[i0], &transVtxBufferUP[i1], &transVtxBufferUP[i2], lib);
		}
	}
	if(clipRes == CLIPPED_PARTIAL)
	{
		for(int i = 0; i < nIndx; i += 3)
		{
			int i0 = indx[i + 0];
			int i1 = indx[i + 1];
			int i2 = indx[i + 2];

			if(transClipCodesUP[i0] | transClipCodesUP[i1] | transClipCodesUP[i2])
			{
				if(!(transClipCodesUP[i0] & transClipCodesUP[i1] & transClipCodesUP[i2]))
				{
					clippedPoly[0] = transVtxBufferUP[i0];
					clippedPoly[1] = transVtxBufferUP[i1];
					clippedPoly[2] = transVtxBufferUP[i2];
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
				clippedPoly[0] = transVtxBufferUP[i0];
				clippedPoly[1] = transVtxBufferUP[i1];
				clippedPoly[2] = transVtxBufferUP[i2];
				NormaliseClippedVertexBuffer(clippedPoly, 3, lib);
				DrawTriangle(&clippedPoly[0], &clippedPoly[1], &clippedPoly[2], lib);
			}
		}
	}
	DisownBlitter();
}

/*****************************************************************************/

void magDrawIndexedPolygonsUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a1, UWORD *indx), REG(d1, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	for(int i = 0; i < nVerts; ++i)
	{
		vtxBufferUP[i] = vtx[i];
	}
	PrepareVertexBuffer(vtxBufferUP, nVerts);

	TransformVertexBuffer(transVtxBufferUP, vtxBufferUP, nVerts, lib);

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
		LightBuffer(lib, transVtxBufferUP, vtxBufferUP, nVerts);

	int clipRes = ComputeClipCodes(transClipCodesUP, transVtxBufferUP, nVerts);

	if(clipRes == CLIPPED_OUT)
		return;

	struct GfxBase *GfxBase = lib->gfxBase;

	OwnBlitter();
	WaitBlit();

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(transVtxBufferUP, nVerts, lib);
		int indxPos = 0;
		while(indxPos < nIndx)
		{
			int nPolyVerts = 0;
			for(int i = indxPos; i < nIndx; ++i)
			{
				if(indx[i] == 0xffff)
					break;

				nPolyVerts++;
			}
			if(nPolyVerts >= 3)
			{
				DrawIndexedPolygon(transVtxBufferUP, &indx[indxPos], nPolyVerts, lib);
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
				if(indx[i] == 0xffff)
				{
					break;
				}
				nPolyVerts++;
				clippedAll &= transClipCodesUP[indx[i]];
				clippedAny |= transClipCodesUP[indx[i]];
			}
			if(clippedAll | (nPolyVerts < 3))
			{
				indxPos += nPolyVerts + 1;
				continue;
			}

			for(int i = 0; i < nPolyVerts; ++i)
			{
				clippedPoly[i] = transVtxBufferUP[indx[i + indxPos]];
			}
			indxPos += nPolyVerts + 1;
			if(clippedAny)
			{
				int nClippedVerts = ClipPolygon(clippedPoly, nPolyVerts);
				if(nClippedVerts > 2)
				{
					NormaliseClippedVertexBuffer(clippedPoly, nClippedVerts, lib);
					DrawPolygon(clippedPoly, nClippedVerts, lib);
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
	struct MaggieVertex *vtx = GetVBVertices(lib->vertexBuffers[lib->vBuffer]) + startVtx;
	struct MaggieTransVertex *transVtx = GetVBTransVertices(lib->vertexBuffers[lib->vBuffer]) + startVtx;
	UBYTE *clipCodes = GetVBClipCodes(lib->vertexBuffers[lib->vBuffer]) + startVtx;

	TransformVertexBuffer(transVtx, vtx, nVerts, lib);

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
		LightBuffer(lib, transVtx, vtx, nVerts);

	int clipRes = ComputeClipCodes(clipCodes, transVtx, nVerts);

	if(clipRes == CLIPPED_OUT)
		return;

	struct GfxBase *GfxBase = lib->gfxBase;
	OwnBlitter();
	WaitBlit();

	if(clipRes == CLIPPED_IN)
	{
		NormaliseVertexBuffer(transVtx, nVerts, clipCodes, lib);

		for(int i = 0; i < nVerts; i += 3)
		{
			DrawTriangle(&transVtx[i + 0], &transVtx[i + 1], &transVtx[i + 2], lib);
		}
	}
	else if(clipRes == CLIPPED_PARTIAL)
	{
		for(int i = 0; i < nVerts; i += 3)
		{
			if(clipCodes[i + 0] | clipCodes[i + 1] | clipCodes[i + 2])
			{
				if(!(clipCodes[i + 0] & clipCodes[i + 1] & clipCodes[i + 2]))
				{
					clippedPoly[0] = transVtx[i + 0];
					clippedPoly[1] = transVtx[i + 1];
					clippedPoly[2] = transVtx[i + 2];
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
				NormaliseClippedVertexBuffer(&transVtx[i], 3, lib);
				DrawTriangle(&transVtx[i + 0], &transVtx[i + 1], &transVtx[i + 2], lib);
			}
		}
	}
	DisownBlitter();
}

/*****************************************************************************/

void magDrawIndexedTriangles(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(d2, UWORD startIndx), REG(d3, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	UWORD *indexBuffer = GetIBIndices(lib->indexBuffers[lib->iBuffer]) + startIndx;
	struct MaggieVertex *vtx = GetVBVertices(lib->vertexBuffers[lib->vBuffer]);
	struct MaggieTransVertex *transVtx = GetVBTransVertices(lib->vertexBuffers[lib->vBuffer]);
	UBYTE *clipCodes = GetVBClipCodes(lib->vertexBuffers[lib->vBuffer]);

	TransformVertexBuffer(&transVtx[startVtx], &vtx[startVtx], nVerts, lib);

	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
		LightBuffer(lib, &transVtx[startVtx], &vtx[startVtx], nVerts);

	int clipRes = ComputeClipCodes(&clipCodes[startVtx], &transVtx[startVtx], nVerts);

	if(clipRes == CLIPPED_OUT)
		return;

	struct GfxBase *GfxBase = lib->gfxBase;
	OwnBlitter();
	WaitBlit();

	if(clipRes == CLIPPED_IN)
	{
		NormaliseClippedVertexBuffer(transVtx, nVerts, lib);
		for(int i = 0; i < nIndx; i += 3)
		{
			int i0 = indexBuffer[i + 0];
			int i1 = indexBuffer[i + 1];
			int i2 = indexBuffer[i + 2];
			DrawTriangle(&transVtx[i0], &transVtx[i1], &transVtx[i2], lib);
		}
	}
	if(clipRes == CLIPPED_PARTIAL)
	{
		for(int i = 0; i < nIndx; i += 3)
		{
			int i0 = indexBuffer[i + 0];
			int i1 = indexBuffer[i + 1];
			int i2 = indexBuffer[i + 2];

			if(!(clipCodes[i0] & clipCodes[i1] & clipCodes[i2]))
			{
				clippedPoly[0] = transVtx[i0];
				clippedPoly[1] = transVtx[i1];
				clippedPoly[2] = transVtx[i2];
				if(clipCodes[i0] | clipCodes[i1] | clipCodes[i2])
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
}

/*****************************************************************************/

void magDrawIndexedPolygons(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(d2, UWORD startIndx), REG(d3, UWORD nIndx), REG(a6, MaggieBase *lib))
{
	UWORD *indexBuffer = GetIBIndices(lib->indexBuffers[lib->iBuffer]) + startIndx;
	struct MaggieVertex *vtx = GetVBVertices(lib->vertexBuffers[lib->vBuffer]);
	struct MaggieTransVertex *transVtx = GetVBTransVertices(lib->vertexBuffers[lib->vBuffer]);
	UBYTE *clipCodes = GetVBClipCodes(lib->vertexBuffers[lib->vBuffer]);

	TransformVertexBuffer(&transVtx[startVtx], &vtx[startVtx], nVerts, lib);
	if(lib->drawMode & MAG_DRAWMODE_LIGHTING)
		LightBuffer(lib, &transVtx[startVtx], &vtx[startVtx], nVerts);

	int clipRes = ComputeClipCodes(&clipCodes[startVtx], &transVtx[startVtx], nVerts);

	if(clipRes == CLIPPED_OUT)
		return;

	struct GfxBase *GfxBase = lib->gfxBase;
	OwnBlitter();
	WaitBlit();
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
}

/*****************************************************************************/

void magDrawLinearSpan(REG(a0, struct SpanPosition *start), REG(a1, struct SpanPosition *end), REG(a6, MaggieBase *lib))
{
}

/*****************************************************************************/

void magDrawSpan(REG(a0, struct MaggieClippedVertex *start), REG(a1, struct MaggieClippedVertex *end), REG(a6, MaggieBase *lib))
{
}

/*****************************************************************************/
