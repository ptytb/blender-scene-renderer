#include <string.h>
#include <math.h>
#include <stdio.h>

#include "matrix.h"

void v3f_subv(const vec3f_t *a, const vec3f_t *b, vec3f_t *r)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
}

void v3f_addv(const vec3f_t *a, const vec3f_t *b, vec3f_t *r)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
}

float v3f_norm(const vec3f_t *v)
{
	return v->x * v->x + v->y * v->y + v->z * v->z;
}

void v3f_normalize(vec3f_t *v)
{
	float hyp;

	hyp = sqrt(v3f_norm(v));
	v->x /= hyp;
	v->y /= hyp;
	v->z /= hyp;
}

void v4f_subv(const vec4f_t *a, const vec4f_t *b, vec4f_t *r)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
	r->w = 1.0f;
}

void v4f_addv(const vec4f_t *a, const vec4f_t *b, vec4f_t *r)
{
	r->x = a->x + b->x;
	r->y = a->y + b->y;
	r->z = a->z + b->z;
	r->w = 1.0f;
}

float v4f_mulv_scal(const vec4f_t *a, const vec4f_t *b)
{
	return (a->x * b->x + a->y * b->y + a->z * b->z) /
		sqrt( (a->x * a->x + a->y * a->y + a->z * a->z) *
				(b->x * b->x + b->y * b->y + b->z * b->z) );
}

void v4f_norm(vec4f_t *a)
{
	float hyp;

	hyp = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
	a->x = a->x / hyp;
	a->y = a->y / hyp;
	a->z = a->z / hyp;
}

void v4f_dir_cos(const vec4f_t *a, vec4f_t *r)
{
	float hyp;

	hyp = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
	r->x = a->x / hyp;
	r->y = a->y / hyp;
	r->z = a->z / hyp;
}

void m4f_set_zero(mat4f_t *m)
{
	memset(m, '\0', sizeof(mat4f_t));
}

void m4f_set_id(mat4f_t *m)
{
	m->sx = 1.0f;
	m->sy = 1.0f;
	m->sz = 1.0f;
	m->w = 1.0f;
	m->hxy = 0.0f;
	m->hxz = 0.0f;
	m->t = 0.0f;
	m->hyx = 0.0f;
	m->hyz = 0.0f;
	m->u = 0.0f;
	m->hzx = 0.0f;
	m->hzy = 0.0f;
	m->v = 0.0f;
	m->tx = 0.0f;
	m->ty = 0.0f;
	m->tz = 0.0f;
}

void m4f_op_mulv(const mat4f_t *m, const vec4f_t *v, vec4f_t *r)
{
	r->x = m->sx  * v->x + m->hxy * v->y + m->hxz * v->z + m->tx * v->w;
	r->y = m->hyx * v->x + m->sy  * v->y + m->hyz * v->z + m->ty * v->w;
	r->z = m->hzx * v->x + m->hzy * v->y + m->sz  * v->z + m->tz * v->w;
	r->w = m->t   * v->x + m->u   * v->y + m->v   * v->z + m->w  * v->w;
}

void m4f_op_mul(const mat4f_t *a, const mat4f_t *b, mat4f_t *r)
{
	r->sx  = a->sx  * b->sx  + a->hxy * b->hyx + a->hxz * b->hzx + a->tx * b->t;
	r->hyx = a->hyx * b->sx  + a->sy  * b->hyx + a->hyz * b->hzx + a->ty * b->t;
	r->hzx = a->hzx * b->sx  + a->hzy * b->hyx + a->sz  * b->hzx + a->tz * b->t;
	r->t   = a->t   * b->sx  + a->u   * b->hyx + a->v   * b->hzx + a->w  * b->t;

	r->hxy = a->sx  * b->hxy + a->hxy * b->sy  + a->hxz * b->hzy + a->tx * b->u;
	r->sy  = a->hyx * b->hxy + a->sy  * b->sy  + a->hyz * b->hzy + a->ty * b->u;
	r->hzy = a->hzx * b->hxy + a->hzy * b->sy  + a->sz  * b->hzy + a->tz * b->u;
	r->u   = a->t   * b->hxy + a->u   * b->sy  + a->v   * b->hzy + a->w  * b->u;

	r->hxz = a->sx  * b->hxz + a->hxy * b->hyz + a->hxz * b->sz  + a->tx * b->v;
	r->hyz = a->hyx * b->hxz + a->sy  * b->hyz + a->hyz * b->sz  + a->ty * b->v;
	r->sz  = a->hzx * b->hxz + a->hzy * b->hyz + a->sz  * b->sz  + a->tz * b->v;
	r->v   = a->t   * b->hxz + a->u   * b->hyz + a->v   * b->sz  + a->w  * b->v;

	r->tx  = a->sx  * b->tx  + a->hxy * b->ty  + a->hxz * b->tz  + a->tx * b->w;
	r->ty  = a->hyx * b->tx  + a->sy  * b->ty  + a->hyz * b->tz  + a->ty * b->w;
	r->tz  = a->hzx * b->tx  + a->hzy * b->ty  + a->sz  * b->tz  + a->tz * b->w;
	r->w   = a->t   * b->tx  + a->u   * b->ty  + a->v   * b->tz  + a->w  * b->w;
}

void m4f_rotx(mat4f_t *m, float theta)
{
	float sin_theta, cos_theta;

	sin_theta = sin(theta);
	cos_theta = cos(theta);

	m->sy  = cos_theta;
	m->hyz = - sin_theta;
	m->hzy = sin_theta;
	m->sz  = cos_theta;
}

void m4f_roty(mat4f_t *m, float theta)
{
	float sin_theta, cos_theta;

	sin_theta = sin(theta);
	cos_theta = cos(theta);
	
	m->sx  = cos_theta;
	m->hxz = sin_theta;
	m->hzx = - sin_theta;
	m->sz  = cos_theta;
}

void m4f_rotz(mat4f_t *m, float theta)
{
	float sin_theta, cos_theta;

	sin_theta = sin(theta);
	cos_theta = cos(theta);

	m->sx  = cos_theta;
	m->hxy = - sin_theta;
	m->hyx = sin_theta;
	m->sy  = cos_theta;
}

void m4f_rot_xyz(mat4f_t *m, float ax, float ay, float az)
{
	float sin_ax, sin_ay, sin_az;
	float cos_ax, cos_ay, cos_az;

	sin_ax = sin(ax);
	sin_ay = sin(ay);
	sin_az = sin(az);

	cos_ax = cos(ax);
	cos_ay = cos(ay);
	cos_az = cos(az);

	m->sx  = cos_ay * cos_az;
	m->hyx = - cos_ay * sin_az;
	m->hzx = sin_ay;

	m->hxy = cos_ax * sin_az + sin_ax * sin_ay * cos_az;
	m->sy  = cos_ax * cos_az - sin_ax * sin_ay * sin_az;
	m->hzy = - sin_ax * cos_ay;

	m->hxz = sin_ax * sin_az - cos_ax * sin_ay * cos_az;
	m->hyz = sin_ax * cos_az + cos_ax * sin_ay * sin_az;
	m->sz  = cos_ax * cos_ay;
}

void m4f_rot_zxy(mat4f_t *m, float ax, float ay, float az)
{
	float sin_1, sin_2, sin_3;
	float cos_1, cos_2, cos_3;

	sin_2 = sin(ax);
	sin_3 = sin(ay);
	sin_1 = sin(az);

	cos_2 = cos(ax);
	cos_3 = cos(ay);
	cos_1 = cos(az);

	m->sx  = cos_1 * cos_3 - sin_1 * sin_2 * sin_3;
	m->hxy = cos_3 * sin_1 + cos_1 * sin_2 * sin_3;
	m->hxz = - cos_2 * sin_3;

	m->hyx = - cos_2 * sin_1;
	m->sy  = cos_1 * cos_2;
	m->hyz = sin_2;

	m->hzx = cos_1 * sin_3 + cos_3 * sin_1 * sin_2;
	m->hzy = sin_1 * sin_3 - cos_1 * cos_3 * sin_2;
	m->sz  = cos_2 * cos_3;
}

/* Active rotate Euler YXZ, not transposed */
void m4f_rot_yxz(mat4f_t *m, float ax, float ay, float az)
{
	float sin_1, sin_2, sin_3;
	float cos_1, cos_2, cos_3;

	sin_2 = sin(ax);
	sin_1 = sin(ay);
	sin_3 = sin(az);

	cos_2 = cos(ax);
	cos_1 = cos(ay);
	cos_3 = cos(az);

	m->sx  = cos_1 * cos_3 + sin_1 * sin_2 * sin_3;
	m->hxy = cos_3 * sin_1 * sin_2 - cos_1 * sin_3;
	m->hxz = cos_2 * sin_1;

	m->hyx = cos_2 * sin_3;
	m->sy  = cos_2 * cos_3;
	m->hyz = - sin_2;

	m->hzx = cos_1 * sin_2 * sin_3 - cos_3 * sin_1;
	m->hzy = sin_1 * sin_3 + cos_1 * cos_3 * sin_2;
	m->sz  = cos_1 * cos_2;
}

void m4f_transp(const mat4f_t *m, mat4f_t *r)
{
	r->sx = m->sx;
	r->sy = m->sy;
	r->sz = m->sz;

	r->hxy = m->hyx;
	r->hxz = m->hzx;

	r->hyx = m->hxy;
	r->hyz = m->hzy;
	
	r->hzx = m->hxz;
	r->hzy = m->hyz;

	r->tx = m->t;
	r->ty = m->u;
	r->tz = m->v;

	r->t = m->tx;
	r->u = m->ty;
	r->v = m->tz;
	r->w = m->w;
}

void m4f_op_mulc(const mat4f_t *m, const float c, mat4f_t *r)
{
	r->sx = 1.0f;
	r->hxy = m->hxy * c;
	r->hxz = m->hxz * c;
	r->tx = m->tx * c;
	r->hyx = m->hyx * c;
	r->sy = 1.0f;
	r->hyz = m->hyz * c;
	r->ty = m->ty * c;
	r->hzx = m->hzx * c;
	r->hzy = m->hzy * c;
	r->sz = 1.0f;
	r->tz = m->tz * c;
	r->t = m->t * c;
	r->u = m->u * c;
	r->v = m->v * c;
	r->w = 1.0;
}

void sphec_from_ort(sphec_t *s, const vec4f_t *o)
{
	s->r = sqrt(o->x * o->x + o->y * o->y + o->z * o->z);
	s->theta = atan2(sqrt(o->x * o->x + o->y * o->y), o->z);
	s->phi = atan2(o->y, o->x);
}

void sphec_to_ort(const sphec_t *s, vec4f_t *o)
{
	o->x = s->r * sin(s->theta) * cos(s->phi);
	o->y = s->r * sin(s->theta) * sin(s->phi);
	o->z = s->r * cos(s->theta);
}

void sphec_delta(const sphec_t *a, const sphec_t *b, sphec_t *r)
{
	r->r = a->r - b->r;
	r->theta = a->theta - b->theta;
	r->phi = a->phi - b->phi;
}

void _m4f_dump(mat4f_t *m)
{
	printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
			m->sx,  m->hxy, m->hxz, m->tx, 
	                m->hyx, m->sy,  m->hyz, m->ty,
                        m->hzx, m->hzy, m->sz,  m->tz,
                        m->t,   m->u,   m->v,   m->w);
}

void quat_from_sphec(vec4f_t *q, const float latitude,
		const float longitude, const float angle)
{
	float sin_a = sin( angle / 2.0f );
	float cos_a = cos( angle / 2.0f );

	float sin_lat = sin( latitude );
	float cos_lat = cos( latitude );

	float sin_long = sin( longitude );
	float cos_long = cos( longitude );

	q->x = sin_a * cos_lat * sin_long;
	q->y = sin_a * sin_lat;
	q->z = sin_a * sin_lat * cos_long;
	q->w = cos_a;
}

float quat_norm(const vec4f_t *q)
{
	return q->x * q->x + q->y * q->y + q->z * q->z +
		q->w * q->w;
}

void quat_normalize(vec4f_t *q)
{
	float norm;

	norm = quat_norm(q);
	q->x = q->x / norm;
	q->y = q->y / norm;
	q->z = q->z / norm;
	q->w = q->w / norm;
}

void quat_inv(const vec4f_t *q, vec4f_t *r)
{
	r->x = q->x;
	r->y = q->y;
	r->z = q->z;
	r->w = - q->w;
}

void quat_from_axang(vec4f_t *q, const float ax, const float ay,
		const float az, const float angle)
{
	float half_alpha, sin_ha;
	vec3f_t axis;

	axis.x = ax;
	axis.y = ay;
	axis.z = az;
	v3f_normalize(&axis);

	half_alpha = angle * 0.5f;
	sin_ha = sin(half_alpha);

	q->x = axis.x * sin_ha;
	q->y = axis.y * sin_ha;
	q->z = axis.z * sin_ha;
	q->w = cos(half_alpha);
}

void quat_to_m4f(const vec4f_t *q, mat4f_t *m)
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = q->x + q->x;
	y2 = q->y + q->y;
	z2 = q->z + q->z;
	xx = q->x * x2;  xy = q->x * y2;  xz = q->x * z2;
	yy = q->y * y2;  yz = q->y * z2;  zz = q->z * z2;
	wx = q->w * x2;  wy = q->w * y2;  wz = q->w * z2;

	m->sx = 1.0f - (yy + zz); m->hxy = xy - wz; m->hxz = xz + wy;
	m->hyx = xy + wz; m->sy = 1.0f - (xx + zz); m->hyz = yz - wx;
	m->hzx = xz - wy; m->hzy = yz + wx; m->sz = 1.0f - (xx + yy);

	m->tx = m->ty = m->tz = 0.0f;
	m->t = m->u = m->v = 0.0f;
	m->w = 1.0f;
}

void quat_op_mul(const vec4f_t *a, const vec4f_t *b, vec4f_t *r)
{
	float A, B, C, D, E, F, G, H;

	A = (a->w + a->x) * (b->w + b->x);
	B = (a->z - a->y) * (b->y - b->z);
	C = (a->x - a->w) * (b->y + b->z);
	D = (a->y + a->z) * (b->x - b->w);
	E = (a->x + a->z) * (b->x + b->y);
	F = (a->x - a->z) * (b->x - b->y);
	G = (a->w + a->y) * (b->w - b->z);
	H = (a->w - a->y) * (b->w + b->z);

	r->w = B + (-E - F + G + H) * 0.5f;
	r->x = A - ( E + F + G + H) * 0.5f; 
	r->y =-C + ( E - F + G - H) * 0.5f;
	r->z =-D + ( E - F - G + H) * 0.5f;
}


