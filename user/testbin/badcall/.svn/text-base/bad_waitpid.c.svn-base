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
 * bad calls to waitpid()
 */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include "config.h"
#include "test.h"

static
void
wait_badpid(int pid, const char *desc)
{
	int rv, x;
	rv = waitpid(pid, &x, 0);
	report_test2(rv, errno, EINVAL, NOSUCHPID_ERROR, desc);
}

static
void
wait_badstatus(void *ptr, const char *desc)
{
	int rv, pid, x;

	pid = fork();
	if (pid<0) {
		warn("UH-OH: fork failed");
		return;
	}
	if (pid==0) {
		exit(0);
	}

	rv = waitpid(pid, ptr, 0);
	report_test(rv, errno, EFAULT, desc);
	waitpid(pid, &x, 0);
}

static
void
wait_unaligned(void)
{
	int rv, pid, x;
	int status[2];	/* will have integer alignment */
	char *ptr;

	pid = fork();
	if (pid<0) {
		warn("UH-OH: fork failed");
		return;
	}
	if (pid==0) {
		exit(0);
	}

	/* start with proper integer alignment */
	ptr = (char *)(&status[0]);

	/* generate improper alignment on platforms with restrictions*/
	ptr++;

	rv = waitpid(pid, (int *)ptr, 0);
	report_survival(rv, errno, "wait with unaligned status");
	if (rv<0) {
		waitpid(pid, &x, 0);
	}
}

static
void
wait_badflags(void)
{
	int rv, x, pid;

	pid = fork();
	if (pid<0) {
		warn("UH-OH: fork failed");
		return;
	}
	if (pid==0) {
		exit(0);
	}

	rv = waitpid(pid, &x, 309429);
	report_test(rv, errno, EINVAL, "wait with bad flags");
	waitpid(pid, &x, 0);
}

static
void
wait_self(void)
{
	int rv, x;
	rv = waitpid(getpid(), &x, 0);
	report_survival(rv, errno, "wait for self");
}

static
void
wait_parent(void)
{
	int mypid, childpid, rv, x;

	mypid = getpid();
	childpid = fork();
	if (childpid<0) {
		warn("UH-OH: can't fork");
		return;
	}
	if (childpid==0) {
		/* Child. Wait for parent. */
		rv = waitpid(mypid, &x, 0);
		report_survival(rv, errno, "wait for parent (from child)");
		_exit(0);
	}
	rv = waitpid(childpid, &x, 0);
	report_survival(rv, errno, "wait for parent test (from parent)");
}

////////////////////////////////////////////////////////////

static
void
wait_siblings_child(void)
{
	int pids[2], mypid, otherpid, fd, rv, x;

	mypid = getpid();

	fd = open(TESTFILE, O_RDONLY);
	if (fd<0) {
		warn("UH-OH: child process (pid %d) can't open %s",
		     mypid, TESTFILE);
		return;
	}

	/*
	 * Busy-wait until the parent writes the pids into the file.
	 * This sucks, but there's not a whole lot else we can do.
	 */
	do {
		rv = lseek(fd, 0, SEEK_SET);
		if (rv<0) {
			warn("UH-OH: child process (pid %d) lseek error",
			     mypid);
			return;
		}
		rv = read(fd, pids, sizeof(pids));
		if (rv<0) {
			warn("UH-OH: child process (pid %d) read error",
			     mypid);
			return;
		}
	} while (rv < (int)sizeof(pids));

	if (mypid==pids[0]) {
		otherpid = pids[1];
	}
	else if (mypid==pids[1]) {
		otherpid = pids[0];
	}
	else {
		warn("UH-OH: child process (pid %d) got garbage in comm file",
		     mypid);
		return;
	}
	close(fd);

	rv = waitpid(otherpid, &x, 0);
	report_survival(rv, errno, "sibling wait");
}

static
void
wait_siblings(void)
{
	int pids[2], fd, rv, x;

	/* Note: this may also blow up if FS synchronization is substandard */

	fd = open_testfile(NULL);
	if (fd<0) {
		return;
	}

	pids[0] = fork();
	if (pids[0]<0) {
		warn("UH-OH: can't fork");
		return;
	}
	if (pids[0]==0) {
		close(fd);
		wait_siblings_child();
		_exit(0);
	}

	pids[1] = fork();
	if (pids[1]<0) {
		warn("UH-OH: can't fork");
		/* abandon the other child process :( */
		return;
	}
	if (pids[1]==0) {
		close(fd);
		wait_siblings_child();
		_exit(0);
	}

	rv = write(fd, pids, sizeof(pids));
	if (rv < 0) {
		warn("UH-OH: write error on %s", TESTFILE);
		/* abandon child procs :( */
		return;
	}
	if (rv != (int)sizeof(pids)) {
		warnx("UH-OH: write error on %s: short count", TESTFILE);
		/* abandon child procs :( */
		return;
	}

	rv = waitpid(pids[0], &x, 0);
	if (rv<0) {
		warn("UH-OH: error waiting for child 0 (pid %d)", pids[0]);
	}
	rv = waitpid(pids[1], &x, 0);
	if (rv<0) {
		warn("UH-OH: error waiting for child 1 (pid %d)", pids[1]);
	}
	warnx("passed: siblings wait for each other");
	close(fd);
	remove(TESTFILE);
}

////////////////////////////////////////////////////////////

void
test_waitpid(void)
{
	wait_badpid(-8, "wait for pid -8");
	wait_badpid(-1, "wait for pid -1");
	wait_badpid(0, "pid zero");
	wait_badpid(NONEXIST_PID, "nonexistent pid");

	wait_badstatus(NULL, "wait with NULL status");
	wait_badstatus(INVAL_PTR, "wait with invalid pointer status");
	wait_badstatus(KERN_PTR, "wait with kernel pointer status");

	wait_unaligned();

	wait_badflags();

	wait_self();
	wait_parent();
	wait_siblings();
}
