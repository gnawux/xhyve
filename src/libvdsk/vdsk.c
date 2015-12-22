/*-
 * Copyright (c) 2014 Marcel Moolenaar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>

#include <sys/disk.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xhyve/libvdsk/vdsk.h>

#include "vdsk_int.h"

static inline int
vdsk_is_dev(struct vdsk *vdsk)
{

	return ((S_ISCHR(vdsk->fsbuf.st_mode)) ? 1 : 0);
}

static struct vdsk *
vdsk_deref(vdskctx ctx)
{
	struct vdsk *vdsk = ctx;

	return (vdsk - 1);
}

static struct vdsk_format *
vdsk_probe(struct vdsk *vdsk)
{
	struct vdsk_format **fmts;
	struct vdsk_format *f, *fmt;
	size_t idx, nfmts;
	int error, probe;

	/*
	 * Create a mutable copy of the linker set.
	 */
	nfmts = (size_t)SET_COUNT(vdsk_formats);
	fmts = malloc(nfmts * sizeof(*fmts));
	if (fmts == NULL)
		return (NULL);
	memcpy(fmts, SET_BEGIN(vdsk_formats), nfmts * sizeof(*fmts));

	fmt = NULL;
	probe = VDSKFMT_HAS_HEADER | VDSKFMT_HAS_FOOTER;
	probe |= (vdsk_is_dev(vdsk)) ? VDSKFMT_DEVICE_OK : 0;
	probe |= (vdsk->fflags & FWRITE) ? VDSKFMT_CAN_WRITE : 0;
	while (fmt == NULL && probe >= 0) {
		for (idx = 0; idx < nfmts; idx++) {
			f = fmts[idx];
			/* Skip formats we've probed already. */
			if (f == NULL)
				continue;
			/* Skip formats we shouldn't probe now. */
			if ((f->flags & probe) != probe)
				continue;
			/* White-out this format and probe it. */
			fmts[idx] = NULL;
			error = f->probe(vdsk);
			if (!error) {
				/* We have a match. */
				fmt = f;
				break;
			}
		}
		if (fmt == NULL)
			probe -= VDSKFMT_HAS_FOOTER;
	}
	free(fmts);
	if (fmt == NULL)
		errno = EFTYPE;
	return (fmt);
}

vdskctx
vdsk_open(const char *path, int flags, size_t size)
{
	vdskctx ctx;
	struct vdsk *vdsk;
	int lck;

	ctx = NULL;

	do {
		size += sizeof(struct vdsk);
		vdsk = calloc(1, size);
		if (vdsk == NULL)
			break;

		vdsk->fflags = flags + 1;
		if ((vdsk->fflags & ~(O_ACCMODE | O_SYNC)) != 0) {
			errno = EINVAL;
			break;
		}

		vdsk->filename = realpath(path, NULL);
		if (vdsk->filename == NULL)
			break;

		flags = (flags & O_ACCMODE) | O_CLOEXEC;
		vdsk->fd = open(vdsk->filename, flags);
		if (vdsk->fd == -1)
			break;

		if (fstat(vdsk->fd, &vdsk->fsbuf) == -1)
			break;

		if (vdsk_is_dev(vdsk)) {
			perror("xhyve: raw device support unimplemented");
			break;
		} else {
			vdsk->capacity = vdsk->fsbuf.st_size;
			vdsk->sectorsize = DEV_BSIZE;
		}

		vdsk->fmt = vdsk_probe(vdsk);
		if (vdsk->fmt == NULL)
			break;

		lck = (vdsk->fflags & FWRITE) ? LOCK_EX : LOCK_SH;
		if (flock(vdsk->fd, lck | LOCK_NB) == -1)
			break;

		errno = vdsk->fmt->open(vdsk);
		if (errno != 0) {
			flock(vdsk->fd, LOCK_UN);
			break;
		}

		/* Complete... */
		ctx = vdsk + 1;
	} while (0);

	if (ctx == NULL) {
		if (vdsk != NULL) {
			if (vdsk->fd != -1)
				close(vdsk->fd);
			if (vdsk->filename != NULL)
				free(vdsk->filename);
			free(vdsk);
		}
	}

	return (ctx);
}

int
vdsk_close(vdskctx ctx)
{
	struct vdsk *vdsk = vdsk_deref(ctx);

	vdsk->fmt->close(vdsk);
	flock(vdsk->fd, LOCK_UN);
	close(vdsk->fd);
	free(vdsk->filename);
	free(vdsk);
	return (0);
}

off_t
vdsk_capacity(vdskctx ctx)
{
	struct vdsk *vdsk = vdsk_deref(ctx);

	return (vdsk->capacity);
}

int
vdsk_sectorsize(vdskctx ctx)
{
	struct vdsk *vdsk = vdsk_deref(ctx);

	return (vdsk->sectorsize);
}

int
vdsk_read(vdskctx ctx, off_t offset, const struct iovec *iov, int iovcnt)
{
	struct vdsk *vdsk = vdsk_deref(ctx);

	return (vdsk->fmt->read(vdsk, offset, iov, iovcnt));
}

int
vdsk_write(vdskctx ctx, off_t offset, const struct iovec *iov, int iovcnt)
{
	struct vdsk *vdsk = vdsk_deref(ctx);

	if ((vdsk->fflags & FWRITE) == 0)
		return (EROFS);
	return (vdsk->fmt->write(vdsk, offset, iov, iovcnt));
}

int
vdsk_trim(vdskctx ctx, off_t offset, ssize_t length)
{
	struct vdsk *vdsk = vdsk_deref(ctx);

	if ((vdsk->fflags & FWRITE) == 0)
		return (EROFS);
	return (vdsk->fmt->trim(vdsk, offset, length));
}

int
vdsk_flush(vdskctx ctx)
{
	struct vdsk *vdsk = vdsk_deref(ctx);

	if ((vdsk->fflags & FWRITE) == 0)
		return (0);
	return (vdsk->fmt->flush(vdsk));
}

