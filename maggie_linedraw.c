#include "maggie_internal.h"
#include "maggie_vertex.h"

/*****************************************************************************/

void DrawLine(magEdgePos * restrict edge, int tex, const struct MaggieTransVertex * restrict v0, const struct MaggieTransVertex * restrict v1)
{
	int y0 = (int)v0->pos.y;
	int y1 = (int)v1->pos.y;

	int lineLen = (int)(y1 - y0);

	if(lineLen <= 0)
		return;

	float ooLineLen = 1.0f / lineLen;

	edge += y0;

	float yFrac0 = v0->pos.y - y0;
	float yFrac1 = v1->pos.y - y1;
	float preStep0 = 1.0f - yFrac0;
	float preStep1 = 1.0f - yFrac1;

	float xLen = v1->pos.x - v0->pos.x;
	float yLen = v1->pos.y - v0->pos.y;
	float zLen = v1->pos.z - v0->pos.z;
	float wLen = v1->pos.w - v0->pos.w;
	float uLen = v1->tex[tex].u - v0->tex[tex].u;
	float vLen = v1->tex[tex].v - v0->tex[tex].v;
	int iLen = ((int)v1->colour - (int)v0->colour) * 0x0101;

	float preRatio0 = preStep0 / yLen;
	float preRatio1 = preStep1 / yLen;

	float xPreStep = xLen * (preRatio0 - preRatio1);
	float zPreStep = zLen * (preRatio0 - preRatio1);
	float wPreStep = wLen * (preRatio0 - preRatio1);
	float uPreStep = uLen * (preRatio0 - preRatio1);
	float vPreStep = vLen * (preRatio0 - preRatio1);
	float iPreStep = iLen * (preRatio0 - preRatio1);
 
	float xDDA = (xLen - xPreStep) * ooLineLen;
	float zDDA = (zLen - zPreStep) * ooLineLen;
	float wDDA = (wLen - wPreStep) * ooLineLen;
	float uDDA = (uLen - uPreStep) * ooLineLen;
	float vDDA = (vLen - vPreStep) * ooLineLen;
	float iDDA = (iLen - iPreStep) * ooLineLen;

	float xVal = v0->pos.x + preStep0 * xDDA;
	float oow = v0->pos.w + preStep0 * wDDA;
	float zow = v0->pos.z + preStep0 * zDDA;
	float uow = v0->tex[tex].u + preStep0 * uDDA;
	float vow = v0->tex[tex].v + preStep0 * vDDA;
	float iow = (int)v0->colour * 0x0101 + preStep0 * iDDA;

	for(int i = 0; i < lineLen; ++i)
	{
		edge[i].xPos = xVal;
		edge[i].oow = oow;
		edge[i].uow = uow;
		edge[i].vow = vow;
		edge[i].zow = zow;
		edge[i].iow = iow;
		xVal += xDDA;
		oow += wDDA;
		zow += zDDA;
		uow += uDDA;
		vow += vDDA;
		iow += iDDA;
	}
}

/*****************************************************************************/

void DrawEdge(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, MaggieBase *lib)
{
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
}
