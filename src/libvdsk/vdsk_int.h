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
 *
 * $FreeBSD$
 */

#ifndef __VDSK_INT_H__
#define	__VDSK_INT_H__

#include <assert.h>
#include <xhyve/support/linker_set.h>

struct vdsk;

/*
 * The disk format registration structure.
 */
struct vdsk_format {
	const char	*name;
	const char	*description;
	int	flags;
	char __padding[4];
#define	VDSKFMT_DEVICE_OK	1
#define	VDSKFMT_CAN_WRITE	2
#define	VDSKFMT_NO_METADATA	0
#define	VDSKFMT_HAS_FOOTER	4
#define	VDSKFMT_HAS_HEADER	8
	int	(*probe)(struct vdsk *);
	int	(*open)(struct vdsk *);
	int	(*close)(struct vdsk *);
	int	(*read)(struct vdsk *, off_t, const struct iovec *, int);
	int	(*write)(struct vdsk *, off_t, const struct iovec *, int);
	int	(*trim)(struct vdsk *, off_t, ssize_t);
	int	(*flush)(struct vdsk *);
};

SET_DECLARE(vdsk_formats, struct vdsk_format);
#define	FORMAT_DEFINE(nm)	DATA_SET(vdsk_formats, nm)

/*
 * The internal representation of a "disk".
 */
struct vdsk {
	struct vdsk_format *fmt;
	int	fd;
	int	fflags;
	char	*filename;
	struct stat fsbuf;
	off_t	capacity;
	int	sectorsize;
	char __padding[12];
} __attribute__((aligned(16)));


inline static ssize_t
preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	off_t res;

	res = lseek(fd, offset, SEEK_SET);
	assert(res == offset);
	return readv(fd, iov, iovcnt);
}

inline static ssize_t
pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	off_t res;

	res = lseek(fd, offset, SEEK_SET);
	assert(res == offset);
	return writev(fd, iov, iovcnt);
}

#endif /* __VDSK_INT_H__ */
