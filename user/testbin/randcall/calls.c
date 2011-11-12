/* Automatically generated file; do not edit */
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

#include "extern.h"

typedef void (*tryfunc)(int dofork);

static
void
try_execv(int dofork)
{
	void * a0 = randptr();
	void * a1 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "execv(%p, %p)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = execv(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_waitpid(int dofork)
{
	int a0 = randint();
	void * a1 = randptr();
	int a2 = randint();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "waitpid(%d, %p, %d)",
		(a0), (a1), (a2));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = waitpid(a0, a1, a2);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_open(int dofork)
{
	void * a0 = randptr();
	int a1 = randint();
	int a2 = randint();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "open(%p, %d, %d)",
		(a0), (a1), (a2));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = open(a0, a1, a2);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_read(int dofork)
{
	int a0 = randint();
	void * a1 = randptr();
	size_t a2 = randsize();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "read(%d, %p, %lu)",
		(a0), (a1), (unsigned long)(a2));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = read(a0, a1, a2);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_write(int dofork)
{
	int a0 = randint();
	void * a1 = randptr();
	size_t a2 = randsize();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "write(%d, %p, %lu)",
		(a0), (a1), (unsigned long)(a2));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = write(a0, a1, a2);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_close(int dofork)
{
	int a0 = randint();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "close(%d)",
		(a0));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = close(a0);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_ioctl(int dofork)
{
	int a0 = randint();
	int a1 = randint();
	void * a2 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "ioctl(%d, %d, %p)",
		(a0), (a1), (a2));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = ioctl(a0, a1, a2);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_lseek(int dofork)
{
	int a0 = randint();
	off_t a1 = randoff();
	int a2 = randint();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "lseek(%d, %ld, %d)",
		(a0), (long)(a1), (a2));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = lseek(a0, a1, a2);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_fsync(int dofork)
{
	int a0 = randint();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "fsync(%d)",
		(a0));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = fsync(a0);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_ftruncate(int dofork)
{
	int a0 = randint();
	off_t a1 = randoff();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "ftruncate(%d, %ld)",
		(a0), (long)(a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = ftruncate(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_fstat(int dofork)
{
	int a0 = randint();
	void * a1 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "fstat(%d, %p)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = fstat(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_remove(int dofork)
{
	void * a0 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "remove(%p)",
		(a0));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = remove(a0);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_rename(int dofork)
{
	void * a0 = randptr();
	void * a1 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "rename(%p, %p)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = rename(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_link(int dofork)
{
	void * a0 = randptr();
	void * a1 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "link(%p, %p)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = link(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_mkdir(int dofork)
{
	void * a0 = randptr();
	int a1 = randint();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "mkdir(%p, %d)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = mkdir(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_rmdir(int dofork)
{
	void * a0 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "rmdir(%p)",
		(a0));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = rmdir(a0);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_chdir(int dofork)
{
	void * a0 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "chdir(%p)",
		(a0));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = chdir(a0);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_getdirentry(int dofork)
{
	int a0 = randint();
	void * a1 = randptr();
	size_t a2 = randsize();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "getdirentry(%d, %p, %lu)",
		(a0), (a1), (unsigned long)(a2));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = getdirentry(a0, a1, a2);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_symlink(int dofork)
{
	void * a0 = randptr();
	void * a1 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "symlink(%p, %p)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = symlink(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_readlink(int dofork)
{
	void * a0 = randptr();
	void * a1 = randptr();
	size_t a2 = randsize();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "readlink(%p, %p, %lu)",
		(a0), (a1), (unsigned long)(a2));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = readlink(a0, a1, a2);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_dup2(int dofork)
{
	int a0 = randint();
	int a1 = randint();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "dup2(%d, %d)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = dup2(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_pipe(int dofork)
{
	void * a0 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "pipe(%p)",
		(a0));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = pipe(a0);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try___time(int dofork)
{
	void * a0 = randptr();
	void * a1 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "__time(%p, %p)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = __time(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try___getcwd(int dofork)
{
	void * a0 = randptr();
	size_t a1 = randsize();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "__getcwd(%p, %lu)",
		(a0), (unsigned long)(a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = __getcwd(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_stat(int dofork)
{
	void * a0 = randptr();
	void * a1 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "stat(%p, %p)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = stat(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static
void
try_lstat(int dofork)
{
	void * a0 = randptr();
	void * a1 = randptr();
	int result, pid, status;
	char buf[128];

	snprintf(buf, sizeof(buf), "lstat(%p, %p)",
		(a0), (a1));
	printf("%-47s", buf);

	pid = dofork ? fork() : 0;
	if (pid<0) {
		err(1, "fork");
	}
	if (pid>0) {
		waitpid(pid, &status, 0);
		return;
	}

	result = lstat(a0, a1);
	printf(" result %d, errno %d\n", result, errno);
	if (dofork) {
		exit(0);
	}
}

static tryfunc funcs2[] = {
	try_execv,
	try_waitpid,
	try_open,
	try_read,
	try_write,
	try_close,
	try_lseek,
	try_chdir,
	try_dup2,
	try___getcwd,
	NULL
};

static tryfunc funcs3[] = {
	try_execv,
	try_waitpid,
	try_open,
	try_read,
	try_write,
	try_close,
	try_lseek,
	try_chdir,
	try_dup2,
	try___getcwd,
	NULL
};

static tryfunc funcs4[] = {
	try_execv,
	try_waitpid,
	try_open,
	try_read,
	try_write,
	try_close,
	try_lseek,
	try_fsync,
	try_ftruncate,
	try_fstat,
	try_remove,
	try_rename,
	try_mkdir,
	try_rmdir,
	try_chdir,
	try_getdirentry,
	try_dup2,
	try___getcwd,
	NULL
};

static tryfunc funcs5[] = {
	try_execv,
	try_waitpid,
	try_open,
	try_read,
	try_write,
	try_close,
	try_ioctl,
	try_lseek,
	try_fsync,
	try_ftruncate,
	try_fstat,
	try_remove,
	try_rename,
	try_link,
	try_mkdir,
	try_rmdir,
	try_chdir,
	try_getdirentry,
	try_symlink,
	try_readlink,
	try_dup2,
	try_pipe,
	try___time,
	try___getcwd,
	try_stat,
	try_lstat,
	NULL
};

static tryfunc *tables[4] = {
	funcs2,
	funcs3,
	funcs4,
	funcs5,
};

void
trycalls(int asst, int dofork, int count)
{
	tryfunc *list;
	int i, j;

	assert(asst>=2 && asst<=5);
	list = tables[asst-2];

	for (i=0; i<count; i++) {
		for (j=0; list[j]; j++) {
			(*list[j])(dofork);
		}
	}
}

