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

#define DBG_MODULE_ON
#include <diegos/debug_traces.h>
#include <diegos/kernel.h>
#include <diegos/kernel_dump.h>
#include <diegos/alarms.h>
#include <diegos/barriers.h>
#include <diegos/poll.h>
#include <chunks.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "system_parser.h"
#include "system_parser_macros.h"

BEGIN_ALT_COMMAND(show)
    ALT_COMMAND(interfaces, "network interfaces")
    ALT_COMMAND_FUNC0(threads, "system threads", threads_dump)
    ALT_COMMAND_FUNC0(mutexes, "system mutexes", mutexes_dump)
    ALT_COMMAND(barriers, "barriers")
    END_ALT_COMMAND()

    CREATE_ALTERNATE(show)

    BEGIN_ALT_COMMAND(root)
    ALT_COMMAND_NEXT(show, "show system informations", show)
    ALT_COMMAND(help, "help !!!")
    END_ALT_COMMAND()

    CREATE_ALTERNATE(root)
#define TIMER_POLL      (30*1000)
#define TIMER_STAGE1    (2)
#define TIMER_STAGE2    (TIMER_STAGE1 + 1)
static const char *timeout_msg[] = {
	"\n *** Login session has been idle for 1 minute  ***",
	"\n *** Login session will time out in 30 seconds ***",
	"\n *** Time out ***"
};

static unsigned volatile timeout_stage = 0;

/*
static void print_input_help (void)
{
    puts("\n Type in a command, you moron!");
}
*/

static void print_banner(void)
{
	puts("\nDiegOS  Copyright (C) 2012-2021  Diego Gallizioli\n");
	puts("This program comes with ABSOLUTELY NO WARRANTY.");
	puts("This is free software, and you are welcome to redistribute it");
	puts("under certain conditions.");
	puts("\n");
}

static void print_login_time(void)
{
	time_t tmp = time(NULL);
	printf("Login time: %s\n", ctime(&tmp));
}

static void print_time(void)
{
	time_t tmp = time(NULL);
	printf("Local time: %s\n", ctime(&tmp));
}

static void console_main_entry(void)
{
	char buffer[128] = { 0 };
	unsigned buffer_head;
	int retval;
	BOOL cmd_typein = TRUE;
	struct pollfd waitinp = {.fd = 0,.events = 0,.revents = POLLIN };

	print_banner();
	print_login_time();

	while (TRUE) {
		printf("# ");
		fflush(stdout);
		buffer_head = 0;
		while (cmd_typein && (buffer_head < sizeof(buffer))) {
			retval = poll(&waitinp, 1, 0);
			retval = getchar();
			timeout_stage = 0;
			if (isprint(retval)) {
				buffer[buffer_head++] = (char)retval;
				putchar(retval);
			} else if (iscntrl(retval)) {
				switch (retval) {
				case '\b':
					if (buffer_head) {
						buffer[buffer_head--] = '\0';
						putchar(retval);
					}
					break;
				case '\t':
					buffer[buffer_head++] = (char)retval;
					/* FALLTHRU */
					/* no break */
				case '\r':
					/* FALLTHRU */
					/* no break */
				case '\n':
					cmd_typein = FALSE;
					buffer[buffer_head++] = '\0';
					break;
				}
			}
			fflush(stdout);
		}

		puts("\r");

		system_parser(buffer, buffer_head);

		cmd_typein = TRUE;
	}
}

static BOOL perform_login(void)
{
	unsigned repeat = 4;
	char username[9];
	char password[9];

	while (--repeat) {
		memset(username, 0, sizeof(username));
		memset(password, 0, sizeof(password));
		printf("\n\n Username: ");
		if (1 == scanf("%8s", username)) {
			if (memcmp("diego", username, 5)) {
				msleep(100);
				continue;
			}
		}
		printf("\n Password: ");
		if (1 == scanf("%8s", password)) {
			if (memcmp("diego", password, 5)) {
				msleep(100);
				continue;
			}
		}
		break;
	}
	return ((repeat) ? (TRUE) : (FALSE));
}

static void login_main_entry(void)
{
	alarm_t *timer;
	ev_queue_t *events;
	event_t *timeout;
	STATUS retcode;
	BOOL retlogin;
	uint8_t tid;

	events = event_init_queue("Login");
	timer = alarm_create("Login", 0xdead, TIMER_POLL, TRUE, events);
	assert(events != NULL);
	assert(timer != NULL);

	retcode = event_watch_queue(events);
	assert(EOK == retcode);

	sleep(1);

	while (TRUE) {
		do {
			sleep(1);
			printf("\n\n\n\n\t press Enter to start...\n");
			while (EOF == getchar()) {
				clearerr(stdin);
			}
			retlogin = perform_login();
		} while (FALSE == retlogin);

		retlogin = thread_create("Console",
					 THREAD_PRIO_NORMAL, console_main_entry, 0, 8192, &tid);
		if (!retlogin) {
			TRACE_PRINT("Failed spawning thread Console")
			    continue;
		}

		timeout_stage = 0;
		alarm_set(timer, TRUE);

		while (timeout_stage < TIMER_STAGE2 + 1) {
			wait_for_events(events);
			timeout = event_get(events);
			alarm_dump(NULL);
			threads_dump();

			if ((CLASS_ALARM == timeout->classid) && (0xdead == timeout->eventid)) {
				alarm_acknowledge(timer);
				++timeout_stage;
				if (TIMER_STAGE1 == timeout_stage) {
					puts(timeout_msg[0]);
				} else if (TIMER_STAGE2 == timeout_stage) {
					puts(timeout_msg[1]);
				}
				print_time();
			}
		}

		puts(timeout_msg[2]);
		alarm_set(timer, FALSE);
		alarm_done(timer);
		thread_kill(tid);
	}
}

void console_run(void)
{
	uint8_t pid;
	BOOL retcode;

	system_parser_init(&alternates_root);

	retcode = thread_create("Login", THREAD_PRIO_NORMAL, login_main_entry, 0, 4096, &pid);

	assert(TRUE == retcode);
}
