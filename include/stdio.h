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

#ifndef _STDIO_H_
#define	_STDIO_H_

#include <stddef.h>
#include <stdarg.h>
#include <types_common.h>

/*
 * Focus point of all stdio activity.
 */
typedef struct iobuf FILE;

extern FILE	*stdin;
extern FILE	*stdout;
extern FILE	*stderr;

/* The following definitions are also in <unistd.h>. They should not
 * conflict.
 */
#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2

#define	EOF		(-1)

#define BUFSIZ 1024

#define	FOPEN_MAX	20

#define	FILENAME_MAX	255

#define	TMP_MAX		999
#define	TMP_NAMELEN	(sizeof("/tmp/") + FILENAME_MAX)
#define __STDIO_VA_LIST__	void *

#define _IOFBF  (1<<10)
#define _IOLBF  (1<<9)
#define _IONBF  (1<<8)

typedef long int	fpos_t;

void clearerr (FILE *stream);
int fclose (FILE *stream);
int feof (FILE *stream);
int ferror (FILE *stream);
int fflush (FILE *stream);
int fgetc (FILE *stream);
int fgetpos (FILE *stream, fpos_t *pos);
char *fgets (char *s, int n, FILE *stream);
FILE *fopen (const char *filename, const char *mode);
int fprintf (FILE *stream, const char *format, ...);
int fputc (int c, FILE *stream);
int fputs (const char *s, FILE *stream);
size_t fread (void *ptr, size_t size, size_t nmemb, FILE *stream);
FILE *freopen (const char *filename, const char *mode, FILE *stream);
int fscanf (FILE *stream, const char *format, ...);
int fseek(FILE *stream, long offset, int origin);
int fsetpos (FILE *stream, fpos_t *pos);
long ftell (FILE *stream);
size_t fwrite (const void *ptr, size_t size, size_t count, FILE *stream);
int getc (FILE *stream);
#define getchar()   getc(stdin)
char *gets (char *s);
void perror (const char *s);
int printf (const char *format, ...);
int putc (int c, FILE *stream);
#define putchar(c)  putc(c,stdout)
int puts (const char *s);
int remove (const char *filename);
int rename (const char *old, const char *new);
void rewind (FILE *stream);
int scanf (const char *format, ...);
void setbuf (FILE *stream, char *buf);
int setvbuf (FILE *stream, char *buf, int mode, size_t size);
int snprintf (char *_s, size_t _n, const char *_format, ...);
int sprintf (char *s, const char *format, ...);
int sscanf (const char *s, const char *format, ...);
FILE *tmpfile (void);
char *tmpnam (char *s);
int ungetc (int _c, FILE *stream);
int vfprintf ( FILE * stream, const char * format, va_list arg);
int vfscanf ( FILE * stream, const char * format, va_list arg);
int vprintf (const char * format, va_list arg);
int vscanf (const char *_format, char *_arg);
int vsnprintf (char *s, size_t _n, const char *format, char *arg);
int vsprintf (char * s, const char * format, va_list arg );
int vsscanf (const char *s, const char *format, va_list ap);

int fileno (FILE *stream);
FILE *fdopen(int fildes, const char *mode);

#define L_ctermid 255	/* required by POSIX */
#define L_cuserid 255	/* required by POSIX */

FILE *popen (const char *_command, const char *_type);
int pclose (FILE *stream);

char *fgetln (FILE *stream, size_t *len);

#endif
