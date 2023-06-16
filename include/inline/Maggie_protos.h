#ifndef _VBCCINLINE_MAGGIE_H
#define _VBCCINLINE_MAGGIE_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

void __magSetScreenMemory(__reg("a6") struct Library *, __reg("a0") APTR * pixels, __reg("d0") UWORD xres, __reg("d1") UWORD yres)="\tjsr\t-30(a6)";
#define magSetScreenMemory(pixels, xres, yres) __magSetScreenMemory(MaggieBase, (pixels), (xres), (yres))

void __magSetTexture(__reg("a6") struct Library *, __reg("d0") UWORD unit, __reg("d1") UWORD data)="\tjsr\t-36(a6)";
#define magSetTexture(unit, data) __magSetTexture(MaggieBase, (unit), (data))

void __magSetDrawMode(__reg("a6") struct Library *, __reg("d0") UWORD mode)="\tjsr\t-42(a6)";
#define magSetDrawMode(mode) __magSetDrawMode(MaggieBase, (mode))

void __magSetRGB(__reg("a6") struct Library *, __reg("d0") ULONG rgb)="\tjsr\t-48(a6)";
#define magSetRGB(rgb) __magSetRGB(MaggieBase, (rgb))

UWORD * __magGetDepthBuffer(__reg("a6") struct Library *)="\tjsr\t-54(a6)";
#define magGetDepthBuffer() __magGetDepthBuffer(MaggieBase)

void __magSetWorldMatrix(__reg("a6") struct Library *, __reg("a0") float * matrix)="\tjsr\t-60(a6)";
#define magSetWorldMatrix(matrix) __magSetWorldMatrix(MaggieBase, (matrix))

void __magSetViewMatrix(__reg("a6") struct Library *, __reg("a0") float * matrix)="\tjsr\t-66(a6)";
#define magSetViewMatrix(matrix) __magSetViewMatrix(MaggieBase, (matrix))

void __magSetPerspectiveMatrix(__reg("a6") struct Library *, __reg("a0") float * matrix)="\tjsr\t-72(a6)";
#define magSetPerspectiveMatrix(matrix) __magSetPerspectiveMatrix(MaggieBase, (matrix))

void __magDrawTrianglesUP(__reg("a6") struct Library *, __reg("a0") struct MaggieVertex * vtx, __reg("d0") UWORD nVerts)="\tjsr\t-78(a6)";
#define magDrawTrianglesUP(vtx, nVerts) __magDrawTrianglesUP(MaggieBase, (vtx), (nVerts))

void __magDrawIndexedTrianglesUP(__reg("a6") struct Library *, __reg("a0") struct MaggieVertex * vtx, __reg("d0") UWORD nVtx, __reg("a1") UWORD * indx, __reg("d1") UWORD nIndx)="\tjsr\t-84(a6)";
#define magDrawIndexedTrianglesUP(vtx, nVtx, indx, nIndx) __magDrawIndexedTrianglesUP(MaggieBase, (vtx), (nVtx), (indx), (nIndx))

void __magDrawIndexedPolygonsUP(__reg("a6") struct Library *, __reg("a0") struct MaggieVertex * vtx, __reg("d0") UWORD nVtx, __reg("a1") UWORD * indx, __reg("d1") UWORD nIndx)="\tjsr\t-90(a6)";
#define magDrawIndexedPolygonsUP(vtx, nVtx, indx, nIndx) __magDrawIndexedPolygonsUP(MaggieBase, (vtx), (nVtx), (indx), (nIndx))

void __magSetVertexBuffer(__reg("a6") struct Library *, __reg("d0") WORD vBuffer)="\tjsr\t-96(a6)";
#define magSetVertexBuffer(vBuffer) __magSetVertexBuffer(MaggieBase, (vBuffer))

void __magSetIndexBuffer(__reg("a6") struct Library *, __reg("d0") WORD iBuffer)="\tjsr\t-102(a6)";
#define magSetIndexBuffer(iBuffer) __magSetIndexBuffer(MaggieBase, (iBuffer))

void __magDrawTriangles(__reg("a6") struct Library *, __reg("d0") UWORD startVtx, __reg("d1") UWORD nVtx)="\tjsr\t-108(a6)";
#define magDrawTriangles(startVtx, nVtx) __magDrawTriangles(MaggieBase, (startVtx), (nVtx))

void __magDrawIndexedTriangles(__reg("a6") struct Library *, __reg("d0") UWORD firstVtx, __reg("d1") UWORD nVtx, __reg("d2") UWORD startIndx, __reg("d3") UWORD nIndx)="\tjsr\t-114(a6)";
#define magDrawIndexedTriangles(firstVtx, nVtx, startIndx, nIndx) __magDrawIndexedTriangles(MaggieBase, (firstVtx), (nVtx), (startIndx), (nIndx))

void __magDrawIndexedPolygons(__reg("a6") struct Library *, __reg("d0") UWORD firstVtx, __reg("d1") UWORD nVtx, __reg("d2") UWORD startIndx, __reg("d3") UWORD nIndx)="\tjsr\t-120(a6)";
#define magDrawIndexedPolygons(firstVtx, nVtx, startIndx, nIndx) __magDrawIndexedPolygons(MaggieBase, (firstVtx), (nVtx), (startIndx), (nIndx))

void __magDrawLinearSpan(__reg("a6") struct Library *, __reg("a0") struct SpanPosition * start, __reg("a1") struct SpanPosition * end)="\tjsr\t-126(a6)";
#define magDrawLinearSpan(start, end) __magDrawLinearSpan(MaggieBase, (start), (end))

void __magDrawSpan(__reg("a6") struct Library *, __reg("a0") struct MaggieClippedVertex * start, __reg("a1") struct MaggieClippedVertex * end)="\tjsr\t-132(a6)";
#define magDrawSpan(start, end) __magDrawSpan(MaggieBase, (start), (end))

UWORD __magAllocateVertexBuffer(__reg("a6") struct Library *, __reg("d0") UWORD nVerts)="\tjsr\t-138(a6)";
#define magAllocateVertexBuffer(nVerts) __magAllocateVertexBuffer(MaggieBase, (nVerts))

void __magUploadVertexBuffer(__reg("a6") struct Library *, __reg("d0") UWORD vBuffer, __reg("a0") struct MaggieVertex * vtx, __reg("d1") UWORD startVtx, __reg("d2") UWORD nVerts)="\tjsr\t-144(a6)";
#define magUploadVertexBuffer(vBuffer, vtx, startVtx, nVerts) __magUploadVertexBuffer(MaggieBase, (vBuffer), (vtx), (startVtx), (nVerts))

void __magFreeVertexBuffer(__reg("a6") struct Library *, __reg("d0") UWORD vBuffer)="\tjsr\t-150(a6)";
#define magFreeVertexBuffer(vBuffer) __magFreeVertexBuffer(MaggieBase, (vBuffer))

UWORD __magAllocateIndexBuffer(__reg("a6") struct Library *, __reg("d0") UWORD nIndx)="\tjsr\t-156(a6)";
#define magAllocateIndexBuffer(nIndx) __magAllocateIndexBuffer(MaggieBase, (nIndx))

void __magUploadIndexBuffer(__reg("a6") struct Library *, __reg("d0") UWORD iBuffer, __reg("a0") UWORD * indx, __reg("d1") UWORD startIndx, __reg("d2") UWORD nIndx)="\tjsr\t-162(a6)";
#define magUploadIndexBuffer(iBuffer, indx, startIndx, nIndx) __magUploadIndexBuffer(MaggieBase, (iBuffer), (indx), (startIndx), (nIndx))

void __magFreeIndexBuffer(__reg("a6") struct Library *, __reg("d0") UWORD iBuffer)="\tjsr\t-168(a6)";
#define magFreeIndexBuffer(iBuffer) __magFreeIndexBuffer(MaggieBase, (iBuffer))

UWORD __magAllocateTexture(__reg("a6") struct Library *, __reg("d0") UWORD size)="\tjsr\t-174(a6)";
#define magAllocateTexture(size) __magAllocateTexture(MaggieBase, (size))

void __magUploadTexture(__reg("a6") struct Library *, __reg("d0") UWORD txtr, __reg("d1") UWORD mipmap, __reg("a0") APTR data, __reg("d2") UWORD format)="\tjsr\t-180(a6)";
#define magUploadTexture(txtr, mipmap, data, format) __magUploadTexture(MaggieBase, (txtr), (mipmap), (data), (format))

void __magFreeTexture(__reg("a6") struct Library *, __reg("d0") UWORD txtr)="\tjsr\t-186(a6)";
#define magFreeTexture(txtr) __magFreeTexture(MaggieBase, (txtr))

void __magBeginScene(__reg("a6") struct Library *)="\tjsr\t-192(a6)";
#define magBeginScene() __magBeginScene(MaggieBase)

void __magEndScene(__reg("a6") struct Library *)="\tjsr\t-198(a6)";
#define magEndScene() __magEndScene(MaggieBase)

void __magBegin(__reg("a6") struct Library *)="\tjsr\t-204(a6)";
#define magBegin() __magBegin(MaggieBase)

void __magEnd(__reg("a6") struct Library *)="\tjsr\t-210(a6)";
#define magEnd() __magEnd(MaggieBase)

void __magVertex(__reg("a6") struct Library *, __reg("fp0") float x, __reg("fp1") float y, __reg("fp2") float z)="\tjsr\t-216(a6)";
#define magVertex(x, y, z) __magVertex(MaggieBase, (x), (y), (z))

void __magNormal(__reg("a6") struct Library *, __reg("fp0") float x, __reg("fp1") float y, __reg("fp2") float z)="\tjsr\t-222(a6)";
#define magNormal(x, y, z) __magNormal(MaggieBase, (x), (y), (z))

void __magTexCoord(__reg("a6") struct Library *, __reg("d0") int texReg, __reg("fp0") float u, __reg("fp1") float v)="\tjsr\t-228(a6)";
#define magTexCoord(texReg, u, v) __magTexCoord(MaggieBase, (texReg), (u), (v))

void __magTexCoord3(__reg("a6") struct Library *, __reg("d0") int texReg, __reg("fp0") float u, __reg("fp1") float v, __reg("fp2") float w)="\tjsr\t-234(a6)";
#define magTexCoord3(texReg, u, v, w) __magTexCoord3(MaggieBase, (texReg), (u), (v), (w))

void __magColour(__reg("a6") struct Library *, __reg("d0") ULONG col)="\tjsr\t-240(a6)";
#define magColour(col) __magColour(MaggieBase, (col))

void __magClear(__reg("a6") struct Library *, __reg("d0") UWORD buffers)="\tjsr\t-246(a6)";
#define magClear(buffers) __magClear(MaggieBase, (buffers))

void __magSetLightType(__reg("a6") struct Library *, __reg("d0") UWORD light, __reg("d1") UWORD colour)="\tjsr\t-252(a6)";
#define magSetLightType(light, colour) __magSetLightType(MaggieBase, (light), (colour))

void __magSetLightPosition(__reg("a6") struct Library *, __reg("d0") UWORD light, __reg("fp0") float x, __reg("fp1") float y, __reg("fp2") float z)="\tjsr\t-258(a6)";
#define magSetLightPosition(light, x, y, z) __magSetLightPosition(MaggieBase, (light), (x), (y), (z))

void __magSetLightDirection(__reg("a6") struct Library *, __reg("d0") UWORD light, __reg("fp0") float x, __reg("fp1") float y, __reg("fp2") float z)="\tjsr\t-264(a6)";
#define magSetLightDirection(light, x, y, z) __magSetLightDirection(MaggieBase, (light), (x), (y), (z))

void __magSetLightCone(__reg("a6") struct Library *, __reg("d0") UWORD light, __reg("fp0") float phi)="\tjsr\t-270(a6)";
#define magSetLightCone(light, phi) __magSetLightCone(MaggieBase, (light), (phi))

void __magSetLightAttenuation(__reg("a6") struct Library *, __reg("d0") UWORD light, __reg("fp0") float attenuation)="\tjsr\t-276(a6)";
#define magSetLightAttenuation(light, attenuation) __magSetLightAttenuation(MaggieBase, (light), (attenuation))

void __magSetLightColour(__reg("a6") struct Library *, __reg("d0") UWORD light, __reg("d1") ULONG colour)="\tjsr\t-282(a6)";
#define magSetLightColour(light, colour) __magSetLightColour(MaggieBase, (light), (colour))

void __magClearColour(__reg("a6") struct Library *, __reg("d0") ULONG colour)="\tjsr\t-288(a6)";
#define magClearColour(colour) __magClearColour(MaggieBase, (colour))

void __magClearDepth(__reg("a6") struct Library *, __reg("d0") UWORD depth)="\tjsr\t-294(a6)";
#define magClearDepth(depth) __magClearDepth(MaggieBase, (depth))

#endif /*  _VBCCINLINE_MAGGIE_H  */
