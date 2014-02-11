#include "win32.h"

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	DWORD msec;
	msec = GetTickCount();
	tp->tv_sec = msec / 1000;
	tp->tv_nsec = (msec % 1000) * 1000000L;
	return 0;
}

