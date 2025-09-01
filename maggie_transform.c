#include "maggie_internal.h"
#include "maggie_debug.h"
#include <proto/graphics.h>
#include <math.h>
#include <string.h>

/*****************************************************************************/
/*****************************************************************************/

static void DecompNormal(vec3 *dest, MaggieNormal *n)
{
	dest->x = n->x / 256.0f;
	dest->y = n->y / 256.0f;
	dest->z = n->z / 256.0f;
}

/*****************************************************************************/

void TexGenBuffer(VertexBufferMemory *src, int startVtx, int nVerts, MaggieBase *lib)
{
	struct MaggieTransVertex *dstVtx = src->transVerts;
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
					dstVtx[i + startVtx].tex[j].u = src->uvs[i + startVtx].u;
					dstVtx[i + startVtx].tex[j].v = src->uvs[i + startVtx].v;
					dstVtx[i + startVtx].tex[j].w = src->uvs[i + startVtx].w;
				}
			}
		} break;
		case MAG_DRAWMODE_TEXGEN_POS :
		{
			for(UWORD i = 0; i < nVerts; ++i)
			{
				for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
				{
					dstVtx[i + startVtx].tex[j].u = src->positions[i + startVtx].x * 256.0f * 65536.0f;
					dstVtx[i + startVtx].tex[j].v = src->positions[i + startVtx].y * 256.0f * 65536.0f;
					dstVtx[i + startVtx].tex[j].w = 1.0f;
				}
			}
		} break;
		case MAG_DRAWMODE_TEXGEN_NORMAL :
		{
			for(UWORD i = 0; i < nVerts; ++i)
			{
				vec3 viewNormal, normal;
				DecompNormal(&normal, &src->normals[i + startVtx]);
				vec3_tform(&viewNormal, &lib->modelView, &normal, 0.0f);
				for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
				{
					dstVtx[i + startVtx].tex[j].u = (viewNormal.x * 0.5f + 0.5f) * 256.0f * 65536.0f;
					dstVtx[i + startVtx].tex[j].v = (viewNormal.y * 0.5f + 0.5f) * 256.0f * 65536.0f;
					dstVtx[i + startVtx].tex[j].w = 1.0f;
				}
			}
		} break;
		case MAG_DRAWMODE_TEXGEN_REFLECT :
		{
			vec3 viewNormal, viewPos;
			for(UWORD i = 0; i < nVerts; ++i)
			{
				vec3 reflected;
				vec3 viewReflected;
				vec3 normScale;
				vec3 normal;
				DecompNormal(&normal, &src->normals[i + startVtx]);
				vec3_tform(&viewNormal, &lib->modelView, &normal, 0.0f);
				vec3_tform(&viewPos, &lib->modelView, &src->positions[i + startVtx], 1.0f);

				float VdotN = vec3_dot(&viewPos, &viewNormal);
				vec3_scale(&normScale, &viewNormal, VdotN * 2.0f);
				vec3_add(&reflected, &viewPos, &normScale);

				vec3_normalise(&viewReflected, &reflected);

				for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
				{
					dstVtx[i + startVtx].tex[j].u = (viewReflected.x * 0.5f + 0.5f) * 256.0f * 65536.0f;
					dstVtx[i + startVtx].tex[j].v = (viewReflected.y * 0.5f + 0.5f) * 256.0f * 65536.0f;
					dstVtx[i + startVtx].tex[j].w = 1.0f;
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

static void updateMatrices(MaggieBase *lib)
{
	if(lib->dirtyMatrix)
	{
		lib->dirtyMatrix = 0;
		mat4_mul(&lib->modelView, &lib->viewMatrix, &lib->worldMatrix);
		mat4_mul(&lib->modelViewProj, &lib->perspectiveMatrix, &lib->modelView);
	}
}

/*****************************************************************************/

void TransformVertexPositions(struct MaggieTransVertex * restrict dstVtx, vec3 * restrict vtx, UWORD nVerts, MaggieBase *lib)
{
#if PROFILE
	ULONG transStart = GetClocks();
#endif
	updateMatrices(lib);
	for(UWORD i = 0; i < nVerts; ++i)
	{
		vec3_tformh(&dstVtx[i].pos, &lib->modelViewProj, &vtx[i], 1.0f);
	}
#if PROFILE
	lib->profile.trans += GetClocks() - transStart;
#endif
}

/*****************************************************************************/

void TransformSpriteBuffer(struct MaggieSpriteVertex * restrict dstVtx, struct MaggieSpriteVertex * restrict vtx, UWORD nVerts, MaggieBase *lib)
{
#if PROFILE
	ULONG transStart = GetClocks();
#endif
	updateMatrices(lib);
	for(UWORD i = 0; i < nVerts; ++i)
	{
		vec3_tform(&dstVtx[i].pos, &lib->modelView, &vtx[i].pos, 1.0f);
		dstVtx[i].colour = vtx[i].colour;
	}
#if PROFILE
	lib->profile.trans += GetClocks() - transStart;
#endif
}

/*****************************************************************************/

void TransformToSpriteBuffer(struct MaggieSpriteVertex * restrict dstVtx, vec3 * restrict vtx, UWORD nVerts, MaggieBase *lib)
{
#if PROFILE
	ULONG transStart = GetClocks();
#endif
	updateMatrices(lib);
	for(UWORD i = 0; i < nVerts; ++i)
	{
		vec3_tform(&dstVtx[i].pos, &lib->modelView, &vtx[i], 1.0f);
	}
#if PROFILE
	lib->profile.trans += GetClocks() - transStart;
#endif
}

/*****************************************************************************/

void PrepareVertexBuffer(struct MaggieTransVertex * restrict transDst, struct MaggieVertex * restrict vtx, UWORD nVerts)
{
	for(UWORD i = 0; i < nVerts; ++i)
	{
		for(UWORD j = 0; j < MAGGIE_MAX_TEXCOORDS; ++j)
		{
			vtx[i].tex[j].u = vtx[i].tex[j].u * 256.0f * 65536.0f;
			vtx[i].tex[j].v = vtx[i].tex[j].v * 256.0f * 65536.0f;
			transDst[i].tex[j].u = vtx[i].tex[j].u;
			transDst[i].tex[j].v = vtx[i].tex[j].v;
			transDst[i].tex[j].w = vtx[i].tex[j].w;
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

void magScissor(REG(d0, UWORD x0), REG(d1, UWORD y0), REG(d2, UWORD x1), REG(d3, UWORD y1), REG(a6, MaggieBase *lib))
{
	if(x0 < 0)
		x0 = 0;
	if(y0 < 0)
		y0 = 0;
	if(x1 >= lib->xres)
		x1 = lib->xres - 1;
	if(y1 >= lib->yres)
		y1 = lib->yres - 1;

	lib->scissor.x0 = x0;
	lib->scissor.y0 = y0;
	lib->scissor.x1 = x1;
	lib->scissor.y1 = y1;
}

/*****************************************************************************/

