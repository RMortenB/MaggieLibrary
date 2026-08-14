	section	.text

	include <exec/types.i>
	include "raster/raster_structs.i"

;------------------------------------------------------------------------------

_DrawScanlines32:
.bpp = 4
.polyIntensity = 0
	include "raster/raster_perspective16.inc"
	public _DrawScanlines32

;------------------------------------------------------------------------------

_DrawScanlines32Poly:
.bpp = 4
.polyIntensity = 1
	include "raster/raster_perspective16.inc"
	public _DrawScanlines32Poly

;------------------------------------------------------------------------------

_DrawScanlines32IZ:
.bpp = 4
.polyIntensity = 0
	include "raster/raster_perspective16_depth.inc"
	public _DrawScanlines32IZ

;------------------------------------------------------------------------------

_DrawScanlines32IZPoly:
.bpp = 4
.polyIntensity = 1
	include "raster/raster_perspective16_depth.inc"
	public _DrawScanlines32IZPoly

;------------------------------------------------------------------------------

_DrawScanlines32Affine:
.bpp = 4
.polyIntensity = 0
	include "raster/raster_affine.inc"
	public _DrawScanlines32Affine

;------------------------------------------------------------------------------

_DrawScanlines32AffinePoly:
.bpp = 4
.polyIntensity = 1
	include "raster/raster_affine.inc"
	public _DrawScanlines32AffinePoly

;------------------------------------------------------------------------------

_DrawScanlines32ZAffine:
.bpp = 4
.polyIntensity = 0
	include "raster/raster_affine_depth.inc"
	public _DrawScanlines32ZAffine

;------------------------------------------------------------------------------

_DrawScanlines32ZAffinePoly:
.bpp = 4
.polyIntensity = 1
	include "raster/raster_affine_depth.inc"
	public _DrawScanlines32ZAffinePoly

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

_DrawScanlines16:
.bpp = 2
.polyIntensity = 0
	include "raster/raster_perspective16.inc"
	public _DrawScanlines16

;------------------------------------------------------------------------------

_DrawScanlines16Poly:
.bpp = 2
.polyIntensity = 1
	include "raster/raster_perspective16.inc"
	public _DrawScanlines16Poly

;------------------------------------------------------------------------------

_DrawScanlines16IZ:
.bpp = 2
.polyIntensity = 0
	include "raster/raster_perspective16_depth.inc"
	public _DrawScanlines16IZ

;------------------------------------------------------------------------------

_DrawScanlines16IZPoly:
.bpp = 2
.polyIntensity = 1
	include "raster/raster_perspective16_depth.inc"
	public _DrawScanlines16IZPoly

;------------------------------------------------------------------------------

_DrawScanlines16Affine:
.bpp = 2
.polyIntensity = 0
	include "raster/raster_affine.inc"
	public _DrawScanlines16Affine

;------------------------------------------------------------------------------

_DrawScanlines16AffinePoly:
.bpp = 2
.polyIntensity = 1
	include "raster/raster_affine.inc"
	public _DrawScanlines16AffinePoly

;------------------------------------------------------------------------------

_DrawScanlines16ZAffine:
.bpp = 2
.polyIntensity = 0
	include "raster/raster_affine_depth.inc"
	public _DrawScanlines16ZAffine

;------------------------------------------------------------------------------

_DrawScanlines16ZAffinePoly:
.bpp = 2
.polyIntensity = 1
	include "raster/raster_affine_depth.inc"
	public _DrawScanlines16ZAffinePoly

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
