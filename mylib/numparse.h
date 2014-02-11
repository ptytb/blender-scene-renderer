#pragma once

/* Comment this for more parse speed and more rough models */
#define MODEL_SCAN_DOUBLE 1

/* Use default C atoi, atof functoins */
//#define MODEL_SCAN_DEFAULT 1

#ifdef MODEL_SCAN_DEFAULT

#define _atof(pp) atof(*(pp)); while (isspace(**pp)) ++*pp;\
	while (!isspace(**pp)) ++*pp;\
	while (isspace(**pp)) ++*pp;

#define _atoi(pp) atoi(*(pp)); while (isspace(**pp)) ++*pp;\
	while (!isspace(**pp)) ++*pp;\
	while (isspace(**pp)) ++*pp

#else /* MODEL_SCAN_DEFAULT */

#ifdef MODEL_SCAN_DOUBLE

#define FLOAT_DEC_DIGITS 16
#define SCAN_FLOAT double
#define SCAN_INT long int

#else

#define FLOAT_DEC_DIGITS 8
#define SCAN_FLOAT float
#define SCAN_INT int

#endif

float _atof(char **pp);
int _atoi(char **pp);

#endif /* MODEL_SCAN_DEFAULT */

