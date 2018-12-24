/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2015 Diego Gallizioli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MATH_H_
#define _MATH_H_

#define INFINITY	(__infinity())
#define NAN		(__qnan())
#define HUGE_VAL	INFINITY

double __infinity (void);
double __qnan (void);

double acos (double _x);
double asin (double _x);
double atan (double _x);
double atan2 (double _y, double _x);
double ceil (double _x);
double cos (double _x);
double cosh (double _x);
double exp (double _x);
double fabs (double _x);
float fabsf (float _x);
double floor (double _x);
double fmod (double _x, double _y);
double frexp (double _x, int *_exp);
double ldexp (double _x, int _exp);
double log (double _x);
double log10 (double _x);
double modf (double _x, double *_iptr);
double pow (double _x, double _y);
double rint (double _x);
double scalbn (double _x, int _exp);
float scalbnf (float _x, int _exp);
double scalbln (double _x, long _exp);
float scalblnf (float _x, long _exp);
double sin (double _x);
double sinh (double _x);
double sqrt (double _x);
double tan (double _x);
double tanh (double _x);
double hypot (double _x, double _y);

#include <mathconst.h>

#define FP_INFINITE  1
#define FP_NAN       2
#define FP_NORMAL    3
#define FP_SUBNORMAL 4
#define FP_ZERO      5

int fpclassify (double x);
int isfinite (double x);
int isinf (double x);
int isnan (double x);
int isnormal (double x);
int signbit (double x);
int isgreater (double x, double y);
int isgreaterequal (double x, double y);
int isless (double x, double y);
int islessequal (double x, double y);
int islessgreater (double x, double y);
int isunordered (double x, double y);
double nearbyint (double x);
double remainder (double x, double y);
double trunc (double x);

float powf (float, float);
float fmodf (float, float);
double erf (double);
double erfc (double);
double gamma (double);
double j0 (double);
double j1 (double);
double jn (int, double);
double lgamma (double);
double y0 (double);
double y1 (double);
double yn (int, double);
double acosh (double);
double asinh (double);
double atanh (double);
double cbrt (double);
double expm1 (double);
int    ilogb (double);
double log1p (double);
double logb (double);
double nextafter (double, double);
double scalb (double, double);

#endif 
