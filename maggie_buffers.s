	section .text

;	void magFastClear(void *buffer __asm("a0"), ULONG nBytes__asm("d0"), ULONG data __asm("d1"));

_magFastClear:
	vperm	#$45674567,d1,d1,e0
	lsr.l	#4,d0
.loop:
	store	e0,(a0)+
	store	e0,(a0)+
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

_DrawLineAsm:
    fmovem    fp2-fp7,-(a7)

    fmove.s      (a1),fp2
    fmove.s     8(a1),fp3
    fmove.s    12(a1),fp4
    fmove.s    16(a1),fp5
    fmove.s    20(a1),fp6
    fmove.l    28(a1),fp7

    fmove.s      (a2),e0
    fmove.s     8(a2),e1
    fmove.s    12(a2),e2
    fmove.s    16(a2),e3
    fmove.s    20(a2),e4
    fmove.l    28(a2),e5

    fsub    fp2,e0
    fsub    fp3,e1
    fsub    fp4,e2
    fsub    fp5,e3
    fsub    fp6,e4
    fsub    fp7,e5

    fmul    fp0,e0
    fmul    fp0,e1
    fmul    fp0,e2
    fmul    fp0,e3
    fmul    fp0,e4
    fmul    fp0,e5

    fmul    fp1,e0,e6
    fmul    fp1,e1,e7
    fmul    fp1,e2,e8
    fmul    fp1,e3,e9
    fmul    fp1,e4,e10
    fmul    fp1,e5,e11

    fadd    e6,fp2
    fadd    e7,fp3
    fadd    e8,fp4
    fadd    e9,fp5
    fadd    e10,fp6
    fadd    e11,fp7
.loop:
    fmove.s    fp2,(a0)+
    fmove.s    fp3,(a0)+
    fmove.s    fp4,(a0)+
    fmove.s    fp5,(a0)+
    fmove.s    fp6,(a0)+
    fmove.s    fp7,(a0)+

    fadd    e0,fp2
    fadd    e1,fp3
    fadd    e2,fp4
    fadd    e3,fp5
    fadd    e4,fp6
    fadd    e5,fp7

    subq.l    #1,d0
    bne.s    .loop

    fmovem    (a7)+,fp2-fp7

    rts
	public _DrawLineAsm
	