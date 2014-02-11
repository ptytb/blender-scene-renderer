#include "filemap.h"

#ifdef __unix__
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

int file_open(const char *ifname, file_info_t *fi)
{
#ifdef __unix__
	fi->fd_in = open(ifname, O_RDONLY);
	if (fi->fd_in < 0)
		return FM_ERROR_OPEN;
	
	fstat(fi->fd_in, &fi->s_in);
	fi->l_in = fi->s_in.st_size;

	fi->fp_in = mmap(NULL, fi->l_in, PROT_READ, MAP_PRIVATE,
			fi->fd_in, 0);
	if (fi->fp_in == MAP_FAILED)
		return FM_ERROR_MAP;
#else
	fi->fh_in = CreateFile(ifname, GENERIC_READ, 0, 0, OPEN_EXISTING,
			0, 0);
	if (fi->fh_in == INVALID_HANDLE_VALUE)
		return FM_ERROR_OPEN;

	fi->l_in = GetFileSize(fi->fh_in, NULL);

	fi->fmap_in = CreateFileMapping(fi->fh_in, 0, PAGE_READONLY,
			0, 0, ifname);
	if (fi->fmap_in == NULL)
		return FM_ERROR_MAP;

	fi->fp_in = MapViewOfFile(fi->fmap_in, FILE_MAP_READ, 0, 0, 0);
	if (fi->fp_in == NULL)
		return FM_ERROR_MAPVIEW;
#endif
	return FM_ERROR_OK;
}

void file_close(file_info_t *fi)
{
#ifdef __unix__
	munmap(fi->fp_in, fi->l_in);
	close(fi->fd_in);
#else
	UnmapViewOfFile(fi->fp_in);
	CloseHandle(fi->fmap_in);
	CloseHandle(fi->fh_in);
#endif
}
