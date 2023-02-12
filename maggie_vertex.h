#ifndef MAGGIE_VERTEX_H_INCLUDED
#define MAGGIE_VERTEX_H_INCLUDED

#include <exec/types.h>
#include "maggie_vec.h"

/*****************************************************************************/

struct MaggieVertex;
struct MaggieClippedVertex;
struct SpanPosition;

/*****************************************************************************/

#define MAG_MAX_POLYSIZE 256
#define MAGGIE_MAX_TEXCOORDS 1

/*****************************************************************************/

struct MaggieTexCoord
{
	float u, v, w;
};

/*****************************************************************************/

struct MaggieVertex
{
	vec3 pos;
	struct MaggieTexCoord tex[MAGGIE_MAX_TEXCOORDS];
	ULONG rgba;
};

/*****************************************************************************/

struct MaggieTransVertex
{
	vec4 pos;
	struct MaggieTexCoord tex[MAGGIE_MAX_TEXCOORDS];
	ULONG rgba;
};

/*****************************************************************************/

struct MaggieClippedVertex
{
	float xow, yow, zow, oow;
	ULONG uow, vow;
	ULONG cow;
};

/*****************************************************************************/

struct SpanPosition
{
	ULONG u, v;
	UWORD depth;
	UWORD intensity;
};


#endif // MAGGIE_VERTEX_H_INCLUDED
