	section	.text

	include	<exec/types.i>
	include	"raster/raster_structs.i"

;------------------------------------------------------------------------------
; ULONG MaggieSetupTri(struct MaggieTransVertex *v0 __asm("a0"),
;                      struct MaggieTransVertex *v1 __asm("a1"),
;                      struct MaggieTransVertex *v2 __asm("a2"),
;                      MaggieBase *lib             __asm("a6"));
;
; Per-triangle setup: backface cull, screen-y span, and the five per-polygon attribute gradients.
;
;   returns 0  -> skip this triangle (backfacing, or zero scanlines tall)
;   returns 1  -> draw it; lib->gradients, lib->primMinY and lib->primMaxY have been written
;
; The caller still emits the three edges and dispatches the span renderer.
; Asm because the gradient block wants ~15 live floats: GCC only knows fp0-fp7 and spills, while e0-e23 are invisible to the C ABI and cost no prologue.
;
; Instruction order is scheduled, not readable: one FP issue per cycle, results ~6 cycles later (~10 for fdiv), in-order, stalling only on an unready operand.
; So independent work is parked in the cull chain's stalls, the five gradient chains get their own temps and are issued stage by stage, and the y span uses integer compares so it fits inside the FPU compare latencies.
;------------------------------------------------------------------------------
; REGISTERS
;   e0,e1,e2   y0,y1,y2, truncated in place
;   e3         x0, dead once both x deltas are formed
;   e4,e5      dx1,dx2, then recycled as the intensity chain's temps (safe: operands are read at issue, in-order)
;   e6,e7      dy1,dy2, live until the last numerator
;   e8         p1 -> denom, live until the reciprocal
;   e9         p2
;   e10..e14   v0's w,u,v,z,i
;   e15        -1.0 -> -1/denom (see the sign note at the deltas)
;   e16..e23   the other four chains' temps
;   fp0        cullSign, then area
;   d0,d1,d2   iy0,iy1,iy2 -> min in d0, max in d1
;
; Clobbers d0/d1/d2 (d2 saved) and e0-e23; fp0 is the cull compare (caller-saved), fp1-fp7 and the other callee-saved registers are untouched.
;------------------------------------------------------------------------------

_MaggieSetupTri:
	movem.l	d2/a2,-(sp)

;--- positions -----------------------------------------------------------------

	fmove.s	TransVtx_PosY(a0),e0
	fmove.s	TransVtx_PosY(a1),e1
	fmove.s	TransVtx_PosY(a2),e2
	fmove.s	TransVtx_PosX(a0),e3
	fmove.s	TransVtx_PosX(a1),e4
	fmove.s	TransVtx_PosX(a2),e5

;--- the cull chain, with its stalls filled ------------------------------------
; One cross product serves as both the cull area and the gradient denominator.

	fsub	e0,e1,e6		; dy1 = y1 - y0
	fsub	e0,e2,e7		; dy2 = y2 - y0
	fintrz	e0,e0			; independent: fills the cull's stalls
	fsub	e3,e4			; dx1 = x1 - x0
	fsub	e3,e5			; dx2 = x2 - x0
	fintrz	e1,e1
	fintrz	e2,e2
	fmove.s	MB_cullSign(a6),fp0
	fmove.s	TransVtx_PosW(a0),e10
	fmove.s	TransVtx_U(a0),e11
	fmove.s	TransVtx_V(a0),e12
	fmove.s	TransVtx_PosZ(a0),e13
	fmove.l	TransVtx_I(a0),e14
	fmove.w	#-1,e15			; hoisted so the fdiv never waits on its numerator

	fmul	e7,e4,e8		; p1 = dx1 * dy2
	fmul	e6,e5,e9		; p2 = dx2 * dy1

;--- attribute deltas ----------------------------------------------------------
; Both deltas per attribute are formed NEGATED, as (f0 - f1) and (f0 - f2), so the load fuses into one fsub.s <ea>,eSrc,eDst; the negated reciprocal corrects it.
; Bit-exact against the straightforward form except for the sign of an exact zero, which the DDA cannot observe.
; They sit ahead of the cull's ftst purely as filler for the area multiply's latency; e4/e5 are recycled from dx1/dx2.

	fsub.s	TransVtx_PosW(a1),e10,e16
	fsub.s	TransVtx_PosW(a2),e10,e17
	fsub.s	TransVtx_U(a1),e11,e18
	fsub.s	TransVtx_U(a2),e11,e19
	fsub.s	TransVtx_V(a1),e12,e20

	fsub	e9,e8			; denom = p1 - p2, its inputs now well clear

	fsub.s	TransVtx_V(a2),e12,e21
	fsub.s	TransVtx_PosZ(a1),e13,e22
	fsub.s	TransVtx_PosZ(a2),e13,e23
	fsub.l	TransVtx_I(a1),e14,e4
	fsub.l	TransVtx_I(a2),e14,e5
	fmove.l	e0,d0			; iy0   three independent extractions, so none of
	fmove.l	e1,d1			; iy1   them waits on the one before
	fmove.l	e2,d2			; iy2

	fmul	e8,fp0			; area = denom * cullSign

;--- screen-y span, in integer -------------------------------------------------
; Truncation is monotonic, so min(trunc(y)) == trunc(min(y)), matching the C exactly.
; Sitting between ftst and its fbgt is deliberate: CCR compares cannot disturb the pending FPCC, so they cover the ftst latency for free.

	cmp.l	d1,d0
	ble.s	.sorted
	exg	d0,d1			; d0 = min(iy0,iy1), d1 = max(iy0,iy1)
.sorted:
	cmp.l	d2,d0
	ble.s	.gotMin
	move.l	d2,d0
.gotMin:
	lea	MB_gradients(a6),a0	; v0 is fully loaded, so a0 is free

	ftst	fp0

	cmp.l	d2,d1
	bge.s	.gotMax
	move.l	d2,d1
.gotMax:
	cmp.l	d1,d0
	beq.w	.skip			; zero scanlines tall

	move.l	d0,MB_primMinY(a6)
	move.l	d1,MB_primMaxY(a6)

	fbgt	.skip			; backfacing

;--- the reciprocal, and the stages that do not need it ------------------------
; area is cull-signed, so it is <= 0 here and the epsilon test is one compare; a zero scale zeroes every gradient, as the C did for a degenerate triangle.
; The fdiv cannot be hoisted above the test: a fully degenerate triangle survives the cull with denom exactly 0, which would trap where FPU exceptions are enabled.
; Scale by the reciprocal only AFTER the subtraction - folding it into dy1/dy2 saves five multiplies but costs up to 0.4% relative error on slivers.
; The * dy2 stage MUST stay ahead of the fcmp: FP arithmetic sets the FPCC from its result, so nothing but integer instructions may sit between the fcmp and the fbgt.
	fmul	e7,e16			; * dy2
	fmul	e7,e18
	fmul	e7,e20
	fmul	e7,e22
	fmul	e7,e4

; The epsilon MUST be a hex bit pattern: vasm silently assembles "#-1e-6" as -0.0, which would remove the guard and let the fdiv see a zero denom.
	fcmp.s	#$B58637BD,fp0		; -1e-6
	fbgt	.degenerate
	fdiv	e8,e15			; -1.0 / denom
	bra.s	.scaled
.degenerate:
	fmove.w	#0,e15
.scaled:

;--- the remaining stages ------------------------------------------------------
; Intensity used fsub.l above - a signed long, matching the (float)(int) cast in C; it is clamped to 0..0xffff upstream.

	fmul	e6,e17			; * dy1
	fmul	e6,e19
	fmul	e6,e21
	fmul	e6,e23
	fmul	e6,e5

	fsub	e17,e16
	fsub	e19,e18
	fsub	e21,e20
	fsub	e23,e22
	fsub	e5,e4

	fmul	e15,e16			; * -1/denom
	fmul	e15,e18
	fmul	e15,e20
	fmul	e15,e22
	fmul	e15,e4

	fmove.s	e16,Grad_oowDDA(a0)
	fmove.s	e18,Grad_uowDDA(a0)
	fmove.s	e20,Grad_vowDDA(a0)
	fmove.s	e22,Grad_zDDA(a0)
	fmove.s	e4,Grad_iDDA(a0)

	moveq	#1,d0
	movem.l	(sp)+,d2/a2
	rts

.skip:
	moveq	#0,d0
	movem.l	(sp)+,d2/a2
	rts

	public	_MaggieSetupTri
