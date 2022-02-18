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

#include <stdio.h>
#include <string.h>
#include <diegos/devices.h>
#include <diegos/kernel.h>
#include <errno.h>

#if 0
static char teststring[] = "%.3u Uart write test - very very long string "
    "but very very very long long long schlong string"
    "but very very very long long long schlong string"
    "but very very very long long long schlong string"
    "but very very very long long long schlong string"
    "but very very very long long long schlong string";

static void demo_thread_entry(void)
{
	device_t *testser;
	char test[260];
	unsigned i = 0;
	unsigned j = 10;
	//int retcode = 1;

	printf("START TX\n");

	testser = device_lookup("uart", 0);

	if (!testser) {
		printf("ERROR 1 !!!\n");
	}

	memset(test, 0, sizeof(test));

	while (1) {
		snprintf(test, sizeof(test), teststring, i++);
		j++;
		if (j > strlen(test)) {
			j = 0;
			memset(test, 0, sizeof(test));
		}
		printf("%s\n", test);
		//retcode = device_io(testser, FALSE, test, j);
		//if (retcode < 0) {
		//    printf("ERROR %d !!!\n", retcode);
		//}
		//thread_suspend();
		thread_delay(500);
	}
}
#endif

static void demo_thread_entry2(void)
{
	device_t *testser;
	char test[100];
	int retcode;

	printf("START RX\n");

	testser = device_lookup("uart", 0);

	if (!testser) {
		printf("ERROR 1 !!!\n");
	}

	while (1) {
		/*
		 * This is blocking !!!
		 */
#if 0
		retcode = device_io(testser, TRUE, test, sizeof(test) - 1);
		if (retcode < 0) {
			printf("ERROR 2 !!!\n");
			thread_may_suspend();
		} else if (retcode == 0) {
			printf("NULLLLLLLLLLLLLLLLLLLLLLL");
		} else {
			test[retcode] = 0;
			printf("%s\n\r", test);
		}
#else
		memset(test, 0, sizeof(test));
		retcode = scanf("%99s", test);
		if (retcode) {
			printf("%s\r\n", test);
		}
#endif
	}
}

void platform_run(void)
{
	uint8_t pid;

#if 0
	thread_create("DemoTX", THREAD_PRIO_NORMAL, demo_thread_entry, 0, 4096, &pid);
#endif				// 0
	thread_create("DemoRX", THREAD_PRIO_NORMAL, demo_thread_entry2, 0, 4096, &pid);
}
