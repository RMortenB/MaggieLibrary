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
		dstVtx[i].colour = vtx[i].colour;
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
		vtx[i].colour = RGBToGrayScale(vtx[i].colour) * 256.0f;
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
		dst[i].colour = RGBToGrayScale(src[i].colour) * 256.0f;
	}
}

/*****************************************************************************/

// These are reset on EndDraw.

void magSetWorldMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib))
{
	memcpy(&lib->worldMatrix, matrix, sizeof(mat4));
	mat4_mul(&lib->modelviewProj, &lib->viewMatrix, &lib->worldMatrix);
	mat4_mul(&lib->modelviewProj, &lib->perspectiveMatrix, &lib->modelviewProj);
}

/*****************************************************************************/

void magSetViewMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib))
{
	memcpy(&lib->viewMatrix, matrix, sizeof(mat4));
	mat4_mul(&lib->modelviewProj, &lib->viewMatrix, &lib->worldMatrix);
	mat4_mul(&lib->modelviewProj, &lib->perspectiveMatrix, &lib->modelviewProj);
}

/*****************************************************************************/

void magSetPerspectiveMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib))
{
	memcpy(&lib->perspectiveMatrix, matrix, sizeof(mat4));
	mat4_mul(&lib->modelviewProj, &lib->viewMatrix, &lib->worldMatrix);
	mat4_mul(&lib->modelviewProj, &lib->perspectiveMatrix, &lib->modelviewProj);
}

/*****************************************************************************/


