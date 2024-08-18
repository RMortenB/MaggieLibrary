#ifndef _INLINE_MAGGIE_H
#define _INLINE_MAGGIE_H

#ifndef CLIB_MAGGIE_PROTOS_H
#define CLIB_MAGGIE_PROTOS_H
#endif

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef MAGGIE_BASE_NAME
#define MAGGIE_BASE_NAME MaggieBase
#endif

#define magSetScreenMemory(pixels, xres, yres) \
	LP3NR(0x1e, magSetScreenMemory, APTR *, pixels, a0, UWORD, xres, d0, UWORD, yres, d1, \
	, MAGGIE_BASE_NAME)

#define magSetTexture(unit, data) \
	LP2NR(0x24, magSetTexture, UWORD, unit, d0, UWORD, data, d1, \
	, MAGGIE_BASE_NAME)

#define magSetDrawMode(mode) \
	LP1NR(0x2a, magSetDrawMode, UWORD, mode, d0, \
	, MAGGIE_BASE_NAME)

#define magSetRGB(rgb) \
	LP1NR(0x30, magSetRGB, ULONG, rgb, d0, \
	, MAGGIE_BASE_NAME)

#define magGetDepthBuffer() \
	LP0(0x36, UWORD *, magGetDepthBuffer, \
	, MAGGIE_BASE_NAME)

#define magSetWorldMatrix(matrix) \
	LP1NR(0x3c, magSetWorldMatrix, float *, matrix, a0, \
	, MAGGIE_BASE_NAME)

#define magSetViewMatrix(matrix) \
	LP1NR(0x42, magSetViewMatrix, float *, matrix, a0, \
	, MAGGIE_BASE_NAME)

#define magSetPerspectiveMatrix(matrix) \
	LP1NR(0x48, magSetPerspectiveMatrix, float *, matrix, a0, \
	, MAGGIE_BASE_NAME)

#define magDrawTrianglesUP(vtx, nVerts) \
	LP2NR(0x4e, magDrawTrianglesUP, struct MaggieVertex *, vtx, a0, UWORD, nVerts, d0, \
	, MAGGIE_BASE_NAME)

#define magDrawIndexedTrianglesUP(vtx, nVtx, indx, nIndx) \
	LP4NR(0x54, magDrawIndexedTrianglesUP, struct MaggieVertex *, vtx, a0, UWORD, nVtx, d0, UWORD *, indx, a1, UWORD, nIndx, d1, \
	, MAGGIE_BASE_NAME)

#define magDrawIndexedPolygonsUP(vtx, nVtx, indx, nIndx) \
	LP4NR(0x5a, magDrawIndexedPolygonsUP, struct MaggieVertex *, vtx, a0, UWORD, nVtx, d0, UWORD *, indx, a1, UWORD, nIndx, d1, \
	, MAGGIE_BASE_NAME)

#define magSetVertexBuffer(vBuffer) \
	LP1NR(0x60, magSetVertexBuffer, WORD, vBuffer, d0, \
	, MAGGIE_BASE_NAME)

#define magSetIndexBuffer(iBuffer) \
	LP1NR(0x66, magSetIndexBuffer, WORD, iBuffer, d0, \
	, MAGGIE_BASE_NAME)

#define magDrawTriangles(startVtx, nVtx) \
	LP2NR(0x6c, magDrawTriangles, UWORD, startVtx, d0, UWORD, nVtx, d1, \
	, MAGGIE_BASE_NAME)

#define magDrawIndexedTriangles(firstVtx, nVtx, startIndx, nIndx) \
	LP4NR(0x72, magDrawIndexedTriangles, UWORD, firstVtx, d0, UWORD, nVtx, d1, UWORD, startIndx, d2, UWORD, nIndx, d3, \
	, MAGGIE_BASE_NAME)

#define magDrawIndexedPolygons(firstVtx, nVtx, startIndx, nIndx) \
	LP4NR(0x78, magDrawIndexedPolygons, UWORD, firstVtx, d0, UWORD, nVtx, d1, UWORD, startIndx, d2, UWORD, nIndx, d3, \
	, MAGGIE_BASE_NAME)

#define magDrawLinearSpan(start, end) \
	LP2NR(0x7e, magDrawLinearSpan, struct SpanPosition *, start, a0, struct SpanPosition *, end, a1, \
	, MAGGIE_BASE_NAME)

#define magDrawSpan(start, end) \
	LP2NR(0x84, magDrawSpan, struct MaggieClippedVertex *, start, a0, struct MaggieClippedVertex *, end, a1, \
	, MAGGIE_BASE_NAME)

#define magAllocateVertexBuffer(nVerts) \
	LP1(0x8a, UWORD, magAllocateVertexBuffer, UWORD, nVerts, d0, \
	, MAGGIE_BASE_NAME)

#define magUploadVertexBuffer(vBuffer, vtx, startVtx, nVerts) \
	LP4NR(0x90, magUploadVertexBuffer, UWORD, vBuffer, d0, struct MaggieVertex *, vtx, a0, UWORD, startVtx, d1, UWORD, nVerts, d2, \
	, MAGGIE_BASE_NAME)

#define magFreeVertexBuffer(vBuffer) \
	LP1NR(0x96, magFreeVertexBuffer, UWORD, vBuffer, d0, \
	, MAGGIE_BASE_NAME)

#define magAllocateIndexBuffer(nIndx) \
	LP1(0x9c, UWORD, magAllocateIndexBuffer, UWORD, nIndx, d0, \
	, MAGGIE_BASE_NAME)

#define magUploadIndexBuffer(iBuffer, indx, startIndx, nIndx) \
	LP4NR(0xa2, magUploadIndexBuffer, UWORD, iBuffer, d0, UWORD *, indx, a0, UWORD, startIndx, d1, UWORD, nIndx, d2, \
	, MAGGIE_BASE_NAME)

#define magFreeIndexBuffer(iBuffer) \
	LP1NR(0xa8, magFreeIndexBuffer, UWORD, iBuffer, d0, \
	, MAGGIE_BASE_NAME)

#define magAllocateTexture(size) \
	LP1(0xae, UWORD, magAllocateTexture, UWORD, size, d0, \
	, MAGGIE_BASE_NAME)

#define magUploadTexture(txtr, mipmap, data, format) \
	LP4NR(0xb4, magUploadTexture, UWORD, txtr, d0, UWORD, mipmap, d1, APTR, data, a0, UWORD, format, d2, \
	, MAGGIE_BASE_NAME)

#define magFreeTexture(txtr) \
	LP1NR(0xba, magFreeTexture, UWORD, txtr, d0, \
	, MAGGIE_BASE_NAME)

#define magBeginScene() \
	LP0NR(0xc0, magBeginScene, \
	, MAGGIE_BASE_NAME)

#define magEndScene() \
	LP0NR(0xc6, magEndScene, \
	, MAGGIE_BASE_NAME)

#define magBegin() \
	LP0NR(0xcc, magBegin, \
	, MAGGIE_BASE_NAME)

#define magEnd() \
	LP0NR(0xd2, magEnd, \
	, MAGGIE_BASE_NAME)

#define magVertex(x, y, z) \
	LP3NR(0xd8, magVertex, float, x, fp0, float, y, fp1, float, z, fp2, \
	, MAGGIE_BASE_NAME)

#define magNormal(x, y, z) \
	LP3NR(0xde, magNormal, float, x, fp0, float, y, fp1, float, z, fp2, \
	, MAGGIE_BASE_NAME)

#define magTexCoord(texReg, u, v) \
	LP3NR(0xe4, magTexCoord, int, texReg, d0, float, u, fp0, float, v, fp1, \
	, MAGGIE_BASE_NAME)

#define magTexCoord3(texReg, u, v, w) \
	LP4NR(0xea, magTexCoord3, int, texReg, d0, float, u, fp0, float, v, fp1, float, w, fp2, \
	, MAGGIE_BASE_NAME)

#define magColour(col) \
	LP1NR(0xf0, magColour, ULONG, col, d0, \
	, MAGGIE_BASE_NAME)

#define magClear(buffers) \
	LP1NR(0xf6, magClear, UWORD, buffers, d0, \
	, MAGGIE_BASE_NAME)

#define magSetLightType(light, colour) \
	LP2NR(0xfc, magSetLightType, UWORD, light, d0, UWORD, colour, d1, \
	, MAGGIE_BASE_NAME)

#define magSetLightPosition(light, x, y, z) \
	LP4NR(0x102, magSetLightPosition, UWORD, light, d0, float, x, fp0, float, y, fp1, float, z, fp2, \
	, MAGGIE_BASE_NAME)

#define magSetLightDirection(light, x, y, z) \
	LP4NR(0x108, magSetLightDirection, UWORD, light, d0, float, x, fp0, float, y, fp1, float, z, fp2, \
	, MAGGIE_BASE_NAME)

#define magSetLightCone(light, phi) \
	LP2NR(0x10e, magSetLightCone, UWORD, light, d0, float, phi, fp0, \
	, MAGGIE_BASE_NAME)

#define magSetLightAttenuation(light, attenuation) \
	LP2NR(0x114, magSetLightAttenuation, UWORD, light, d0, float, attenuation, fp0, \
	, MAGGIE_BASE_NAME)

#define magSetLightColour(light, colour) \
	LP2NR(0x11a, magSetLightColour, UWORD, light, d0, ULONG, colour, d1, \
	, MAGGIE_BASE_NAME)

#define magClearColour(colour) \
	LP1NR(0x120, magClearColour, ULONG, colour, d0, \
	, MAGGIE_BASE_NAME)

#define magClearDepth(depth) \
	LP1NR(0x126, magClearDepth, UWORD, depth, d0, \
	, MAGGIE_BASE_NAME)

#define magScissor(x0, y0, x1, y1) \
	LP4NR(0x12c, magScissor, UWORD, x0, d0, UWORD, y0, d1, UWORD, x1, d2, UWORD, y1, d3, \
	, MAGGIE_BASE_NAME)

#endif /*  _INLINE_MAGGIE_H  */
