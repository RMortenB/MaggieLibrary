// C replacement for maggie_setuptri.s, for A/B testing it: same register-bound signature, so it links in place of maggie_setuptri.o with no other change.
// Build with:  make SETUP=c

#include "maggie_vec.h"
#include "maggie_vertex.h"
#include "maggie_flags.h"
#include "maggie_internal.h"

ULONG MaggieSetupTri(struct MaggieTransVertex *v0 __asm("a0"),
				struct MaggieTransVertex *v1 __asm("a1"),
				struct MaggieTransVertex *v2 __asm("a2"),
				MaggieBase *lib __asm("a6"))
{
	float x1 = v1->pos.x - v0->pos.x;
	float y1 = v1->pos.y - v0->pos.y;
	float x2 = v2->pos.x - v0->pos.x;
	float y2 = v2->pos.y - v0->pos.y;

	float denom = x1 * y2 - x2 * y1;

	// cull: triangles reject area > 0 (polygons use >= 0)
	float area = denom * lib->cullSign;
	if(area > 0.0f)
		return 0;

	// screen-y span, truncated exactly as the asm's fintrz does
	int iy0 = (int)v0->pos.y;
	int iy1 = (int)v1->pos.y;
	int iy2 = (int)v2->pos.y;

	int miny = iy0, maxy = iy0;
	if(iy1 < miny) miny = iy1;
	if(iy2 < miny) miny = iy2;
	if(iy1 > maxy) maxy = iy1;
	if(iy2 > maxy) maxy = iy2;

	// clamped to the scissor here, so the edge table only ever covers rows the span renderer will paint
	if(miny < lib->scissor.y0)
		miny = lib->scissor.y0;
	if(maxy > lib->scissor.y1)
		maxy = lib->scissor.y1;

	if(miny >= maxy)
		return 0;					// zero scanlines tall, or wholly outside the scissor

	lib->primMinY = miny;
	lib->primMaxY = maxy;

	magGradients *g = &lib->gradients;

	if((denom > -1e-6f) && (denom < 1e-6f))
	{
		g->oowDDA = g->uowDDA = g->vowDDA = g->zDDA = g->iDDA = 0.0f;
		return 1;
	}

	float ooDenom = 1.0f / denom;

	g->oowDDA = ((v1->pos.w    - v0->pos.w)    * y2 - (v2->pos.w    - v0->pos.w)    * y1) * ooDenom;
	g->uowDDA = ((v1->tex[0].u - v0->tex[0].u) * y2 - (v2->tex[0].u - v0->tex[0].u) * y1) * ooDenom;
	g->vowDDA = ((v1->tex[0].v - v0->tex[0].v) * y2 - (v2->tex[0].v - v0->tex[0].v) * y1) * ooDenom;
	g->zDDA   = ((v1->pos.z    - v0->pos.z)    * y2 - (v2->pos.z    - v0->pos.z)    * y1) * ooDenom;
	g->iDDA   = (((float)v1->colour - (float)v0->colour) * y2
	           - ((float)v2->colour - (float)v0->colour) * y1) * ooDenom;

	return 1;
}
