#include "maggie_internal.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"

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

void DrawSpans(int miny, int maxy, MaggieBase *lib)
{
	if(miny < lib->scissor.y0)
		miny = lib->scissor.y0;
	if(maxy > lib->scissor.y1)
		maxy = lib->scissor.y1;
	if(miny >= maxy)
		return;
#if PROFILE
	ULONG spansStart = GetClocks();
#endif
	if(((!lib->hasMaggie) || lib->txtrIndex == 0xffff) || !lib->textures[lib->txtrIndex])
	{
		if(lib->drawMode & MAG_DRAWMODE_DEPTHBUFFER)
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				DrawSpansSW32ZBuffer(miny, maxy, lib);
			}
			else
			{
				DrawSpansSW16ZBuffer(miny, maxy, lib);
			}
		}
		else
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				DrawSpansSW32(miny, maxy, lib);
			}
			else
			{
				DrawSpansSW16(miny, maxy, lib);
			}
		}
	}
	else
	{
		if(lib->drawMode & MAG_DRAWMODE_DEPTHBUFFER)
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				DrawSpansHW32ZBuffer(miny, maxy, lib);
			}
			else
			{
				DrawSpansHW16ZBuffer(miny, maxy, lib);
			}
		}
		else
		{
			if(lib->drawMode & MAG_DRAWMODE_32BIT)
			{
				DrawSpansHW32(miny, maxy, lib);
			}
			else
			{
				DrawSpansHW16(miny, maxy, lib);
			}
		}
	}
#if PROFILE
	lib->profile.spans += GetClocks() - spansStart;
#endif
}
