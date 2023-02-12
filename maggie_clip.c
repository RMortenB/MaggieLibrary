#include <exec/types.h>
#include "maggie_vertex.h"
#include "maggie_internal.h"
#include <math.h>

/*****************************************************************************/

static void LerpVertex(struct MaggieTransVertex *res, const struct MaggieTransVertex *v0, const struct MaggieTransVertex *v1, float t)
{
	res->pos.x = (v1->pos.x - v0->pos.x) * t + v0->pos.x;
	res->pos.y = (v1->pos.y - v0->pos.y) * t + v0->pos.y;
	res->pos.z = (v1->pos.z - v0->pos.z) * t + v0->pos.z;
	res->pos.w = (v1->pos.w - v0->pos.w) * t + v0->pos.w;
	for(int i = 0; i < MAGGIE_MAX_TEXCOORDS; ++i)
	{
		res->tex[i].u = (v1->tex[i].u - v0->tex[i].u) * t + v0->tex[i].u;
		res->tex[i].v = (v1->tex[i].v - v0->tex[i].v) * t + v0->tex[i].v;
		res->tex[i].w = (v1->tex[i].w - v0->tex[i].w) * t + v0->tex[i].w;
	}
	res->rgba = (v1->rgba - v0->rgba) * t + v0->rgba;
}

/*****************************************************************************/

int ClipPolygon(struct MaggieTransVertex *verts, int nVerts)
{
	static struct MaggieTransVertex tmpPoly0[MAG_MAX_POLYSIZE + 6];
	static struct MaggieTransVertex tmpPoly1[MAG_MAX_POLYSIZE + 6];

	const struct MaggieTransVertex *inBuffer = verts;
	struct MaggieTransVertex *outBuffer = tmpPoly1;
	int prevVertex = nVerts - 1;
	const struct MaggieTransVertex *vLast;
	vLast = &inBuffer[prevVertex];
	int lastOut = -vLast->pos.w > vLast->pos.x;
	int nOutput = 0;

	for(int i = 0; i < nVerts; ++i)
	{
		const struct MaggieTransVertex *v0 = &inBuffer[prevVertex];
		const struct MaggieTransVertex *v1 = &inBuffer[i];
		int out = -v1->pos.w > v1->pos.x;
		if(out != lastOut)
		{
			// Intersect
			float t = (v0->pos.w + v0->pos.x) / (v0->pos.x - v1->pos.x + v0->pos.w - v1->pos.w);
			if(t < 0.0f) t = 0.0f;
			if(t > 1.0f) t = 1.0f;
			LerpVertex(&outBuffer[nOutput++], v0, v1, t);
		}
		if(!out)
		{
			outBuffer[nOutput++] = inBuffer[i];
		}
		prevVertex = i;
		lastOut = out;
	}
	if(nOutput < 3)
		return 0;

	inBuffer = tmpPoly1;
	outBuffer = tmpPoly0;
	nVerts = nOutput;
	prevVertex = nVerts - 1;
	vLast = &inBuffer[prevVertex];
	lastOut = vLast->pos.w < vLast->pos.x;
	nOutput = 0;

	for(int i = 0; i < nVerts; ++i)
	{
		const struct MaggieTransVertex *v0 = &inBuffer[prevVertex];
		const struct MaggieTransVertex *v1 = &inBuffer[i];
		int out = v1->pos.w < v1->pos.x;
		if(out != lastOut)
		{
			// Intersect
			float t = (v0->pos.w - v0->pos.x) / (v1->pos.x - v0->pos.x - v1->pos.w + v0->pos.w);
			if(t < 0.0f) t = 0.0f;
			if(t > 1.0f) t = 1.0f;
			LerpVertex(&outBuffer[nOutput++], v0, v1, t);
		}
		if(!out)
		{
			outBuffer[nOutput++] = inBuffer[i];
		}
		prevVertex = i;
		lastOut = out;
	}
	if(nOutput < 3)
		return 0;

	inBuffer = tmpPoly0;
	outBuffer = tmpPoly1;
	nVerts = nOutput;
	prevVertex = nVerts - 1;
	vLast = &inBuffer[prevVertex];
	lastOut = -vLast->pos.w > vLast->pos.y;
	nOutput = 0;

	for(int i = 0; i < nVerts; ++i)
	{
		const struct MaggieTransVertex *v0 = &inBuffer[prevVertex];
		const struct MaggieTransVertex *v1 = &inBuffer[i];
		int out = -v1->pos.w > v1->pos.y;
		if(out != lastOut)
		{
			// Intersect
			float t = (v0->pos.w + v0->pos.y) / (v0->pos.y - v1->pos.y + v0->pos.w - v1->pos.w);
			if(t < 0.0f) t = 0.0f;
			if(t > 1.0f) t = 1.0f;
			LerpVertex(&outBuffer[nOutput++], v0, v1, t);
		}
		if(!out)
		{
			outBuffer[nOutput++] = inBuffer[i];
		}
		prevVertex = i;
		lastOut = out;
	}
	if(nOutput < 3)
		return 0;

	inBuffer = tmpPoly1;
	outBuffer = tmpPoly0;
	nVerts = nOutput;
	prevVertex = nVerts - 1;
	vLast = &inBuffer[prevVertex];
	lastOut = vLast->pos.w < vLast->pos.y;
	nOutput = 0;

	for(int i = 0; i < nVerts; ++i)
	{
		const struct MaggieTransVertex *v0 = &inBuffer[prevVertex];
		const struct MaggieTransVertex *v1 = &inBuffer[i];
		int out = v1->pos.w < v1->pos.y;
		if(out != lastOut)
		{
			// Intersect
			float t = (v0->pos.w - v0->pos.y) / (v1->pos.y - v0->pos.y - v1->pos.w + v0->pos.w);
			if(t < 0.0f) t = 0.0f;
			if(t > 1.0f) t = 1.0f;
			LerpVertex(&outBuffer[nOutput++], v0, v1, t);
		}
		if(!out)
		{
			outBuffer[nOutput++] = inBuffer[i];
		}
		prevVertex = i;
		lastOut = out;
	}
	if(nOutput < 3)
		return 0;

	inBuffer = tmpPoly0;
	outBuffer = tmpPoly1;
	nVerts = nOutput;
	prevVertex = nVerts - 1;
	vLast = &inBuffer[prevVertex];
	lastOut = vLast->pos.z < 0.0f;
	nOutput = 0;

	for(int i = 0; i < nVerts; ++i)
	{
		const struct MaggieTransVertex *v0 = &inBuffer[prevVertex];
		const struct MaggieTransVertex *v1 = &inBuffer[i];
		int out = v1->pos.z < 0.0f;
		if(out != lastOut)
		{
			// Intersect
			float t = v0->pos.z / (v0->pos.z - v1->pos.z);
			if(t < 0.0f) t = 0.0f;
			if(t > 1.0f) t = 1.0f;
			LerpVertex(&outBuffer[nOutput++], v0, v1, t);
		}
		if(!out)
		{
			outBuffer[nOutput++] = inBuffer[i];
		}
		prevVertex = i;
		lastOut = out;
	}
	if(nOutput < 3)
		return 0;

	inBuffer = tmpPoly1;
	outBuffer = verts;
	nVerts = nOutput;
	prevVertex = nVerts - 1;
	vLast = &inBuffer[prevVertex];
	lastOut = vLast->pos.w < vLast->pos.z;
	nOutput = 0;

	for(int i = 0; i < nVerts; ++i)
	{
		const struct MaggieTransVertex *v0 = &inBuffer[prevVertex];
		const struct MaggieTransVertex *v1 = &inBuffer[i];
		int out = v1->pos.w < v1->pos.z;
		if(out != lastOut)
		{
			// Intersect
			float t = (v0->pos.w - v0->pos.z) / (v1->pos.z - v0->pos.z - v1->pos.w + v0->pos.w);
			if(t < 0.0f) t = 0.0f;
			if(t > 1.0f) t = 1.0f;
			LerpVertex(&outBuffer[nOutput++], v0, v1, t);
		}
		if(!out)
		{
			outBuffer[nOutput++] = inBuffer[i];
		}
		prevVertex = i;
		lastOut = out;
	}
	if(nOutput < 3)
		return 0;

	return nOutput;
}

/*****************************************************************************/

