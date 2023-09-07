#include "maggie_internal.h"
#include "maggie_debug.h"
#include <proto/graphics.h>
#include <math.h>
#include <string.h>

/*****************************************************************************/
/*****************************************************************************/

void TexGenBuffer(struct MaggieTransVertex *dstVtx, struct MaggieVertex *vtx, UWORD nVerts, MaggieBase *lib)
{
#if PROFILE
	ULONG texGenStart = GetClocks();
#endif
	int uvMode = lib->drawMode & MAG_DRAWMODE_TEXGEN_MASK;

	switch(uvMode)
	{
		case MAG_DRAWMODE_TEXGEN_UV :
		{
			for(UWORD i = 0; i < nVerts; ++i)
			{
				for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
				{
					dstVtx[i].tex[j].u = vtx[i].tex[j].u;
					dstVtx[i].tex[j].v = vtx[i].tex[j].v;
					dstVtx[i].tex[j].w = vtx[i].tex[j].w;
				}
			}
		} break;
		case MAG_DRAWMODE_TEXGEN_POS :
		{
			for(UWORD i = 0; i < nVerts; ++i)
			{
				for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
				{
					dstVtx[i].tex[j].u = vtx[i].pos.x * 256.0f * 65536.0f;
					dstVtx[i].tex[j].v = vtx[i].pos.y * 256.0f * 65536.0f;
					dstVtx[i].tex[j].w = 1.0f;
				}
			}
		} break;
		case MAG_DRAWMODE_TEXGEN_NORMAL :
		{
			for(UWORD i = 0; i < nVerts; ++i)
			{
				vec3 viewNormal;
				vec3_tform(&viewNormal, &lib->modelView, &vtx[i].normal, 0.0f); // This is nonsense..
				for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
				{
					dstVtx[i].tex[j].u = (viewNormal.x * 0.5 + 0.5f) * 256.0f * 65536.0f;
					dstVtx[i].tex[j].v = (viewNormal.y * 0.5 + 0.5f) * 256.0f * 65536.0f;
					dstVtx[i].tex[j].w = 1.0f;
				}
			}
		} break;
		case MAG_DRAWMODE_TEXGEN_REFLECT :
		{
			mat4 iWorld;
			mat4_inverseLight(&iWorld, &lib->modelView);
			vec3 camPos = { iWorld.m[3][0], iWorld.m[3][1], iWorld.m[3][2] };
			for(UWORD i = 0; i < nVerts; ++i)
			{
				vec3 viewVec;
				vec3 viewDir;
				vec3 reflected;
				vec3 viewReflected;
				vec3 normScale;
				vec3_sub(&viewVec, &vtx[i].pos, &camPos);
				vec3_normaliseApprox(&viewDir, &viewVec);
				float VdotN = vec3_dot(&viewDir, &vtx[i].normal);
				vec3_scale(&normScale, &vtx[i].normal, VdotN * 2.0f);
				vec3_sub(&reflected, &normScale, &viewDir);

				vec3_tform(&viewReflected, &lib->modelView, &reflected, 0.0f); // This is nonsense..

				for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
				{
					dstVtx[i].tex[j].u = (reflected.x * 0.5f + 0.5f) * 256.0f * 65536.0f;
					dstVtx[i].tex[j].v = (reflected.y * 0.5f + 0.5f) * 256.0f * 65536.0f;
					dstVtx[i].tex[j].w = 1.0f;
				}
			}
		} break;
	}
#if PROFILE
	lib->profile.texgen += GetClocks() - texGenStart;
#endif
}

/*****************************************************************************/

void LoadMatrix(mat4 *mat __asm("a0"));
void TransformH(vec4 *dst __asm("a0"), vec3 *src __asm("a1"));

/*****************************************************************************/

void TransformVertexBuffer(struct MaggieTransVertex *dstVtx, struct MaggieVertex *vtx, UWORD nVerts, MaggieBase *lib)
{
#if PROFILE
	ULONG transStart = GetClocks();
#endif
	if(lib->dirtyMatrix)
	{
		lib->dirtyMatrix = 0;
		mat4_mul(&lib->modelView, &lib->viewMatrix, &lib->worldMatrix);
		mat4_mul(&lib->modelViewProj, &lib->perspectiveMatrix, &lib->modelView);
	}
#if 0
	LoadMatrix(&lib->modelViewProj);

	for(int i = 0; i < nVerts; ++i)
	{
		TransformH(&dstVtx[i].pos, &vtx[i].pos);
#else
	for(UWORD i = 0; i < nVerts; ++i)
	{
		vec3_tformh(&dstVtx[i].pos, &lib->modelViewProj, &vtx[i].pos, 1.0f);
#endif
		dstVtx[i].colour = vtx[i].colour;
	}
#if PROFILE
	lib->profile.trans += GetClocks() - transStart;
#endif
}

/*****************************************************************************/

void PrepareVertexBuffer(struct MaggieTransVertex *transDst, struct MaggieVertex *vtx, UWORD nVerts)
{
	for(UWORD i = 0; i < nVerts; ++i)
	{
		for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
		{
			vtx[i].tex[j].u = vtx[i].tex[j].u * 256.0f * 65536.0f;
			vtx[i].tex[j].v = vtx[i].tex[j].v * 256.0f * 65536.0f;
			vtx[i].tex[j].w = vtx[i].tex[j].w;
			transDst[i].tex[j].u = vtx[i].tex[j].u;
			transDst[i].tex[j].v = vtx[i].tex[j].v;
		}
		ULONG gray = RGBToGrayScale(vtx[i].colour);
		transDst[i].colour = vtx[i].colour = gray + (gray << 8);
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


