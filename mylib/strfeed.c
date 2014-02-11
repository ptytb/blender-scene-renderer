#include "strfeed.h"

void str_cpyfeed(char *d, const char *s, int n)
{
	while (n-- && (*d++ = *s++) != '\n')
		;
	*--d = '\0';
}

int str_cmpfeed(const char *d, const char *s)
{
	while (*d != '\n' && *d == *s) {
		++d;
		++s;
	}

	return (*d == '\n' || *d == '\0') &&
			(*s == '\n' || *s == '\0');
}


