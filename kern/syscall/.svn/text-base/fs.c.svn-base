#include <types.h>
#include <lib.h>
#include <synch.h>
#include <array.h>
#include <vfs.h>
#include <vnode.h>
#include <fs.h>
#include <uio.h>
#include <device.h>
#include <kern/limits.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <thread.h>
#include <current.h>
#include <file.h>
#include <kern/fcntl.h>
#include <copyinout.h>
#include <kern/seek.h>
#include <file.h>
#include <limits.h>
#include <kern/stat.h>
#include <spl.h>
#include <current.h>

int sys_mkdir(userptr_t pathname, int32_t mode, int32_t *retval)
{
	char name[NAME_MAX+1];
	size_t len;

	if (copyinstr(pathname, name, NAME_MAX, &len) != 0) {
		kprintf("oops something went wrong in copying the string %s\n", name);
		*retval = -1;
		return 1;
	}

	*retval = vfs_mkdir(name, mode);
	if (*retval != 0) {
		*retval = -1;
		kprintf("%s: could not make a directory\n", __func__);
		return 1;
	}

	return 0;
}

int sys_rmdir(userptr_t pathname, int32_t *retval)
{
	char name[NAME_MAX+1];
	size_t len;

	if (copyinstr(pathname, name, NAME_MAX, &len) != 0) {
		kprintf("oops something went wrong in copying the string %s\n", name);
		*retval = -1;
		return 1;
	}

	*retval = vfs_rmdir(name);
	if (*retval != 0) {
		*retval = -1;
		kprintf("%s: could not remove a directory\n", __func__);
		return 1;
	}
	return 0;
}

int sys_remove(userptr_t pathname, int32_t *retval)
{
	char name[NAME_MAX+1];
	size_t len;

	if (copyinstr(pathname, name, NAME_MAX, &len) != 0) {
		kprintf("oops something went wrong in copying the string %s\n", name);
		*retval = -1;
		return 1;
	}

	*retval = vfs_remove(name);
	if (*retval != 0) {
		*retval = -1;
		kprintf("%s: could not remove a directory\n", __func__);
		return 1;
	}
	return 0;
}
	
int sys_rename(userptr_t old, userptr_t new, int32_t *retval)
{
	char old_name[NAME_MAX+1];
	char new_name[NAME_MAX+1];
	size_t len;

	if (copyinstr(old, old_name, NAME_MAX, &len) != 0) {
		kprintf("oops something went wrong in copying the string %s\n", old_name);
		*retval = -1;
		return 1;
	}

	if (copyinstr(new, new_name, NAME_MAX, &len) != 0) {
		kprintf("oops something went wrong in copying the string %s\n", new_name);
		*retval = -1;
		return 1;
	}


	*retval = vfs_rename(old_name, new_name);
	if (*retval != 0) {
		*retval = -1;
		kprintf("%s: could not remove a directory\n", __func__);
		return 1;
	}
	return 0;
}

int sys_getdirentry(int32_t fd, userptr_t buf, int32_t buflen, int32_t *retval)
{
	struct uio ku;
	struct iovec iov;

	uio_kinit(&iov, &ku, buf, buflen, 0, UIO_READ);
	
	kprintf("%s: checking LS", __func__);	
	*retval = VOP_GETDIRENTRY(get_filetab(fd), &ku);

	if (*retval) {
		*retval = -1;
		kprintf("%s failed\n", __func__);
		return ENOENT;
	}
	return 0;
}

int sys_fstat(int32_t fd, userptr_t statbuf, int32_t *retval)
{
	struct stat *sbuf = (struct stat *) statbuf;


	*retval = VOP_STAT(get_filetab(fd), sbuf);
	if (*retval) {
		*retval = -1;
		kprintf("%s failed\n", __func__);
		return ENOENT;
	}

	return 0;
}



