	include "lvo/exec_lib.i"
	include "lvo/lowlevel_lib.i"
	include "lvo/Maggie_lvo.i"
	include "maggie_flags.i"

	include "libraries/lowlevel.i"
	include "exec/memory.i"
	include "hardware/custom.i"
	include "hardware/cia.i"

LIBCALL	MACRO
	jsr	_LVO\1(a6)
	ENDM

	code

StartCode::
	movem.l	d1-d7/a0-a6,-(a7)

	move.w	$dff002,d0
	andi.w	#$0180,d0
	move.w	d0,oldDmacon
	move.w	#$0180,$dff096

	bsr		LoadLibraries
	bsr		SetupResources

	bsr		SetupScreen

	bsr		RunDraw

	bsr		FreeResources
	bsr		CloseLibraries

	move.w	oldDmacon,d0
	or.w	#$8000,d0
	move.w	d0,$dff096

	movem.l	(a7)+,d1-d7/a0-a6
	moveq	#0,d0

	rts

VBLInterrupt:
	addq.l	#1,(a1)
	moveq	#0,d0

	rts

WaitVBL:
	tst.l	vblPassed
	beq.s	WaitVBL

	moveq	#0,d0
	move.l	d0,vblPassed

	rts

RunDraw:
	bsr		WaitVBL

	bsr		UpdateWorldMatrix

	move.l	MaggieBase,a6
	LIBCALL magBeginScene

	move.l	FrameNumber,d0

	lea		FramePixels,a0

	move.l	(a0,d0*4),d1
	move.l	d1,$dff1ec

	addq	#1,d0
	cmp.w	#3,d0
	bne.s	.noReset
	moveq	#0,d0
.noReset:
	move.l	d0,FrameNumber

	move.l	(a0,d0*4),a0
	move.l	#640,d0
	move.l	#360,d1
	LIBCALL	magSetScreenMemory

	lea		PerspectiveMatrix,a0
	LIBCALL	magSetPerspectiveMatrix
	lea		WorldMatrix,a0
	LIBCALL	magSetWorldMatrix
	lea		ViewMatrix,a0
	LIBCALL	magSetViewMatrix

	move.w	#MAG_DRAWMODE_BILINEAR|MAG_DRAWMODE_32BIT|MAG_DRAWMODE_MIPMAP,d0
	LIBCALL	magSetDrawMode

	move.l	#$00112233,d0
	LIBCALL	magClearColour

	moveq	#MAG_CLEAR_COLOUR,d0
	LIBCALL	magClear

	move.w	vHandle,d0
	LIBCALL	magSetVertexBuffer
	move.w	iHandle,d0
	LIBCALL	magSetIndexBuffer
	moveq	#0,d0
	move.w	tHandle,d1
	LIBCALL	magSetTexture

	moveq	#0,d0
	move.w	#4*6,d1
	move.w	#0,d2
	move.w	#6*5-1,d3
	LIBCALL	magDrawIndexedPolygons

	LIBCALL	magEndScene

	btst	#6,$bfe001
	bne.s	RunDraw

	rts

MatrixMul:
	moveq	#3,d0
mulLoop:
	fmove.s	(a1)+,fp0
	fmove.s	(a1)+,fp1
	fmove.s	(a1)+,fp2
	fmove.s	(a1)+,fp3

	fmul.s	0*4(a0),fp0,fp4
	fmul.s	4*4(a0),fp1,fp5
	fmul.s	8*4(a0),fp2,fp6
	fmul.s	12*4(a0),fp3,fp7
	fadd	fp5,fp4
	fadd	fp7,fp6
	fadd	fp6,fp4
	fmove.s	fp4,(a2)+

	fmul.s	1*4(a0),fp0,fp4
	fmul.s	5*4(a0),fp1,fp5
	fmul.s	9*4(a0),fp2,fp6
	fmul.s	13*4(a0),fp3,fp7
	fadd	fp5,fp4
	fadd	fp7,fp6
	fadd	fp6,fp4
	fmove.s	fp4,(a2)+

	fmul.s	2*4(a0),fp0,fp4
	fmul.s	6*4(a0),fp1,fp5
	fmul.s	10*4(a0),fp2,fp6
	fmul.s	14*4(a0),fp3,fp7
	fadd	fp5,fp4
	fadd	fp7,fp6
	fadd	fp6,fp4
	fmove.s	fp4,(a2)+

	fmul.s	3*4(a0),fp0,fp4
	fmul.s	7*4(a0),fp1,fp5
	fmul.s	11*4(a0),fp2,fp6
	fmul.s	15*4(a0),fp3,fp7
	fadd	fp5,fp4
	fadd	fp7,fp6
	fadd	fp6,fp4
	fmove.s	fp4,(a2)+

	dbra	d0,mulLoop

	rts

UpdateWorldMatrix:
;X
	fmove.s	XAngle,fp0
	fsin	fp0,fp1
	fcos	fp0,fp2
	fadd.s	#0.007,fp0
	fmove.s	fp0,XAngle

	lea		XRot,a0
	fmove.s	fp2,20(a0)
	fmove.s	fp1,24(a0)
	fneg	fp1
	fmove.s	fp1,36(a0)
	fmove.s	fp2,40(a0)
;Y
	fmove.s	YAngle,fp0
	fsin	fp0,fp1
	fcos	fp0,fp2
	fadd.s	#0.008,fp0
	fmove.s	fp0,YAngle

	lea		YRot,a0
	fmove.s	fp2,(a0)
	fmove.s	fp1,8(a0)
	fneg	fp1
	fmove.s	fp1,32(a0)
	fmove.s	fp2,40(a0)
;Z
	fmove.s	ZAngle,fp0
	fsin	fp0,fp1
	fcos	fp0,fp2
	fadd.s	#0.009,fp0
	fmove.s	fp0,ZAngle

	lea		ZRot,a0
	fmove.s	fp2,(a0)
	fmove.s	fp1,4(a0)
	fneg	fp1
	fmove.s	fp1,16(a0)
	fmove.s	fp2,20(a0)

	lea		ZRot,a0
	lea		YRot,a1
	lea		WorldMatrix,a2
	bsr		MatrixMul	

	lea		XRot,a0
	lea		WorldMatrix,a1
	lea		WorldMatrix,a2
	bsr		MatrixMul	

	rts

SetupResources:
	move.l	MaggieBase,a6

	move.w	#6*4,d0										; number of vertices
	LIBCALL	magAllocateVertexBuffer
	move.w	d0,vHandle

	lea		Vertices,a0
	moveq	#0,d1										; offset to uploaded data
	move.w	#6*4,d2										; Number of uploaded vertices
	LIBCALL	magUploadVertexBuffer

	move.w	#5*6-1,d0									; number of indices
	LIBCALL	magAllocateIndexBuffer
	move.w	d0,iHandle

	lea		Indices,a0
	moveq	#0,d1										; offset to uploaded indices
	move.w	#5*6-1,d2									; Number of uploaded indices
	LIBCALL	magUploadIndexBuffer

	moveq	#8,d0										; Size of texture is 256x256
	LIBCALL	magAllocateTexture
	move.w	d0,tHandle

	lea		Texture,a2
	move.l	a2,a0
	move.w	tHandle,d0
	moveq	#8,d1										; Miplevel 256x256
	moveq	#MAG_TEXFMT_DXT1,d2
	LIBCALL	magUploadTexture

	add.l	#256*128,a2
	move.l	a2,a0
	move.w	tHandle,d0
	moveq	#7,d1										; Miplevel 128x128
	LIBCALL	magUploadTexture

	add.l	#128*64,a2
	move.l	a2,a0
	move.w	tHandle,d0
	moveq	#6,d1										; Miplevel 64x64
	LIBCALL	magUploadTexture

	add.l	#64*32,a2
	move.l	a2,a0
	move.w	tHandle,d0
	moveq	#5,d1										; Miplevel 32x32
	LIBCALL	magUploadTexture

	rts

FreeResources:
	move.l	LowLevelBase,a6
	lea		ReleaseTags,a1
	LIBCALL	SystemControlA

	move.l	intHandle,a1
	LIBCALL	RemVBlankInt

	move.l	4.w,a6
	move.l	ScreenMem,a0
	LIBCALL	FreeVec

	move.l	MaggieBase,a6
	move.w	vHandle,d0
	LIBCALL	magFreeVertexBuffer

	move.w	iHandle,d0
	LIBCALL	magFreeIndexBuffer

	move.w	tHandle,d0
	LIBCALL	magFreeTexture

	move.l	OldScreenPtr,$dff1ec
	move.w	OldScreenMode,$dff1f4

	rts

SetupScreen:
	move.l	4.w,a6
	move.l	#640*360*4*3+31,d0
	move.l	#MEMF_CLEAR,d1
	LIBCALL	AllocVec
	move.l	d0,ScreenMem
	add.l	#$0000001f,d0
	andi.l	#$ffffffe0,d0
	move.l	d0,FramePixels
	add.l	#640*360*4,d0
	move.l	d0,FramePixels+4
	add.l	#640*360*4,d0
	move.l	d0,FramePixels+8

	move.l	LowLevelBase,a6
	lea		TakeOverTags,a1
	LIBCALL	SystemControlA

	lea		VBLInterrupt,a0
	lea		vblPassed,a1
	LIBCALL	AddVBlankInt
	move.l	d0,intHandle

	move.l	$dfe1ec,OldScreenPtr
	move.w	$dfe1f4,OldScreenMode

	move.l	FramePixels,a0

	move.w	#$0b05,$dff1f4
	move.l	a0,$dff1ec

	rts

LoadLibraries:
	move.l	4.w,a6

	lea		MaggieName,a1
	moveq	#0,d0
	LIBCALL	OpenLibrary
	move.l	d0,MaggieBase

	lea		LowLevelName,a1
	moveq	#0,d0
	LIBCALL	OpenLibrary
	move.l	d0,LowLevelBase

	rts

CloseLibraries:
	move.l	4.w,a6

	move.l	LowLevelBase,a1
	LIBCALL	CloseLibrary

	move.l	MaggieBase,a1
	LIBCALL	RemLibrary

	move.l	MaggieBase,a1
	LIBCALL	CloseLibrary

	rts

;------------------------------------------------------------------------------

	data

vblPassed:		dc.l	0
intHandle:		dc.l	0
oldDmacon:		dc.w	0

	align 2

MaggieBase:		dc.l	0
MaggieName:		dc.b	"maggie.library",0

	align 2

LowLevelBase:	dc.l	0
LowLevelName:	dc.b	"lowlevel.library",0

	align 2

TakeOverTags:	dc.l	SCON_TakeOverSys,1,TAG_END,0
ReleaseTags:	dc.l	SCON_TakeOverSys,0,TAG_END,0

OldScreenPtr:	dc.l	0
OldScreenMode:	dc.w	0

	align 2

ScreenMem:		dc.l	0
FramePixels:	dc.l	0,0,0
FrameNumber:	dc.l	0
CurrentFrame:	dc.l	0

vHandle:		dc.w	$ffff
iHandle:		dc.w	$ffff
tHandle:		dc.w	$ffff

	align 2

PerspectiveMatrix: ; 60 degree view angle, 16:9 aspect ratio, 0.5 near, 100.0 far
				dc.s	1.73205,0.0,0.0,0.0
				dc.s	0.0,3.079202,0.0,0.0
				dc.s	0.0,0.0,1.005025,1.0
				dc.s	0.0,0.0,-0.502513,0.0
WorldMatrix:
				dc.s	1.0,0.0,0.0,0.0
				dc.s	0.0,1.0,0.0,0.0
				dc.s	0.0,0.0,1.0,0.0
				dc.s	0.0,0.0,0.0,1.0
ViewMatrix:
				dc.s	1.0,0.0,0.0,0.0
				dc.s	0.0,1.0,0.0,0.0
				dc.s	0.0,0.0,1.0,0.0
				dc.s	0.0,0.0,3.0,1.0

XRot:
				dc.s	1.0,0.0,0.0,0.0
				dc.s	0.0,1.0,0.0,0.0
				dc.s	0.0,0.0,1.0,0.0
				dc.s	0.0,0.0,0.0,1.0
YRot:
				dc.s	1.0,0.0,0.0,0.0
				dc.s	0.0,1.0,0.0,0.0
				dc.s	0.0,0.0,1.0,0.0
				dc.s	0.0,0.0,0.0,1.0
ZRot:
				dc.s	1.0,0.0,0.0,0.0
				dc.s	0.0,1.0,0.0,0.0
				dc.s	0.0,0.0,1.0,0.0
				dc.s	0.0,0.0,0.0,1.0

XAngle:			dc.s	0.0
YAngle:			dc.s	0.0
ZAngle:			dc.s	0.0

Vertices:				; pos{X, Y, Z}, normal{NX, NY, NZ}, tex{U, V, W}, colour
				dc.s	-1.0,-1.0,-1.0,0.0,0.0,-1.0,0.99609375,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	-1.0,1.0,-1.0,0.0,0.0,-1.0,0.99609375,0.0,1.0
				dc.l	$00ffffff
				dc.s	1.0,1.0,-1.0,0.0,0.0,-1.0,0.0,0.0,1.0
				dc.l	$00ffffff
				dc.s	1.0,-1.0,-1.0,0.0,0.0,-1.0,0.0,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	-1.0,-1.0,1.0,0.0,0.0,1.0,0.0,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	1.0,-1.0,1.0,0.0,0.0,1.0,0.99609375,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	1.0,1.0,1.0,0.0,0.0,1.0,0.99609375,0.0,1.0
				dc.l	$00ffffff
				dc.s	-1.0,1.0,1.0,0.0,0.0,1.0,0.0,0.0,1.0
				dc.l	$00ffffff
				dc.s	-1.0,-1.0,-1.0,0.0,-1.0,0.0,0.0,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	1.0,-1.0,-1.0,0.0,-1.0,0.0,0.99609375,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	1.0,-1.0,1.0,0.0,-1.0,0.0,0.99609375,0.0,1.0
				dc.l	$00ffffff
				dc.s	-1.0,-1.0,1.0,0.0,-1.0,0.0,0.0,0.0,1.0
				dc.l	$00ffffff
				dc.s	-1.0,1.0,-1.0,0.0,1.0,0.0,0.99609375,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	-1.0,1.0,1.0,0.0,1.0,0.0,0.99609375,0.0,1.0
				dc.l	$00ffffff
				dc.s	1.0,1.0,1.0,0.0,1.0,0.0,0.0,0.0,1.0
				dc.l	$00ffffff
				dc.s	1.0,1.0,-1.0,0.0,1.0,0.0,0.0,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	-1.0,-1.0,-1.0,-1.0,0.0,0.0,0.99609375,0.0,1.0
				dc.l	$00ffffff
				dc.s	-1.0,-1.0,1.0,-1.0,0.0,0.0,0.0,0.0,1.0
				dc.l	$00ffffff
				dc.s	-1.0,1.0,1.0,-1.0,0.0,0.0,0.0,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	-1.0,1.0,-1.0,-1.0,0.0,0.0,0.99609375,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	1.0,-1.0,-1.0,1.0,0.0,0.0,0.99609375,0.99609375,1.0
				dc.l	$00ffffff
				dc.s	1.0,1.0,-1.0,1.0,0.0,0.0,0.99609375,0.0,1.0
				dc.l	$00ffffff
				dc.s	1.0,1.0,1.0,1.0,0.0,0.0,0.0,0.0,1.0
				dc.l	$00ffffff
				dc.s	1.0,-1.0,1.0,1.0,0.0,0.0,0.0,0.99609375,1.0
				dc.l	$00ffffff

Indices:		dc.w	0,1,2,3,$ffff,4,5,6,7,$ffff,8,9,10,11,$ffff
				dc.w	12,13,14,15,$ffff,16,17,18,19,$ffff,20,21,22,23
	align 2
Texture:		include "Fog.i"
