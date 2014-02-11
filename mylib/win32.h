#pragma once

#include <windows.h>
#include <time.h>

#define NAME_MAX 255
#define M_PI 3.14159265358979323846

struct timespec {
	time_t   tv_sec;        /* seconds */
	long     tv_nsec;       /* nanoseconds */
};

typedef int clockid_t;
int clock_gettime(clockid_t clk_id, struct timespec *tp);

#define CLOCK_MONOTONIC 0


