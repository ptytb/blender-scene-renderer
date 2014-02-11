#include <GL/gl.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "texldr.h"
#include "targa.h"
#include "strfeed.h"

#ifdef WIN32
#include "win32.h"
#endif

int _tex_glob_id_low;
int _tex_glob_id_high;

void tex_glob_id_low(int id)
{
	_tex_glob_id_low = id;
	_tex_glob_id_high = id;
}

void tex_init(texldr_t *t, const char *path)
{
	svec_init(&t->tex_pending, 10, sizeof(texinfo_t));
	t->id_low = _tex_glob_id_high;
	t->id_high = t->id_low;
	t->path = path;
}

void tex_free(texldr_t *t)
{
	svec_free(&t->tex_pending);
}

int tex_get_by_filename(texldr_t *t, const char *filename)
{
	svec_iter_t i;
	texinfo_t *text;
	int n;

	svec_iter_init(&i, &t->tex_pending);

	for (n = 0; (text = svec_iter_next(&i)); ++n ) {
		if ( str_cmpfeed(filename, text->filename ) )
			return t->id_low + n;
	}

	return -1;
}

/* Returns id */
int tex_queue(texldr_t *t, const char *filename)
{
	int id;

	id = tex_get_by_filename(t, filename);
	if (id < 0) {
		str_cpyfeed( ( (texinfo_t *) svec_put(&t->tex_pending) )->filename,
				filename, TEX_MAX_NAME);
	}

	return id >= 0 ? id : t->id_high++;
}

/* Load all pending */
void tex_load(texldr_t *t)
{
	Targa tga;
	int ta_len, i, tex_w, tex_h;
	char *ptex, *fullname, *pname;
	svec_iter_t t_iter;
	texinfo_t *t_info;

	fullname = (char *) malloc(NAME_MAX);
	strcpy(fullname, t->path);
	pname = fullname + strlen(fullname);

	svec_iter_init(&t_iter, &t->tex_pending);

	for (i = 0; (t_info = svec_iter_next(&t_iter)); ++i) {
		strcpy(pname, t_info->filename);
//		printf("%s\n", fullname);
//		fflush(stdout);
		targa_init(&tga);
		targa_loadFromFile(&tga, fullname);
		targa_getRgbaTexture(&tga, &ptex, &ta_len);
		targa_getDimensions(&tga, &tex_w, &tex_h);

		glBindTexture(GL_TEXTURE_2D, i + t->id_low);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_w, tex_h, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, ptex);

		targa_free(&tga);
	}

	_tex_glob_id_high = t->id_high;

	free(fullname);
}

