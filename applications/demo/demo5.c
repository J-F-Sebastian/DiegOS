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

#include <libs/chunks.h>
#include <stdio.h>

void platform_run(void)
{
	chunks_pool_t *chunk;
	void *test[120];
	unsigned i;

	printf("START\n");

	chunk = chunks_pool_create(NULL, 8, 50, 10, 50);

	if (!chunk) {
		printf("ERROR 1\n");
	}

	for (i = 0; i < 100; i++) {
		test[i] = chunks_pool_malloc(chunk);

		/*    printf("%d --> %p\n",i, test[i]); */
	}

	/* chunks_pool_dump(chunk); */

	chunks_pool_free(chunk, test[11]);
	chunks_pool_free(chunk, test[1]);
	chunks_pool_free(chunk, test[8]);
	chunks_pool_free(chunk, test[10]);
	chunks_pool_free(chunk, test[78]);
	chunks_pool_free(chunk, test[101]);

	chunks_pool_dump(chunk);

	for (i = 0; i < 100; i++) {

		chunks_pool_free(chunk, test[i]);
	}

	/* chunks_pool_dump(chunk); */
	printf("DONE\n");
}
