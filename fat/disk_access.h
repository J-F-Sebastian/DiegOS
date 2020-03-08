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

#ifndef DISK_ACCESS_H
#define DISK_ACCESS_H

/**
 *  This API is the only "virtual" code of the FAT file system driver.
 *  It gives access to storage media, and exports functions to read and write sectors.
 *  While the interface must be common across platforms, the implementation need to be
 *  customized for the specific target system it will run on.
 *  The implementation of this API is the only code to be changed to port the driver to
 *  any Operating System or Application Environment.
 *  The description of each API call explains what the implementation is expected to do.
 */

#include <stdint.h>

struct disk_geometry
{
    uint16_t bytes_per_sector;
    uint32_t num_of_sectors;
};

/**
 * @brief disk_init setup access to the storage media as specified by dev, and returns
 * a context in ctx.
 * The context is used by other API calls to access the proper physical media.
 * Contents of the string pointed to dev and contents of the context as well its memory allocation
 * are platform and implementation dependent, or in other words, are up to the implementation.
 * Any implementation can use a different syntax for both, provided that all API calls use the same
 * syntax.
 * The context returned by disk_init() will be used when calling any other API in this interface,
 * so it must be internally supported.
 *
 * @param dev a C string specifing which device (disk, CD, mmc, NAND flash, NOR flash...)
 *            need to be initialized and set up for further read&write.
 * @param ctx A context storing informations to be used by all other API calls to address the
 *            proper physical device.
 *            The API store an opaque pointer in *ctx, the caller must not be aware of its contents.
 *
 * @return 0 in case of success
 * @return -1 in case of error
 */
int disk_init (const char *dev, void **ctx);

/**
 * @brief disk_done terminate access to the storage media addressed by ctx; the API must release
 * the context memory if previously allocated, and must invalidate the context, or in other words,
 * after this call the context cannot be used anymore.
 * A call to disk_init() is required to get a new valid context.
 *
 * @param ctx A valid context as returned by disk_init()
 *
 * @return 0 in case of success
 * @return -1 in case of error
 */
int disk_done (void *ctx);

/**
 * @brief disk_get_geometry returns a basic media geometry description in geom; the returned
 * values are the number of bytes per sector and the total number of accessible sectors.
 *
 * @param ctx A context returned by disk_init()
 * @param geom A pointer to a struct disk_geometry structure
 *
 * @return 0 in case of success
 * @return -1 in case of error
 */
int disk_get_geometry (void *ctx, struct disk_geometry *geom);

/**
 * @brief disk_read read multiple sectors from the device addressed by ctx and write the data
 * into buf. Sectors are read as whole units, no partial reads are allowed.
 * Trying to read 0 sectors or starting at an offset beyond the device storage capacity will
 * return an error.
 * The destination buffer is managed by the caller and must sized properly.
 *
 * @param ctx A context returned by disk_init()
 * @param sec Starting sector, its contents will be read into buf
 * @param numsec Number of sectors to be read, must be greater than 0
 * @param buf Destination buffer
 *
 * @return 0 in case of success
 * @return -1 in case of error
 */
int disk_read (void *ctx, unsigned sec, unsigned numsec, char *buf);

/**
 * @brief disk_write write multiple sectors to the device addressed by ctx and read the data
 * from buf. Sectors are written as whole units, no partial writes are allowed.
 * Trying to write 0 sectors or starting at an offset beyond the device storage capacity will
 * return an error.
 * The source buffer is managed by the caller and must sized properly.
 *
 * @param ctx A context returned by disk_init()
 * @param sec Starting sector, its contents will be overwritten
 * @param numsec Number of sectors to be written, must be greater than 0
 * @param buf Source buffer
 *
 * @return 0 in case of success
 * @return -1 in case of error
 */
int disk_write (void *ctx, unsigned sec, unsigned numsec, char *buf);

#endif // DISK_ACCESS_H
