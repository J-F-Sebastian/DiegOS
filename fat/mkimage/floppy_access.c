/*
 * FAT File System
 *
 * Copyright (C) 2019 - 2026 Diego Gallizioli
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
#include <errno.h>

#include "../disk_access.h"

struct my_context {
	struct disk_geometry geom;
	FILE *disk;
};

/*
 * Initialize the disk context
 * assuming the target image is for
 * a floppy disk with 512 bytes/sector
 * density 2MB and formatted size 1.44MB.
 * These account for 2880 sectors of 512 bytes each.
 * Geometry is fixed to 2 heads, 80 cylinders, and 18 sectors per track.
 */
int disk_init(const char *dev, void **ctx)
{
	FILE *disk = 0;
	struct disk_geometry geom;
	struct my_context *context;
	unsigned i;

	/* EINVAL */
	if (!dev || !ctx)
		return -EINVAL;

	*ctx = NULL;
	disk = fopen(dev, "wb");
	if (!disk) {
		/* ENODEV */
		return -ENODEV;
	}

	/* ENODEV */
	if (!disk)
		return -ENODEV;

	for (i = 0; i < 2880 * 512 / sizeof(i); i++)
		fwrite(&i, sizeof(i), 1, disk);

	if (fseek(disk, 0, SEEK_END)) {
		fclose(disk);
		/* ENODEV? */
		return -ENODEV;
	}

	geom.bytes_per_sector = 512;
	geom.num_of_sectors = (uint32_t) (ftell(disk) / 512L);
	if (ftell(disk) % 512L) {
		fclose(disk);
		/* EINVAL? */
		return -EINVAL;
	}

	context = malloc(sizeof(*context));

	/* ENOMEM */
	if (!context) {
		fclose(disk);
		return -ENOMEM;
	}

	context->disk = disk;
	context->geom = geom;
	*ctx = context;

	return 0;
}

int disk_done(void *ctx)
{
	struct my_context *mctx = (struct my_context *)ctx;

	/* EINVAL */
	if (!mctx)
		return -EINVAL;

	fflush(mctx->disk);
	fclose(mctx->disk);
	memset(mctx, 0, sizeof(*mctx));
	free(mctx);

	return 0;
}

int disk_get_geometry(void *ctx, struct disk_geometry *geom)
{
	struct my_context *mctx = (struct my_context *)ctx;

	if (!mctx || !geom) {
		/* EINVAL */
		return -EINVAL;
	}

	*geom = mctx->geom;

	return 0;
}

int disk_read(void *ctx, unsigned sec, unsigned numsec, char *buf)
{
	struct my_context *mctx = (struct my_context *)ctx;
	unsigned retval;

	/* EINVAL */
	if (!mctx ||
	    !numsec ||
	    !buf ||
	    (sec > mctx->geom.num_of_sectors) ||
	    (numsec > mctx->geom.num_of_sectors) || ((sec + numsec) > mctx->geom.num_of_sectors)) {
		return -EINVAL;
	}

	if (fseek(mctx->disk, (long)sec * mctx->geom.bytes_per_sector, SEEK_SET))
		return -EINVAL;

	retval = fread(buf, mctx->geom.bytes_per_sector, numsec, mctx->disk);

	return (numsec != retval);
}

int disk_write(void *ctx, unsigned sec, unsigned numsec, const char *buf)
{
	struct my_context *mctx = (struct my_context *)ctx;
	unsigned retval;

	/* EINVAL */
	if (!mctx ||
	    !numsec ||
	    !buf ||
	    (sec > mctx->geom.num_of_sectors) ||
	    (numsec > mctx->geom.num_of_sectors) || ((sec + numsec) > mctx->geom.num_of_sectors)) {
		return -EINVAL;
	}

	if (fseek(mctx->disk, (long)sec * mctx->geom.bytes_per_sector, SEEK_SET))
		return -EINVAL;

	retval = fwrite(buf, mctx->geom.bytes_per_sector, numsec, mctx->disk);
	fflush(mctx->disk);

	return (numsec != retval);
}
