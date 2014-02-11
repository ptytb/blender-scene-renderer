#pragma once

#ifdef __unix__
#include <sys/stat.h>
#else
#include <windows.h>
#endif

#define FM_ERROR_OK		0
#define FM_ERROR_OPEN		-1
#define FM_ERROR_SIZE		-2
#define FM_ERROR_MAPVIEW	-3
#define FM_ERROR_MAP		-4

typedef struct {
#ifdef __unix__
	int fd_in;
	struct stat s_in;
#else
	HANDLE fh_in;
	HANDLE fmap_in;
#endif
	int l_in;	/* Input file length */
	char *fp_in;	/* Input file pointer */
} file_info_t;

int file_open(const char *ifname, file_info_t *fi);
void file_close(file_info_t *fi);
