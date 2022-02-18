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

#ifndef FAT_H
#define FAT_H

#include "fat_data.h"

/**
 * @brief FAT_mount mounts a FAT file system. The storage media is accessed through disk_access
 * API calls. The opaque context ctx is provided from the disk_access API.
 * The first sector on disk instructs the FS mount code where to start reading data. On success
 * the FATVolume structure is filled with data.
 * It is a responsibility of the caller to get a context from the disk_access API before
 * mounting a FAT volume.
 *
 * @param ctx a context provided by disk_access to read from and write to the storage media
 * @param firstSecOnDisk the first sector on storage with FAT data
 * @param vol output structure filled with volume informations
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_mount(void *ctx, uint32_t first_sec_on_disk, struct FATVolume *vol);

/**
 * @brief FAT_unmount unmounts a FAT file system. Flags and data in vol are cleared accordingly.
 *
 * @param vol volume structure
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_unmount(struct FATVolume *vol);

/**
 * @brief FAT_get_entry looks for an entry in the file system and copy it to the caller
 * by filling entry.
 * entryname is a standard file name path, it can point to either a file or a directory.
 *
 * @param vol volume structure
 * @param entryname standard path name string
 * @param entry FAT entry return value
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_get_entry(struct FATVolume *vol, const char *entryname, struct FAT *entry);

/**
 * @brief FAT_create_entry creates a new entry in the file system, provided that it is not
 * already present.
 * The attributes parameter values are a responsibility of the caller, in other words, the caller
 * must know what he is doing with attributes.
 * ATTR_READ_ONLY, ATTR_HIDDEN and ATTR_SYSTEM can be attributes of files.
 * ATTR_VOLUME_ID identifies a special file used to store the volume name, it can exists only
 * in the root directory and cannot be combined with other flags.
 * ATTR_DIRECTORY identifies directories rather than files.
 *
 * @param vol volume structure
 * @param entryname standard path name string
 * @param file attributes ATTR_*
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_create_entry(struct FATVolume *vol, const char *entryname, uint8_t attributes);

/**
 * @brief FAT_create_link creates a new entry in the file system, provided that it is not
 * already present.
 * The attribute is set to ATTR_LINK and the link is set to point to entryname.
 * linkname and entryname must be full path names.
 *
 * @param vol volume structure
 * @param linkname standard path name string
 * @param entryname standard path name string
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_create_link(struct FATVolume *vol, const char *linkname, const char *entryname);

/**
 * @brief FAT_modify_entry_attr updates attributes of entryname with attributes.
 * The attributes parameter values are a responsibility of the caller, in other words, the caller
 * must know what he is doing with attributes.
 * Only the following attributes can be set:
 *
 * ATTR_READ_ONLY  0x01
 * ATTR_HIDDEN     0x02
 * ATTR_SYSTEM     0x04
 *
 * Links and volume ID are not editable.
 *
 * @param vol volume structure
 * @param entryname standard path name string
 * @param attributes ATTR_*
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_modify_entry_attr(struct FATVolume *vol, const char *entryname, uint8_t attributes);

/**
 * @brief FAT_rename_entry rename entryname with newname.
 * entryname must be a fully qualified path to a FAT entry.
 * newname must be a valid FAT name.
 * Attributes are not changed.
 * Timestamps are updated accordingly.
 * Any entry (except for the volume name) can be renamed.
 *
 * @param vol volume structure
 * @param entryname standard path name string
 * @param newname new name for this entry
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_rename_entry(struct FATVolume *vol, const char *entryname, const char *newname);

/**
 * @brief FAT_truncate_entry truncates a file, i.e. set its size to 0 and releases the
 * clusters to the file system.
 * Can be applied only to files, not read-only and not hidden.
 *
 * @param vol volume structure
 * @param entryname standard path name string
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_truncate_entry(struct FATVolume *vol, const char *entryname);

/**
 * @brief FAT_delete_entry deletes an FAT entry.
 * If the entry is a directory the same must be void to be deleted,
 * in other words all files in the directory must have been previously
 * deleted.
 * If the entry is read-only or hidden the entry cannot be deleted.
 * If the entry is a link the link will not be followed, i.e. the link itself will
 * be deleted but not the entry it points at.
 *
 * @param vol volume structure
 * @param entryname standard path name string
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_delete_entry(struct FATVolume *vol, const char *entryname);

/**
 * @brief FAT_read reads up to buflen bytes from the file entryname into buffer, starting at offset
 * bytes from the file beginning.
 * The return code gives the status of the operation, the number of bytes copied into buffer is
 * stored in readlen.
 * The number of bytes stored in buffer might be less than buflen and there are several reasons for
 * this: the file is shorter than buflen bytes, the remaining bytes from offset are less than
 * buflen, an error was encountered.
 *
 * @param vol volume structure
 * @param entryname standard path name string
 * @param offset offset in bytes from the beginning if the file
 * @param buffer pointer to a memory buffer
 * @param buflen size of the memory area pointed by buffer
 * @param readlen the actual number of bytes copied into buffer
 *
 * @return 0 on success
 * @return -ERR on failure
 */
int FAT_read(struct FATVolume *vol,
	     const char *entryname,
	     unsigned offset, char *buffer, unsigned buflen, unsigned *readlen);

/**
 * @brief FAT_write writes up to buflen bytes from buffer to the file entryname, starting at offset
 * bytes from the file beginning.
 * The return code gives the status of the operation, the number of bytes copied into the file is
 * stored in writelen.
 * The number of bytes stored in the file might be less than buflen and there are several reasons for
 * this: the file is shorter than buflen bytes, the remaining bytes from offset are less than
 * buflen, an error was encountered.
 *
 * @param vol volume structure
 * @param entryname standard path name string
 * @param offset offset in bytes from the beginning if the file
 * @param buffer pointer to a memory buffer
 * @param buflen size of the memory area pointed by buffer
 * @param writelen the actual number of bytes copied into the file
 *
 * @return
 */
int FAT_write(struct FATVolume *vol,
	      const char *entryname,
	      unsigned offset, char *buffer, unsigned buflen, unsigned *writelen);

/**
 * @brief FAT_list prints in a nice textual fashion the contents of a directory.
 *
 * @param vol volume structure
 * @param directory standard path name string
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_list(struct FATVolume *vol, const char *directory);

/**
 * @brief FAT_available computes the remaining clusters and bytes in the user data area.
 *
 * @param vol volume structure
 * @param bytes returned available bytes
 * @param clusters returned available clusters
 *
 * @return 0 on success
 * @return -1 on error
 */
int FAT_available(struct FATVolume *vol, uint32_t * bytes, uint16_t * clusters);

#endif				// FAT_H
