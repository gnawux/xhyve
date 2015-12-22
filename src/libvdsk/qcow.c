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
#include <xhyve/support/endian.h>
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

/* Flag bits in cluster offsets */
//#define	QCOW_CLSTR_COMPRESSED	(1ULL << 62)
//#define	QCOW_CLSTR_COPIED	(1ULL << 63)

struct qcow_header {
	uint32_t	magic;
#define	QCOW_MAGIC		0x514649fb
	uint32_t	version;
//#define	QCOW_VERSION_1		1
//#define	QCOW_VERSION_2		2
	uint64_t	path_offset;
	uint32_t	path_length;
	uint32_t	clstr_log2sz;	/* v2 only */
	uint64_t	disk_size;
	union {
		struct {
			uint8_t		clstr_log2sz;
			uint8_t		l2_log2sz;
			uint16_t	_pad;
			uint32_t	encryption;
			uint64_t	l1_offset;
		} v1;
		struct {
			uint32_t	encryption;
			uint32_t	l1_entries;
			uint64_t	l1_offset;
			uint64_t	refcnt_offset;
			uint32_t	refcnt_entries;
			uint32_t	snapshot_count;
			uint64_t	snapshot_offset;
		} v2;
	} u;
};

static int
qcow_probe(struct vdsk *vdsk)
{
	struct qcow_header *hdr;

	if (vdsk->sectorsize < 512 || vdsk->sectorsize > 4096)
		return (ENOTBLK);

	hdr = malloc((unsigned long)(vdsk->sectorsize));
	if (hdr == NULL)
		return (errno);

	if (read(vdsk->fd, hdr, (size_t)(vdsk->sectorsize)) != vdsk->sectorsize)
		goto out;

	if (be32dec(&hdr->magic) != QCOW_MAGIC) {
		errno = ENXIO;
		goto out;
	}

	errno = 0;

 out:
	free(hdr);
	return (errno);
}

static int
qcow_open(struct vdsk *vdsk __unused)
{

	return (ENOSYS);
}

static int
qcow_close(struct vdsk *vdsk __unused)
{

	return (ENOSYS);
}

static int
qcow_read(struct vdsk *vdsk __unused, off_t offset __unused,
    const struct iovec *iov __unused, int iovcnt __unused)
{

	return (ENOSYS);
}

static int
qcow_write(struct vdsk *vdsk __unused, off_t offset __unused,
    const struct iovec *iov __unused, int iovcnt __unused)
{

	return (ENOSYS);
}

static int
qcow_trim(struct vdsk *vdsk __unused, off_t offset __unused,
    ssize_t length __unused)
{

	return (ENOSYS);
}

static int
qcow_flush(struct vdsk *vdsk __unused)
{

	return (ENOSYS);
}

static struct vdsk_format qcow_format = {
	.name = "qcow",
	.description = "QEMU Copy-On-Write, version 1",
	.flags = VDSKFMT_CAN_WRITE | VDSKFMT_HAS_HEADER,
	.probe = qcow_probe,
	.open = qcow_open,
	.close = qcow_close,
	.read = qcow_read,
	.write = qcow_write,
	.trim = qcow_trim,
	.flush = qcow_flush,
};
FORMAT_DEFINE(qcow_format);

