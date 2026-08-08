#include "maggie_internal.h"
#include "maggie_vertex.h"

/*****************************************************************************/

// Rasterize one polygon edge into the per-scanline edge table.
//
// Which of the two magEdge columns (left/right) an edge feeds depends on the
// edge direction and the winding, and the DDA routine plus the table stride
// depend on affine-vs-perspective. All four used to be re-derived from
// lib->drawMode on every single edge; BeginDrawBatch() now folds them into
// edgeBaseDown/edgeBaseUp/drawLineFunc/edgeStride once per batch, which leaves
// this as one direction test and one indirect call.

void DrawEdge(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, int miny, MaggieBase *lib)
{
#if PROFILE_EDGES
	ULONG startTime = GetClocks();
	float edgeLen = vtx1->pos.y - vtx0->pos.y;
#endif
	UBYTE *edge;

	if(vtx0->pos.y > vtx1->pos.y)
	{
		struct MaggieTransVertex *swap = vtx0;
		vtx0 = vtx1;
		vtx1 = swap;
		edge = lib->edgeBaseUp;
	}
	else
	{
		edge = lib->edgeBaseDown;
	}

	int y0 = (int)vtx0->pos.y;
	int lineLen = (int)vtx1->pos.y - y0;

	if(lineLen > 0)
	{
		float preStep = 1.0f + y0 - vtx0->pos.y;
		float ooYLen = 1.0f / (vtx1->pos.y - vtx0->pos.y);

		lib->drawLineFunc(edge + (y0 - miny) * lib->edgeStride, vtx0, vtx1, ooYLen, preStep, lineLen);
	}
#if PROFILE_EDGES
	lib->profile.lines += GetClocks() - startTime;
	lib->profile.nLinePixels += (edgeLen < 0.0f) ? -edgeLen : edgeLen;
#endif
}
