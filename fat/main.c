/*
 * FAT File System
 *
 * Copyright (C) 2019 Diego Gallizioli
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "disk_access.h"
#include "fat.h"
#include "mbr.h"
#include "fat_print.h"
#include "fat_format.h"

static struct FATVolume vol;
static struct MBR mbr;
static struct disk_geometry geom;
static void *ctx = 0;

static void do_format(void)
{
	int index;
	struct FATPartition part;
	struct MBR_partition_entry entry;
	char answer;
	int sectors = 0;
	char buffer[17];

	printf(" * FORMAT ROUTINE\n\n");

	do {
		MBR_list(&mbr);
		printf("* SELECT PARTITION TO FORMAT: \n");
		scanf("%d", &index);
		putchar('\n');
	}
	while ((index < 0) || (index > 3));

	if (MBR_get_partition_entry(&mbr, index, &entry)) {
		printf("* ERROR: CANNOT ACCESS SELECTED PARTITION\n");
		return;
	}

	part.startsector = entry.LBA_first_Sector;
	part.endsector = part.startsector + entry.num_of_sectors - 1;
	part.secpercluster = 0;
	memset(buffer, 0, sizeof(buffer));
	printf("* ENTER VOLUME NAME [16 characters]: ");
	scanf("%16s", buffer);
	putchar('\n');
	if (!strlen(buffer))
		memset(part.label, ' ', sizeof(part.label));
	else {
		if (strlen(buffer) < sizeof(part.label))
			memset(buffer + strlen(buffer), ' ', sizeof(buffer) - strlen(buffer));
		strncpy(part.label, buffer, sizeof(part.label));
	}

	printf("* FORMAT PARTITION [%d] SECTORS [%d] LABEL [%s]\n", index, entry.num_of_sectors,
	       part.label);
	do {
		printf(" Yes or No? ");
		scanf("%c", &answer);
		putchar('\n');
	}
	while ((toupper(answer) != 'Y') && (toupper(answer) != 'N'));

	if (toupper(answer) == 'N') {
		return;
	}

	printf("* SECTORS PER CLUSTER ? [0] ");
	scanf("%d", &sectors);
	part.secpercluster = sectors & UCHAR_MAX;

	printf("* FORMAT ROUTINE: FORMATTING ... ");
	if (FAT_format(ctx, &part)) {
		printf("FAILED.\n");
	} else {
		printf("SUCCESS.\n");
	}
}

static void do_mount(void)
{
	int index;
	struct MBR_partition_entry entry;

	printf(" * MOUNT ROUTINE\n\n");

	do {
		MBR_list(&mbr);
		printf("* SELECT PARTITION TO MOUNT: ");
		scanf("%d", &index);
		putchar('\n');
	}
	while ((index < 0) || (index > 3));

	if (MBR_get_partition_entry(&mbr, index, &entry)) {
		printf("* ERROR: CANNOT ACCESS SELECTED PARTITION\n");
		return;
	}

	if (FAT_mount(ctx, entry.LBA_first_Sector, &vol)) {
		printf("* ERROR: CANNOT MOUNT SELECTED PARTITION\n");
		return;
	}

	FAT_print_Volume(&vol);
	putchar('\n');
	putchar('\n');
	FAT_print_PB(&vol.PB);
}

static void do_unmount()
{
	printf("* UNMOUNT ROUTINE\n\n");

	if (FAT_unmount(&vol))
		printf("* ERROR: CANNOT UNMOUNT\n");
}

static void do_create()
{
	char path[PATH_MAX];
	char name[45];

	printf("* CREATE ROUTINE\n\n");

	memset(path, 0, sizeof(path));
	memset(name, 0, sizeof(name));
	printf("* ENTER PATH TO FILE : ");
	scanf("%s", path);
	putchar('\n');
	printf("* ENTER FILE NAME : ");
	scanf("%44s", name);
	if (strlen(path) > 1)
		strncat(path, "\\", 1);
	strncat(path, name, sizeof(path) - sizeof(name));
	printf("* CREATING %s ...\n", path);
	if (FAT_create_entry(&vol, path, 0))
		printf("* CREATION ERROR\n");
}

static void do_create_dir()
{
	char path[PATH_MAX];
	char name[45];

	printf("* DIRECTORY CREATE ROUTINE\n\n");

	memset(path, 0, sizeof(path));
	memset(name, 0, sizeof(name));
	printf("* ENTER PATH TO DIRECTORY : ");
	scanf("%s", path);
	putchar('\n');
	printf("* ENTER DIRECTORY NAME : ");
	scanf("%12s", name);
	if (strlen(path) > 1)
		strncat(path, "\\", 1);
	strncat(path, name, sizeof(path) - sizeof(name));
	printf("* CREATING %s ...\n", path);

	if (FAT_create_entry(&vol, path, ATTR_DIRECTORY))
		printf("* CREATION ERROR\n");
}

static void do_delete()
{
	char path[PATH_MAX];

	printf("* DELETE ROUTINE\n\n");

	memset(path, 0, sizeof(path));
	printf("* ENTER COMPLETE PATH NAME : ");
	scanf("%s", path);
	putchar('\n');

	if (FAT_delete_entry(&vol, path))
		printf("* DELETION ERROR\n");
}

static void do_rename()
{
	char path[PATH_MAX];
	char oldname[45];
	char newname[45];

	printf("* RENAME ROUTINE\n\n");

	memset(path, 0, sizeof(path));
	memset(oldname, 0, sizeof(oldname));
	memset(newname, 0, sizeof(newname));
	printf("* ENTER PATH TO FILE : ");
	scanf("%s", path);
	putchar('\n');
	printf("* ENTER OLD FILE NAME : ");
	scanf("%44s", oldname);
	printf("* ENTER NEW FILE NAME : ");
	scanf("%44s", newname);
	printf("* RENAMING %s TO %s in %s ...\n", oldname, newname, path);
	if (strlen(path) > 1)
		strncat(path, "\\", 1);
	strncat(path, oldname, sizeof(path) - sizeof(oldname));

	if (FAT_rename_entry(&vol, path, newname))
		printf("* RENAMING ERROR\n");
}

static void do_list()
{
	char path[PATH_MAX];
	uint32_t b;
	uint16_t c;

	printf(" * LIST ROUTINE\n\n");

	printf("* SELECT PATH TO LIST: ");
	scanf("%s", path);
	putchar('\n');

	if (FAT_list(&vol, path))
		printf("* ERROR: CANNOT LIST THIS PATH\n");
	if (FAT_available(&vol, &b, &c))
		printf("* ERROR: CANNOT GET AVAILABLE SPACE\n");
	printf("\nAvailable clusters ... : %u"
	       "\nAvailable bytes ...... : %u (%u MB)" "\n\n", c, b, b / (1024 * 1024));
}

static void do_write()
{
	char path[PATH_MAX];
	unsigned len = 0;
	unsigned offset = 0;
	unsigned wrtlen = 0;
	char *buffer = 0;
	unsigned i = 0;

	printf("* WRITE ROUTINE\n\n");

	memset(path, 0, sizeof(path));
	printf("* ENTER COMPLETE PATH NAME : ");
	scanf("%s", path);
	putchar('\n');
	printf("* ENTER OFFSET : ");
	scanf("%u", &offset);
	putchar('\n');
	printf("* ENTER SIZE : ");
	scanf("%u", &len);
	putchar('\n');
	buffer = malloc(len);

	for (i = 0; i < len; i++)
		buffer[i] = ('0' + i % 16);

	if (FAT_write(&vol, path, offset, buffer, len, &wrtlen))
		printf("* WRITE ERROR %d\n", wrtlen);

	free(buffer);
}

static void do_read()
{
	char path[PATH_MAX];
	unsigned len = 0;
	unsigned offset = 0;
	unsigned rdlen = 0;
	char *buffer = 0;
	unsigned i = 0;

	printf("* READ ROUTINE\n\n");

	memset(path, 0, sizeof(path));
	printf("* ENTER COMPLETE PATH NAME : ");
	scanf("%s", path);
	putchar('\n');
	printf("* ENTER OFFSET : ");
	scanf("%u", &offset);
	putchar('\n');
	printf("* ENTER SIZE : ");
	scanf("%u", &len);
	putchar('\n');
	buffer = malloc(len);

	if (FAT_read(&vol, path, offset, buffer, len, &rdlen))
		printf("* READ ERROR %d\n", rdlen);
	else
		for (i = 0; i < rdlen; i++) {
			putchar(buffer[i]);
			if ((i + 1) % 60 == 0) {
				printf(" -- %u\n", i + 1);
			}
		}
	putchar('\n');
	free(buffer);
}

static void do_truncate()
{
	char path[PATH_MAX];
	unsigned wrtlen = 0;
	char *buffer = 0;

	printf("* TRUNCATE ROUTINE\n\n");

	memset(path, 0, sizeof(path));
	printf("* ENTER COMPLETE PATH NAME : ");
	scanf("%s", path);
	putchar('\n');

	if (FAT_truncate_entry(&vol, path))
		printf("* TRUNCATE ERROR %d\n", wrtlen);

	free(buffer);
}

static void do_megatest()
{
	unsigned i = 0;
	char buffer[1024 * 7 / 3];
	unsigned bufsize;
	static const struct {
		const char name[PATH_MAX];
		uint8_t attr;
	} layout[] = {
		{
		"\\read.me", 0}, {
		"\\DiegOS.bin", ATTR_HIDDEN | ATTR_SYSTEM | ATTR_READ_ONLY}, {
		"\\diegos", ATTR_DIRECTORY}, {
		"\\diegos\\i18n", ATTR_DIRECTORY}, {
		"\\diegos\\chkdsk.exe", 0}, {
		"\\diegos\\attrib.exe", 0}, {
		"\\diegos\\format.exe", 0}, {
		"\\diegos\\copy.exe", 0}, {
		"\\diegos\\xcopy.exe", 0}, {
		"\\diegos\\superMegaGigaTonictOOl.com", 0}, {
		"\\diegos\\dosshell.exe", 0}, {
		"\\diegos\\edit.exe", 0}, {
		"\\diegos\\edlin.exe", 0}, {
		"\\diegos\\EMM386.exe", 0}, {
		"\\diegos\\tree.exe", 0}, {
		"\\diegos\\command.com", 0}, {
		"\\DOCS", ATTR_DIRECTORY}, {
		"\\DoCs\\readme!!!.TXT", 0}, {
		"\\diegos\\I18N\\english.inf", ATTR_READ_ONLY}, {
		"\\diegos\\I18N\\german.inf", ATTR_READ_ONLY}, {
		"\\diegos\\I18N\\turkish.inf", ATTR_READ_ONLY}, {
		"\\diegos\\I18N\\suomi.inf", ATTR_READ_ONLY}, {
		"\\diegos\\I18N\\french.inf", ATTR_READ_ONLY}, {
		"\\diegos\\i18n\\english.inf", ATTR_READ_ONLY}, {
		"\\diegos\\i18n\\german.inf", ATTR_READ_ONLY}, {
		"\\diegos\\i18n\\turkish.inf", ATTR_READ_ONLY}, {
		"\\diegos\\i18n\\suomi.inf", ATTR_READ_ONLY}, {
		"\\diegos\\i18n\\french.inf", ATTR_READ_ONLY}, {
		"\\diegos\\dir.exe", 0}, {
		"\\diegos\\robocopy.exe", 0}, {
		"\\diegos\\more.com", 0}, {
		"\\diegos\\defrag.exe", 0}, {
		"\\diegos\\debug.com", 0}, {
		"\\diegos\\keyb.com", 0}, {
		"\\IO.SYS", ATTR_HIDDEN | ATTR_SYSTEM | ATTR_READ_ONLY}, {
		"", 0}
	};

	printf("* MEGATEST ROUTINE\n\n");

	while (layout[i].name[0]) {
		if (FAT_create_entry(&vol, layout[i].name, layout[i].attr))
			printf("* CREATION ERROR FOR %s\n", layout[i].name);
		else {
			bufsize = rand() % (1024 * 7 / 3);
			memset(buffer, rand(), bufsize);
			if (FAT_write(&vol, layout[i].name, 0, buffer, bufsize, &bufsize))
				printf("WRITE ON CREATION ERROR FOR %s\n", layout[i].name);
		}
		++i;
	}
}

static void do_print_fat()
{
	putchar('\n');
	FAT_print_FAT(&vol);
	putchar('\n');
}

static void print_main_menu()
{
	printf("**********************************************\n");
	printf("1\tFORMAT VOLUME\n");
	printf("2\tMOUNT VOLUME\n");
	printf("3\tUNMOUNT VOLUME\n");
	printf("4\tCREATE FILE\n");
	printf("5\tCREATE DIRECTORY\n");
	printf("6\tDELETE FILE OR DIRECTORY\n");
	printf("7\tRENAME FILE OR DIRECTORY\n");
	printf("8\tLIST DIRECTORY\n");
	printf("9\tWRITE TO FILE\n");
	printf("10\tREAD FROM FILE\n");
	printf("11\tTRUNCATE FILE\n");
	printf("21\tMEGA TEST\n");
	printf("22\tPRINT FAT\n");
	printf("----------------------------------------------\n");
	printf("99\tEXIT\n");
	printf("**********************************************\n");
}

int main()
{
	int retcode;

	//if (disk_init("E:\\temp\\FAT16.img",&ctx))
	if (disk_init("output.img", &ctx)) {
		printf("ERROR INITIALIZING DISK\n");
		return -99;
	}

	if (disk_get_geometry(ctx, &geom)) {
		printf("ERROR READING GEOMETRY\n");
		return -98;
	}

	memset(&vol, 0, sizeof(vol));
	if (MBR_read(ctx, &mbr)) {
		printf("ERROR READING MBR\n");
		disk_done(ctx);
		return -97;
	}

	while (1) {
		print_main_menu();
		scanf("%d", &retcode);
		if (retcode == 1)
			do_format();
		else if (retcode == 2)
			do_mount();
		else if (retcode == 3)
			do_unmount();
		else if (retcode == 4)
			do_create();
		else if (retcode == 5)
			do_create_dir();
		else if (retcode == 6)
			do_delete();
		else if (retcode == 7)
			do_rename();
		else if (retcode == 8)
			do_list();
		else if (retcode == 9)
			do_write();
		else if (retcode == 10)
			do_read();
		else if (retcode == 11)
			do_truncate();
		else if (retcode == 21)
			do_megatest();
		else if (retcode == 22)
			do_print_fat();
		else if (retcode == 99)
			break;
	}

	disk_done(ctx);

	return 0;
}
