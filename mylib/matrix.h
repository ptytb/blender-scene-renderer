#pragma once

typedef struct vec3f_struct {
	float x;
	float y;
	float z;
} vec3f_t;

typedef struct vec4f_struct {
	float x;
	float y;
	float z;
	float w;
} vec4f_t;

/* 3D Transform Matrix
 *
 */
typedef struct mat4f_t_struct {
	float sx,  hxy, hxz, tx;
	float hyx, sy,  hyz, ty;
	float hzx, hzy, sz,  tz;
	float t,   u,   v,   w;
} mat4f_t;

/* Spheric coords type
 */
typedef struct sphec_struct {
	float r;
	float theta;
	float phi;
} sphec_t;

void v3f_subv(const vec3f_t *a, const vec3f_t *b, vec3f_t *r);
void v3f_addv(const vec3f_t *a, const vec3f_t *b, vec3f_t *r);
float v3f_norm(const vec3f_t *v);
void v3f_normalize(vec3f_t *v);

void v4f_subv(const vec4f_t *a, const vec4f_t *b, vec4f_t *r);
void v4f_addv(const vec4f_t *a, const vec4f_t *b, vec4f_t *r);
float v4f_mulv_scal(const vec4f_t *a, const vec4f_t *b);
void v4f_norm(vec4f_t *a);
void v4f_dir_cos(const vec4f_t *a, vec4f_t *r);

void m4f_set_zero(mat4f_t *m);
void m4f_set_id(mat4f_t *m);

void m4f_op_mulv(const mat4f_t *m, const vec4f_t *v, vec4f_t *r);
void m4f_op_mul(const mat4f_t *a, const mat4f_t *b, mat4f_t *r);
void m4f_op_mulc(const mat4f_t *m, const float c, mat4f_t *r);

void m4f_rotx(mat4f_t *m, float theta);
void m4f_roty(mat4f_t *m, float theta);
void m4f_rotz(mat4f_t *m, float theta);
void m4f_rot_xyz(mat4f_t *m, float ax, float ay, float az);
void m4f_rot_yxz(mat4f_t *m, float ax, float ay, float az);
void m4f_transp(const mat4f_t *m, mat4f_t *r);
void _m4f_dump(mat4f_t *m);

void sphec_from_ort(sphec_t *s, const vec4f_t *o);
void sphec_to_ort(const sphec_t *s, vec4f_t *o);
void sphec_delta(const sphec_t *a, const sphec_t *b, sphec_t *r);

void quat_from_sphec(vec4f_t *q, const float latitude,
		const float longitude, const float angle);
void quat_from_axang(vec4f_t *q, const float ax, const float ay,
		const float az, const float angle);
void quat_to_m4f(const vec4f_t *q, mat4f_t *m);
void quat_op_mul(const vec4f_t *a, const vec4f_t *b, vec4f_t *r);
float quat_norm(const vec4f_t *q);
void quat_normalize(vec4f_t *q);
void quat_inv(const vec4f_t *q, vec4f_t *r);

