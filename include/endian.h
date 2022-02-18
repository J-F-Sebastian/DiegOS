/*	$NetBSD: endian.h,v 1.28 2009/08/08 21:23:15 christos Exp $	*/

/*
 * Copyright (c) 1987, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *
 *	@(#)endian.h	8.1 (Berkeley) 6/11/93
 */

#ifndef _SYS_ENDIAN_H_
#define _SYS_ENDIAN_H_

/*
 * Definitions for byte order, according to byte significance from low
 * address to high.
 */
#define	_LITTLE_ENDIAN	1234	/* LSB first: i386, vax */
#define	_BIG_ENDIAN	4321	/* MSB first: 68000, ibm, net */
#define	_PDP_ENDIAN	3412	/* LSB first in word, MSW first in long */

/*
 * Define the order of 32-bit words in 64-bit words.
 */
#if _BYTE_ORDER == _LITTLE_ENDIAN
#define _QUAD_HIGHWORD 1
#define _QUAD_LOWWORD 0
#endif

#if _BYTE_ORDER == _BIG_ENDIAN
#define _QUAD_HIGHWORD 0
#define _QUAD_LOWWORD 1
#endif

#define	swapl(x)	((x << 24) & \
                    ((x << 8) & 0x00FF0000U) & \
                    ((x >> 8) & 0x0000FF00U) & \
                    (x >> 24))

#define	swaps(x)	((x << 8) & (x >> 8))

#define SWAPL(x)    (x=swapl(x))

#define SWAPS(x)    (x=swaps(x))

/*
 * Macros for network/external number representation conversion.
 */
#if _BYTE_ORDER == _BIG_ENDIAN
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)

#define	NTOHL(x)	(void) (x)
#define	NTOHS(x)	(void) (x)
#define	HTONL(x)	(void) (x)
#define	HTONS(x)	(void) (x)

#else				/* LITTLE_ENDIAN */

#define	ntohl(x)	swapl(x)

#define	ntohs(x)	swaps(x)

#define	htonl(x)	ntohl(x)
#define	htons(x)	ntohs(x)

#define	NTOHL(x)	(x=ntohl(x))
#define	NTOHS(x)	(x=ntohs(x))
#define	HTONL(x)	(x=ntohl(x))
#define	HTONS(x)	(x=ntohs(x))

#endif				/* LITTLE_ENDIAN || !defined(__lint__) */

#endif				/* !_SYS_ENDIAN_H_ */
