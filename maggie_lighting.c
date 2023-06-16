#include <math.h>
#include "maggie_internal.h"

/*****************************************************************************/

ULONG RGBToGrayScale(ULONG rgb)
{
	UBYTE r = rgb >> 16;
	UBYTE g = rgb >>  8;
	UBYTE b = rgb >>  0;

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

void LightBuffer(struct MaggieTransVertex *dest, struct MaggieVertex *src, int nVerts, MaggieBase *lib)
{
	for(int i = 0; i < nVerts; ++i)
	{
		dest[i].colour = 0;
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
				vec3_tform(&iLightPos, &iWorld, &lib->lights[i].pos, 1.0f);
				float lightColour = lib->lights[i].colour;
				for(int j = 0; j < nVerts; ++j)
				{
					vec3_sub(&lDir, &iLightPos, &src[j].pos);
					float dist = vec3_normalise(&lDir, &lDir);
					float lambert = vec3_dot(&lDir, &src[j].normal);
					if(lambert > 0.0f)
					{
						float attenuation = lib->lights[i].attenuation / (dist * dist);
						dest[j].colour += (int)(src[j].colour * lambert * attenuation * lightColour) >> 8;
					}
				}
			} break;
			case MAG_LIGHT_DIRECTIONAL :
			{
				vec3 iLightDir;
				float lightColour = lib->lights[i].colour;
				vec3_tform(&iLightDir, &iWorld, &lib->lights[i].dir, 0.0f);
				for(int j = 0; j < nVerts; ++j)
				{
					float lambert = -vec3_dot(&iLightDir, &src[j].normal);
					if(lambert > 0.0f)
						dest[j].colour += (int)(src[j].colour * lambert * lightColour) >> 8;
				}
			} break;
			case MAG_LIGHT_SPOT :
			{
				vec3 iLightPos;
				vec3 iLightDir;
				vec3_tform(&iLightPos, &iWorld, &lib->lights[i].pos, 1.0f);
				vec3_tform(&iLightDir, &iWorld, &lib->lights[i].dir, 0.0f);

				float cosPhi = cosf(lib->lights[i].phi);
				float cosTheta = cosf((lib->lights[i].phi) / 2.0f);
				float ooPmT = 1.0f / (cosTheta - cosPhi);
				vec3 lDir;
				float lightColour = lib->lights[i].colour;
				for(int j = 0; j < nVerts; ++j)
				{
					vec3_sub(&lDir, &iLightPos, &src[j].pos);
					float dist = vec3_normalise(&lDir, &lDir);
					float lambert = vec3_dot(&lDir, &src[j].normal);
					float cosAngle = vec3_dot(&iLightDir, &lDir);
					if(lambert > 0.0f)
					{
						if(cosAngle > cosPhi)
						{
							if(cosAngle < cosTheta)
							{
								lambert *= (cosAngle - cosPhi) * ooPmT;
							}
							float attenuation = lib->lights[i].attenuation / (dist * dist);
							dest[j].colour += (int)(src[j].colour * lambert * attenuation * lightColour) >> 8;
						}
					}
				}
			} break;
			case MAG_LIGHT_AMBIENT :
			{
				for(int j = 0; j < nVerts; ++j)
				{
					dest[j].colour += lib->lights[i].colour;
				}
			} break;
		}
	}
	for(int i = 0; i < nVerts; ++i)
	{
		if(dest[i].colour > 0xffff)
			dest[i].colour = 0xffff;
	}
}

/*****************************************************************************/
