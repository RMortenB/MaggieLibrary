#include "maggie_internal.h"
#include "maggie_vertex.h"

/*****************************************************************************/

// Rasterize one polygon edge into the per-scanline edge table.
// Which column an edge feeds, and which attributes go in it, is folded into edgeFuncDown/edgeFuncUp once per batch by BeginDrawBatch(), leaving one direction test and one indirect call here.

void DrawEdge(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, int miny, MaggieBase *lib)
{
#if PROFILE_EDGES
	ULONG startTime = GetClocks();
	float edgeLen = vtx1->pos.y - vtx0->pos.y;
#endif
	magDrawLineFunc walkEdge;

	if(vtx0->pos.y > vtx1->pos.y)
	{
		struct MaggieTransVertex *swap = vtx0;
		vtx0 = vtx1;
		vtx1 = swap;
		walkEdge = lib->edgeFuncUp;
	}
	else
	{
		walkEdge = lib->edgeFuncDown;
	}

	int y0 = (int)vtx0->pos.y;
	int y1 = (int)vtx1->pos.y;

	// The setup clamped [primMinY, primMaxY) to the scissor, and the edge table covers only those rows: trim the walk to them, with preStep carrying the skipped ones.
	if(y0 < miny)
		y0 = miny;
	if(y1 > lib->primMaxY)
		y1 = lib->primMaxY;

	int lineLen = y1 - y0;

	if(lineLen > 0)
	{
		float preStep = 1.0f + y0 - vtx0->pos.y;
		float ooYLen = 1.0f / (vtx1->pos.y - vtx0->pos.y);

		walkEdge(&lib->magEdge[y0 - miny], vtx0, vtx1, ooYLen, preStep, lineLen);
	}
#if PROFILE_EDGES
	lib->profile.lines += GetClocks() - startTime;
	lib->profile.nLinePixels += (edgeLen < 0.0f) ? -edgeLen : edgeLen;
#endif
}
