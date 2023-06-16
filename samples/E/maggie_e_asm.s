	code

;	include "lvo/Maggie_lvo.i"

	XDEF MagInit
MagInit:
	lea		maggieName(pc),a1
	moveq	#0,d0
	move.l	4.w,a6
	jsr		-552(a6) ; OpenLibrary
	lea		maggieBase(pc),a0
	move.l	d0,(a0)
	rts

	XDEF MagClose
MagClose:
	move.l	4.w,a6
	move.l	maggieBase(pc),a1
	jsr		-402(a6) ; RemLibrary
	move.l	maggieBase(pc),a1
	jmp		-414(a6) ; CloseLibrary

;*****************************************************************************

maggieName: dc.b	'maggie.library',0
	align 4
maggieBase: dc.l	0

;*****************************************************************************
;*****************************************************************************

	XDEF MagSetScreenMemory__iii
MagSetScreenMemory__iii:
	move.l	maggieBase(pc),a6
	move.l	12(a7),a0
	move.l	8(a7),d0
	move.l	4(a7),d1
	jmp		-30(a6) ; _LVOmagSetScreenMemory

	XDEF MagSetTexture__ii
MagSetTexture__ii:
	move.l	maggieBase(pc),a6
	move.l	8(a7),d0
	move.l	4(a7),d1
	jmp		-36(a6) ; _LVOmagSetTexture

	XDEF MagSetDrawMode__i
MagSetDrawMode__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-42(a6) ; _LVOmagSetDrawMode

	XDEF MagSetRGB__i
MagSetRGB__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-48(a6) ; _LVOmagSetRGB

	XDEF MagGetDepthBuffer
MagGetDepthBuffer:
	move.l	maggieBase(pc),a6
	jmp		-54(a6) ; _LVOmagGetDepthBuffer

;*****************************************************************************

	XDEF MagSetWorldMatrix__i
MagSetWorldMatrix__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),a0
	jmp		-60(a6) ; _LVOmagSetWorldMatrix

	XDEF MagSetViewMatrix__i
MagSetViewMatrix__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),a0
	jmp		-66(a6) ; _LVOmagSetViewMatrix

	XDEF MagSetPerspectiveMatrix__i
MagSetPerspectiveMatrix__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),a0
	jmp		-72(a6) ; _LVOmagSetPerspectiveMatrix

;*****************************************************************************
	
	XDEF MagDrawTrianglesUP__ii
MagDrawTrianglesUP__ii:
	move.l	maggieBase(pc),a6
	move.l	8(a7),a0
	move.l	4(a7),d0
	jmp		-78(a6) ; _LVOmagDrawTrianglesUP

	XDEF MagDrawIndexedTrianglesUP__iiii
MagDrawIndexedTrianglesUP__iiii:
	move.l	maggieBase(pc),a6
	move.l	16(a7),a0
	move.l	12(a7),d0
	move.l	8(a7),a1
	move.l	4(a7),d1
	jmp		-84(a6) ; _LVOmagDrawIndexedTrianglesUP

	XDEF MagDrawIndexedPolygonsUP__iiii
MagDrawIndexedPolygonsUP__iiii:
	move.l	maggieBase(pc),a6
	move.l	16(a7),a0
	move.l	12(a7),d0
	move.l	8(a7),a1
	move.l	4(a7),d1
	jmp		-90(a6) ; _LVOmagDrawIndexedPolygonsUP

;*****************************************************************************

	XDEF MagSetVertexBuffer__i
MagSetVertexBuffer__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-96(a6) ; _LVOmagSetVertexBuffer

	XDEF MagSetIndexBuffer__i
MagSetIndexBuffer__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-102(a6) ; _LVOmagSetIndexBuffer

;*****************************************************************************

	XDEF MagDrawTriangles__ii
MagDrawTriangles__ii:
	move.l	maggieBase(pc),a6
	move.l	8(a7),d0
	move.l	4(a7),d1
	jmp		-108(a6) ; _LVOmagDrawTriangles

	XDEF MagDrawIndexedTriangles__iiii
MagDrawIndexedTriangles__iiii:
	move.l	d3,-(a7)
	move.l	maggieBase(pc),a6
	move.l	20(a7),d0
	move.l	16(a7),d1
	move.l	12(a7),d2
	move.l	8(a7),d3
	jsr		-114(a6) ; _LVOmagDrawIndexedTriangles
	move.l	(a7)+,d3
	rts

	XDEF MagDrawIndexedPolygons__iiii
MagDrawIndexedPolygons__iiii:
	move.l	d3,-(a7)
	move.l	maggieBase(pc),a6
	move.l	20(a7),d0
	move.l	16(a7),d1
	move.l	12(a7),d2
	move.l	8(a7),d3
	jsr		-120(a6) ; _LVOmagDrawIndexedPolygons
	move.l	(a7)+,d3
	rts

;*****************************************************************************

	XDEF MagAllocateVertexBuffer__i
MagAllocateVertexBuffer__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-138(a6) ; _LVOmagAllocateVertexBuffer

	XDEF MagUploadVertexBuffer__iiii
MagUploadVertexBuffer__iiii:
	move.l	maggieBase(pc),a6
	move.l	16(a7),d0
	move.l	12(a7),a0
	move.l	8(a7),d1
	move.l	4(a7),d2
	jmp		-144(a6) ; _LVOmagUploadVertexBuffer

	XDEF MagFreeVertexBuffer__i
MagFreeVertexBuffer__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-150(a6) ; _LVOmagFreeVertexBuffer

;*****************************************************************************

	XDEF MagAllocateIndexBuffer__i
MagAllocateIndexBuffer__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-156(a6) ; _LVOmagAllocateIndexBuffer

	XDEF MagUploadIndexBuffer__iiii
MagUploadIndexBuffer__iiii:
	move.l	maggieBase(pc),a6
	move.l	16(a7),d0
	move.l	12(a7),a0
	move.l	8(a7),d1
	move.l	4(a7),d2
	jmp		-162(a6) ; _LVOmagUploadIndexBuffer

	XDEF MagFreeIndexBuffer__i
MagFreeIndexBuffer__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-168(a6) ; _LVOmagFreeIndexBuffer

;*****************************************************************************

	XDEF MagAllocateTexture__i
MagAllocateTexture__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-174(a6) ; _LVOmagAllocateTexture

	XDEF MagUploadTexture__iiii
MagUploadTexture__iiii:
	move.l	maggieBase(pc),a6
	move.l	16(a7),d0
	move.l	12(a7),d1
	move.l	8(a7),a0
	move.l	4(a7),d2
	jmp		-180(a6) ; _LVOmagUploadTexture

	XDEF MagFreeTexture__i
MagFreeTexture__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-186(a6) ; _LVOmagFreeTexture

;*****************************************************************************

	XDEF MagBeginScene
MagBeginScene:
	move.l	maggieBase(pc),a6
	jmp		-192(a6) ; _LVOmagBeginScene

	XDEF MagEndScene
MagEndScene:
	move.l	maggieBase(pc),a6
	jmp		-198(a6) ; _LVOmagEndScene

;*****************************************************************************

	XDEF MagBegin
MagBegin:
	move.l	maggieBase(pc),a6
	jmp		-204(a6) ; _LVOmagBegin

	XDEF MagEnd
MagEnd:
	move.l	maggieBase(pc),a6
	jmp		-210(a6) ; _LVOmagEnd

;*****************************************************************************

	XDEF MagVertex__iii
MagVertex__iii:
	move.l	maggieBase(pc),a6
	fmove.s	12(a7),fp0
	fmove.s	8(a7),fp1
	fmove.s	4(a7),fp2
	jmp		-216(a6) ; _LVOmagVertex

	XDEF MagNormal__iii
MagNormal__iii:
	move.l	maggieBase(pc),a6
	fmove.s	12(a7),fp0
	fmove.s	8(a7),fp1
	fmove.s	4(a7),fp2
	jmp		-222(a6) ; _LVOmagNormal

	XDEF MagTexCoord__iii
MagTexCoord__iii:
	move.l	maggieBase(pc),a6
	move.l	12(a7),d0
	fmove.s	8(a7),fp0
	fmove.s	4(a7),fp1
	jmp		-228(a6) ; _LVOmagTexCoord

	XDEF MagTexCoord3__iiii
MagTexCoord3__iiii:
	move.l	maggieBase(pc),a6
	move.l	16(a7),d0
	fmove.s	12(a7),fp0
	fmove.s	8(a7),fp1
	fmove.s	4(a7),fp2
	jmp		-234(a6) ; _LVOmagTexCoord3

	XDEF MagColour__i
MagColour__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-240(a6) ; _LVOmagColour

;*****************************************************************************

	XDEF MagClear__i
MagClear__i:
	move.l	maggieBase(pc),a6
	move.l	4(a7),d0
	jmp		-246(a6) ; _LVOmagClear

;*****************************************************************************

	XDEF MagSetLightType__ii
MagSetLightType__ii:
	move.l	maggieBase(pc),a6
	move.l	8(a7),d0
	move.l	4(a7),d1
	jmp		-252(a6) ; _LVOmagSetLightType

	XDEF MagSetLightPosition__iiii
MagSetLightPosition__iiii:
	move.l	maggieBase(pc),a6
	move.l	16(a7),d0
	fmove.s	12(a7),fp0
	fmove.s	8(a7),fp1
	fmove.s	4(a7),fp2
	jmp		-258(a6) ; _LVOmagSetLightPosition

	XDEF MagSetLightDirection__iiii
MagSetLightDirection__iiii:
	move.l	maggieBase(pc),a6
	move.l	16(a7),d0
	fmove.s	12(a7),fp0
	fmove.s	8(a7),fp1
	fmove.s	4(a7),fp2
	jmp		-264(a6) ; _LVOmagSetLightDirection

	XDEF MagSetLightCone__ii
MagSetLightCone__ii:
	move.l	maggieBase(pc),a6
	move.l	8(a7),d0
	fmove.s	4(a7),fp0
	jmp		-270(a6) ; _LVOmagSetLightCone

	XDEF MagSetLightAttenuation__ii
MagSetLightAttenuation__ii:
	move.l	maggieBase(pc),a6
	move.l	8(a7),d0
	fmove.s	4(a7),fp0
	jmp		-276(a6) ; _LVOmagSetLightAttenuation

	XDEF MagSetLightColour__ii
MagSetLightColour__ii:
	move.l	maggieBase(pc),a6
	move.l	8(a7),d0
	move.l	4(a7),d1
	jmp		-282(a6) ; _LVOmagSetLightColour
