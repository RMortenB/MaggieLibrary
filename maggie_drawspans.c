#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

/*****************************************************************************/

APTR GetScreen(MaggieBase *lib __asm("a6"))
{
	return lib->screen;
}

/*****************************************************************************/

UWORD *GetDepthBuffer(MaggieBase *lib __asm("a6"))
{
	return lib->depthBuffer;
}

/*****************************************************************************/

int GetXRes(MaggieBase *lib __asm("a6"))
{
	return lib->xres;
}

/*****************************************************************************/

APTR GetEdges(MaggieBase *lib __asm("a6"))
{
	return &lib->magEdge[0];
}

/*****************************************************************************/

ScissorRect *GetScissor(MaggieBase *lib __asm("a6"))
{
	return &lib->scissor;
}

/*****************************************************************************/

void DrawSpansHW32ZBuffer(int ymin, int ymax, MaggieBase *lib);
void DrawSpansHW16ZBuffer(int ymin, int ymax, MaggieBase *lib);
void DrawSpansHW32(int ymin, int ymax, MaggieBase *lib);
void DrawSpansHW16(int ymin, int ymax, MaggieBase *lib);

/*****************************************************************************/
void DrawSpansSW32ZBuffer(int ymin, int ymax, MaggieBase *lib);
void DrawSpansSW16ZBuffer(int ymin, int ymax, MaggieBase *lib);
void DrawSpansSW32(int ymin, int ymax, MaggieBase *lib);
void DrawSpansSW16(int ymin, int ymax, MaggieBase *lib);

/*****************************************************************************/

void DrawScanlines32ZAffine(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines32ZAffinePoly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines32Affine(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines32AffinePoly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));

void DrawScanlines16ZAffine(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines16ZAffinePoly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines16Affine(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines16AffinePoly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));

void DrawScanlines32IZ(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines32IZPoly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines32(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines32Poly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));

void DrawScanlines16IZ(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines16IZPoly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines16(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines16Poly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));

/*****************************************************************************/

void DrawScanlinesNIZ(int ymin __asm("d0"), int ymax __asm("d1"), int pixelSize __asm("d2"), MaggieBase *lib __asm("a6"));

/*****************************************************************************/

// Resolve both intensity variants of the span renderer for the current
// drawMode. Called once per batch from BeginDrawBatch(); the per-primitive path
// then just picks one of the two pointers instead of walking this branch tree
// for every triangle.

void SelectScanFunctions(MaggieBase *lib)
{
	UWORD mode = lib->drawMode;

	if(mode & MAG_DRAWMODE_AFFINE_MAPPING)
	{
		if(mode & MAG_DRAWMODE_DEPTHBUFFER)
		{
			if(mode & MAG_DRAWMODE_32BIT)
			{
				lib->scanFuncFlat = DrawScanlines32ZAffine;
				lib->scanFuncPoly = DrawScanlines32ZAffinePoly;
			}
			else
			{
				lib->scanFuncFlat = DrawScanlines16ZAffine;
				lib->scanFuncPoly = DrawScanlines16ZAffinePoly;
			}
		}
		else
		{
			if(mode & MAG_DRAWMODE_32BIT)
			{
				lib->scanFuncFlat = DrawScanlines32Affine;
				lib->scanFuncPoly = DrawScanlines32AffinePoly;
			}
			else
			{
				lib->scanFuncFlat = DrawScanlines16Affine;
				lib->scanFuncPoly = DrawScanlines16AffinePoly;
			}
		}
	}
	else
	{
		if(mode & MAG_DRAWMODE_DEPTHBUFFER)
		{
			if(mode & MAG_DRAWMODE_32BIT)
			{
				lib->scanFuncFlat = DrawScanlines32IZ;
				lib->scanFuncPoly = DrawScanlines32IZPoly;
			}
			else
			{
				lib->scanFuncFlat = DrawScanlines16IZ;
				lib->scanFuncPoly = DrawScanlines16IZPoly;
			}
		}
		else
		{
			if(mode & MAG_DRAWMODE_32BIT)
			{
				lib->scanFuncFlat = DrawScanlines32;
				lib->scanFuncPoly = DrawScanlines32Poly;
			}
			else
			{
				lib->scanFuncFlat = DrawScanlines16;
				lib->scanFuncPoly = DrawScanlines16Poly;
			}
		}
	}

	lib->scanFunc = lib->scanFuncFlat;

	// Only magDrawIndexedPolygons can produce non-planar intensity; collapsing
	// the two pointers here saves the source test in the per-primitive path.
	if(!lib->sourceIsPoly)
		lib->scanFuncPoly = lib->scanFuncFlat;
}

/*****************************************************************************/

// The "can this batch draw at all" test (Maggie present, texture bound) used to
// live here and ran per primitive; BeginDrawBatch() now rejects the whole call
// up front.

void DrawSpans(int miny, int maxy, MaggieBase *lib)
{
	if(miny < lib->scissor.y0)
		miny = lib->scissor.y0;
	if(maxy > lib->scissor.y1)
		maxy = lib->scissor.y1;
	if(miny >= maxy)
		return;
#if PROFILE
	lib->profile.prims++;
	ULONG spansStart = GetClocks();
#endif
	lib->scanFunc(miny, maxy, lib);
#if PROFILE
	lib->profile.spans += GetClocks() - spansStart;
#endif
}
