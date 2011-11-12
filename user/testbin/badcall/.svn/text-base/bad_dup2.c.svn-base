/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Invalid calls to dup2
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <err.h>

#include "config.h"
#include "test.h"

static
void
dup2_fd2(int fd, const char *desc)
{
	int rv;

	rv = dup2(STDIN_FILENO, fd);
	report_test(rv, errno, EBADF, desc);

	if (rv != -1) {
		close(fd);	/* just in case */
	}
}

static
void
dup2_self(void)
{
	struct stat sb;
	int rv;
	int testfd;

	/* use fd that isn't in use */
	testfd = CLOSED_FD;

	rv = dup2(STDIN_FILENO, testfd);
	if (rv == -1) {
		warn("UH-OH: couldn't copy stdin");
		return;
	}

	rv = dup2(testfd, testfd);
	if (rv == testfd) {
		warnx("passed: dup2 to same fd");
	}
	else if (rv<0) {
		warn("FAILURE: dup2 to same fd: error");
	}
	else {
		warnx("FAILURE: dup2 to same fd: returned %d instead", rv);
	}

	rv = fstat(testfd, &sb);
	if (rv==0) {
		warnx("passed: fstat fd after dup2 to itself");
	}
	else if (errno!=EUNIMP && errno!=ENOSYS) {
		warn("FAILURE: fstat fd after dup2 to itself");
	}
	else {
		/* no support for fstat; try lseek */
		rv = lseek(testfd, 0, SEEK_CUR);
		if (rv==0 || (rv==-1 && errno==ESPIPE)) {
			warnx("passed: lseek fd after dup2 to itself");
		}
		else {
			warn("FAILURE: lseek fd after dup2 to itself");
		}
	}

	close(testfd);
}

void
test_dup2(void)
{
	/* This does the first fd. */
	test_dup2_fd();

	/* Any interesting cases added here should also go in common_fds.c */
	dup2_fd2(-1, "dup2 to -1");
	dup2_fd2(-5, "dup2 to -5");
	dup2_fd2(IMPOSSIBLE_FD, "dup2 to impossible fd");
#ifdef OPEN_MAX
	dup2_fd2(OPEN_MAX, "dup2 to OPEN_MAX");
#else
	warnx("Warning: OPEN_MAX not defined - test skipped");
#endif

	dup2_self();
}
