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

#include <stdlib.h>

static void qsort1(char *, char *, size_t);
static int (*qcompar)(const char *, const char *);
static void qexchange(char *, char *, size_t);
static void q3exchange(char *, char *, char *, size_t);

void qsort(void *base, size_t n, size_t size, int (*cmp)(const void *keyval, const void *datum))
{
	if(!base || !n || !size || !cmp)
		return;

	qcompar = (int (*)(const char *, const char *))cmp;
	qsort1(base, (char *)base + (n - 1) * size, size);
}

static void qsort1(char *a1, char *a2, size_t width)
{
	char *left, *right;
	char *lefteq, *righteq;
	int cmp;

	for (;;) {
		if (a2 <= a1)
			return;
		left = a1;
		right = a2;
		lefteq = righteq = a1 + width * (((a2 - a1) + width) / (2 * width));
		/*
		   Pick an element in the middle of the array.
		   We will collect the equals around it.
		   "lefteq" and "righteq" indicate the left and right
		   bounds of the equals respectively.
		   Smaller elements end up left of it, larger elements end
		   up right of it.
		 */
 again:
		while (left < lefteq && (cmp = (*qcompar) (left, lefteq)) <= 0) {
			if (cmp < 0) {
				/* leave it where it is */
				left += width;
			} else {
				/* equal, so exchange with the element to
				   the left of the "equal"-interval.
				 */
				lefteq -= width;
				qexchange(left, lefteq, width);
			}
		}
		while (right > righteq) {
			if ((cmp = (*qcompar) (right, righteq)) < 0) {
				/* smaller, should go to left part
				 */
				if (left < lefteq) {
					/* yes, we had a larger one at the
					   left, so we can just exchange
					 */
					qexchange(left, right, width);
					left += width;
					right -= width;
					goto again;
				}
				/* no more room at the left part, so we
				   move the "equal-interval" one place to the
				   right, and the smaller element to the
				   left of it.
				   This is best expressed as a three-way
				   exchange.
				 */
				righteq += width;
				q3exchange(left, righteq, right, width);
				lefteq += width;
				left = lefteq;
			} else if (cmp == 0) {
				/* equal, so exchange with the element to
				   the right of the "equal-interval"
				 */
				righteq += width;
				qexchange(right, righteq, width);
			} else	/* just leave it */
				right -= width;
		}
		if (left < lefteq) {
			/* larger element to the left, but no more room,
			   so move the "equal-interval" one place to the
			   left, and the larger element to the right
			   of it.
			 */
			lefteq -= width;
			q3exchange(right, lefteq, left, width);
			righteq -= width;
			right = righteq;
			goto again;
		}
		/* now sort the "smaller" part */
		qsort1(a1, lefteq - width, width);
		/* and now the larger, saving a subroutine call
		   because of the for(;;)
		 */
		a1 = righteq + width;
	}
 /*NOTREACHED*/}

static void qexchange(char *p, char *q, size_t n)
{
	long c;
	long *pp = (long *)p;
	long *qq = (long *)q;

	while (n > sizeof(long)) {
		c = *pp;
		*pp++ = *qq;
		*qq++ = c;
		n -= sizeof(long);
	}

	p = (char *)pp;
	q = (char *)qq;

	while (n--) {
		c = *p;
		*p++ = *q;
		*q++ = c;
	}
}

static void q3exchange(char *p, char *q, char *r, size_t n)
{
	long c;
	long *pp = (long *)p;
	long *qq = (long *)q;
	long *rr = (long *)r;

	while (n > sizeof(long)) {
		c = *pp;
		*pp++ = *rr;
		*rr++ = *qq;
		*qq++ = c;
		n -= sizeof(long);
	}

	p = (char *)pp;
	q = (char *)qq;
	r = (char *)rr;

	while (n--) {
		c = *p;
		*p++ = *r;
		*r++ = *q;
		*q++ = c;
	}
}
