#include <math.h>
#include "maggie_internal.h"

/*****************************************************************************/

ULONG RGBToGrayScale(ULONG rgb)
{
	UBYTE r = (rgb >> 16) & 0xff;
	UBYTE g = (rgb >>  8) & 0xff;
	UBYTE b = (rgb >>  0) & 0xff;

	ULONG gray = (ULONG)(r * 0.299f + g * 0.587f + b * 0.114f);
	if(gray > 255)
		gray = 255;
	return gray;
}

/*****************************************************************************/

void magSetLightType(REG(d0, UWORD light), REG(d1, UWORD type), REG(a6, MaggieBase *lib))
{
	if(light >= MAG_MAX_LIGHTS)
		return;
	lib->lights[light].type = type;
}

/*****************************************************************************/

void magSetLightPosition(REG(d0, UWORD light), REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(a6, MaggieBase *lib))
{
	if(light >= MAG_MAX_LIGHTS)
		return;
	lib->lights[light].pos.x = x;
	lib->lights[light].pos.y = y;
	lib->lights[light].pos.z = z;
}

/*****************************************************************************/

void magSetLightDirection(REG(d0, UWORD light), REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(a6, MaggieBase *lib))
{
	if(light >= MAG_MAX_LIGHTS)
		return;
	lib->lights[light].dir.x = x;
	lib->lights[light].dir.y = y;
	lib->lights[light].dir.z = z;
}

/*****************************************************************************/

void magSetLightCone(REG(d0, UWORD light), REG(fp0, float phi), REG(a6, MaggieBase *lib))
{
	if(light >= MAG_MAX_LIGHTS)
		return;
	lib->lights[light].phi = phi;
}

/*****************************************************************************/

void magSetLightAttenuation(REG(d0, UWORD light), REG(fp0, float attenuation), REG(a6, MaggieBase *lib))
{
	if(light >= MAG_MAX_LIGHTS)
		return;
	lib->lights[light].attenuation = attenuation;
}

/*****************************************************************************/

void magSetLightColour(REG(d0, UWORD light), REG(d1, ULONG colour), REG(a6, MaggieBase *lib))
{
	if(light >= MAG_MAX_LIGHTS)
		return;
	lib->lights[light].colour = RGBToGrayScale(colour);
}

/*****************************************************************************/

static void DecompNormal(vec3 *dest, MaggieNormal *n)
{
	dest->x = n->x / 256.0f;
	dest->y = n->y / 256.0f;
	dest->z = n->z / 256.0f;
}

/*****************************************************************************/

static float my_cosf(float x)
{
	float c;

	__asm(
		"fmove.s %0, fp0\n\t"
		"fcos.x fp0\n\t"
		"fmove.s fp0,%1\n\t"
		: "=r"(c)
		: "r"(x)
		: "fp0", "cc"
	);
	return c;
}

void LightBuffer(VertexBufferMemory *src, int startVtx, int nVerts, MaggieBase *lib)
{
	struct MaggieTransVertex *dest = src->transVerts;
#if PROFILE
	ULONG lightStart = GetClocks();
#endif
	for(int i = 0; i < nVerts; ++i)
	{
		dest[i + startVtx].colour = 0;
	}
	mat4 iWorld;
	mat4_inverseLight(&iWorld, &lib->worldMatrix);
	for(int i = 0; i < MAG_MAX_LIGHTS; ++i)
	{
		switch(lib->lights[i].type)
		{
			case MAG_LIGHT_OFF :
			{
			} break;
			case MAG_LIGHT_POINT :
			{
				vec3 iLightPos;
				vec3 lDir;
				vec3 normal;
				vec3_tform(&iLightPos, &iWorld, &lib->lights[i].pos, 1.0f);
				float lightColour = lib->lights[i].colour;
				for(int j = 0; j < nVerts; ++j)
				{
					vec3_sub(&lDir, &iLightPos, &src->positions[startVtx + j]);
					float dist = vec3_normalise(&lDir, &lDir);
					DecompNormal(&normal, &src->normals[startVtx + j]);
					float lambert = vec3_dot(&lDir, &normal);
					if(lambert > 0.0f)
					{
						float attenuation = lib->lights[i].attenuation / (dist * dist);
						dest[j + startVtx].colour += (int)(src->colours[startVtx + j] * lambert * attenuation * lightColour) >> 8;
					}
				}
			} break;
			case MAG_LIGHT_DIRECTIONAL :
			{
				vec3 lDir;
				vec3 normal;
				float lightColour = lib->lights[i].colour;
				vec3_tform(&lDir, &iWorld, &lib->lights[i].dir, 0.0f);
				for(int j = 0; j < nVerts; ++j)
				{
					DecompNormal(&normal, &src->normals[startVtx + j]);
					float lambert = -vec3_dot(&lDir, &normal);
					if(lambert > 0.0f)
						dest[j + startVtx].colour += (int)(src->colours[startVtx + j] * lambert * lightColour) >> 8;
				}
			} break;
			case MAG_LIGHT_SPOT :
			{
				vec3 iLightPos;
				vec3 iLightDir;
				vec3 normal;
				vec3_tform(&iLightPos, &iWorld, &lib->lights[i].pos, 1.0f);
				vec3_tform(&iLightDir, &iWorld, &lib->lights[i].dir, 0.0f);

				float cosPhi = my_cosf(lib->lights[i].phi);
				float cosTheta = my_cosf((lib->lights[i].phi / 2.0f));
				vec3 lDir;
				float lightColour = lib->lights[i].colour;
				float attenuation = lib->lights[i].attenuation;
				for(int j = 0; j < nVerts; ++j)
				{
					vec3_sub(&lDir, &iLightPos, &src->positions[startVtx + j]);
					float dist = vec3_normalise(&lDir, &lDir);
					DecompNormal(&normal, &src->normals[startVtx + j]);
					float lambert = vec3_dot(&lDir, &normal);
					float cosAngle = vec3_dot(&iLightDir, &lDir);
					if(lambert > 0.0f)
					{
						if(cosAngle > cosPhi)
						{
							if(cosAngle < cosTheta)
							{
								lambert *= (cosAngle - cosPhi) / (cosTheta - cosPhi);
							}
							float att = attenuation / (dist * dist);
							dest[j + startVtx].colour += (int)(src->colours[startVtx + j] * lambert * att * lightColour) >> 8;
						}
					}
				}
			} break;
			case MAG_LIGHT_AMBIENT :
			{
				int lightColour = lib->lights[i].colour | (lib->lights[i].colour << 8);
				for(int j = 0; j < nVerts; ++j)
				{
					dest[j + startVtx].colour += lightColour;
				}
			} break;
		}
	}
	for(int i = 0; i < nVerts; ++i)
	{
		if(dest[i + startVtx].colour > 0xffff)
			dest[i + startVtx].colour = 0xffff;
	}
#if PROFILE
	lib->profile.light += GetClocks() - lightStart;
#endif
}

/*****************************************************************************/
