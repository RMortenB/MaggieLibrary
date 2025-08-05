	sub.l	d0,d1; yLen
	ble.w	.noVlinesToDo

	movem.l	d2-d7/a2-a4,-(a7)

	move.l	d1,d7

	GetEdgesPtr a2
	GetXRes d2
	mulu.w	d2,d0
	GetScreenPtr a3
	GetScissorPtr a4

	lea		(a3,d0.l*.bpp),a3			;pixels

	fmove.w	#1,e4
	lea		$dff250,a0
.vLoop:
	fmove.s	EPos_xPosLeft(a2),e0	;xLeft
	fmove.s	EPos_xPosRight(a2),e1	;xRight

	fmove.l	e0,d3					;xStart = (int)xLeft;
	fmove.l	e1,d4					;xEnd = (int)xRight;

	cmp.l	(a4),d3					;if(scissorLeft > x0)
	bge		.noClipX0

	move.l	(a4),d3					;xStart = scissorLeft
.noClipX0:
	cmp.l	Scsr_x1(a4),d4 					;if(scissorRight < x1)
	ble		.noClipX1

	move.l	Scsr_x1(a4),d4					;xEnd = scissorRight
.noClipX1:
	sub.l	d3,d4					;len = x1 - x0
	ble		.noPixels

	fadd.l	d3,e4,e2
	fsub	e0,e1,e3	; xDiff = xRight - xLeft

	lea		(a3,d3.l*.bpp),a1			;dstColPtr = pixels + xStart
	move.l	a1,Maggie_pixDest(a0)

	fmove.s	EPos_iLeft(a2),e12		; iowLeft
	fmove.s	EPos_iRight(a2),e13		; iowRight

	fsub	e0,e2,e2				; preStep = 1 + xStart - edges[i].xPosLeft
	fdiv	e3,e4,e3				; ooXLength = 1.0f / xDiff
	fmove.s	EPos_oowLeft(a2),e14	; oowLeft
	fmove.s	EPos_oowRight(a2),e15	; oowRight
	fmove.s	EPos_uowLeft(a2),e17	; uowLeft
	fmove.s	EPos_uowRight(a2),e18	; uowRight
	fmove.s	EPos_vowLeft(a2),e20	; vowLeft
	fmove.s	EPos_vowRight(a2),e21	; vowRight
	fsub	e12,e13					; iDiff = iRight - iLeft
	fsub	e14,e15					; wDiff = oowRight - oowLeft
	fsub	e17,e18					; uDiff = uRight - uLeft
	fsub	e20,e21					; vDiff = vRight - vLeft
	fmul	e3,e13					; iDDA = iDiff * ooXLength
	fmul	e3,e15					; wDDA = wDiff * ooXLength
	fmul	e3,e18					; uDDA = uDiff * ooXLength
	fmul	e3,e21					; vDDA = vDiff * ooXLength
	fmul	e2,e13,e8				; preStepI = preStep * iDDA
	fmul	e2,e15,e16				; preStepW = preStep * wDDA
	fmul	e2,e18,e19				; preStepU = preStep * uDDA
	fmul	e2,e21,e22				; preStepV = preStep * vDDA
	fadd	e8,e12					; light = iLeft + preStepI
	fadd	e16,e14					; wPos = oowLeft + preStepW
	fadd	e19,e17					; uPos = uLeft + preStepU
	fadd	e22,e20					; vPos = vLeft + preStepV

	fdiv	e14,e17,e5
	fdiv	e14,e20,e6

	moveq	#16,d6

	fmoverz.l	e12,d1
	fmoverz.l	e13,d3

	move.w	d1,Maggie_light(a0)
	move.w	d3,Maggie_lightDelta(a0)

	fmove.l	e5,Maggie_uCoord(a0)
	fmove.l	e6,Maggie_vCoord(a0)

	cmp.l	#16,d4
	blt.s	.singlePixels

	fmul.l	d6,e15,e7		; wDDAFullRun
	fmul.l	d6,e18,e8		; uDDAFullRun
	fmul.l	d6,e21,e9		; vDDAFullRun
.loopSpan16:
	fadd	e7,e14			; +3 (once)
	fadd	e8,e17
	fadd	e9,e20

	fdiv	e14,e17,e10		; +4
	fdiv	e14,e20,e11

	fsub	e5,e10,e12		; +9
	fsub	e6,e11,e13

	fmul.s	#$3d800000,e12	; +3
	fmul.s	#$3d800000,e13

	sub.l	#16,d4

	fmove	e10,e5
	fmove	e11,e6

	fmove.l	e12,Maggie_uDelta(a0) ; +1
	fmove.l	e13,Maggie_vDelta(a0)
	move.w	d6,Maggie_startLength(a0)

	or.l	#$8000,d6

	cmp.l	#16,d4
	bge.s	.loopSpan16
.singlePixels:
	tst.l	d4
	beq.s	.noPixels

	fmul.l	d4,e15
	fmul.l	d4,e18
	fmul.l	d4,e21

	fdiv	d4,e4,e7

	fadd	e15,e14
	fadd	e18,e17
	fadd	e21,e20

	fdiv	e14,e17,e10
	fdiv	e14,e20,e11

	fsub	e5,e10
	fsub	e6,e11

	and.l	#$8000,d6
	or.l	d6,d4

	fmul	e7,e10
	fmul	e7,e11

	fmove.l	e10,Maggie_uDelta(a0)
	fmove.l	e11,Maggie_vDelta(a0)
	move.w	d4,Maggie_startLength(a0)
.noPixels:
	lea		(a3,d2.l*.bpp),a3			;pixels += modulo
	lea		EPos_Size(a2),a2

	subq.l	#1,d7 ; ylen
	bne		.vLoop

	movem.l	(a7)+,d2-d7/a2-a4
.noVlinesToDo:
	rts
