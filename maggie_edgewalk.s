	section	.text

	include <exec/types.i>
	include "raster/raster_structs.i"

; The scanline-edge DDAs, one per (column side x draw mode) combination that needs a different set of columns written. The body and the flag documentation are in raster/edge_walk.inc.
; BeginDrawBatch (maggie_draw.c) binds one left and one right variant per batch into lib->edgeFuncDown / lib->edgeFuncUp, swapped for MAG_DRAWMODE_CULL_CCW. Both write from magEdge itself.

;------------------------------------------------------------------------------
; Left column, perspective + depth: x i u v z oow

_DrawEdgeLeftWZ:
.rightSide = 0
.doI = 1
.doZ = 1
.doOow = 1
	include "raster/edge_walk.inc"
	public _DrawEdgeLeftWZ

;------------------------------------------------------------------------------
; Left column, perspective: x i u v oow

_DrawEdgeLeftW:
.rightSide = 0
.doI = 1
.doZ = 0
.doOow = 1
	include "raster/edge_walk.inc"
	public _DrawEdgeLeftW

;------------------------------------------------------------------------------
; Left column, affine + depth: x i u v z

_DrawEdgeLeftZ:
.rightSide = 0
.doI = 1
.doZ = 1
.doOow = 0
	include "raster/edge_walk.inc"
	public _DrawEdgeLeftZ

;------------------------------------------------------------------------------
; Left column, affine: x i u v

_DrawEdgeLeft:
.rightSide = 0
.doI = 1
.doZ = 0
.doOow = 0
	include "raster/edge_walk.inc"
	public _DrawEdgeLeft

;------------------------------------------------------------------------------
; Right column, polygon source: xPosRight + iRight. Gated on sourceIsPoly, which also decides whether the …Poly span renderers - iRight's only readers - are reachable.

_DrawEdgeRightI:
.rightSide = 1
.doI = 1
.doZ = 0
.doOow = 0
	include "raster/edge_walk.inc"
	public _DrawEdgeRightI

;------------------------------------------------------------------------------
; Right column, triangle source: xPosRight alone. Two instructions per scanline.

_DrawEdgeRight:
.rightSide = 1
.doI = 0
.doZ = 0
.doOow = 0
	include "raster/edge_walk.inc"
	public _DrawEdgeRight

;------------------------------------------------------------------------------
