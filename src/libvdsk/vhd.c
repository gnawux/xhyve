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

static int
vhd_probe(struct vdsk *vdsk __unused)
{

	return (ENOSYS);
}

static int
vhd_open(struct vdsk *vdsk __unused)
{

	return (ENOSYS);
}

static int
vhd_close(struct vdsk *vdsk __unused)
{

	return (ENOSYS);
}

static int
vhd_read(struct vdsk *vdsk __unused, off_t offset __unused,
    const struct iovec *iov __unused, int iovcnt __unused)
{

	return (ENOSYS);
}

static int
vhd_write(struct vdsk *vdsk __unused, off_t offset __unused,
    const struct iovec *iov __unused, int iovcnt __unused)
{

	return (ENOSYS);
}

static int
vhd_trim(struct vdsk *vdsk __unused, off_t offset __unused,
    ssize_t length __unused)
{

	return (ENOSYS);
}

static int
vhd_flush(struct vdsk *vdsk __unused)
{

	return (ENOSYS);
}

static struct vdsk_format vhd_format = {
	.name = "vhd",
	.description = "Virtual Hard Disk",
	.flags = VDSKFMT_CAN_WRITE | VDSKFMT_HAS_HEADER,
	.probe = vhd_probe,
	.open = vhd_open,
	.close = vhd_close,
	.read = vhd_read,
	.write = vhd_write,
	.trim = vhd_trim,
	.flush = vhd_flush,
};
FORMAT_DEFINE(vhd_format);

