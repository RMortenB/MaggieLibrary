#ifndef MAGGIE_VEC3_H_INCLUDED
#define MAGGIE_VEC3_H_INCLUDED

#include <math.h>

typedef struct vec3
{
	float x, y, z;
} vec3;

/*****************************************************************************/

typedef struct vec4
{
	float x, y, z, w;
} vec4;

/*****************************************************************************/

typedef struct quat4
{
	float x, y, z, w;
} quat4;

/*****************************************************************************/

typedef struct mat4
{
	float m[4][4];
} mat4;

/*****************************************************************************/

static float mag_rsqrtf(float num)
{
	const float threehalfs = 1.5F;
	union
	{
		int i;
		float f;
	} val;

	float x2 = num * 0.5F;
	val.f = num;
	val.i  = 0x5f375a86 - (val.i >> 1);
	num  = val.f;
	num  = num * (threehalfs - (x2 * num * num));   /* 1st iteration */

	return num;
}

/*****************************************************************************/

static float mag_sqrtf(float num)
{
        return mag_rsqrtf(num) * num;
}

/*****************************************************************************/

static void vec3_set(vec3 *res, float x, float y, float z)
{
	res->x = x;
	res->y = y;
	res->z = z;
}

/*****************************************************************************/

static void vec3_neg(vec3 *res, vec3 *a)
{
	res->x = -a->x;
	res->y = -a->y;
	res->z = -a->z;
}

/*****************************************************************************/

static float vec3_dot(const vec3 *a, const vec3 *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

/*****************************************************************************/

static void vec3_cross(vec3 *res, const vec3 *a, const vec3 *b)
{
	res->x = a->y * b->z - a->z * b->y;
	res->y = a->z * b->x - a->x * b->z;
	res->z = a->x * b->y - a->y * b->x;
}

/*****************************************************************************/

static void vec3_sub(vec3 *res, const vec3 *a, const vec3 *b)
{
	res->x = a->x - b->x;
	res->y = a->y - b->y;
	res->z = a->z - b->z;
}

/*****************************************************************************/

static void vec3_add(vec3 *res, const vec3 *a, const vec3 *b)
{
	res->x = a->x + b->x;
	res->y = a->y + b->y;
	res->z = a->z + b->z;
}

/*****************************************************************************/

static void vec3_scale(vec3 *res, const vec3 *v, float f)
{
	res->x = v->x * f;
	res->y = v->y * f;
	res->z = v->z * f;
}

/*****************************************************************************/

static float vec3_len(const vec3 *v)
{
	return mag_sqrtf(vec3_dot(v, v));
}

/*****************************************************************************/

static float vec3_lensq(const vec3 *v)
{
	return vec3_dot(v, v);
}

/*****************************************************************************/

static float vec3_normalise(vec3 *res, const vec3 *v)
{
	float len = vec3_len(v);
	float oolen = 1.0f / len;

	res->x = v->x * oolen;
	res->y = v->y * oolen;
	res->z = v->z * oolen;

	return len;
}

/*****************************************************************************/

static void vec3_min(vec3 *res, const vec3 *a, const vec3 *b)
{
	res->x = a->x < b->x ? a->x : b->x;
	res->y = a->y < b->y ? a->y : b->y;
	res->z = a->z < b->z ? a->z : b->z;
}

/*****************************************************************************/

static void vec3_max(vec3 *res, const vec3 *a, const vec3 *b)
{
	res->x = a->x > b->x ? a->x : b->x;
	res->y = a->y > b->y ? a->y : b->y;
	res->z = a->z > b->z ? a->z : b->z;
}

/*****************************************************************************/

static void vec3_clamp(vec3 *res, const vec3 *v, float minVal, float maxVal)
{
	res->x = v->x < maxVal ? (v->x > minVal ? v->x : minVal) : maxVal;
	res->y = v->y < maxVal ? (v->y > minVal ? v->y : minVal) : maxVal;
	res->z = v->z < maxVal ? (v->z > minVal ? v->z : minVal) : maxVal;
}

/*****************************************************************************/

static void vec3_lerp(vec3 *res, const vec3 *a, const vec3 *b, float ratio)
{
	res->x = (b->x - a->x) * ratio + a->x;
	res->y = (b->y - a->y) * ratio + a->y;
	res->z = (b->z - a->z) * ratio + a->z;
}

/*****************************************************************************/

static void vec3_barycentric(vec3 *res, vec3 *a, vec3 *b, vec3 *c, vec3 *p)
{
	vec3 v0, v1, v2;
	float d00, d01, d11, d20, d21;
	float denom;
	vec3_sub(&v0, b, a);
	vec3_sub(&v1, c, a);
	vec3_sub(&v2, p, a);
	d00 = vec3_dot(&v0, &v0);
	d01 = vec3_dot(&v0, &v1);
	d11 = vec3_dot(&v1, &v1);
	d20 = vec3_dot(&v2, &v0);
	d21 = vec3_dot(&v2, &v1);
	denom = d00 * d11 - d01 * d01;
	res->x = (d11 * d20 - d01 * d21) / denom;
	res->y = (d00 * d21 - d01 * d20) / denom;
	res->z = 1.0f - res->x - res->y;
}

/*****************************************************************************/

static void mat4_identity(mat4 *res)
{
	int i, j;
	for(i = 0; i < 4; ++i)
	{
		for(j = 0; j < 4; ++j)
		{
			res->m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
}

/*****************************************************************************/

static void vec3_tformh(vec4 *res, const mat4 *mat, const vec3 *v, float w)
{
	float x = v->x;
	float y = v->y;
	float z = v->z;
	res->x = mat->m[0][0] * x + mat->m[1][0] * y + mat->m[2][0] * z + mat->m[3][0] * w;
	res->y = mat->m[0][1] * x + mat->m[1][1] * y + mat->m[2][1] * z + mat->m[3][1] * w;
	res->z = mat->m[0][2] * x + mat->m[1][2] * y + mat->m[2][2] * z + mat->m[3][2] * w;
	res->w = mat->m[0][3] * x + mat->m[1][3] * y + mat->m[2][3] * z + mat->m[3][3] * w;
}

/*****************************************************************************/

static void vec3_tform(vec3 *res, const mat4 *mat, const vec3 *v, float w)
{
	float x = v->x;
	float y = v->y;
	float z = v->z;
	res->x = mat->m[0][0] * x + mat->m[1][0] * y + mat->m[2][0] * z + mat->m[3][0] * w;
	res->y = mat->m[0][1] * x + mat->m[1][1] * y + mat->m[2][1] * z + mat->m[3][1] * w;
	res->z = mat->m[0][2] * x + mat->m[1][2] * y + mat->m[2][2] * z + mat->m[3][2] * w;
}

/*****************************************************************************/

static void vec4_tform(vec4 *res, const mat4 *mat, const vec4 *v)
{
	float x = v->x;
	float y = v->y;
	float z = v->z;
	float w = v->w;
	res->x = mat->m[0][0] * x + mat->m[1][0] * y + mat->m[2][0] * z + mat->m[3][0] * w;
	res->y = mat->m[0][1] * x + mat->m[1][1] * y + mat->m[2][1] * z + mat->m[3][1] * w;
	res->z = mat->m[0][2] * x + mat->m[1][2] * y + mat->m[2][2] * z + mat->m[3][2] * w;
	res->w = mat->m[0][3] * x + mat->m[1][3] * y + mat->m[2][3] * z + mat->m[3][3] * w;
}

/*****************************************************************************/

static float vec4_dot(const vec4 *a, const vec4 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

/*****************************************************************************/

static void mat4_mul(mat4 *res, const mat4 *a, const mat4 *b)
{
	float a00 = a->m[0][0], a01 = a->m[0][1], a02 = a->m[0][2], a03 = a->m[0][3];
	float a10 = a->m[1][0], a11 = a->m[1][1], a12 = a->m[1][2], a13 = a->m[1][3];
	float a20 = a->m[2][0], a21 = a->m[2][1], a22 = a->m[2][2], a23 = a->m[2][3];
	float a30 = a->m[3][0], a31 = a->m[3][1], a32 = a->m[3][2], a33 = a->m[3][3];
	float b00 = b->m[0][0], b01 = b->m[0][1], b02 = b->m[0][2], b03 = b->m[0][3];
	float b10 = b->m[1][0], b11 = b->m[1][1], b12 = b->m[1][2], b13 = b->m[1][3];
	float b20 = b->m[2][0], b21 = b->m[2][1], b22 = b->m[2][2], b23 = b->m[2][3];
	float b30 = b->m[3][0], b31 = b->m[3][1], b32 = b->m[3][2], b33 = b->m[3][3];

	res->m[0][0] = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
	res->m[0][1] = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
	res->m[0][2] = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
	res->m[0][3] = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;
	res->m[1][0] = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
	res->m[1][1] = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
	res->m[1][2] = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
	res->m[1][3] = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;
	res->m[2][0] = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
	res->m[2][1] = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
	res->m[2][2] = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
	res->m[2][3] = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;
	res->m[3][0] = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
	res->m[3][1] = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
	res->m[3][2] = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
	res->m[3][3] = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;
}

/*****************************************************************************/

static void mat4_rotateX(mat4 *res, float angle)
{
	float s = (float)sin(angle);
	float c = (float)cos(angle);

	mat4_identity(res);

	res->m[1][1] = c;
	res->m[1][2] = s;
	res->m[2][1] = -s;
	res->m[2][2] = c;
}

/*****************************************************************************/

static void mat4_rotateY(mat4 *res, float angle)
{
	float s = (float)sin(angle);
	float c = (float)cos(angle);

	mat4_identity(res);

	res->m[0][0] = c;
	res->m[0][2] = -s;
	res->m[2][0] = s;
	res->m[2][2] = c;
}

/*****************************************************************************/

static void mat4_rotateZ(mat4 *res, float angle)
{
	float s = (float)sin(angle);
	float c = (float)cos(angle);

	mat4_identity(res);

	res->m[0][0] = c;
	res->m[0][1] = -s;
	res->m[1][0] = s;
	res->m[1][1] = c;
}

/*****************************************************************************/

static void mat4_translate(mat4 *res, float x, float y, float z)
{
	mat4_identity(res);
	res->m[3][0] = x;
	res->m[3][1] = y;
	res->m[3][2] = z;
}

/*****************************************************************************/

static void mat4_perspective(mat4 *res, float fov, float aspect, float znear, float zfar)
{
	float w, h;

	fov *= 0.5f * 3.1415927f / 180.0f;
	w = (float)cos(fov) / (float)sin(fov);
	h = w / aspect;

	mat4_identity(res);
	res->m[0][0] = w;
	res->m[1][1] = h;

	res->m[2][2] = zfar / (zfar - znear);
	res->m[3][2] = -znear * zfar / (zfar - znear);
	res->m[2][3] = 1.0f;
	res->m[3][3] = 0.0f;
}

/*****************************************************************************/

static void mat4_inverseLight(mat4 *res, const mat4 *mat)
{
	int i, j;
	for(i = 0; i < 3; ++i)
	{
		for(j = 0; j < 3; ++j)
		{
			res->m[i][j] = mat->m[j][i];
		}
		res->m[i][3] = 0.0f;
	}
	for(i = 0; i < 3; ++i)
	{
		res->m[3][i] = 0.0f;
		for(j = 0; j < 3; ++j)
		{
			res->m[3][i] += -res->m[j][i] * mat->m[3][j];
		}
	}
	res->m[3][3] = 1.0f;
}

/*****************************************************************************/

static void mat4_LookAt(mat4 *mat, const vec3 *pos, const vec3 *target, const vec3 *up)
{
	vec3 xVec, yVec, zVec;

	vec3_sub(&zVec, target, pos);
	vec3_normalise(&zVec, &zVec);
	vec3_cross(&xVec, up, &zVec);
	vec3_normalise(&xVec, &xVec);
	vec3_cross(&yVec, &zVec, &xVec);

	mat->m[0][0] = xVec.x;
	mat->m[0][1] = xVec.y;
	mat->m[0][2] = xVec.z;
	mat->m[0][3] = 0.0f;
	mat->m[1][0] = yVec.x;
	mat->m[1][1] = yVec.y;
	mat->m[1][2] = yVec.z;
	mat->m[1][3] = 0.0f;
	mat->m[2][0] = zVec.x;
	mat->m[2][1] = zVec.y;
	mat->m[2][2] = zVec.z;
	mat->m[2][3] = 0.0f;
	mat->m[3][0] = pos->x;
	mat->m[3][1] = pos->y;
	mat->m[3][2] = pos->z;
	mat->m[3][3] = 1.0f;
}

/*****************************************************************************/

static void mat4_sync(mat4 *mat)
{
	vec3 xVec = {mat->m[0][0], mat->m[0][1], mat->m[0][2] };
	vec3 zVec = {mat->m[2][0], mat->m[2][1], mat->m[2][2] };
	vec3 yVec;

	vec3_cross(&yVec, &zVec, &xVec);
	vec3_normalise(&zVec, &zVec);
	vec3_normalise(&yVec, &yVec);

	vec3_cross(&xVec, &yVec, &zVec);
	vec3_normalise(&xVec, &xVec);

	mat->m[0][0] = xVec.x;
	mat->m[0][1] = xVec.y;
	mat->m[0][2] = xVec.z;
	mat->m[1][0] = yVec.x;
	mat->m[1][1] = yVec.y;
	mat->m[1][2] = yVec.z;
	mat->m[2][0] = zVec.x;
	mat->m[2][1] = zVec.y;
	mat->m[2][2] = zVec.z;
}

/*****************************************************************************/

static void mat4_FromQuat(mat4 *res, quat4 *q)
{
	float qx = q->x;
	float qy = q->y;
	float qz = q->z;
	float qw = q->w;

	res->m[0][0] = 1.0f - 2.0f * qy * qy - 2.0f * qz * qz;
	res->m[0][1] =        2.0f * qx * qy - 2.0f * qz * qw;
	res->m[0][2] =        2.0f * qx * qz + 2.0f * qy * qw;
	res->m[0][3] = 0.0f;

	res->m[1][0] =        2.0f * qx * qy + 2.0f * qz * qw;
	res->m[1][1] = 1.0f - 2.0f * qx * qx - 2.0f * qz * qz;
	res->m[1][2] =        2.0f * qy * qz - 2.0f * qx * qw;
	res->m[1][3] = 0.0f;

	res->m[2][0] =        2.0f * qx * qz - 2.0f * qy * qw;
	res->m[2][1] =        2.0f * qy * qz + 2.0f * qx * qw;
	res->m[2][2] = 1.0f - 2.0f * qx * qx - 2.0f * qy * qy;
	res->m[2][3] = 0.0f;

	res->m[3][0] = 0.0f;
	res->m[3][1] = 0.0f;
	res->m[3][2] = 0.0f;
	res->m[3][3] = 1.0f;
}


/*****************************************************************************/

static void quat4_AxisAngle(quat4 *res, vec3 *axis, float angle)
{
	float s = (float)sin(angle / 2.0f);
	res->x = axis->x * s;
	res->y = axis->y * s;
	res->z = axis->z * s;
	res->w = (float)cos(angle / 2.0f);
}

/*****************************************************************************/
 
 /* MAGGIE_VEC3_H_INCLUDED */
#endif
