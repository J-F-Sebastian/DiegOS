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

#include <time.h>

#include "fat_internals.h"

#define FAT_EOC     0xFFFFU
#define FAT_UNUSED  0

uint16_t FAT_get_cluster_from_table(struct FATVolume *vol, uint16_t N)
{
	if (!vol || !vol->FAT)
		return (uint16_t) - 1U;

	if (N < vol->FATSize) {
		return vol->FAT[N];
	}

	return (uint16_t) - 1U;
}

int FAT_set_cluster_into_table(struct FATVolume *vol, uint16_t N, uint16_t val)
{
	if (!vol || (N < 2) || (!vol->FAT))
		return -1;

	if (N < vol->FATSize) {
		vol->FAT[N] = val;
		return 0;
	}

	return -1;
}

int FAT_set_EOC_into_table(struct FATVolume *vol, uint16_t N)
{
	return FAT_set_cluster_into_table(vol, N, FAT_EOC);
}

int FAT_check_DIR_entry(struct FAT *entry)
{
	const uint8_t mask = ATTR_DIRECTORY | ATTR_VOLUME_ID;

	if (entry->DIR_Name[0] == 0)
		return 0;

	if (entry->DIR_Name[0] != (char)DIR_ENTRY_FREE) {
		/* Found a file, a directory, or a link */
		if ((entry->DIR_Attr & mask) != mask) {
			return 1;
		}
	}

	return -1;
}

int FAT_check_attrib(uint8_t attributes)
{
	const uint8_t mask = ATTR_DIRECTORY | ATTR_VOLUME_ID;

	/* Found a file, a directory, a link, or a volume label */
	if ((attributes & mask) != mask) {
		return 0;
	}

	return -1;
}

uint16_t FAT_get_free_cluster_from_table(struct FATVolume * vol)
{
	uint16_t N;

	if (!vol)
		return FAT_UNUSED;

	for (N = vol->lastFreeFATEntry; N < vol->FATSize; N++) {
		if (FAT_UNUSED == vol->FAT[N]) {
			vol->lastFreeFATEntry = N;
			return N;
		}
	}

	for (N = 2; N < vol->lastFreeFATEntry + 1; N++) {
		if (FAT_UNUSED == vol->FAT[N]) {
			vol->lastFreeFATEntry = N;
			return N;
		}
	}

	return FAT_UNUSED;
}

int FAT_update_timestamps(struct FAT *entry, int flags)
{
	time_t ts = 0;
	struct tm *fmtts = 0;
	struct FAT_date_stamp *dstamp;
	struct FAT_time_stamp *tstamp;

	if (!entry)
		return -1;

	ts = time(NULL);
	fmtts = localtime(&ts);
	if (flags & (FAT_TS_WRITE | FAT_TS_CREATE)) {
		tstamp = &entry->DIR_WrtTime;
		dstamp = &entry->DIR_WrtDate;
		tstamp->msecs = 0;
		tstamp->secs = (uint32_t) fmtts->tm_sec;
		tstamp->mins = (uint32_t) fmtts->tm_min;
		tstamp->hours = (uint32_t) fmtts->tm_hour;
		dstamp->dayOfMonth = (uint16_t) fmtts->tm_mday;
		dstamp->monthOfYear = (uint16_t) fmtts->tm_mon + 1;
		/* Date starts in year 2000 */
		dstamp->years = (fmtts->tm_year > 100) ? ((uint16_t) fmtts->tm_year - 100) : 0;
	}
	if (flags & FAT_TS_CREATE) {
		tstamp = &entry->DIR_CrtTime;
		dstamp = &entry->DIR_CrtDate;
		tstamp->msecs = 0;
		tstamp->secs = (uint32_t) fmtts->tm_sec;
		tstamp->mins = (uint32_t) fmtts->tm_min;
		tstamp->hours = (uint32_t) fmtts->tm_hour;
		dstamp->dayOfMonth = (uint16_t) fmtts->tm_mday;
		dstamp->monthOfYear = (uint16_t) fmtts->tm_mon + 1;
		/* Date starts in year 2000 */
		dstamp->years = (fmtts->tm_year > 100) ? ((uint16_t) fmtts->tm_year - 100) : 0;
	}
	if (flags & (FAT_TS_READ | FAT_TS_WRITE | FAT_TS_CREATE)) {
		tstamp = &entry->DIR_AccTime;
		dstamp = &entry->DIR_AccDate;
		tstamp->msecs = 0;
		tstamp->secs = (uint32_t) fmtts->tm_sec;
		tstamp->mins = (uint32_t) fmtts->tm_min;
		tstamp->hours = (uint32_t) fmtts->tm_hour;
		dstamp->dayOfMonth = (uint16_t) fmtts->tm_mday;
		dstamp->monthOfYear = (uint16_t) fmtts->tm_mon + 1;
		/* Date starts in year 2000 */
		dstamp->years = (fmtts->tm_year > 100) ? ((uint16_t) fmtts->tm_year - 100) : 0;
	}

	return 0;
}

int FAT_update_statistics(struct FATVolume *vol, struct FAT *buffer, struct FAT_statistics *stats)
{
	unsigned i;
	uint32_t totfiles = 0, totdirs = 0, totlinks = 0, totbytes = 0, totavail = 0;

	if (!vol || !buffer || !stats)
		return -1;

	for (i = 0; i < vol->FATEntriesPerSec * vol->PB.SecPerClus; i++, buffer++) {
		if (buffer->DIR_Name[0] == (char)DIR_ENTRY_FREE)
			totavail++;
		else if (buffer->DIR_Name[0] == (char)DIR_ENTRY_LAST_FREE)
			break;
		else {
			if (buffer->DIR_Attr & ATTR_LINK)
				totlinks++;
			if (buffer->DIR_Attr & ATTR_DIRECTORY)
				totdirs++;
			else
				totfiles++;
			totbytes += buffer->DIR_FileSize;
		}
	}

	stats->totdirs += totdirs;
	stats->totavail += totavail;
	stats->totbytes += totbytes;
	stats->totfiles += totfiles;
	stats->totlinks += totlinks;

	return 0;
}
