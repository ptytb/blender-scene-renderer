#include "numparse.h"

#include <math.h>
#include <ctype.h>

#ifndef MODEL_SCAN_DEFAULT

float _atof(char **pp)
{
	char *p = *pp;
	
	SCAN_INT integer	= 0;
	SCAN_INT frac   	= 0;
	SCAN_INT expo		= 0;
	int int_sign		= 1;
	int exp_sign;
	int digits 		= FLOAT_DEC_DIGITS;

	SCAN_FLOAT result;

	while (isspace(*p))
		++p;

	if (*p == '-') {
		++p;
		int_sign = -int_sign;
	} 

	while (isdigit(*p)) {
		integer = integer * 10 + ((digits > 0) ? *p - '0' : 0);
		--digits;
		++p;
	}

	result = integer;

	integer = 1;

	if (*p == '.') {
		++p;

		while (isdigit(*p)) {
			if (digits > 0) {
				frac = frac * 10 + *p - '0';
				--digits;
			}
			integer *= 10;
			++p;
		}
	}

	result += (float) frac / integer;

	/* For 'e' or 'E', bits 69 (dec) = 1000101 (bin) are set
	 * this is safe test, since 'em are not set
	 * in ' ', '0'--'9', '\n' */
	if ((*p & 69) == 69) {	
		++p;
		exp_sign = (*p == '-');

		if (!isdigit(*p))
			++p;
		
		while (isdigit(*p)) {
			int d = *p - '0';

			if (expo || d != 0) {
				expo = expo * 10 + d;
			}

			++p;
		}

		if (exp_sign)
			expo = -expo;
	}

	*pp = p;

	return result * pow(10.0, expo) * int_sign;
}

int _atoi(char **pp)
{
	char *p;
	int value;

	p = *pp;
	value = 0;

	while (isspace(*p))
		++p;

	for (value = 0; isdigit(*p); ++p)
		value = value * 10 + *p - '0';

	*pp = p;
	return value;
}

#endif

