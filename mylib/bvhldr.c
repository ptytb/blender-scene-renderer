#include "bvhldr.h"
#include "filemap.h"
#include "slabvec.h"
#include "strfeed.h"
#include "numparse.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void bvh_init(bvh_t *h)
{
	svec_init(&h->jointv, 20, sizeof(bvh_joint_t));
	h->motion.frames = 0;
	h->motion.channels = 0;
}

void bvh_free(bvh_t *h)
{
	svec_free(&h->jointv);
	if (h->motion.frames)
	{
		svec_free(&h->motion.framev);
		svec_free(&h->motion.inv_framev);
		svec_free(&h->motion.rest_framev);
	}
}

static float deg2rad(const float d)
{
	return d / 360.0f * (2 * M_PI);
}

static void bvh_calc_frame(bvh_t *h, const float *frame,
		mat4f_t *inv_matrixv, mat4f_t *inv_matrixv_next)
{
	bvh_iter_t h_iter;
	svec_t parent_stack;
	bvh_joint_t *joint;
	int xti, yti, zti, xri, yri, zri;
	mat4f_t tmp_mat, *matrix, *parent_matrix, *parent_inv_matrix,
		inv_matrix, trans, rot, inv_rot;

	bvh_iter_init(&h_iter, h);
	svec_init(&parent_stack, 10, 2 * sizeof(mat4f_t));
	matrix = svec_put(&h->motion.framev);
	parent_matrix = NULL;
	parent_inv_matrix = NULL;

	while ( (joint = bvh_iter_next(&h_iter)) )
	{
		/* Order is T-R-H-S
		 * Inverse order is S-H-R-T
		 *
		 * Result matrix M equals:
		 * M = Mp x Mj x (Mj-1)^-1 x (Mp-1)^1,
		 * Mp = Mchild x Mparent x Mgrandparent x ...
		 * where p is for parent*/

		/* Compute matrix Mj for current frame and current bone */

		xti = joint->channel_mapv[0];
		yti = joint->channel_mapv[1];
		zti = joint->channel_mapv[2];
		xri = joint->channel_mapv[3];
		yri = joint->channel_mapv[4];
		zri = joint->channel_mapv[5];

		m4f_set_id(&trans);
		trans.tx = ( (xti >= 0) ? frame[xti] : joint->offset.x );
		trans.ty = ( (yti >= 0) ? frame[yti] : joint->offset.y );
		trans.tz = ( (zti >= 0) ? frame[zti] : joint->offset.z );
/*
		trans.tx = ( (xti >= 0) ? frame[xti] : 0.0f );
		trans.ty = ( (yti >= 0) ? frame[yti] : 0.0f );
		trans.tz = ( (zti >= 0) ? frame[zti] : 0.0f );

		trans.tx += joint->offset.x;
		trans.ty += joint->offset.y;
		trans.tz += joint->offset.z;
*/
		/* Euler's angles in order YXZ */

		m4f_set_id(&rot);
		m4f_rot_yxz(&rot, deg2rad( (xri >= 0) ? frame[xri] : 0.0f ),
				deg2rad( (yri >= 0) ? frame[yri] : 0.0f ),
				deg2rad( (zri >= 0) ? frame[zri] : 0.0f ));

		/* Compute inverse matrix for current bone's matrix */

		m4f_transp(&rot, &inv_rot);

		*matrix = rot;
		matrix->tx = trans.tx;
		matrix->ty = trans.ty;
		matrix->tz = trans.tz;
	
	//	m4f_op_mul(&trans, &rot, matrix);

		trans.tx = -trans.tx;
		trans.ty = -trans.ty;
		trans.tz = -trans.tz;

		m4f_op_mul(&inv_rot, &trans, &inv_matrix);

		/* Multiply Mj^-1 with inv parent matrix Mp^-1 */

		if (parent_inv_matrix)
		{
			m4f_op_mul(&inv_matrix, parent_inv_matrix, inv_matrixv_next);
		} else
		{
			*inv_matrixv_next = inv_matrix;
		}

		/* Multiply parent matrix Mp with Mj */

		if (parent_matrix) {
			m4f_op_mul(parent_matrix, matrix, &tmp_mat);
			*matrix = tmp_mat;
		}

		if (h_iter.course > 0)
		{	/* Will move to child next iteration */
			parent_matrix = svec_put(&parent_stack);
			parent_inv_matrix = parent_matrix + 1;
			*parent_matrix = *matrix;
			*parent_inv_matrix = *inv_matrixv_next;
		} else if (h_iter.course < 0)
		{ 	/* Will move to parents */
			while (parent_stack.elems && h_iter.course < 0) {
				svec_pop(&parent_stack);
				++h_iter.course;
			}
			parent_matrix = svec_peek(&parent_stack);
			parent_inv_matrix = (parent_matrix
					? parent_matrix + 1
					: NULL);
		}

		/* Compose current matrix with inverse matrix
		 * of same bone but previous frame */

		m4f_op_mul(matrix, inv_matrixv, &tmp_mat);
		*matrix = tmp_mat;

	//	printf("\n\nFRAME\tJOINT %s\n", joint->name);
	//	_m4f_dump(matrix);

		frame += joint->channels;
		++matrix;
		++inv_matrixv;
		++inv_matrixv_next;
	}

	bvh_iter_free(&h_iter);
	svec_free(&parent_stack);
}

#define _BVH_SKIP_WHITESPACE(p) while (*p == '\t' || *p == ' ') ++p;
#define _BVH_SKIP_WHITESPACE2(p) while (isspace(*p)) ++p;

void bvh_load(const char *filename, bvh_t *h)
{
	file_info_t fi;
	int f_err;
	char *p, *p_end;
	enum BVH_Line line;
	enum BVH_State state;
	svec_t stack;
	bvh_joint_t *joint, **sibling;

	f_err = file_open(filename, &fi);
	
	if (f_err)
		return;
	
	svec_init(&stack, 8, sizeof(void *));

	p = fi.fp_in;
	p_end = p + fi.l_in;

	while (p < p_end)
	{
		_BVH_SKIP_WHITESPACE(p);

		if (*p == '\n') {
			++p;
			continue;
		} else if (* (unsigned int *) p == 0x52454948 ) { /* HIER */
			line = BL_HIERARCHY;
		} else if (* (unsigned int *) p == 0x544F4F52 ) { /* ROOT */
			line = BL_ROOT;
		} else if (* (unsigned int *) p == 0x5346464F ) { /* OFFS */
			line = BL_OFFSET;
		} else if (* (unsigned int *) p == 0x4E414843 ) { /* CHAN */
			line = BL_CHANNELS;
		} else if (* (unsigned int *) p == 0x4E494F4A ) { /* JOIN */
			line = BL_JOINT;
		} else if (* (unsigned int *) p == 0x20646E45 ) { /* `End ` */
			line = BL_END_SITE;
		} else if (* (unsigned int *) p == 0x49544F4D ) { /* `MOTI` */
			line = BL_MOTION;
		}else if ( *p == '{' ) {
			line = BL_BEGIN;
		} else if ( *p == '}' ) {
			line = BL_END;
		} else {
			line = BL_UNKNOWN;
		}

		switch (line)
		{
			case BL_HIERARCHY:
				p += 9;
				state = BS_HIERARCHY;
				break;
			case BL_MOTION:
				p += 6;
				state = BS_MOTION;

				_BVH_SKIP_WHITESPACE2(p);
				p += 7; /* Skip `Frames:` */
				h->motion.frames = _atoi(&p);
				_BVH_SKIP_WHITESPACE2(p);
				p += 11; /* Skip `Frame Time:` */
				h->motion.frame_time = _atof(&p);
				_BVH_SKIP_WHITESPACE2(p);
				
				if (h->motion.frames)
				{
					svec_init(&h->motion.framev, h->motion.frames,
							h->jointv.elems * sizeof(mat4f_t));
					svec_init(&h->motion.inv_framev, h->motion.frames,
							h->jointv.elems * sizeof(mat4f_t));
					svec_init(&h->motion.rest_framev, h->motion.frames,
							h->jointv.elems * sizeof(mat4f_t));
				}
				break;
			default:
				break;
		}

		if (state == BS_HIERARCHY)
		{
			/* Load Tree */
			switch (line)
			{

				case BL_JOINT:
				case BL_END_SITE:
					p += 1;
				case BL_ROOT:
					p += 5;

					joint = svec_put(&h->jointv);
					joint->next = NULL;
					joint->child = NULL;
					
					if (line != BL_END_SITE) {
						str_cpyfeed(joint->name, p,
								BVH_JOINT_NAME_MAX);
					} else {
						joint->name[0] = '\0';
					}
					
					joint->channels = 0;
					memset(joint->channel_mapv, -1,
							sizeof(joint->channel_mapv));

					if (line == BL_ROOT)
						h->root = joint;
					else
						*sibling = joint;

					sibling = &joint->next;
					break;

				case BL_BEGIN:
					sibling = &joint->next;
					svec_add(&stack, &sibling);
					//	printf("PUSH: %p\n", sibling);
					sibling = &joint->child;
					break;

				case BL_END:
					sibling = * (bvh_joint_t ***) svec_pop(&stack);
					//	printf("POP: %p\n", sibling);
					break;

				case BL_OFFSET:
					p += 7;
					joint->offset.x = _atof(&p);
					joint->offset.y = _atof(&p);
					joint->offset.z = _atof(&p);
					continue;

				case BL_CHANNELS:
					p += 8;
					joint->channels = _atoi(&p);
					h->motion.channels += joint->channels;
					{
						int chan, chan_index;
						
						for (chan = 0; chan < joint->channels; ++chan)
						{
							_BVH_SKIP_WHITESPACE(p);
							chan_index =
								*p - 'X' + ((*(p + 1) == 'r')
									? 3
									: 0);
							joint->channel_mapv[chan_index] = chan;
							p += 9; /* `Xxxxxtion` */
						}
					}
					break;

				default:
					break;
			}
		} else if (state == BS_MOTION)
		{
			/* Load animation frames */
			int i, chan;
			float *frame_datav, *data;
			mat4f_t *inv_matrixv, *inv_matrixv_next, *im, *matrixv, tm1,
				*offset_matrixv, *rest_matrixv;
			bvh_iter_t h_iter;
			bvh_joint_t *joint;
			vec3f_t legacy_offset;
			svec_iter_t inv_frame_iter;

			frame_datav = (float *) malloc(h->motion.channels * sizeof(float));
			offset_matrixv = (mat4f_t *) malloc(h->jointv.elems * sizeof(mat4f_t));

			bvh_iter_init(&h_iter, h);

			inv_matrixv = offset_matrixv;
			inv_matrixv_next = svec_put(&h->motion.inv_framev);
			im = offset_matrixv;
			legacy_offset = (vec3f_t) { .x = 0.0f, .y = 0.0f, .z = 0.0f };

			while ( (joint = bvh_iter_step(&h_iter)) )
			{
				/* Initial inverse matrix.
				 * We say that every joint like has been shifted from
				 * global origin. */

				if (h_iter.course >= 0) {
					m4f_set_id(im);
					im->tx = - legacy_offset.x - joint->offset.x;
					im->ty = - legacy_offset.y - joint->offset.y;
					im->tz = - legacy_offset.z - joint->offset.z;
				
				//	printf("====== \%s\n", joint->name);
				//	_m4f_dump(im);

					++im;
				}

				if (h_iter.course > 0)
					v3f_addv(&legacy_offset, &joint->offset,
							&legacy_offset);
				else if (h_iter.course < 0)
					v3f_subv(&legacy_offset, &joint->offset,
							&legacy_offset);
			}

			/* Read frame data and calc frame and
			 * inverse frame matrices */

			for (i = h->motion.frames; i--;)
			{
				data = frame_datav;

				for (chan = h->motion.channels; chan; --chan ) {
					*data++ = _atof(&p);
				}

				bvh_calc_frame(h, frame_datav,
						inv_matrixv, inv_matrixv_next);

				if (i) {
					inv_matrixv = inv_matrixv_next;
					inv_matrixv_next = svec_put(&h->motion.inv_framev);
				}
			}

			/* Compose each inverse frame with first frame
			 * and create rest martices*/

			svec_iter_init(&inv_frame_iter, &h->motion.inv_framev);

			while ( (inv_matrixv = svec_iter_next(&inv_frame_iter)) )
			{
				matrixv = svec_get(&h->motion.framev, 0);
				im = offset_matrixv;

				rest_matrixv = svec_put(&h->motion.rest_framev);

				for (i = h->jointv.elems; i--;)
				{
					inv_matrixv->tx -= im->tx;
					inv_matrixv->ty -= im->ty;
					inv_matrixv->tz -= im->tz;
					
					*rest_matrixv = *inv_matrixv;

					m4f_op_mul(matrixv, inv_matrixv, &tm1);
					*inv_matrixv = tm1;

					++matrixv;
					++inv_matrixv;
					++rest_matrixv;
					++im;
				}
			}

			state = BS_UNKNOWN;
			free(frame_datav);
			free(offset_matrixv);
			bvh_iter_free(&h_iter);
		}

		while (p < p_end && *p != '\n')
			++p;
	}

	file_close(&fi);
	svec_free(&stack);
}

void bvh_iter_init(bvh_iter_t *i, const bvh_t *h)
{
	i->joint = h->root;
	i->parent = NULL;
	svec_init(&i->stack, 8, sizeof(void *));
}

void bvh_iter_free(bvh_iter_t *i)
{
	svec_free(&i->stack);
}

bvh_joint_t *bvh_iter_step(bvh_iter_t *i)
{
	bvh_joint_t *joint;

	joint = i->joint;
	i->course = 0;

	if (!i->joint)
	{
		if (i->stack.elems)
		{
			i->joint = * (bvh_joint_t **) svec_pop(&i->stack);
			joint = i->joint;
			i->joint = i->joint->next;
			i->parent = svec_peek(&i->stack);
			--i->course;
		} 
	} else {
		if (i->joint->child) {
			i->parent = i->joint;
			svec_add(&i->stack, &i->joint);
			i->joint = i->joint->child;
			++i->course;
		} else {
			i->joint = i->joint->next;
		}
	}	

	return joint;
}

bvh_joint_t *bvh_iter_next(bvh_iter_t *i)
{
	bvh_joint_t *joint;

	joint = i->joint;
	i->course = 0;

	if (i->joint)
	{
		if (i->joint->child) {
			i->parent = i->joint;
			svec_add(&i->stack, &i->joint);
			i->joint = i->joint->child;
			++i->course;
		} else {
			i->joint = i->joint->next;

			while (!i->joint && i->stack.elems) {
				i->joint = * (bvh_joint_t **) svec_pop(&i->stack);
				i->joint = i->joint->next;
				i->parent = svec_peek(&i->stack);
				--i->course;
			} 
		}
	}

	return joint;
}

void bvh_player_init(bvh_player_t *bp, bvh_t *h)
{
	bp->bvh = h;
	bp->state = BPS_NONE;
	svec_iter_init(&bp->framev_iter, &bp->bvh->motion.framev);
	svec_iter_init(&bp->inv_framev_iter, &bp->bvh->motion.inv_framev);
	svec_iter_init(&bp->rest_framev_iter, &bp->bvh->motion.rest_framev);
}

void bvh_player_rewind(bvh_player_t *bp)
{
	bp->state |= BPS_REWIND;
}

void bvh_player_rest(bvh_player_t *bp)
{
	bp->state |= BPS_REST | BPS_REWIND;
}

void bvh_player_next(bvh_player_t *bp)
{
	if ( ! (bp->state & (BPS_REWIND | BPS_REST)) )
	{
		if ( (bp->matrixv = svec_iter_next(&bp->framev_iter)) )
		{
			bp->inv_matrixv = svec_iter_next(&bp->inv_framev_iter);
			bp->rest_matrixv = svec_iter_next(&bp->rest_framev_iter);
			return;
		}
	}

	svec_iter_init(&bp->framev_iter, &bp->bvh->motion.framev);
	svec_iter_init(&bp->inv_framev_iter, &bp->bvh->motion.inv_framev);
	svec_iter_init(&bp->rest_framev_iter, &bp->bvh->motion.rest_framev);
	
	if (bp->state & BPS_REST)
	{
		bp->matrixv = bp->rest_matrixv;
		bp->state &= ~(BPS_REWIND | BPS_REST);
		return;
	}

	bp->matrixv = bp->inv_matrixv;

	svec_iter_next(&bp->framev_iter);
	bp->inv_matrixv = svec_iter_next(&bp->inv_framev_iter);
	bp->rest_matrixv = svec_iter_next(&bp->rest_framev_iter);

	bp->state &= ~BPS_REWIND;
}

void bvh_dump(bvh_t *h)
{
	bvh_iter_t i;
	bvh_joint_t *j;
	int tabs;

	bvh_iter_init(&i, h);
	tabs = 0;
	
	while ( (j = bvh_iter_step(&i)) ) {
		if (i.course >= 0)
		printf("%*c%s\n%*c%f %f %f\n%*c%d %d %d %d %d %d\n",
				tabs, ' ',
				j->name,
				tabs, ' ',
				j->offset.x, j->offset.y, j->offset.z,
				tabs, ' ',
				j->channel_mapv[0], j->channel_mapv[1],
				j->channel_mapv[2], j->channel_mapv[3],
				j->channel_mapv[4], j->channel_mapv[5]);
		tabs += i.course * 4;
	}

	bvh_iter_free(&i);
}


