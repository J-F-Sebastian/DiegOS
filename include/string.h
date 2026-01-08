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

#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

void *memchr(const void *_s, int _c, size_t _n);
int memcmp(const void *_s1, const void *_s2, size_t _n);
void *memcpy(void *_s1, const void *_s2, size_t _n);
void *memmove(void *_s1, const void *_s2, size_t _n);
void *memset(void *_s, int _c, size_t _n);
char *strcat(char *_s1, const char *_s2);
char *strchr(const char *_s, int _c);
int strncmp(const char *_s1, const char *_s2, size_t _n);
int strcmp(const char *_s1, const char *_s2);
int strnicmp(const char *_s1, const char *_s2, size_t _n);
int stricmp(const char *_s1, const char *_s2);
#define strncasecmp strnicmp
#define strcasecmp stricmp
int strcoll(const char *_s1, const char *_s2);
char *strcpy(char *_s1, const char *_s2);
size_t strcspn(const char *_s1, const char *_s2);
char *strerror(int _errnum);
size_t strlen(const char *_s);
char *strncat(char *_s1, const char *_s2, size_t _n);
char *strncpy(char *_s1, const char *_s2, size_t _n);
char *strpbrk(const char *_s1, const char *_s2);
char *strrchr(const char *_s, int _c);
size_t strspn(const char *_s1, const char *_s2);
char *strstr(const char *_s1, const char *_s2);
char *strtok(char *_s1, const char *_s2);
char *strtok_r(char *_s1, const char *_s2, char **saveptr);

#endif
