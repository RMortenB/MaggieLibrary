OPT MODULE
OPT 020
OPT 881
OPT FPEXP

/* Draw mode constants */
EXPORT CONST MAG_DRAWMODE_NORMAL := $0000
EXPORT CONST MAG_DRAWMODE_DEPTHBUFFER := $0001
EXPORT CONST MAG_DRAWMODE_BILINEAR := $0002
EXPORT CONST MAG_DRAWMODE_32BIT := $0004
EXPORT CONST MAG_DRAWMODE_LIGHTING := $0008
EXPORT CONST MAG_DRAWMODE_CULL_CCW := $0010

/* Clear flags */
EXPORT CONST MAG_CLEAR_COLOUR := $0001
EXPORT CONST MAG_CLEAR_DEPTH := $0002

/* Max lights */
EXPORT CONST MAG_MAX_LIGHTS = 8

/* Light types */
EXPORT CONST MAG_LIGHT_OFF := $0000
EXPORT CONST MAG_LIGHT_POINT := $0001
EXPORT CONST MAG_LIGHT_DIRECTIONAL := $0002
EXPORT CONST MAG_LIGHT_SPOT := $0003
EXPORT CONST MAG_LIGHT_AMBIENT := $0004

/* vector structs */
EXPORT OBJECT vec3
	x:LONG
	y:LONG
	z:LONG
ENDOBJECT

EXPORT OBJECT vec4
	x:LONG
	y:LONG
	z:LONG
	w:LONG
ENDOBJECT

/* Matrix struct */
EXPORT OBJECT mat4
	m[16]:ARRAY OF LONG
ENDOBJECT

/* Vector functions */

EXPORT PROC vec3_set(res:PTR TO vec3, x: LONG, y: LONG, z: LONG)
	res.x := x;
	res.y := y;
	res.z := z;
ENDPROC

EXPORT PROC vec3_dot(a:PTR TO vec3, b:PTR TO vec3)
	DEF res:LONG
	res := (!(!a.x * b.x) + (!a.y * b.y) + (!a.z * b.z))
ENDPROC res

EXPORT PROC vec3_cross(res:PTR TO vec3, a:PTR TO vec3, b:PTR TO vec3)
	res.x := (!(!a.y * b.z) - (!a.z * b.y))
	res.y := (!(!a.z * b.x) - (!a.x * b.z))
	res.z := (!(!a.x * b.y) - (!a.y * b.x))
ENDPROC

EXPORT PROC vec3_sub(res:PTR TO vec3, a:PTR TO vec3, b:PTR TO vec3)
	res.x := !a.x - b.x
	res.y := !a.y - b.y
	res.z := !a.z - b.z
ENDPROC

EXPORT PROC vec3_add(res:PTR TO vec3, a:PTR TO vec3, b:PTR TO vec3)
	res.x := !a.x + b.x
	res.y := !a.y + b.y
	res.z := !a.z + b.z
ENDPROC

EXPORT PROC vec3_scale(res:PTR TO vec3, a:PTR TO vec3, f:LONG)
	res.x := !a.x * f
	res.y := !a.y * f
	res.z := !a.z * f
ENDPROC

EXPORT PROC vec3_len(v:PTR TO vec3)
	DEF len : LONG
	len := Fsqrt(vec3_dot(v, v))
ENDPROC len

EXPORT PROC vec3_lensq(v:PTR TO vec3)
	DEF lensq : LONG
	lensq := vec3_dot(v, v)
ENDPROC lensq

EXPORT PROC vec3_normalise(res:PTR TO vec3, v:PTR TO vec3)
	DEF len:LONG
	DEF oolen:LONG

	len := vec3_len(v)
	oolen := !1.0 / len

	res.x := !v.x * oolen
	res.y := !v.y * oolen
	res.z := !v.z * oolen
ENDPROC len

/* Matrix functions */

EXPORT PROC mat4_identity(res:PTR TO mat4)
	DEF i, j
	FOR i := 0 TO 3
		FOR j := 0 TO 3
			IF i = j
				res.m[i * 4 + j] := 1.0
			ELSE
				res.m[i * 4 + j] := 0.0
			ENDIF
		ENDFOR
	ENDFOR
ENDPROC

EXPORT PROC mat4_inverseLight(res:PTR TO mat4, mat:PTR TO mat4)
	DEF i, j
	DEF v: LONG

	FOR i := 0 TO 2
		FOR j := 0 TO 2
			res.m[i * 4 + j] := mat.m[j * 4 + i]
		ENDFOR
		res.m[i * 4 + 3] := 0.0
	ENDFOR

	FOR i := 0 TO 3
		v := 0
		FOR j := 0 TO 3
			v := !-mat.m[3 * 4 + j] * res.m[j * 4 + i] + v
		ENDFOR
		res.m[3 * 4 + i] := v
	ENDFOR
	res.m[3 * 4 + 3] := 1.0
ENDPROC


EXPORT PROC mat4_perspective(res:PTR TO mat4, fov:LONG, aspect:LONG, znear:LONG, zfar:LONG)
	DEF w:LONG
	DEF h:LONG
	DEF denom:LONG

	fov := !fov * 0.5 * 3.1415927 / 180.0

	w := !Fcos(fov) / Fsin(fov)
	h := !w / aspect

	mat4_identity(res)
	res.m[0 * 4 + 0] := w
	res.m[1 * 4 + 1] := h

	denom := !zfar - znear

	res.m[2 * 4 + 2] := !zfar / denom
	res.m[3 * 4 + 2] := !-znear * zfar / denom
	res.m[2 * 4 + 3] := 1.0
	res.m[3 * 4 + 3] := 0.0
ENDPROC

EXPORT PROC mat4_rotateX(res:PTR TO mat4, angle:LONG)
	DEF s:LONG
	DEF c:LONG

	s := Fsin(angle)
	c := Fcos(angle)

	mat4_identity(res);

	res.m[1 * 4 + 1] := c
	res.m[1 * 4 + 2] := s
	res.m[2 * 4 + 1] := !-s
	res.m[2 * 4 + 2] := c
ENDPROC

/*****************************************************************************/

EXPORT PROC mat4_rotateY(res:PTR TO mat4, angle:LONG)
	DEF s:LONG
	DEF c:LONG

	s := Fsin(angle)
	c := Fcos(angle)

	mat4_identity(res);

	res.m[0 * 4 + 0] := c
	res.m[0 * 4 + 2] := !-s
	res.m[2 * 4 + 0] := s
	res.m[2 * 4 + 2] := c
ENDPROC

/*****************************************************************************/

EXPORT PROC mat4_rotateZ(res:PTR TO mat4, angle:LONG)
	DEF s:LONG
	DEF c:LONG

	s := Fsin(angle)
	c := Fcos(angle)

	mat4_identity(res);

	res.m[0 * 4 + 0] := c
	res.m[0 * 4 + 1] := !-s
	res.m[1 * 4 + 0] := s
	res.m[1 * 4 + 1] := c
ENDPROC

/*****************************************************************************/

EXPORT PROC mat4_translate(res:PTR TO mat4, x:LONG, y:LONG, z:LONG)
	mat4_identity(res);
	res.m[3 * 4 + 0] := x;
	res.m[3 * 4 + 1] := y;
	res.m[3 * 4 + 2] := z;
ENDPROC


EXPORT PROC mat4_LookAt(res:PTR TO mat4, pos:PTR TO vec3, target:PTR TO vec3, up:PTR TO vec3)
	DEF xVec:vec3
	DEF yVec:vec3
	DEF zVec:vec3

	vec3_sub(zVec, target, pos);
	vec3_normalise(zVec, zVec);
	vec3_cross(xVec, up, zVec);
	vec3_normalise(xVec, xVec);
	vec3_cross(yVec, zVec, xVec);

	res.m[0 * 4 + 0] := xVec.x
	res.m[0 * 4 + 1] := xVec.y
	res.m[0 * 4 + 2] := xVec.z
	res.m[0 * 4 + 3] := 0.0
	res.m[1 * 4 + 0] := yVec.x
	res.m[1 * 4 + 1] := yVec.y
	res.m[1 * 4 + 2] := yVec.z
	res.m[1 * 4 + 3] := 0.0
	res.m[2 * 4 + 0] := zVec.x
	res.m[2 * 4 + 1] := zVec.y
	res.m[2 * 4 + 2] := zVec.z
	res.m[2 * 4 + 3] := 0.0
	res.m[3 * 4 + 0] := pos.x
	res.m[3 * 4 + 1] := pos.y
	res.m[3 * 4 + 2] := pos.z
	res.m[3 * 4 + 3] := 1.0
ENDPROC

EXPORT PROC mat4_mul(res:PTR TO mat4, a:PTR TO mat4, b:PTR TO mat4)
	DEF a00:LONG, a01:LONG, a02:LONG, a03:LONG
	DEF a10:LONG, a11:LONG, a12:LONG, a13:LONG
	DEF a20:LONG, a21:LONG, a22:LONG, a23:LONG
	DEF a30:LONG, a31:LONG, a32:LONG, a33:LONG
	DEF b00:LONG, b01:LONG, b02:LONG, b03:LONG
	DEF b10:LONG, b11:LONG, b12:LONG, b13:LONG
	DEF b20:LONG, b21:LONG, b22:LONG, b23:LONG
	DEF b30:LONG, b31:LONG, b32:LONG, b33:LONG
	a00 := a.m[0 * 4 + 0] ; a01 := a.m[0 * 4 + 1] ; a02 := a.m[0 * 4 + 2] ; a03 := a.m[0 * 4 + 3]
	a10 := a.m[1 * 4 + 0] ; a11 := a.m[1 * 4 + 1] ; a12 := a.m[1 * 4 + 2] ; a13 := a.m[1 * 4 + 3]
	a20 := a.m[2 * 4 + 0] ; a21 := a.m[2 * 4 + 1] ; a22 := a.m[2 * 4 + 2] ; a23 := a.m[2 * 4 + 3]
	a30 := a.m[3 * 4 + 0] ; a31 := a.m[3 * 4 + 1] ; a32 := a.m[3 * 4 + 2] ; a33 := a.m[3 * 4 + 3]
	b00 := b.m[0 * 4 + 0] ; b01 := b.m[0 * 4 + 1] ; b02 := b.m[0 * 4 + 2] ; b03 := b.m[0 * 4 + 3]
	b10 := b.m[1 * 4 + 0] ; b11 := b.m[1 * 4 + 1] ; b12 := b.m[1 * 4 + 2] ; b13 := b.m[1 * 4 + 3]
	b20 := b.m[2 * 4 + 0] ; b21 := b.m[2 * 4 + 1] ; b22 := b.m[2 * 4 + 2] ; b23 := b.m[2 * 4 + 3]
	b30 := b.m[3 * 4 + 0] ; b31 := b.m[3 * 4 + 1] ; b32 := b.m[3 * 4 + 2] ; b33 := b.m[3 * 4 + 3]

	res.m[0 * 4 + 0] := !(!a00 * b00) + (!a10 * b01) + (!a20 * b02) + (!a30 * b03)
	res.m[0 * 4 + 1] := !(!a01 * b00) + (!a11 * b01) + (!a21 * b02) + (!a31 * b03)
	res.m[0 * 4 + 2] := !(!a02 * b00) + (!a12 * b01) + (!a22 * b02) + (!a32 * b03)
	res.m[0 * 4 + 3] := !(!a03 * b00) + (!a13 * b01) + (!a23 * b02) + (!a33 * b03)
	res.m[1 * 4 + 0] := !(!a00 * b10) + (!a10 * b11) + (!a20 * b12) + (!a30 * b13)
	res.m[1 * 4 + 1] := !(!a01 * b10) + (!a11 * b11) + (!a21 * b12) + (!a31 * b13)
	res.m[1 * 4 + 2] := !(!a02 * b10) + (!a12 * b11) + (!a22 * b12) + (!a32 * b13)
	res.m[1 * 4 + 3] := !(!a03 * b10) + (!a13 * b11) + (!a23 * b12) + (!a33 * b13)
	res.m[2 * 4 + 0] := !(!a00 * b20) + (!a10 * b21) + (!a20 * b22) + (!a30 * b23)
	res.m[2 * 4 + 1] := !(!a01 * b20) + (!a11 * b21) + (!a21 * b22) + (!a31 * b23)
	res.m[2 * 4 + 2] := !(!a02 * b20) + (!a12 * b21) + (!a22 * b22) + (!a32 * b23)
	res.m[2 * 4 + 3] := !(!a03 * b20) + (!a13 * b21) + (!a23 * b22) + (!a33 * b23)
	res.m[3 * 4 + 0] := !(!a00 * b30) + (!a10 * b31) + (!a20 * b32) + (!a30 * b33)
	res.m[3 * 4 + 1] := !(!a01 * b30) + (!a11 * b31) + (!a21 * b32) + (!a31 * b33)
	res.m[3 * 4 + 2] := !(!a02 * b30) + (!a12 * b31) + (!a22 * b32) + (!a32 * b33)
	res.m[3 * 4 + 3] := !(!a03 * b30) + (!a13 * b31) + (!a23 * b32) + (!a33 * b33)
ENDPROC

EXPORT PROC mat4_inverseLite(res:PTR TO mat4, mat:PTR TO mat4)
	DEF mat00:LONG, mat01:LONG, mat02:LONG, mat03:LONG
	DEF mat10:LONG, mat11:LONG, mat12:LONG, mat13:LONG
	DEF mat20:LONG, mat21:LONG, mat22:LONG, mat23:LONG
	DEF mat30:LONG, mat31:LONG, mat32:LONG, mat33:LONG

	mat00 := mat.m[0 * 4 + 0]; mat01 := mat.m[0 * 4 + 1]; mat02 := mat.m[0 * 4 + 2]; mat03 := mat.m[0 * 4 + 3]
	mat10 := mat.m[1 * 4 + 0]; mat11 := mat.m[1 * 4 + 1]; mat12 := mat.m[1 * 4 + 2]; mat13 := mat.m[1 * 4 + 3]
	mat20 := mat.m[2 * 4 + 0]; mat21 := mat.m[2 * 4 + 1]; mat22 := mat.m[2 * 4 + 2]; mat23 := mat.m[2 * 4 + 3]
	mat30 := mat.m[3 * 4 + 0]; mat31 := mat.m[3 * 4 + 1]; mat32 := mat.m[3 * 4 + 2]; mat33 := mat.m[3 * 4 + 3]

	res.m[0 * 4 + 0] := mat00; res.m[0 * 4 + 1] := mat10; res.m[0 * 4 + 2] := mat20; res.m[0 * 4 + 3] := 0.0
	res.m[1 * 4 + 0] := mat01; res.m[1 * 4 + 1] := mat11; res.m[1 * 4 + 2] := mat21; res.m[1 * 4 + 3] := 0.0
	res.m[2 * 4 + 0] := mat02; res.m[2 * 4 + 1] := mat12; res.m[2 * 4 + 2] := mat22; res.m[2 * 4 + 3] := 0.0

	res.m[3 * 4 + 0] := !(!mat30 * mat00) + (!mat31 * mat01) + (!mat32 * mat02)
	res.m[3 * 4 + 1] := !(!mat30 * mat10) + (!mat31 * mat11) + (!mat32 * mat12)
	res.m[3 * 4 + 2] := !(!mat30 * mat20) + (!mat31 * mat21) + (!mat32 * mat22)
	res.m[3 * 4 + 3] := 1.0
ENDPROC

