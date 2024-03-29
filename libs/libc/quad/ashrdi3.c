/*	$NetBSD: ashrdi3.c,v 1.2 2009/03/15 22:31:12 cegger Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)ashrdi3.c	8.1 (Berkeley) 6/4/93";
#else
__RCSID("$NetBSD: ashrdi3.c,v 1.2 2009/03/15 22:31:12 cegger Exp $");
#endif
#endif				/* LIBC_SCCS and not lint */

#include "quad.h"

/*
 * Shift a (signed) quad value right (arithmetic shift right).
 */
quad_t __ashrdi3(quad_t a, qshift_t shift)
{
	union uu aa;

	if (shift == 0)
		return (a);
	aa.q = a;
	if (shift >= INT_BITS) {
		int s;

		/*
		 * Smear bits rightward using the machine's right-shift
		 * method, whether that is sign extension or zero fill,
		 * to get the `sign word' s.  Note that shifting by
		 * INT_BITS is undefined, so we shift (INT_BITS-1),
		 * then 1 more, to get our answer.
		 */
		/* LINTED inherits machine dependency */
		s = (aa.sl[H] >> (INT_BITS - 1)) >> 1;
		/* LINTED inherits machine dependency */
		aa.ul[L] = aa.sl[H] >> (shift - INT_BITS);
		aa.ul[H] = s;
	} else {
		aa.ul[L] = (aa.ul[L] >> shift) | (aa.ul[H] << (INT_BITS - shift));
		/* LINTED inherits machine dependency */
		aa.sl[H] >>= shift;
	}
	return (aa.q);
}
