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
	vec3 normal;
	struct MaggieTexCoord tex[MAGGIE_MAX_TEXCOORDS];
	ULONG colour;
};

/*****************************************************************************/

struct MaggieTransVertex
{
	vec4 pos;
	struct MaggieTexCoord tex[MAGGIE_MAX_TEXCOORDS];
	ULONG colour;
};

/*****************************************************************************/

struct SpanPosition
{
	ULONG u, v;
	UWORD depth;
	UWORD intensity;
};

/* MAGGIE_VERTEX_H_INCLUDED */
#endif
