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

void DrawScanlines16IZ(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines16IZPoly(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));
void DrawScanlines16(int ymin __asm("d0"), int ymax __asm("d1"), MaggieBase *lib __asm("a6"));

/*****************************************************************************/

void DrawScanlinesNIZ(int ymin __asm("d0"), int ymax __asm("d1"), int pixelSize __asm("d2"), MaggieBase *lib __asm("a6"));

/*****************************************************************************/

void DrawSpans(int miny, int maxy, MaggieBase *lib)
{
	if(((!lib->hasMaggie) || lib->txtrIndex == 0xffff) || !lib->textures[lib->txtrIndex])
	{
		return;
	}

	if(miny < lib->scissor.y0)
		miny = lib->scissor.y0;
	if(maxy > lib->scissor.y1)
		maxy = lib->scissor.y1;
	if(miny >= maxy)
		return;
#if PROFILE
	ULONG spansStart = GetClocks();
#endif
	if(lib->drawMode & MAG_DRAWMODE_AFFINE_MAPPING)
	{
		if(lib->drawMode & MAG_DRAWMODE_DEPTHBUFFER)
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				if(lib->polyIntensity)
					DrawScanlines32ZAffinePoly(miny, maxy, lib);
				else
					DrawScanlines32ZAffine(miny, maxy, lib);
			}
			else
			{
				if(lib->polyIntensity)
					DrawScanlines16ZAffinePoly(miny, maxy, lib);
				else
					DrawScanlines16ZAffine(miny, maxy, lib);
			}
		}
		else
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				if(lib->polyIntensity)
					DrawScanlines32AffinePoly(miny, maxy, lib);
				else
					DrawScanlines32Affine(miny, maxy, lib);
			}
			else
			{
				if(lib->polyIntensity)
					DrawScanlines16AffinePoly(miny, maxy, lib);
				else
					DrawScanlines16Affine(miny, maxy, lib);
			}
		}
	}
	else
	{
		if(lib->drawMode & MAG_DRAWMODE_DEPTHBUFFER)
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				if(lib->polyIntensity)
					DrawScanlines32IZPoly(miny, maxy, lib);
				else
					DrawScanlines32IZ(miny, maxy, lib);
			}
			else
			{
				if(lib->polyIntensity)
					DrawScanlines16IZPoly(miny, maxy, lib);
				else
					DrawScanlines16IZ(miny, maxy, lib);
			}
		}
		else
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				DrawScanlines32(miny, maxy, lib);
			}
			else
			{
				DrawScanlines16(miny, maxy, lib);
			}
		}
	}
#if PROFILE
	lib->profile.spans += GetClocks() - spansStart;
#endif
}
