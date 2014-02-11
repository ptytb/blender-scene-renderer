#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include "engine.hpp"
#include "inst.hpp"

#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <tr1/functional>

extern "C"
{
#include "render.h"
#include "../mylib/matrix.h"
#include "../mylib/objldr.h"
#include "../mylib/mtlldr.h"
#include "../mylib/texldr.h"
#include "../mylib/objvbo.h"
#include "../mylib/bvhldr.h"
}

/* ------------------------------------------------------------------
 * Platform requisites
 */

/* Windows */
#if (defined WIN32) || (defined _WIN32)

extern "C"
{
#include "../mylib/win32.h"
}

void stub(void) { }

#endif

/* ------------------------------------------------------------------
 * Config and Definitions
 */

#define FPS_MAX 20
#define TICKS_MAX 20
#define INFO_TICK_MS 300

/* ------------------------------------------------------------------
 * Schedulers
 */

class scheduler_t
{
	public:
		struct timespec now;
		struct timespec prev;
		int wait_delta;

	public:
		int ticks_max;
		int ticks_fact;
		std::tr1::function<void (void)> func;

		void schedule();
};

static long int ts_delta(struct timespec *a, struct timespec *b)
{
	return ( (a->tv_sec > b->tv_sec)
			?
			1000000000L *
			(a->tv_sec - b->tv_sec - 1L) +
			1000000000L +
			a->tv_nsec - b->tv_nsec
			:
			a->tv_nsec - b->tv_nsec );
}

static void glut_callback_wrapper(int arg)
{
	scheduler_t *s = reinterpret_cast<scheduler_t *> (arg);
	s->func();
}

void scheduler_t::schedule()
{
	int sum_delta; /* Planned time slice plus cycles slice */

	clock_gettime(CLOCK_MONOTONIC, &this->now);

	sum_delta = ts_delta(&this->now, &this->prev) / 1000000L;
	
	this->wait_delta = 1000 / this->ticks_max - ( sum_delta - this->wait_delta );

	if (this->wait_delta < 0)
		this->wait_delta = 0;

	if (sum_delta > 0)
		this->ticks_fact = 1000 / sum_delta;

	this->prev = this->now;

	glutTimerFunc(this->wait_delta, glut_callback_wrapper,
			reinterpret_cast<int> (this));
}

scheduler_t sched_frame;
scheduler_t sched_anim;

/*
 * Anim
 */

//int tick_stop = 1;

/*
 * Main window
 */

char main_win_title[100];
float main_win_ratio;
int main_win_w = 800;
int main_win_h = 600;
int main_win_fs = 0;

/*
 * Camera
 */

struct eye_struct
{
	public:
		vec4f_t eye, at, up;
};

eye_struct eye = (eye_struct) {
	eye.eye = (vec4f_t) {
		eye.eye.x = -150.0f,
		eye.eye.y = 30.0f,
		eye.eye.z = 30.0f,
		eye.eye.w = 1.0f
	}, 
	eye.at = (vec4f_t) {
		eye.at.x = 0.0f,
		eye.at.y = 0.0f,
		eye.at.z = 15.0f,
		eye.at.w = 1.0f
	},
	eye.up = (vec4f_t) {
		eye.up.x = 0.0f,
		eye.up.y = 0.0f,
		eye.up.z = 1.0f,
		eye.up.w = 1.0f
	}
};

#define EYE_NONE 0
#define	EYE_XY 1
#define	EYE_ZOOM 2
#define	EYE_Z 4
#define	EYE_LIGHT_XY 8
#define	EYE_LIGHT_OZ 16
#define	EYE_FREELOOK 32

int eye_cntl = EYE_NONE;

/*
 * Input
 */

struct mouse_coords_struct
{
	int x, y;
};

mouse_coords_struct mcs_old;
mouse_coords_struct mcs;
mouse_coords_struct mcs_delta;

/*
 * Axis
 */

int is_draw_axis = 0;

/*
 * Light
 */

struct linf {
	public:
		GLfloat amb[4], dif[4], spec[4], pos[4];
};

linf light_l = {
	{ 0.1, 0.1f, 0.1f,  1.0f },
	{ 0.8f, 0.8f, 0.8f, 1.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	{ 0.0f, 0.0f, 150.0f, 1.0f }
};

/* ------------------------------------------------------------------
 * Control
 */

void manip(float *mdx, float *mdy)
{
	vec4f_t base_at;
	float dx, dy, dx_strafe, dy_strafe, hyp;

	v4f_subv(&eye.at, &eye.eye, &base_at);
	hyp = hypot(base_at.x, base_at.y);

	dx = base_at.x / hyp * mcs_delta.y * -0.5f;
	dy = base_at.y / hyp * mcs_delta.y * -0.5f;
	dx_strafe = base_at.y / hyp * mcs_delta.x * 0.5f;
	dy_strafe = base_at.x / hyp * mcs_delta.x * -0.5f;

	*mdx += dx + dx_strafe;
	*mdy += dy + dy_strafe;
}

void eye_calc(void)
{
	if (!eye_cntl)
		return;
	
	mcs_delta.x = mcs.x - mcs_old.x;
	mcs_delta.y = mcs.y - mcs_old.y;

	if (eye_cntl & EYE_XY)
	{
		manip(&eye.eye.x, &eye.eye.y);
		manip(&eye.at.x, &eye.at.y);
	}

	if (eye_cntl & EYE_Z)
	{
		eye.eye.z -= mcs_delta.y;
		eye.at.z -= mcs_delta.y;
	}

	if (eye_cntl & EYE_LIGHT_XY)
	{
		manip(&light_l.pos[0], &light_l.pos[1]);
	}
	
	if (eye_cntl & EYE_LIGHT_OZ)
	{
		light_l.pos[2] -= mcs_delta.y;
	}

	if (eye_cntl & EYE_FREELOOK)
	{
		vec4f_t base_at, p_at, q_at;
		mat4f_t eye_mat;
		float rxy, rz, hyp;

		rz  = mcs_delta.x / -200.0f;
		rxy = mcs_delta.y / 200.0f;
		v4f_subv(&eye.at, &eye.eye, &base_at);
		hyp = hypot(base_at.x, base_at.y);

		m4f_set_id(&eye_mat);
		m4f_rotz(&eye_mat, rz);
		m4f_op_mulv(&eye_mat, &base_at, &p_at);

		m4f_set_id(&eye_mat);
		m4f_rotx(&eye_mat, -1.0f * rxy * base_at.y / hyp);
		m4f_op_mulv(&eye_mat, &p_at, &q_at);

		m4f_set_id(&eye_mat);
		m4f_roty(&eye_mat, rxy * base_at.x / hyp);
		m4f_op_mulv(&eye_mat, &q_at, &p_at);

		v4f_addv(&p_at, &eye.eye, &eye.at);
	}
}

void info_tick(int v)
{
	sprintf(main_win_title, "%d fps, %d aps",
			sched_frame.ticks_fact,
			sched_anim.ticks_fact);
	glutSetWindowTitle(main_win_title);
	glutTimerFunc(INFO_TICK_MS, info_tick, 0);
}

void keyspecial( int key, int x, int y )
{
	switch (key)
	{
		case GLUT_KEY_PAGE_UP:
			sched_frame.ticks_max += 5;
			break;
		case GLUT_KEY_PAGE_DOWN:
			if (sched_frame.ticks_max > 5)
				sched_frame.ticks_max -= 5;
			break;
		case GLUT_KEY_HOME:
			sched_anim.ticks_max += 5;
			break;
		case GLUT_KEY_END:
			if (sched_anim.ticks_max > 5)
				sched_anim.ticks_max -= 5;
			break;
		case GLUT_KEY_INSERT:
			/*
			tick_stop = !tick_stop;
			if (!tick_stop) {
				clock_gettime(CLOCK_MONOTONIC, &sched_anim.prev);
				glutTimerFunc(0, tick, 0);
			}
			*/
			break;
	}
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'q':
		case 'Q':
		case 27:
			glutLeaveMainLoop();
			break;
		case 'f':
		case 'F':
			main_win_fs = !main_win_fs;
			if (main_win_fs)
				glutFullScreen();
			else
				glutReshapeWindow(main_win_w, main_win_h);
			break;
		case 'a':
		case 'A':
			is_draw_axis = !is_draw_axis;
			break;
	/*
		case 'r':
		case 'R':
			bvh_player_rewind(&bvh_player);
			bvh_player_next(&bvh_player);
			vbo_animate(&modelv[0].vbo_mdl, &modelv[0].d_mdl, &bvh_player);
			break;
		case 'n':
		case 'N':
			bvh_player_next(&bvh_player);
			vbo_animate(&modelv[0].vbo_mdl, &modelv[0].d_mdl, &bvh_player);
			break;
		case 's':
		case 'S':
			bvh_player_rest(&bvh_player);
			bvh_player_next(&bvh_player);
			vbo_animate(&modelv[0].vbo_mdl, &modelv[0].d_mdl, &bvh_player);
			break;
	*/
	}
}

void mouse(int button, int state, int x, int y)
{
	int mod = glutGetModifiers();

	if (state == GLUT_DOWN)
	{
		mcs.x = x;
		mcs.y = y;
	}

	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				eye_cntl |= (mod & GLUT_ACTIVE_SHIFT)
					? EYE_FREELOOK
					: EYE_XY;
			} else if (state == GLUT_UP) {
				eye_cntl &= ~(EYE_XY | EYE_FREELOOK);
			}
			break;

		case GLUT_MIDDLE_BUTTON:
			if (state == GLUT_DOWN) {
				eye_cntl |= (mod & GLUT_ACTIVE_SHIFT)
					? EYE_LIGHT_OZ
					: EYE_LIGHT_XY;
			} else if (state == GLUT_UP) {
				eye_cntl &= ~(EYE_LIGHT_OZ | EYE_LIGHT_XY);
			}
			break;

		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_DOWN) {
				eye_cntl |= EYE_Z;
			} else if (state == GLUT_UP) {
				eye_cntl &= ~EYE_Z;
			}
			break;
	}
}

void mouse_move(int x, int y)
{
	mcs_old = mcs;
	mcs.x = x;
	mcs.y = y;

	eye_calc();
}

void change_size(int w, int h)
{
	main_win_ratio = (float) w / h;
	main_win_w = w;
	main_win_h = h;
	/* Prevent a divide by zero, when window is too short*/
	/* (you cant make a window of zero width).*/
	if (h == 0)
		h = 1;

	/* Use the Projection Matrix*/
	glMatrixMode(GL_PROJECTION);

	/* Reset Matrix*/
	glLoadIdentity();

	/* Set the viewport to be the entire window*/
	glViewport(0, 0, w, h);

	/* Set the correct perspective.*/
	gluPerspective(45.0f, main_win_ratio, 0.1f, 1000.0f);

	/* Get Back to the Modelview*/
	glMatrixMode(GL_MODELVIEW);
}

/* ------------------------------------------------------------------
 * Draw part here
 */

static void draw_models(svec_t *instv)
{
	svec_iter_t iter;
	inst_t **inst;

	svec_iter_init(&iter, instv);

	while ( (inst = (inst_t **) svec_iter_next(&iter)) )
	{
		if ((*inst)->entity->get_flags() & EF_DRAW)
			vbo_draw_model((vbo_model_t *) svec_get(&(*inst)->vbo_modelv, 0),
					( ((*inst)->entity->get_flags() & EF_STATIC)
					? NULL
					: (model_d_t *) svec_get(&(*inst)->model_dv, 0) ) );
	}
}

static void draw_lights(svec_t *lightv)
{
	svec_iter_t iter;
	light_t **light;

	svec_iter_init(&iter, lightv);

	while ( (light = (light_t **) svec_iter_next(&iter)) )
	{
		draw_light_position((GLfloat *) &light_l.pos);

		glLightfv(GL_LIGHT0, GL_POSITION, light_l.pos);
	}
}

/* ------------------------------------------------------------------
 * Main control
 */

static void engine_init_ext(void)
{
	glewExperimental = GL_TRUE;
	GLenum glew_err = glewInit();

	if (glew_err != GLEW_OK)
	{
		std::printf("Error: %s\n", glewGetErrorString(glew_err));
		exit(0);
	}

	if (!GLEW_ARB_vertex_buffer_object)
	{
		std::printf("VBO is not supported.\n");
		exit(0);
	}

	std::printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}

static void engine_init_context(int argc, char *argv[])
{
	/* init GLUT and create window*/
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(main_win_w, main_win_h);
	glutCreateWindow("");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	engine_init_ext();

	glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
//	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_LINE_SMOOTH);
//	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glEnable(GL_MAP2_TEXTURE_COORD_2);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LESS);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
//	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
//	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_LIGHT0);
//	glEnable(GL_NORMALIZE);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_l.amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_l.dif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_l.spec);
	glLightfv(GL_LIGHT0, GL_POSITION, light_l.pos);
	glEnable(GL_LIGHTING);
}

static void engine_register_callbacks(void)
{
	/* register callbacks*/
#ifdef _WIN32_
	glutDisplayFunc(stub);
#endif
	glutReshapeFunc(change_size);

	/* Input event handlers */
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyspecial);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_move);
}

void engine_t::init_schedulers(void)
{
	sched_frame.func = std::tr1::bind(&engine_t::render, this);
	sched_frame.ticks_max = FPS_MAX;

	sched_anim.func = std::tr1::bind(&engine_t::tick, this);
	sched_anim.ticks_max = TICKS_MAX;

	clock_gettime(CLOCK_MONOTONIC, &sched_frame.prev);
	sched_anim.prev = sched_frame.prev;
}

void engine_t::init(int argc, char *argv[])
{
	engine_init_context(argc, argv);
	engine_register_callbacks();

	mtl_glob_tex_path("img/");
	tex_glob_id_low(100);
}

void engine_t::run()
{
	init_schedulers();

//	if (!tick_stop)
	tick();

	render();
	::info_tick(0);

	/* enter GLUT event processing cycle*/
	glutMainLoop();
}

void engine_t::render()
{
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(eye.eye.x, eye.eye.y, eye.eye.z,
			eye.at.x, eye.at.y, eye.at.z,
			eye.up.x, eye.up.y, eye.up.z);

	if (is_draw_axis)
		draw_axis();

	draw_models(&scene->instv);
	draw_lights(&scene->lightv);

	glutSwapBuffers();
	sched_frame.schedule();
}

void engine_t::tick()
{
	svec_iter_t iter;
	inst_t **inst;

	svec_iter_init(&iter, &this->scene->instv);

	while ( (inst = (inst_t **) svec_iter_next(&iter)) )
	{
		if ( (*inst)->entity->get_flags() & EF_STATIC )
			continue;

		int model_id;
		vbo_model_t *vbo_model;
		model_d_t *model_d;
		bvh_player_t *bp;

		(*inst)->controller->think();

		model_id = (*inst)->controller->get_model_id();
		vbo_model = (vbo_model_t *) svec_get(&(*inst)->vbo_modelv, model_id);
		model_d = (model_d_t *) svec_get(&(*inst)->model_dv, model_id);
		bp = &(*inst)->anim_player;

		if ( (*inst)->controller->is_anim_changed() )
		{
			bvh_player_rest(bp);
			bvh_player_next(bp);

			vbo_animate(vbo_model, model_d, bp,
					(*inst)->get_matrix(),
					(*inst)->get_inv_matrix());

			bvh_player_init(bp,
					(bvh_t *) svec_get(&(*inst)->entity->animv,
						(*inst)->controller->get_anim_id()));
		}

		bvh_player_next(bp);

		vbo_animate(vbo_model, model_d, bp,
				(*inst)->get_matrix(),
				(*inst)->get_inv_matrix());

	}

//	if (!tick_stop)
	sched_anim.schedule();
}

