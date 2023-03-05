#ifndef MAGGIE_FLAGS_H_INCLUDED
#define MAGGIE_FLAGS_H_INCLUDED

/*****************************************************************************/

#define MAG_DRAWMODE_NORMAL			0x0000
#define MAG_DRAWMODE_DEPTHBUFFER	0x0001
#define MAG_DRAWMODE_BILINEAR		0x0002
#define MAG_DRAWMODE_32BIT			0x0004
#define MAG_DRAWMODE_LIGHTING		0x0008
#define MAG_DRAWMODE_CULL_CCW		0x0010

/*****************************************************************************/

#define MAG_CLEAR_COLOUR 0x0001
#define MAG_CLEAR_DEPTH 0x0002

/*****************************************************************************/

#define MAG_MAX_LIGHTS			8

#define MAG_LIGHT_OFF			0x0000
#define MAG_LIGHT_POINT			0x0001
#define MAG_LIGHT_DIRECTIONAL	0x0002
#define MAG_LIGHT_SPOT			0x0003
#define MAG_LIGHT_AMBIENT		0x0004

/*****************************************************************************/

/* MAGGIE_FLAGS_H_INCLUDED */
#endif
