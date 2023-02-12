#include "maggie_internal.h"
#include <proto/graphics.h>
#include <math.h>
#include <string.h>

/*****************************************************************************/

void TransformVertexBuffer(struct MaggieTransVertex *dstVtx, struct MaggieVertex *vtx, UWORD nVerts, MaggieBase *lib)
{
	for(int i = 0; i < nVerts; ++i)
	{
		vec3_tformh(&dstVtx[i].pos, &lib->modelviewProj, &vtx[i].pos, 1.0f);
		for(int j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
		{
			dstVtx[i].tex[j].u = vtx[i].tex[j].u;
			dstVtx[i].tex[j].v = vtx[i].tex[j].v;
			dstVtx[i].tex[j].w = vtx[i].tex[j].w;
		}
		dstVtx[i].rgba = vtx[i].rgba;
	}
}

/*****************************************************************************/

void PrepareVertexBuffer(struct MaggieVertex *vtx, UWORD nVerts)
{
	for(int i = 0; i < nVerts; ++i)
	{
		for(int j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
		{
			vtx[i].tex[j].u = vtx[i].tex[j].u * 256.0f * 65536.0f;
			vtx[i].tex[j].v = vtx[i].tex[j].v * 256.0f * 65536.0f;
			vtx[i].tex[j].w = vtx[i].tex[j].w;
		}
		vtx[i].rgba = ((vtx[i].rgba >> 8) & 0xff) * 0x101;
	}
}

/*****************************************************************************/

void TransformVertexBufferUP(struct MaggieTransVertex * restrict dst, struct MaggieVertex * restrict src, UWORD nVerts, MaggieBase *lib)
{
	for(int i = 0; i < nVerts; ++i)
	{
		vec3_tformh(&dst[i].pos, &lib->modelviewProj, &src[i].pos, 1.0f);
		for(int j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
		{
			dst[i].tex[j].u = src[i].tex[j].u * 256.0f * 65536.0f;
			dst[i].tex[j].v = src[i].tex[j].v * 256.0f * 65536.0f;
			dst[i].tex[j].w = src[i].tex[j].w;
		}
		dst[i].rgba = ((src[i].rgba >> 8) & 0xff) * 0x101;
	}
}

/*****************************************************************************/

// These are reset on EndDraw.
void magSetPerspective(REG(a0, float *matrix), REG(a6, MaggieBase *lib))
{
	memcpy(&lib->perspective, matrix, sizeof(mat4));
	mat4_mul(&lib->modelviewProj, &lib->perspective, &lib->modelview);
}

/*****************************************************************************/

void magSetModelView(REG(a0, float *matrix), REG(a6, MaggieBase *lib))
{
	memcpy(&lib->modelview, matrix, sizeof(mat4));
	mat4_mul(&lib->modelviewProj, &lib->perspective, &lib->modelview);
}

/*****************************************************************************/

