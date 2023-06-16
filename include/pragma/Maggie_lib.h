#ifndef _INCLUDE_PRAGMA_MAGGIE_LIB_H
#define _INCLUDE_PRAGMA_MAGGIE_LIB_H

#ifndef CLIB_MAGGIE_PROTOS_H
#include <clib/Maggie_protos.h>
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(MaggieBase,0x01e,magSetScreenMemory(a0,d0,d1))
#pragma amicall(MaggieBase,0x024,magSetTexture(d0,d1))
#pragma amicall(MaggieBase,0x02a,magSetDrawMode(d0))
#pragma amicall(MaggieBase,0x030,magSetRGB(d0))
#pragma amicall(MaggieBase,0x036,magGetDepthBuffer())
#pragma amicall(MaggieBase,0x03c,magSetWorldMatrix(a0))
#pragma amicall(MaggieBase,0x042,magSetViewMatrix(a0))
#pragma amicall(MaggieBase,0x048,magSetPerspectiveMatrix(a0))
#pragma amicall(MaggieBase,0x04e,magDrawTrianglesUP(a0,d0))
#pragma amicall(MaggieBase,0x054,magDrawIndexedTrianglesUP(a0,d0,a1,d1))
#pragma amicall(MaggieBase,0x05a,magDrawIndexedPolygonsUP(a0,d0,a1,d1))
#pragma amicall(MaggieBase,0x060,magSetVertexBuffer(d0))
#pragma amicall(MaggieBase,0x066,magSetIndexBuffer(d0))
#pragma amicall(MaggieBase,0x06c,magDrawTriangles(d0,d1))
#pragma amicall(MaggieBase,0x072,magDrawIndexedTriangles(d0,d1,d2,d3))
#pragma amicall(MaggieBase,0x078,magDrawIndexedPolygons(d0,d1,d2,d3))
#pragma amicall(MaggieBase,0x07e,magDrawLinearSpan(a0,a1))
#pragma amicall(MaggieBase,0x084,magDrawSpan(a0,a1))
#pragma amicall(MaggieBase,0x08a,magAllocateVertexBuffer(d0))
#pragma amicall(MaggieBase,0x090,magUploadVertexBuffer(d0,a0,d1,d2))
#pragma amicall(MaggieBase,0x096,magFreeVertexBuffer(d0))
#pragma amicall(MaggieBase,0x09c,magAllocateIndexBuffer(d0))
#pragma amicall(MaggieBase,0x0a2,magUploadIndexBuffer(d0,a0,d1,d2))
#pragma amicall(MaggieBase,0x0a8,magFreeIndexBuffer(d0))
#pragma amicall(MaggieBase,0x0ae,magAllocateTexture(d0))
#pragma amicall(MaggieBase,0x0b4,magUploadTexture(d0,d1,a0,d2))
#pragma amicall(MaggieBase,0x0ba,magFreeTexture(d0))
#pragma amicall(MaggieBase,0x0c0,magBeginScene())
#pragma amicall(MaggieBase,0x0c6,magEndScene())
#pragma amicall(MaggieBase,0x0cc,magBegin())
#pragma amicall(MaggieBase,0x0d2,magEnd())
#pragma amicall(MaggieBase,0x0d8,magVertex(fp0,fp1,fp2))
#pragma amicall(MaggieBase,0x0de,magNormal(fp0,fp1,fp2))
#pragma amicall(MaggieBase,0x0e4,magTexCoord(d0,fp0,fp1))
#pragma amicall(MaggieBase,0x0ea,magTexCoord3(d0,fp0,fp1,fp2))
#pragma amicall(MaggieBase,0x0f0,magColour(d0))
#pragma amicall(MaggieBase,0x0f6,magClear(d0))
#pragma amicall(MaggieBase,0x0fc,magSetLightType(d0,d1))
#pragma amicall(MaggieBase,0x102,magSetLightPosition(d0,fp0,fp1,fp2))
#pragma amicall(MaggieBase,0x108,magSetLightDirection(d0,fp0,fp1,fp2))
#pragma amicall(MaggieBase,0x10e,magSetLightCone(d0,fp0))
#pragma amicall(MaggieBase,0x114,magSetLightAttenuation(d0,fp0))
#pragma amicall(MaggieBase,0x11a,magSetLightColour(d0,d1))
#pragma amicall(MaggieBase,0x120,magClearColour(d0))
#pragma amicall(MaggieBase,0x126,magClearDepth(d0))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall MaggieBase magSetScreenMemory     01e 10803
#pragma  libcall MaggieBase magSetTexture          024 1002
#pragma  libcall MaggieBase magSetDrawMode         02a 001
#pragma  libcall MaggieBase magSetRGB              030 001
#pragma  libcall MaggieBase magGetDepthBuffer      036 00
#pragma  libcall MaggieBase magSetWorldMatrix      03c 801
#pragma  libcall MaggieBase magSetViewMatrix       042 801
#pragma  libcall MaggieBase magSetPerspectiveMatrix 048 801
#pragma  libcall MaggieBase magDrawTrianglesUP     04e 0802
#pragma  libcall MaggieBase magDrawIndexedTrianglesUP 054 190804
#pragma  libcall MaggieBase magDrawIndexedPolygonsUP 05a 190804
#pragma  libcall MaggieBase magSetVertexBuffer     060 001
#pragma  libcall MaggieBase magSetIndexBuffer      066 001
#pragma  libcall MaggieBase magDrawTriangles       06c 1002
#pragma  libcall MaggieBase magDrawIndexedTriangles 072 321004
#pragma  libcall MaggieBase magDrawIndexedPolygons 078 321004
#pragma  libcall MaggieBase magDrawLinearSpan      07e 9802
#pragma  libcall MaggieBase magDrawSpan            084 9802
#pragma  libcall MaggieBase magAllocateVertexBuffer 08a 001
#pragma  libcall MaggieBase magUploadVertexBuffer  090 218004
#pragma  libcall MaggieBase magFreeVertexBuffer    096 001
#pragma  libcall MaggieBase magAllocateIndexBuffer 09c 001
#pragma  libcall MaggieBase magUploadIndexBuffer   0a2 218004
#pragma  libcall MaggieBase magFreeIndexBuffer     0a8 001
#pragma  libcall MaggieBase magAllocateTexture     0ae 001
#pragma  libcall MaggieBase magUploadTexture       0b4 281004
#pragma  libcall MaggieBase magFreeTexture         0ba 001
#pragma  libcall MaggieBase magBeginScene          0c0 00
#pragma  libcall MaggieBase magEndScene            0c6 00
#pragma  libcall MaggieBase magBegin               0cc 00
#pragma  libcall MaggieBase magEnd                 0d2 00
#pragma flibcall MaggieBase magVertex              0d8 1211100003
#pragma flibcall MaggieBase magNormal              0de 1211100003
#pragma flibcall MaggieBase magTexCoord            0e4 1110000003
#pragma flibcall MaggieBase magTexCoord3           0ea 121110000004
#pragma  libcall MaggieBase magColour              0f0 001
#pragma  libcall MaggieBase magClear               0f6 001
#pragma  libcall MaggieBase magSetLightType        0fc 1002
#pragma flibcall MaggieBase magSetLightPosition    102 121110000004
#pragma flibcall MaggieBase magSetLightDirection   108 121110000004
#pragma flibcall MaggieBase magSetLightCone        10e 10000002
#pragma flibcall MaggieBase magSetLightAttenuation 114 10000002
#pragma  libcall MaggieBase magSetLightColour      11a 1002
#pragma  libcall MaggieBase magClearColour         120 001
#pragma  libcall MaggieBase magClearDepth          126 001
#endif

#endif	/*  _INCLUDE_PRAGMA_MAGGIE_LIB_H  */
