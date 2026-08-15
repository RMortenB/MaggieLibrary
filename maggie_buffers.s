	section .text

	include <exec/types.i>
	include "raster/raster_structs.i"
;	void magFastClear(void *buffer __asm("a0"), ULONG nBytes__asm("d0"), ULONG data __asm("d1"));

_magFastClear:
;	vperm	#$45674567,d1,d1,e0
	lsr.l	#4,d0
.loop:
	move.l	d1,(a0)+
	move.l	d1,(a0)+
	move.l	d1,(a0)+
	move.l	d1,(a0)+
;	store	e0,(a0)+
;	store	e0,(a0)+
	subq.l	#1,d0
	bne.s	.loop
	rts

	public _magFastClear

; void LoadMatrix(mat4 *mat __asm("a0"));
_LoadMatrix:
	fmove.s	(a0)+,e0
	fmove.s	(a0)+,e1
	fmove.s	(a0)+,e2
	fmove.s	(a0)+,e3
	fmove.s	(a0)+,e4
	fmove.s	(a0)+,e5
	fmove.s	(a0)+,e6
	fmove.s	(a0)+,e7
	fmove.s	(a0)+,e8
	fmove.s	(a0)+,e9
	fmove.s	(a0)+,e10
	fmove.s	(a0)+,e11
	fmove.s	(a0)+,e12
	fmove.s	(a0)+,e13
	fmove.s	(a0)+,e14
	fmove.s	(a0)+,e15
	rts
	public _LoadMatrix

; void TransformH(vec4 *dst __asm("a0"), vec3 *src __asm("a1"), mat4 *mat __asm("a0"));
_TransformH:
	fmovem	fp2/fp3,-(a7)
	fmove.s	(a1)+,e16
	fmove.s	(a1)+,e17
	fmove.s	(a1)+,e18

	fmul	e0,e16,fp0
	fmul	e1,e16,fp1
	fmul	e2,e16,fp2
	fmul	e3,e16,fp3
	fmul	e4,e17,e20
	fmul	e5,e17,e21
	fmul	e6,e17,e22
	fmul	e7,e17,e23
	fadd	e20,fp0
	fadd	e21,fp1
	fadd	e22,fp2
	fadd	e23,fp3
	fmul	e8,e18,e20
	fmul	e9,e18,e21
	fmul	e10,e18,e22
	fmul	e11,e18,e23
	fadd	e20,fp0
	fadd	e21,fp1
	fadd	e22,fp2
	fadd	e23,fp3
	fadd	e12,fp0
	fadd	e13,fp1
	fadd	e14,fp2
	fadd	e15,fp3
	fmove.s	fp0,(a0)+
	fmove.s	fp1,(a0)+
	fmove.s	fp2,(a0)+
	fmove.s	fp3,(a0)+
	fmovem	(a7)+,fp2/fp3
	rts
	public _TransformH

_GetClocks:
	movec.l	ccc,d0
	rts
	public _GetClocks

; The scanline-edge DDAs live in maggie_edgewalk.s, generated per column side and draw mode from raster/edge_walk.inc.

_setRoundingModeRZ:
	fmove.l	fpcr,d0
	and.l	#$ffffffcf,d0
	or.l	#$00000010,d0
	fmove.l	d0,fpcr
	rts

	public _setRoundingModeRZ
