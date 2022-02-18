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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "disk_access.h"
#include "fat_internals.h"
#include "fat_names.h"
#include "fat_print.h"

#define MIN(a,b) ((a < b) ? a : b)

typedef unsigned (*fn_foreach) (const char *name, struct FAT * buffer, unsigned FATEntries);

static inline int diskRead(struct FATVolume *vol, unsigned sec, unsigned numsec, char *buf)
{
	return disk_read(vol->ctx, vol->FirstSector + sec, numsec, buf);
}

static inline int diskWrite(struct FATVolume *vol, unsigned sec, unsigned numsec, char *buf)
{
	return disk_write(vol->ctx, vol->FirstSector + sec, numsec, buf);
}

static inline int diskReadC(struct FATVolume *vol, unsigned sec, char *buf)
{
	return disk_read(vol->ctx, vol->FirstSector + sec, vol->PB.SecPerClus, buf);
}

static inline int diskWriteC(struct FATVolume *vol, unsigned sec, char *buf)
{
	return disk_write(vol->ctx, vol->FirstSector + sec, vol->PB.SecPerClus, buf);
}

/**************************************************************************************************
 **************************************************************************************************
 **************************************************************************************************
 **************************************************************************************************
 **************************************************************************************************
 **************************************************************************************************
 **************************************************************************************************
 **************************************************************************************************
 **************************************************************************************************
 **************************************************************************************************/

#define FAT_EOC     0xFFF8U
#define FAT_BAD     0xFFF7U
#define FAT_UNUSED  0

static inline int FAT_cluster_isEOC(uint16_t FATContent)
{
	return (FATContent >= FAT_EOC) ? 1 : 0;
}

static inline int FAT_cluster_isBAD(uint16_t FATContent)
{
	return (FATContent == FAT_BAD) ? 1 : 0;
}

static inline int FAT_cluster_isFREE(uint16_t FATContent)
{
	return (FATContent == FAT_UNUSED) ? 1 : 0;
}

static inline int FAT_cluster_isReadable(uint16_t FATContent)
{
	return (FAT_cluster_isEOC(FATContent) ||
		FAT_cluster_isBAD(FATContent) || FAT_cluster_isFREE(FATContent)) ? 0 : 1;
}

/*
 * Compute the cluster found at offset bytes from the very first cluster of the
 * directory entry. 
 */
static uint16_t FAT_lookup_FAT_table(struct FATVolume *vol, struct FAT *entry, unsigned offset)
{
	uint16_t cluster = entry->DIR_FstClus;

	offset /= vol->BytesPerCluster;

	while (offset--) {
		cluster = FAT_get_cluster_from_table(vol, cluster);
		if (!FAT_cluster_isReadable(cluster)) {
			break;
		}
	}

	return cluster;
}

/*
 * Search an array of FAT entries for a specific name.
  * return FATEntries if not found
 * return [0, vol->FATEntriesPerSec) if found
 */
static unsigned FAT_search_loop(const char *name, struct FAT *buffer, unsigned FATEntries)
{
	unsigned i = 0;
	size_t namelen = strlen(name);

	for (i = 0; i < FATEntries; i++) {
		if (buffer[i].DIR_Name[0] == (char)DIR_ENTRY_FREE)
			continue;
		if (buffer[i].DIR_Name[0] == (char)DIR_ENTRY_LAST_FREE)
			return FATEntries;
		if (strncmp(buffer[i].DIR_Name, name, namelen) == 0)
			break;
	}

	return i;
}

/*
 * Search an array of FAT entries for a free entry.
 * return FATEntries if not found.
 * return [0, vol->FATEntriesPerSec) if found.
 */
static unsigned FAT_search_free_loop(const char *name, struct FAT *buffer, unsigned FATEntries)
{
	unsigned i = 0;

	for (i = 0; i < FATEntries; i++) {
		if ((buffer[i].DIR_Name[0] == (char)DIR_ENTRY_FREE) ||
		    (buffer[i].DIR_Name[0] == (char)DIR_ENTRY_LAST_FREE)) {
			break;
		}

	}

	return i;
}

/*
 * Read a cluster and apply the fn function to each FAT directory entry.
 * return 0 if a record was found, values are set in result
 * return -1 if a record was not found
 */
static int FAT_dir_foreach(struct FATVolume *vol,
			   const char *name,
			   uint16_t cluster,
			   struct FAT_direntry_search_result *result, fn_foreach fn)
{
	unsigned i = 0;
	struct FAT *entry = 0;
	unsigned sector, items;

	items = vol->FATEntriesPerSec * vol->PB.SecPerClus;
	sector = FAT_FSoC(vol, cluster);

	entry = (struct FAT *)vol->buffer;
	if (diskReadC(vol, sector, vol->buffer)) {
		return -1;
	}

	i = fn(name, entry, items);

	if (i < items) {
		result->entry = *(entry + i);
		result->sector = sector + i / vol->FATEntriesPerSec;
		result->offset = i % vol->FATEntriesPerSec;
	}

	return (i < items) ? 0 : -1;
}

/*
 * Search for an entry in the file system parsing directory from the root to the final name.
 * If an entry is found, its values are stored in result.
 * returns 0 on success.
 */
static int FAT_search_DIR_entry(struct FATVolume *vol,
				const char *directory, struct FAT_direntry_search_result *result)
{
	char name[DIR_NAMELEN];
	size_t dirlen;
	size_t startpos = 0;
	size_t endpos = 0;
	size_t len = 0;
	int found = 0;
	uint16_t cluster, contcluster;
	struct FAT_direntry_search_result entryfound;

	dirlen = strlen(directory);
	if (!dirlen || (dirlen > FILENAME_MAX))
		return -1;

	contcluster = cluster = vol->PB.RootClus;
	do {
		found = 0;
		len = FAT_parse_directory(directory, dirlen, &startpos, &endpos);
		if (!len || (len > sizeof(entryfound.entry.DIR_Name)))
			return -1;

		FAT_build_DIR_name(directory + startpos, len, name);
		if (!name[0])
			return -1;

		while (!FAT_cluster_isEOC(cluster)) {
			found = FAT_dir_foreach(vol, name, cluster, &entryfound, FAT_search_loop);
			if (!found)
				break;

			cluster = FAT_get_cluster_from_table(vol, cluster);
		}

		if (found) {
			return -1;
		}
		if (endpos == dirlen) {
			entryfound.container = contcluster;
			*result = entryfound;
			return 0;
		}
		if ((entryfound.entry.DIR_Attr & ATTR_DIRECTORY) == 0)
			return -1;

		contcluster = cluster = entryfound.entry.DIR_FstClus;
		startpos = endpos;
	} while (startpos < dirlen);

	return -1;
}

/*
 * Read a sector in memory, updates timestamps, update the sector data in memory and then
 * on disk.
 * The sector belongs to a directory.
 * return 0 on success, -1 on error.
 */
static int FAT_write_DIR_entry(struct FATVolume *vol, struct FAT_direntry_search_result *entry)
{
	struct FAT *buf = (struct FAT *)vol->buffer;

	if (diskRead(vol, entry->sector, 1, vol->buffer))
		return -1;

	buf[entry->offset] = entry->entry;

	if (diskWrite(vol, entry->sector, 1, vol->buffer))
		return -1;

	return 0;
}

/*
 * Reads a cluster belonging to a directory and prints to standard output the contents.
 * Data read from disk is assumed to be a directory.
 */
static void FAT_list_cluster(struct FATVolume *vol, uint16_t cluster, struct FAT_statistics *stats)
{
	struct FAT *entry = (struct FAT *)vol->buffer;

	if (!diskReadC(vol, FAT_FSoC(vol, cluster), vol->buffer)) {
		FAT_print_loop(vol, entry);
		FAT_update_statistics(vol, entry, stats);
	}
}

/*
 * Search a directory for allocated entries, return 0 if the directory is void or has only
 * deallocated (deleted) entries, -1 in any other case. 
 */
static int FAT_directory_is_void(struct FATVolume *vol, struct FAT *dir)
{
	uint32_t sector;
	uint16_t cluster;
	unsigned limit = vol->FATEntriesPerSec * vol->PB.SecPerClus;
	struct FAT *entry = (struct FAT *)vol->buffer;

	/* Skip over '.' and '..' entries */
	entry += 2;
	cluster = dir->DIR_FstClus;

	while (!FAT_cluster_isEOC(cluster)) {
		sector = FAT_FSoC(vol, cluster);
		if (diskReadC(vol, sector, vol->buffer))
			return -1;

		while (entry < entry + limit) {
			if (entry->DIR_Name[0] != (char)DIR_ENTRY_FREE) {
				return (entry->DIR_Name[0] == (char)DIR_ENTRY_LAST_FREE) ? 0 : -1;
			}
			++entry;
		}

		entry = (struct FAT *)vol->buffer;
		cluster = FAT_get_cluster_from_table(vol, cluster);
	}

	return 0;
}

/*
 * Flush the FAT to disk
 */
static int FAT_write_FAT_table(struct FATVolume *vol)
{
	unsigned i, sec = vol->PB.ResvdSecCnt;

	for (i = 0; i < vol->PB.NumFATs; i++) {
		if (diskWrite(vol, sec, vol->PB.FATSz16, (char *)vol->FAT))
			return -1;
		sec += vol->PB.FATSz16;
	}

	return 0;
}

/*
 * Clean a cluster chain previously allocated to a directory or a file.
 */
static int FAT_clear_cluster_chain(struct FATVolume *vol, struct FAT *entry)
{
	uint16_t cluster;
	uint16_t N = entry->DIR_FstClus;

	if (FAT_cluster_isFREE(N) || (!entry->DIR_FileSize && !(entry->DIR_Attr & ATTR_DIRECTORY)))
		return 0;

	while (!FAT_cluster_isEOC(N)) {
		cluster = FAT_get_cluster_from_table(vol, N);

		if (FAT_set_cluster_into_table(vol, N, DIR_ENTRY_FREE))
			return -1;

		N = cluster;
	}

	return 0;
}

/*
 * Look for a free cluster and appends it to the cluster chain to which cluster belongs to.
 * cluster needs not to be the first cluster of a chain.
 * If a free cluster is found it is appended at the end of the chain and returned in newcluster.
 * Return 0 on success, -1 on failure
 */
static int FAT_append_cluster_chain(struct FATVolume *vol, uint16_t cluster, uint16_t * newcluster)
{
	uint16_t FATContent;
	uint16_t freecluster = FAT_get_free_cluster_from_table(vol);

	if (!freecluster) {
		return -1;
	}

	if (cluster) {
		FATContent = FAT_get_cluster_from_table(vol, cluster);
		while (!FAT_cluster_isEOC(FATContent)) {
			cluster = FATContent;
			FATContent = FAT_get_cluster_from_table(vol, cluster);
		}

		if (FAT_set_cluster_into_table(vol, cluster, freecluster)) {
			return -1;
		}
	}

	if (FAT_set_EOC_into_table(vol, freecluster)) {
		/*
		 * TODO handle failure, in this case the FAT is corrupted and the
		 * cluster chain points somewhere but not where it should.
		 */
		FAT_set_EOC_into_table(vol, cluster);
		return -1;
	}

	*newcluster = freecluster;
	return 0;
}

/**************************************************************************************************
 **************************************************************************************************
 **************************             PUBLIC API             ************************************
 **************************************************************************************************
 **************************************************************************************************/

int FAT_mount(void *ctx, uint32_t first_sec_on_disk, struct FATVolume *vol)
{
	char *buffer = 0;
	struct ParameterBlock *bpb = 0;
	struct disk_geometry geom;

	if (!ctx || !vol)
		return -1;

	if (vol->flags & FLAG_MOUNTED) {
		return -1;
	}

	vol->ctx = ctx;
	vol->FirstSector = first_sec_on_disk;

	/* EINVAL ? */
	if (disk_get_geometry(ctx, &geom))
		return -1;

	buffer = malloc(geom.bytes_per_sector);
	if (!buffer)
		return -1;

	if (diskRead(vol, 0, 1, buffer)) {
		free(buffer);
		return -1;
	}

	memcpy(&vol->PB, buffer, sizeof(vol->PB));
	free(buffer);
	bpb = &vol->PB;

	vol->FirstDataSector = bpb->ResvdSecCnt + (bpb->NumFATs * bpb->FATSz16);
	vol->DataSectors = bpb->TotSec - (bpb->ResvdSecCnt + (bpb->NumFATs * bpb->FATSz16));
	vol->CountOfClusters = vol->DataSectors / bpb->SecPerClus;
	vol->FATSize = (uint16_t) (vol->CountOfClusters + 2);
	vol->FATEntriesPerSec = vol->PB.BytsPerSec / sizeof(struct FAT);
	vol->BytesPerCluster = vol->PB.BytsPerSec * vol->PB.SecPerClus;
	vol->FAT = malloc(bpb->FATSz16 * bpb->BytsPerSec);
	if (!vol->FAT) {
		memset(bpb, 0, sizeof(*bpb));
		vol->DataSectors = vol->CountOfClusters = vol->FirstDataSector = 0;
		vol->FATSize = 0;
		vol->FATEntriesPerSec = 0;
		return -1;
	}

	vol->buffer = malloc(vol->BytesPerCluster);
	if (!vol->buffer) {
		memset(bpb, 0, sizeof(*bpb));
		free(vol->FAT);
		vol->FAT = NULL;
		vol->DataSectors = vol->CountOfClusters = vol->FirstDataSector = 0;
		vol->FATSize = 0;
		vol->FATEntriesPerSec = 0;
		return -1;
	}

	if (diskRead(vol, bpb->ResvdSecCnt, bpb->FATSz16, (char *)vol->FAT)) {
		memset(bpb, 0, sizeof(*bpb));
		free(vol->FAT);
		free(vol->buffer);
		vol->FAT = NULL;
		vol->buffer = NULL;
		vol->FAT = 0;
		vol->DataSectors = vol->CountOfClusters = vol->FirstDataSector = 0;
		vol->FATSize = 0;
		vol->FATEntriesPerSec = 0;
		return -1;
	}

	vol->flags |= FLAG_MOUNTED;
	vol->lastFreeFATEntry = 2;

	return 0;
}

int FAT_unmount(struct FATVolume *vol)
{
	if (!vol)
		return -1;

	if (vol->flags & FLAG_ACTIVE)
		return -1;

	if (!(vol->flags & FLAG_MOUNTED))
		return -1;

	/* flush FATs */
	FAT_write_FAT_table(vol);
	free(vol->FAT);
	free(vol->buffer);
	vol->FAT = NULL;
	vol->buffer = NULL;
	vol->ctx = 0;
	vol->flags &= ~FLAG_MOUNTED;

	return 0;
}

int FAT_get_entry(struct FATVolume *vol, const char *entryname, struct FAT *entry)
{
	struct FAT_direntry_search_result search;

	if (!vol || !entryname || !entry || !strlen(entryname))
		return -1;

	if (FAT_search_DIR_entry(vol, entryname, &search)) {
		memset(entry, 0, sizeof(*entry));
		return -1;
	}

	if (FAT_check_DIR_entry(&search.entry) > 0) {
		*entry = search.entry;
		return 0;
	}

	memset(entry, 0, sizeof(*entry));
	return -1;
}

int FAT_create_entry(struct FATVolume *vol, const char *entryname, uint8_t attributes)
{
	size_t pos;
	size_t len;
	char localname[DIR_NAMELEN];
	uint16_t cluster, FATcluster;
	struct FAT_direntry_search_result containerfound, entryfound;
	char *name = 0;
	struct FAT *buffer = 0;
	struct FAT *dirbuffer = 0;

	/* Check parameters */
	if (!vol || !entryname)
		return -1;

	len = strlen(entryname);
	if ((len < 2) || (entryname[0] != '\\'))
		return -1;

	/* Check attributes, links cannot be create within this function */
	if (FAT_check_attrib(attributes) || (attributes & ATTR_LINK))
		return -1;

	/* Find the last folder separator */
	pos = FAT_parse_directory_last(entryname, len);
	if ((len - pos - 1) > sizeof(entryfound.entry.DIR_Name))
		goto terminate;

	/* Copy the containing folder's name */
	name = malloc(pos + 1);
	if (!name)
		goto terminate;

	strncpy(name, entryname, pos);
	name[pos] = 0;

	if (pos) {
		/* Look for the container */
		if (FAT_search_DIR_entry(vol, name, &containerfound))
			goto terminate;
	}

	/* Build the desired entry name */
	if (FAT_build_DIR_name(entryname + pos + 1, len - pos - 1, localname))
		goto terminate;

	cluster = (pos) ? containerfound.entry.DIR_FstClus : vol->PB.RootClus;

	while (!FAT_cluster_isEOC(cluster)) {
		if (0 == FAT_dir_foreach(vol, localname, cluster, &entryfound, FAT_search_loop))
			goto terminate;

		cluster = FAT_get_cluster_from_table(vol, cluster);
	}

	/*
	 * Names are valid, parent path exists, the new name is unique.
	 * Search for an empty directory entry.
	 */
	cluster = (pos) ? containerfound.entry.DIR_FstClus : vol->PB.RootClus;

	while (!FAT_cluster_isEOC(cluster)) {
		if (0 == FAT_dir_foreach(vol, 0, cluster, &entryfound, FAT_search_free_loop))
			break;

		cluster = FAT_get_cluster_from_table(vol, cluster);
	}

	/*
	 * If we did not find an empty entry in a directory different than the root directory,
	 * and we hit the End Of File marker, we need more space so we need to allocate
	 * a new cluster for this directory.
	 */
	if (FAT_cluster_isEOC(cluster)) {
		FATcluster = FAT_get_free_cluster_from_table(vol);
		if (!FATcluster)
			goto terminate;

		if (FAT_set_cluster_into_table(vol, cluster, FATcluster))
			goto terminate;

		if (FAT_set_EOC_into_table(vol, FATcluster)) {
			/*
			 * TODO handle failure, in this case the FAT is corrupted and the
			 * cluster chain points somewhere but not where it should.
			 */
			FAT_set_EOC_into_table(vol, cluster);
			goto terminate;
		}

		if (FAT_write_FAT_table(vol))
			goto terminate;

		entryfound.sector = FAT_FSoC(vol, cluster);
		entryfound.offset = 0;
		entryfound.container = (pos) ? containerfound.entry.DIR_FstClus : vol->PB.RootClus;
		if (FAT_write_FAT_table(vol))
			goto terminate;
	}

	/* Clear entry */
	memset(&entryfound.entry, 0, sizeof(entryfound.entry));
	memcpy(entryfound.entry.DIR_Name, localname, sizeof(entryfound.entry.DIR_Name));
	entryfound.entry.DIR_Attr = attributes;
	entryfound.entry.DIR_FstClus = 0;

	if (FAT_update_timestamps(&entryfound.entry, FAT_TS_CREATE))
		goto terminate;

	/* Directories need a special treatment */
	if (attributes & ATTR_DIRECTORY) {
		FATcluster = FAT_get_free_cluster_from_table(vol);
		if (!FATcluster)
			goto terminate;

		entryfound.entry.DIR_FstClus = FATcluster;

		/*
		 * We might not be in the root directory, we need to create the special entries
		 * "." and ".."
		 */
		dirbuffer = malloc(vol->BytesPerCluster);
		if (!dirbuffer)
			goto terminate;

		memset(dirbuffer, 0, vol->BytesPerCluster);
		dirbuffer[0] = entryfound.entry;
		memset(dirbuffer[0].DIR_Name, ' ', sizeof(dirbuffer[0].DIR_Name));
		dirbuffer[0].DIR_Name[0] = '.';
		dirbuffer[0].DIR_FstClus = entryfound.entry.DIR_FstClus;
		dirbuffer[1] = dirbuffer[0];
		dirbuffer[1].DIR_Name[1] = '.';
		dirbuffer[1].DIR_FstClus = entryfound.container;

		if (diskWriteC(vol, FAT_FSoC(vol, FATcluster), (char *)dirbuffer))
			goto terminate;

		free(dirbuffer);
		dirbuffer = 0;
		FAT_set_EOC_into_table(vol, FATcluster);
		FAT_write_FAT_table(vol);
	}

	/* Finally update the free entry and write it to disk */
	if (diskRead(vol, entryfound.sector, 1, vol->buffer))
		goto terminate;

	buffer = (struct FAT *)vol->buffer;
	buffer[entryfound.offset] = entryfound.entry;
	if (diskWrite(vol, entryfound.sector, 1, vol->buffer))
		goto terminate;

	free(name);
	return 0;

 terminate:
	if (name)
		free(name);
	if (dirbuffer)
		free(dirbuffer);
	return -1;
}

int FAT_create_link(struct FATVolume *vol, const char *linkname, const char *entryname)
{
	size_t pos;
	size_t entrylen, linklen;
	char localname[DIR_NAMELEN];
	uint16_t cluster, FATcluster;
	struct FAT_direntry_search_result containerfound, entryfound;
	char *name = 0;
	struct FAT *buffer = 0;
	char *dirbuffer = 0;

	/* Check parameters */
	if (!vol || !linkname || !entryname)
		return -1;

	linklen = strlen(linkname);
	if ((linklen < 2) || (linklen > PATH_MAX) || (linkname[0] != '\\'))
		return -1;

	entrylen = strlen(entryname);
	if ((entrylen < 2) || (entrylen > PATH_MAX) || (entryname[0] != '\\'))
		return -1;

	/* Find the last folder separator */
	pos = FAT_parse_directory_last(linkname, linklen);
	if ((linklen - pos - 1) > sizeof(entryfound.entry.DIR_Name))
		goto terminate;

	/* Copy the containing folder's name */
	name = malloc(pos + 1);
	if (!name)
		goto terminate;

	strncpy(name, linkname, pos);
	name[pos] = 0;

	if (pos) {
		/* Look for the container */
		if (FAT_search_DIR_entry(vol, name, &containerfound))
			goto terminate;
	}

	/* Build the desired entry name */
	if (FAT_build_DIR_name(linkname + pos + 1, linklen - pos - 1, localname))
		goto terminate;

	cluster = (pos) ? containerfound.entry.DIR_FstClus : vol->PB.RootClus;

	while (!FAT_cluster_isEOC(cluster)) {
		if (0 == FAT_dir_foreach(vol, localname, cluster, &entryfound, FAT_search_loop))
			goto terminate;

		cluster = FAT_get_cluster_from_table(vol, cluster);
	}

	/*
	 * Names are valid, parent path exists, the new name is unique.
	 * Search for an empty directory entry.
	 */
	cluster = (pos) ? containerfound.entry.DIR_FstClus : vol->PB.RootClus;

	while (!FAT_cluster_isEOC(cluster)) {
		if (0 == FAT_dir_foreach(vol, 0, cluster, &entryfound, FAT_search_free_loop))
			break;

		cluster = FAT_get_cluster_from_table(vol, cluster);
	}

	/*
	 * If we did not find an empty entry in a directory different than the root directory,
	 * and we hit the End Of File marker, we need more space so we need to allocate
	 * a new cluster for this directory.
	 */
	if (FAT_cluster_isEOC(cluster)) {
		FATcluster = FAT_get_free_cluster_from_table(vol);
		if (!FATcluster)
			goto terminate;

		if (FAT_set_cluster_into_table(vol, cluster, FATcluster))
			goto terminate;

		if (FAT_set_EOC_into_table(vol, FATcluster)) {
			/*
			 * TODO handle failure, in this case the FAT is corrupted and the
			 * cluster chain points somewhere but not where it should.
			 */
			FAT_set_EOC_into_table(vol, cluster);
			goto terminate;
		}

		if (FAT_write_FAT_table(vol))
			goto terminate;

		entryfound.sector = FAT_FSoC(vol, cluster);
		entryfound.offset = 0;
		entryfound.container = (pos) ? containerfound.entry.DIR_FstClus : vol->PB.RootClus;
		if (FAT_write_FAT_table(vol))
			goto terminate;
	}

	/* Clear entry */
	memset(&entryfound.entry, 0, sizeof(entryfound.entry));
	memcpy(entryfound.entry.DIR_Name, localname, sizeof(entryfound.entry.DIR_Name));
	entryfound.entry.DIR_Attr = ATTR_LINK;
	entryfound.entry.DIR_FileSize = (uint32_t) entrylen;

	if (FAT_update_timestamps(&entryfound.entry, FAT_TS_CREATE))
		goto terminate;

	/* Links need to store the file they points at */
	FATcluster = FAT_get_free_cluster_from_table(vol);
	if (!FATcluster)
		goto terminate;

	entryfound.entry.DIR_FstClus = FATcluster;

	dirbuffer = malloc(vol->BytesPerCluster);
	if (!dirbuffer)
		goto terminate;

	memset(dirbuffer, 0, vol->BytesPerCluster);
	strncpy(dirbuffer, entryname, entrylen);

	if (diskWriteC(vol, FAT_FSoC(vol, FATcluster), dirbuffer))
		goto terminate;

	free(dirbuffer);
	dirbuffer = 0;
	FAT_set_EOC_into_table(vol, FATcluster);
	FAT_write_FAT_table(vol);

	/* Finally update the free entry and write it to disk */
	if (diskRead(vol, entryfound.sector, 1, vol->buffer))
		goto terminate;

	buffer = (struct FAT *)vol->buffer;
	buffer[entryfound.offset] = entryfound.entry;
	if (diskWrite(vol, entryfound.sector, 1, vol->buffer))
		goto terminate;

	free(name);
	return 0;

 terminate:
	if (name)
		free(name);
	if (dirbuffer)
		free(dirbuffer);
	return -1;
}

int FAT_modify_entry_attr(struct FATVolume *vol, const char *entryname, uint8_t attributes)
{
	struct FAT_direntry_search_result result;
	const uint8_t allowed = (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM);

	if (!vol || !entryname)
		return -1;

	/*
	 * Only a sub set of flags can be set, namely READ_ONLY, HIDDEN, SYSTEM.
	 * Other flags cannot be edited directly.
	 */
	if (attributes & ~allowed)
		return -1;

	if (FAT_search_DIR_entry(vol, entryname, &result))
		return -1;

	if (result.entry.DIR_Attr & (ATTR_LINK | ATTR_VOLUME_ID))
		return -1;

	/* clear and set attributes */
	result.entry.DIR_Attr &= ~attributes;
	result.entry.DIR_Attr |= attributes;

	return FAT_write_DIR_entry(vol, &result);
}

int FAT_rename_entry(struct FATVolume *vol, const char *entryname, const char *newname)
{
	struct FAT_direntry_search_result entry, renentry;
	char localname[DIR_NAMELEN];
	uint16_t cluster;
	const uint8_t NOT_ALLOWED = (ATTR_SYSTEM | ATTR_VOLUME_ID);

	if (!vol || !entryname || !newname)
		return -1;

	if (FAT_search_DIR_entry(vol, entryname, &entry))
		return -1;

	if (entry.entry.DIR_Attr & NOT_ALLOWED)
		return -1;

	/* Build the desired new entry name */
	if (FAT_build_DIR_name(newname, strlen(newname), localname))
		return -1;

	/* if container is not null the entry is not in the root directory */
	cluster = entry.container;

	while (!FAT_cluster_isEOC(cluster)) {
		if (0 == FAT_dir_foreach(vol, localname, cluster, &renentry, FAT_search_loop))
			return -1;

		cluster = FAT_get_cluster_from_table(vol, cluster);
	}

	memcpy(entry.entry.DIR_Name, localname, sizeof(entry.entry.DIR_Name));

	return FAT_write_DIR_entry(vol, &entry);
}

int FAT_truncate_entry(struct FATVolume *vol, const char *entryname)
{
	struct FAT_direntry_search_result entry;
	const uint8_t NOT_ALLOWED =
	    (ATTR_READ_ONLY | ATTR_SYSTEM | ATTR_DIRECTORY | ATTR_VOLUME_ID);

	if (!vol || !entryname)
		return -1;

	if (FAT_search_DIR_entry(vol, entryname, &entry))
		return -1;

	if (entry.entry.DIR_Attr & NOT_ALLOWED)
		return -1;

	if (FAT_clear_cluster_chain(vol, &entry.entry))
		return -1;

	entry.entry.DIR_FstClus = DIR_ENTRY_LAST_FREE;
	entry.entry.DIR_FileSize = 0;

	return (FAT_write_DIR_entry(vol, &entry) || FAT_write_FAT_table(vol));
}

int FAT_delete_entry(struct FATVolume *vol, const char *entryname)
{
	struct FAT_direntry_search_result entry;
	const uint8_t NOT_ALLOWED = (ATTR_READ_ONLY | ATTR_SYSTEM | ATTR_VOLUME_ID);

	if (!vol || !entryname)
		return -1;

	if (FAT_search_DIR_entry(vol, entryname, &entry))
		return -1;

	if (entry.entry.DIR_Attr & NOT_ALLOWED)
		return -1;

	if (entry.entry.DIR_Attr & ATTR_DIRECTORY) {
		if (FAT_directory_is_void(vol, &entry.entry))
			return -1;
	}

	entry.entry.DIR_Name[0] = (char)DIR_ENTRY_FREE;

	return (FAT_write_DIR_entry(vol, &entry) ||
		FAT_clear_cluster_chain(vol, &entry.entry) || FAT_write_FAT_table(vol));
}

static void FAT_print_statistics(struct FAT_statistics *stats)
{
	printf("\nFiles ......... : %u"
	       "\nDirectories ... : %u"
	       "\nLinks ......... : %u"
	       "\nAvailable ..... : %u"
	       "\nBytes total ... : %u"
	       "\n",
	       stats->totfiles, stats->totdirs, stats->totlinks, stats->totavail, stats->totbytes);
}

int FAT_list(struct FATVolume *vol, const char *directory)
{
	struct FAT_direntry_search_result search;
	uint16_t cluster;
	struct FAT_statistics stats = { 0, 0, 0, 0, 0 };

	if (!vol || !directory)
		return -1;

	if ((directory[0] == '\\') && (directory[1] == '\0')) {
		/* Search the root directory */
		FAT_list_cluster(vol, vol->PB.RootClus, &stats);
		FAT_print_statistics(&stats);
		return 0;
	}

	if (FAT_search_DIR_entry(vol, directory, &search)) {
		return -1;
	}

	if ((search.entry.DIR_Attr & ATTR_DIRECTORY) == 0) {
		FAT_print_entry(&search.entry);
		printf("\t\t1 Files\n");
		printf("\t\t0 Directory\n");
		return 0;
	}

	cluster = search.entry.DIR_FstClus;

	while (!FAT_cluster_isEOC(cluster)) {
		FAT_list_cluster(vol, cluster, &stats);
		cluster = FAT_get_cluster_from_table(vol, cluster);
	}

	FAT_print_statistics(&stats);
	return 0;
}

int FAT_read(struct FATVolume *vol,
	     const char *entryname,
	     unsigned offset, char *buffer, unsigned buflen, unsigned *readlen)
{
	struct FAT_direntry_search_result search;
	uint32_t sector;
	uint16_t cluster = 0;

	if (!vol || !entryname || !buffer || !buflen || !readlen)
		return -1;

	*readlen = 0;
	if (FAT_search_DIR_entry(vol, entryname, &search)) {
		/* ENOENT */
		return -1;
	}

	if (search.entry.DIR_Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) {
		/* EISDIR */
		return -1;
	}

	if (search.entry.DIR_FileSize <= offset) {
		/* EOF */
		return 0;
	}

	/* Trim buffer size */
	if (search.entry.DIR_FileSize - offset < buflen)
		buflen = search.entry.DIR_FileSize - offset;

	/*
	 *
	 *                          File
	 *      +----------+----------+----------+---------+
	 *      |          |          |          |         |
	 * .... |  cluster |  cluster |  cluster |  cluster| ....
	 *      |          |          |          |         |
	 *      +----------+----------+----------+---------+
	 *           ^                             ^
	 *           |                             |
	 *        Offset ---------------------- buflen
	 *
	 * Given a random offset the buffer to be copied can span clusters
	 * and may require partial copies if the offset is not aligned to the cluster size or
	 * the buffer length as well.
	 */

	cluster = FAT_lookup_FAT_table(vol, &search.entry, offset);
	if (!FAT_cluster_isReadable(cluster)) {
		/* EIO or EOF */
		return -1;
	}
	sector = FAT_FSoC(vol, cluster);

	if (FAT_update_timestamps(&search.entry, FAT_TS_READ) || FAT_write_DIR_entry(vol, &search)) {
		/* EIO */
		return -1;
	}

	if (offset % vol->BytesPerCluster) {
		if (diskReadC(vol, sector, vol->buffer))
			return -1;
		/* EIO */

		/* prune the offset to be relative to the starting sector */
		offset %= vol->BytesPerCluster;
		memcpy(buffer, vol->buffer + offset, MIN(buflen, vol->BytesPerCluster - offset));
		*readlen += MIN(buflen, vol->BytesPerCluster - offset);
		buflen -= MIN(buflen, vol->BytesPerCluster - offset);
		if (!buflen)
			return 0;

		buffer += MIN(buflen, vol->BytesPerCluster - offset);
		cluster = FAT_get_cluster_from_table(vol, cluster);
		sector = FAT_FSoC(vol, cluster);
	}

	while (buflen > vol->BytesPerCluster) {
		if (!FAT_cluster_isReadable(cluster)) {
			/* EIO or EOK */
			if (FAT_cluster_isEOC(cluster))
				return 0;

			return -1;
		}

		if (diskReadC(vol, sector, vol->buffer))
			return -1;
		/* EIO */

		*readlen += vol->BytesPerCluster;
		buffer += vol->BytesPerCluster;
		buflen -= vol->BytesPerCluster;
		cluster = FAT_get_cluster_from_table(vol, cluster);
		sector = FAT_FSoC(vol, cluster);
	}

	if (buflen) {

		/*
		 * We are left with a final read of one cluster
		 */
		if (!FAT_cluster_isReadable(cluster)) {
			/* EIO or EOK */
			if (FAT_cluster_isEOC(cluster))
				return 0;

			return -1;
		}

		if (diskReadC(vol, sector, vol->buffer))
			return -1;
		/* EIO */

		memcpy(buffer, vol->buffer, buflen);
		*readlen += buflen;
	}

	return 0;
}

/*
 * static functions in use by FAT_write()
 */
static void
FAT_write_update_on_disk(int appended,
			 uint32_t filepos,
			 struct FATVolume *vol, struct FAT_direntry_search_result *direntry)
{
	if (filepos > direntry->entry.DIR_FileSize)
		direntry->entry.DIR_FileSize = filepos;
	if (appended)
		FAT_write_FAT_table(vol);
	FAT_update_timestamps(&direntry->entry, FAT_TS_WRITE);
	FAT_write_DIR_entry(vol, direntry);
}

static int
FAT_write_get_first(unsigned filepos,
		    struct FATVolume *vol, struct FAT *entry, uint16_t * newcluster)
{
	uint16_t cluster = FAT_lookup_FAT_table(vol, entry, filepos);

	/*
	 * clusters 0 and 1 are reserved; cluster 0 is stored in FAT entries for void files.
	 * EOC means the offset requires a new free cluster to be appended.
	 */
	if (FAT_cluster_isFREE(cluster)) {
		return (FAT_append_cluster_chain(vol, 0, newcluster)) ? -1 : 1;
	}
	if (FAT_cluster_isEOC(cluster)) {
		return (FAT_append_cluster_chain(vol, entry->DIR_FstClus, newcluster)) ? -1 : 1;
	}
	if (FAT_cluster_isBAD(cluster)) {
		return -1;
	}

	*newcluster = cluster;
	return 0;
}

static int FAT_write_get_next(uint16_t cluster, struct FATVolume *vol, uint16_t * newcluster)
{
	uint16_t FATContent = FAT_get_cluster_from_table(vol, cluster);

	/* This is the last cluster in the chain */
	if (FAT_cluster_isEOC(FATContent)) {
		return (FAT_append_cluster_chain(vol, cluster, newcluster)) ? -1 : 1;
	} else if (FAT_cluster_isBAD(FATContent) || FAT_cluster_isFREE(FATContent)) {
		return -1;
	}

	*newcluster = FATContent;
	return 0;
}

int FAT_write(struct FATVolume *vol,
	      const char *entryname,
	      unsigned offset, char *buffer, unsigned buflen, unsigned *writelen)
{
	struct FAT_direntry_search_result search;
	uint32_t sector;
	uint16_t cluster = 0;
	unsigned origbuflen = buflen;
	int appended = 0;
	uint32_t filepos;

	if (!vol || !entryname || !buffer || !buflen || !writelen)
		return -1;

	*writelen = 0;

	if (FAT_search_DIR_entry(vol, entryname, &search)) {
		/* ENOENT */
		return -1;
	}

	if (search.entry.DIR_Attr & (ATTR_DIRECTORY | ATTR_READ_ONLY | ATTR_LINK | ATTR_VOLUME_ID)) {
		/* EISDIR */
		return -1;
	}

	/*
	 * The check is slightly different here than in FAT_read, the reason is, it is not possible
	 * to read PAST the last byte, but it MUST BE POSSIBLE to write PAST the last byte (off by one!)
	 * to append data.
	 */
	if (search.entry.DIR_FileSize < offset) {
		/* EOF */
		return -1;
	}
	filepos = offset;

	/*
	 *
	 *                          File
	 *      +----------+----------+----------+---------+
	 *      |          |          |          |         |
	 * .... |  cluster |  cluster |  cluster |  cluster| ....
	 *      |          |          |          |         |
	 *      +----------+----------+----------+---------+
	 *           ^                             ^
	 *           |                             |
	 *        Offset ---------------------- buflen
	 *
	 * Given a random offset the buffer to be written can span clusters
	 * and may require partial copies if the offset is not aligned to the cluster size or
	 * the buffer length as well.
	 */

	appended = FAT_write_get_first(filepos, vol, &search.entry, &cluster);
	if (appended < 0) {
		/* EIO or EOF */
		return -1;
	} else if ((appended > 0) && (filepos < vol->BytesPerCluster)) {
		search.entry.DIR_FstClus = cluster;
	}

	sector = FAT_FSoC(vol, cluster);

	if (offset % vol->BytesPerCluster) {
		if (diskReadC(vol, sector, vol->buffer)) {
			/* EIO ? */
			return -1;
		}

		/* prune the offset to be relative to the starting sector */
		offset %= vol->BytesPerCluster;
		memcpy(vol->buffer + offset, buffer, MIN(buflen, vol->BytesPerCluster - offset));
		if (diskWriteC(vol, sector, vol->buffer)) {
			/* EIO ? */
			return -1;
		}
		buffer += MIN(buflen, vol->BytesPerCluster - offset);
		filepos += MIN(buflen, vol->BytesPerCluster - offset);
		buflen -= MIN(buflen, vol->BytesPerCluster - offset);
		*writelen = origbuflen - buflen;
		if (!buflen) {
			FAT_write_update_on_disk(appended, filepos, vol, &search);
			return 0;
		}
		appended = FAT_write_get_next(cluster, vol, &cluster);
		if (appended < 0) {
			FAT_write_update_on_disk(appended, filepos, vol, &search);
			/* EIO */
			return -1;
		}
		sector = FAT_FSoC(vol, cluster);
	}

	while (buflen > vol->BytesPerCluster) {
		if (diskWriteC(vol, sector, buffer)) {
			/* EIO */
			FAT_write_update_on_disk(appended, filepos, vol, &search);
			return -1;
		}

		filepos += vol->BytesPerCluster;
		buffer += vol->BytesPerCluster;
		buflen -= vol->BytesPerCluster;
		*writelen = origbuflen - buflen;
		if (!buflen) {
			FAT_write_update_on_disk(appended, filepos, vol, &search);
			return 0;
		}

		appended = FAT_write_get_next(cluster, vol, &cluster);
		if (appended < 0) {
			FAT_write_update_on_disk(appended, filepos, vol, &search);
			/* EIO */
			return -1;
		}
		sector = FAT_FSoC(vol, cluster);
	}

	if (buflen) {
		/*
		 * We are left with a final write of one cluster
		 */
		if (diskReadC(vol, sector, vol->buffer)) {
			/* EIO */
			FAT_write_update_on_disk(appended, filepos, vol, &search);
			return -1;
		}
		memcpy(vol->buffer, buffer, buflen);
		if (appended)
			memset(vol->buffer + buflen, 0, vol->BytesPerCluster - buflen);
		if (diskWriteC(vol, sector, vol->buffer)) {
			/* EIO */
			FAT_write_update_on_disk(appended, filepos, vol, &search);
			return -1;
		}
		filepos += buflen;
	}

	*writelen = origbuflen;
	FAT_write_update_on_disk(appended, filepos, vol, &search);
	return 0;
}

int FAT_available(struct FATVolume *vol, uint32_t * bytes, uint16_t * clusters)
{
	unsigned i;
	uint16_t freeclusters = 0;

	if (!vol || !vol->FAT || !(bytes || clusters))
		return -1;

	for (i = 2; i < vol->FATSize; i++)
		if (FAT_cluster_isFREE(vol->FAT[i]))
			freeclusters++;

	if (clusters)
		*clusters = freeclusters;
	if (bytes)
		*bytes = freeclusters * vol->BytesPerCluster;

	return 0;
}
