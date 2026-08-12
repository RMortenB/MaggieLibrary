	section	.text

	include	<exec/types.i>
	include	"raster/raster_structs.i"

;------------------------------------------------------------------------------
; ULONG MaggieSetupPoly(struct MaggieTransVertex *vtx __asm("a0"),
;                       UWORD *indx                   __asm("a1"),
;                       int n                         __asm("d0"),
;                       MaggieBase *lib               __asm("a6"));
;
; Per-polygon setup: fan-sum area (which is also the gradient denominator),
; screen-y span, and the five attribute gradients. indx may be NULL for a
; contiguous fan; n must be >= 3, which the caller already checks.
;
;   returns 0  -> skip this polygon (backfacing, or zero scanlines tall)
;   returns 1  -> draw it; lib->gradients, lib->primMinY and lib->primMaxY
;                 have been written
;
; The caller still emits the edges and dispatches the span renderer.
;
;------------------------------------------------------------------------------
; WHY THIS SHAPE
;
; The C version walked the fan twice: once for the area and the y span, then
; again for the five numerators. Under the 68080 model (one FP instruction issued
; per cycle, ~6 cycle results, in-order, stalls only on an unready operand) those
; measured 57 and 157 cycles per fan step, 61% and 50% idle - so both passes were
; spending most of their time waiting, and each had exactly the independent work
; the other needed to fill those gaps.
;
; Fusing them is therefore cheaper on BOTH paths, which is worth stating because
; it is the opposite of the usual trade: a culled polygon now computes numerators
; it throws away, but it does so in cycles the area pass was stalling in anyway,
; and it no longer pays for a second loop setup.
;
; The six accumulators - denom plus the five numerators - are six independent
; chains. Each attribute's two deltas are multiplied, subtracted and accumulated
; entirely IN PLACE in the register pair that held the deltas, so six chains need
; no temporaries at all, and issuing them stage by stage leaves exactly six
; independent instructions between every producer and its consumer. That covers
; the whole 6-cycle latency.
;
; The accumulate is loop-carried, but the next step's add to any given
; accumulator is ~38 instructions later, far beyond the latency, so the carry
; never stalls.
;
; Both deltas are recomputed each step rather than carried from the previous one.
; Carrying would save seven subtracts but cost seven register moves, so it is a
; wash - and recomputing keeps the accumulation order identical to the C, which
; makes the result bit-exact rather than merely equivalent.
;
; Modelled at ~52 cycles per fan step against ~214 for the two C loops.
;------------------------------------------------------------------------------
; SIGNS
;
; Every delta is formed as (v0 - vi) rather than (vi - v0), because that lets the
; load fuse into the subtract as fsub.s <ea>,eSrc,eDst. Both the attribute delta
; AND its y multiplier are negated this way, so the two negations cancel inside
; each product:
;
;   (-di)*(-dyj) - (-dj)*(-dyi)  ==  di*dyj - dj*dyi
;
; The accumulators therefore hold exactly the C values and the scale is +1/denom.
; (Contrast MaggieSetupTri, which negates only the attribute deltas and so needs
; -1/denom.)
;------------------------------------------------------------------------------
; REGISTERS
;   e0..e6     v0's x,y,w,u,v,z,i     the fan origin, live for the whole loop
;   e7..e12    accumulators: denom, nOow, nUow, nVow, nZ, nI
;   e13,e14    -dyi, -dyj             multipliers, so not consumed in place
;   e15..e20   vi's deltas for x,w,u,v,z,i   -> products -> differences, in place
;   e21..e23   vj's deltas for x,w,u
;   fp2..fp4   vj's deltas for v,z,i
;   fp5        vj's truncated y, for the span
;   fp0        cullSign, then area
;   d1 min y   d2 max y   d3 scratch/index   d4 trip count
;   a1 vi      a2 vj      a3 index walk      a4 vertex base
;
; fp2-fp5 are callee-saved, so they are saved on entry - but this is a loop, so
; that is paid once per fan rather than per step.
;------------------------------------------------------------------------------

_MaggieSetupPoly:
	movem.l	d2-d4/a2-a4,-(sp)
	fmovem	fp2-fp5,-(sp)

	movea.l	a0,a4			; vertex base
	move.l	d0,d4
	subq.l	#2,d4			; trip count = n - 2
	movea.l	a1,a3			; index walk, or NULL
	fmove.s	MB_cullSign(a6),fp0	; constant: hoisted clear of the loop

	move.l	a3,d3			; MOVE to a data register sets Z, MOVEA would not
	beq.w	.linearSetup

;--- indexed: resolve v0, then point vi/vj at vertices 1 and 2 ------------------

	moveq	#0,d3
	move.w	(a3)+,d3		; indx[0]
	lsl.l	#5,d3
	lea	(a4,d3.l),a0		; v0
	moveq	#0,d3
	move.w	(a3)+,d3		; indx[1]
	lsl.l	#5,d3
	lea	(a4,d3.l),a1		; vi = vertex 1
	moveq	#0,d3
	move.w	(a3)+,d3		; indx[2]
	lsl.l	#5,d3
	lea	(a4,d3.l),a2		; vj = vertex 2
	bra.w	.loaded

.linearSetup:
	lea	TransVtx_Size(a0),a1	; vi = vertex 1
	lea	TransVtx_Size*2(a0),a2	; vj = vertex 2

;--- the fan origin, and the y span seeded from vertices 0 and 1 ----------------

.loaded:
	fmove.s	TransVtx_PosX(a0),e0
	fmove.s	TransVtx_PosY(a0),e1
	fmove.s	TransVtx_PosW(a0),e2
	fmove.s	TransVtx_U(a0),e3
	fmove.s	TransVtx_V(a0),e4
	fmove.s	TransVtx_PosZ(a0),e5
	fmove.l	TransVtx_I(a0),e6

	fmove.w	#0,e7			; denom
	fmove.w	#0,e8			; nOow
	fmove.w	#0,e9			; nUow
	fmove.w	#0,e10			; nVow
	fmove.w	#0,e11			; nZ
	fmove.w	#0,e12			; nI

	fintrz	e1,e13
	fmove.s	TransVtx_PosY(a1),e14
	fintrz	e14,e14
	fmove.l	e13,d1			; trunc(y0)
	fmove.l	e14,d2			; trunc(y1)
	cmp.l	d2,d1
	ble.s	.seeded
	exg	d1,d2
.seeded:					; d1 = min, d2 = max over vertices 0 and 1

	move.l	a3,d3
	beq.w	.linearLoop

;==============================================================================
; indexed fan
;==============================================================================

.indexedLoop:
	fmove.s	TransVtx_PosY(a2),fp5	; the span's y, issued at the top so its
	fintrz	fp5,fp5			; truncation is ready by the loop tail

	fsub.s	TransVtx_PosY(a1),e1,e13	; -dyi = y0 - yi
	fsub.s	TransVtx_PosY(a2),e1,e14	; -dyj = y0 - yj
	fsub.s	TransVtx_PosX(a1),e0,e15
	fsub.s	TransVtx_PosW(a1),e2,e16
	fsub.s	TransVtx_U(a1),e3,e17
	fsub.s	TransVtx_V(a1),e4,e18
	fsub.s	TransVtx_PosZ(a1),e5,e19
	fsub.l	TransVtx_I(a1),e6,e20
	fsub.s	TransVtx_PosX(a2),e0,e21
	fsub.s	TransVtx_PosW(a2),e2,e22
	fsub.s	TransVtx_U(a2),e3,e23
	fsub.s	TransVtx_V(a2),e4,fp2
	fsub.s	TransVtx_PosZ(a2),e5,fp3
	fsub.l	TransVtx_I(a2),e6,fp4

	fmul	e14,e15			; vi's delta * -dyj
	fmul	e14,e16
	fmul	e14,e17
	fmul	e14,e18
	fmul	e14,e19
	fmul	e14,e20

	fmul	e13,e21			; vj's delta * -dyi
	fmul	e13,e22
	fmul	e13,e23
	fmul	e13,fp2
	fmul	e13,fp3
	fmul	e13,fp4

	fsub	e21,e15
	fsub	e22,e16
	fsub	e23,e17
	fsub	fp2,e18
	fsub	fp3,e19
	fsub	fp4,e20

	fadd	e15,e7
	fadd	e16,e8
	fadd	e17,e9
	fadd	e18,e10
	fadd	e19,e11
	fadd	e20,e12

	fmove.l	fp5,d3
	cmp.l	d3,d1
	ble.s	.iNoMin
	move.l	d3,d1
.iNoMin:
	cmp.l	d3,d2
	bge.s	.iNoMax
	move.l	d3,d2
.iNoMax:
	subq.l	#1,d4
	beq.w	.fanDone		; tested BEFORE advancing, so the last step does
	movea.l	a2,a1			; not read indx[n] past the polygon
	moveq	#0,d3
	move.w	(a3)+,d3
	lsl.l	#5,d3
	lea	(a4,d3.l),a2
	bra.w	.indexedLoop

;==============================================================================
; contiguous fan
;==============================================================================

.linearLoop:
	fmove.s	TransVtx_PosY(a2),fp5
	fintrz	fp5,fp5

	fsub.s	TransVtx_PosY(a1),e1,e13
	fsub.s	TransVtx_PosY(a2),e1,e14
	fsub.s	TransVtx_PosX(a1),e0,e15
	fsub.s	TransVtx_PosW(a1),e2,e16
	fsub.s	TransVtx_U(a1),e3,e17
	fsub.s	TransVtx_V(a1),e4,e18
	fsub.s	TransVtx_PosZ(a1),e5,e19
	fsub.l	TransVtx_I(a1),e6,e20
	fsub.s	TransVtx_PosX(a2),e0,e21
	fsub.s	TransVtx_PosW(a2),e2,e22
	fsub.s	TransVtx_U(a2),e3,e23
	fsub.s	TransVtx_V(a2),e4,fp2
	fsub.s	TransVtx_PosZ(a2),e5,fp3
	fsub.l	TransVtx_I(a2),e6,fp4

	fmul	e14,e15
	fmul	e14,e16
	fmul	e14,e17
	fmul	e14,e18
	fmul	e14,e19
	fmul	e14,e20

	fmul	e13,e21
	fmul	e13,e22
	fmul	e13,e23
	fmul	e13,fp2
	fmul	e13,fp3
	fmul	e13,fp4

	fsub	e21,e15
	fsub	e22,e16
	fsub	e23,e17
	fsub	fp2,e18
	fsub	fp3,e19
	fsub	fp4,e20

	fadd	e15,e7
	fadd	e16,e8
	fadd	e17,e9
	fadd	e18,e10
	fadd	e19,e11
	fadd	e20,e12

	fmove.l	fp5,d3
	cmp.l	d3,d1
	ble.s	.lNoMin
	move.l	d3,d1
.lNoMin:
	cmp.l	d3,d2
	bge.s	.lNoMax
	move.l	d3,d2
.lNoMax:
	subq.l	#1,d4
	beq.s	.fanDone
	movea.l	a2,a1
	lea	TransVtx_Size(a2),a2
	bra.w	.linearLoop

;--- cull, span and the gradient scale -----------------------------------------
; Polygons reject area >= 0 where triangles reject area > 0 - preserved from the
; C. area is the cull-signed fan area, so after that test it is < 0 and the old
; two-sided epsilon check collapses to one compare.

.fanDone:
	fmul	e7,fp0			; area = denom * cullSign
	fmove.w	#1,e13			; the reciprocal's numerator

	cmp.l	d2,d1
	beq.w	.skip			; zero scanlines tall

	move.l	d1,MB_primMinY(a6)
	move.l	d2,MB_primMaxY(a6)
	lea	MB_gradients(a6),a0

	ftst	fp0
	fbge	.skip			; backfacing

; Hex bit pattern, not a decimal literal: vasm silently assembles "#-1e-6" as
; -0.0, which would make the fbgt below always false and kill the guard.
	fcmp.s	#$B58637BD,fp0		; -1e-6
	fbgt	.degenerate
	fdiv	e7,e13			; 1.0 / denom
	bra.s	.scale
.degenerate:
	fmove.w	#0,e13
.scale:
	fmul	e13,e8
	fmul	e13,e9
	fmul	e13,e10
	fmul	e13,e11
	fmul	e13,e12

	fmove.s	e8,Grad_oowDDA(a0)
	fmove.s	e9,Grad_uowDDA(a0)
	fmove.s	e10,Grad_vowDDA(a0)
	fmove.s	e11,Grad_zDDA(a0)
	fmove.s	e12,Grad_iDDA(a0)

	moveq	#1,d0
	fmovem	(sp)+,fp2-fp5
	movem.l	(sp)+,d2-d4/a2-a4
	rts

.skip:
	moveq	#0,d0
	fmovem	(sp)+,fp2-fp5
	movem.l	(sp)+,d2-d4/a2-a4
	rts

	public	_MaggieSetupPoly
