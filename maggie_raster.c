#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

/*****************************************************************************/

static void SetTexture(APTR txtrData, UWORD size)
{
	maggieRegs->texture = txtrData;
	maggieRegs->texSize = size;
}

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
	UWORD modulo = 2;
	if(mode & MAG_DRAWMODE_32BIT)
	{
		modulo = 4;
	}
	else
	{
		drawMode |= 0x0004;
	}
	maggieRegs->mode = drawMode;
	maggieRegs->modulo = modulo;
	maggieRegs->lightRGBA = lib->colour;
}

/*****************************************************************************/

void DrawSpansHW32ZBuffer(ULONG * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, int modulo);
void DrawSpansHW16ZBuffer(UWORD * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, int modulo);
void DrawSpansHW32(ULONG * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, int modulo);
void DrawSpansHW16(UWORD * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, int modulo);

/*****************************************************************************/

void DrawSpansSW32ZBuffer(ULONG * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo);
void DrawSpansSW16ZBuffer(UWORD * restrict pixels, UWORD * restrict zbuffer, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo);
void DrawSpansSW32(ULONG * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo);
void DrawSpansSW16(UWORD * restrict pixels, const magEdgePos * restrict left, const magEdgePos * restrict right, int ylen, LONG modulo);

/*****************************************************************************/

void DrawSpans(int miny, int maxy, MaggieBase *lib)
{
#if PROFILE
	ULONG spansStart = GetClocks();
#endif
	magEdgePos *left = &lib->magLeftEdge[miny];
	magEdgePos *right = &lib->magRightEdge[miny];
	UWORD *depth = lib->depthBuffer + miny * lib->xres;

	if((lib->txtrIndex == 0xffff) || !lib->textures[lib->txtrIndex])
	{
		if(lib->drawMode & MAG_DRAWMODE_DEPTHBUFFER)
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				ULONG *pixels = ((ULONG *)lib->screen) + miny * lib->xres;
				DrawSpansSW32ZBuffer(pixels, depth, left, right, maxy - miny, lib->xres);
			}
			else
			{
				UWORD *pixels = ((UWORD *)lib->screen) + miny * lib->xres;
				DrawSpansSW16ZBuffer(pixels, depth, left, right, maxy - miny, lib->xres);
			}
		}
		else
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				ULONG *pixels = ((ULONG *)lib->screen) + miny * lib->xres;
				DrawSpansSW32(pixels, left, right, maxy - miny, lib->xres);
			}
			else
			{
				UWORD *pixels = ((UWORD *)lib->screen) + miny * lib->xres;
				DrawSpansSW16(pixels, left, right, maxy - miny, lib->xres);
			}
		}
	}
	else
	{
		APTR txtrData = GetTextureData(lib->textures[lib->txtrIndex]);
		UWORD txtrSize = lib->textures[lib->txtrIndex][0];
		SetTexture(txtrData, txtrSize);
		SetupHW(lib);
		if(lib->drawMode & MAG_DRAWMODE_DEPTHBUFFER)
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				ULONG *pixels = ((ULONG *)lib->screen) + miny * lib->xres;
				DrawSpansHW32ZBuffer(pixels, depth, left, right, maxy - miny, lib->xres);
			}
			else
			{
				UWORD *pixels = ((UWORD *)lib->screen) + miny * lib->xres;
				DrawSpansHW16ZBuffer(pixels, depth, left, right, maxy - miny, lib->xres);
			}
		}
		else
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				ULONG *pixels = ((ULONG *)lib->screen) + miny * lib->xres;
				DrawSpansHW32(pixels, left, right, maxy - miny, lib->xres);
			}
			else
			{
				UWORD *pixels = ((UWORD *)lib->screen) + miny * lib->xres;
				DrawSpansHW16(pixels, left, right, maxy - miny, lib->xres);
			}
		}
	}
#if PROFILE
	lib->profile.spans += GetClocks() - spansStart;
#endif
}
