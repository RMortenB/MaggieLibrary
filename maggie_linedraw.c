#include "maggie_internal.h"
#include "maggie_vertex.h"

/*****************************************************************************/

void DrawLineAsm(magEdgePos *edge __asm("a0"),
				const struct MaggieTransVertex *v0 __asm("a1"),
				const struct MaggieTransVertex *v1 __asm("a2"),
				float corrFactor __asm("fp0"),
				float preStep0 __asm("fp1"),
				int lineLen __asm("d0"));

void DrawLine(magEdgePos * restrict edge, int tex, const struct MaggieTransVertex * restrict v0, const struct MaggieTransVertex * restrict v1)
{
	int y0 = (int)v0->pos.y;
	int y1 = (int)v1->pos.y;

	int lineLen = (int)(y1 - y0);

	if(lineLen <= 0)
		return;

	float ooLineLen = 1.0f / lineLen;

	edge += y0;

	float preStep0 = 1.0f + y0 - v0->pos.y;
	float preStep1 = 1.0f + y1 - v1->pos.y;

	float preRatioDiff = (preStep0 - preStep1) / (v1->pos.y - v0->pos.y);
    float corrFactor = (1.0f - preRatioDiff) * ooLineLen;
#if 1
	DrawLineAsm(edge, v0, v1, corrFactor, preStep0, lineLen);
#else
	float xLen = v1->pos.x - v0->pos.x;
	float zLen = v1->pos.z - v0->pos.z;
	float wLen = v1->pos.w - v0->pos.w;
	float uLen = v1->tex[tex].u - v0->tex[tex].u;
	float vLen = v1->tex[tex].v - v0->tex[tex].v;
	int iLen = ((int)v1->colour - (int)v0->colour);

	float xDDA = xLen * corrFactor;
	float zDDA = zLen * corrFactor;
	float wDDA = wLen * corrFactor;
	float uDDA = uLen * corrFactor;
	float vDDA = vLen * corrFactor;
	float iDDA = iLen * corrFactor;

	float xVal = v0->pos.x + preStep0 * xDDA;
	float zow = v0->pos.z + preStep0 * zDDA;
	float oow = v0->pos.w + preStep0 * wDDA;
	float uow = v0->tex[tex].u + preStep0 * uDDA;
	float vow = v0->tex[tex].v + preStep0 * vDDA;
	float iow = (int)v0->colour + preStep0 * iDDA;

	for(int i = 0; i < lineLen; ++i)
	{
		edge[i].xPos = xVal;
		edge[i].zow = zow;
		edge[i].oow = oow;
		edge[i].uow = uow;
		edge[i].vow = vow;
		edge[i].iow = iow;
		xVal += xDDA;
		zow += zDDA;
		oow += wDDA;
		uow += uDDA;
		vow += vDDA;
		iow += iDDA;
	}
#endif
}

/*****************************************************************************/

void DrawEdge(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, MaggieBase *lib)
{
#if PROFILE
	ULONG startTime = GetClocks();
#endif
	if(lib->drawMode & MAG_DRAWMODE_CULL_CCW)
	{
		if(vtx0->pos.y <= vtx1->pos.y)
		{
			DrawLine(lib->magRightEdge, 0, vtx0, vtx1);
		}
		else
		{
			DrawLine(lib->magLeftEdge, 0, vtx1, vtx0);
		}
	}
	else
	{
		if(vtx0->pos.y <= vtx1->pos.y)
		{
			DrawLine(lib->magLeftEdge, 0, vtx0, vtx1);
		}
		else
		{
			DrawLine(lib->magRightEdge, 0, vtx1, vtx0);
		}
	}
#if PROFILE
	lib->profile.lines += GetClocks() - startTime;
#endif
}
