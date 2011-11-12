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
 * bad calls to execv()
 */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include "config.h"
#include "test.h"

static
int
exec_common_fork(void)
{
	int pid, rv, status;

	pid = fork();
	if (pid<0) {
		warn("UH-OH: fork failed");
		return -1;
	}
	
	if (pid==0) {
		/* child */
		return 0;
	}

	rv = waitpid(pid, &status, 0);
	if (rv == -1) {
		warn("UH-OH: waitpid failed");
		return -1;
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status) != MAGIC_STATUS) {
		warnx("FAILURE: wrong exit code of subprocess");
	}
	return 1;
}

static
void
exec_badprog(const void *prog, const char *desc)
{
	int rv;
	char *args[2];
	args[0] = (char *)"foo";
	args[1] = NULL;

	if (exec_common_fork() != 0) {
		return;
	}

	rv = execv(prog, args);
	report_test(rv, errno, EFAULT, desc);
	exit(MAGIC_STATUS);
}

static
void
exec_emptyprog(void)
{
	int rv;
	char *args[2];
	args[0] = (char *)"foo";
	args[1] = NULL;

	if (exec_common_fork() != 0) {
		return;
	}

	rv = execv("", args);
	report_test2(rv, errno, EINVAL, EISDIR, "exec the empty string");
	exit(MAGIC_STATUS);
}

static
void
exec_badargs(void *args, const char *desc)
{
	int rv;

	if (exec_common_fork() != 0) {
		return;
	}

	rv = execv("/bin/true", args);
	report_test(rv, errno, EFAULT, desc);
	exit(MAGIC_STATUS);
}

static
void
exec_onearg(void *ptr, const char *desc)
{
	int rv;

	char *args[3];
	args[0] = (char *)"foo";
	args[1] = (char *)ptr;
	args[2] = NULL;

	if (exec_common_fork() != 0) {
		return;
	}

	rv = execv("/bin/true", args);
	report_test(rv, errno, EFAULT, desc);
	exit(MAGIC_STATUS);
}

void
test_execv(void)
{
	exec_badprog(NULL, "exec NULL");
	exec_badprog(INVAL_PTR, "exec invalid pointer");
	exec_badprog(KERN_PTR, "exec kernel pointer");

	exec_emptyprog();

	exec_badargs(NULL, "exec /bin/true with NULL arglist");
	exec_badargs(INVAL_PTR, "exec /bin/true with invalid pointer arglist");
	exec_badargs(KERN_PTR, "exec /bin/true with kernel pointer arglist");

	exec_onearg(INVAL_PTR, "exec /bin/true with invalid pointer arg");
	exec_onearg(KERN_PTR, "exec /bin/true with kernel pointer arg");
}
