#include "maggie_internal.h"
#include <proto/graphics.h>
#include <math.h>
#include <string.h>

/*****************************************************************************/

void TransformVertexBuffer(struct MaggieTransVertex *dstVtx, struct MaggieVertex *vtx, UWORD nVerts, MaggieBase *lib)
{
	if(lib->dirtyMatrix)
	{
		lib->dirtyMatrix = 0;
		mat4_mul(&lib->modelviewProj, &lib->viewMatrix, &lib->worldMatrix);
		mat4_mul(&lib->modelviewProj, &lib->perspectiveMatrix, &lib->modelviewProj);
	}

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
		vtx[i].colour = RGBToGrayScale(vtx[i].colour);
	}
}

/*****************************************************************************/

// These are reset on EndDraw.

void magSetWorldMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib))
{
	memcpy(&lib->worldMatrix, matrix, sizeof(mat4));
	lib->dirtyMatrix = 1;
}

/*****************************************************************************/

void magSetViewMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib))
{
	memcpy(&lib->viewMatrix, matrix, sizeof(mat4));
	lib->dirtyMatrix = 1;
}

/*****************************************************************************/

void magSetPerspectiveMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib))
{
	memcpy(&lib->perspectiveMatrix, matrix, sizeof(mat4));
	lib->dirtyMatrix = 1;
}

/*****************************************************************************/


