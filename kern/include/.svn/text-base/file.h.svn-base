#ifndef _FILE_H_
#define _FILE_H_

#include <lib.h>
#include <device.h>
#include <uio.h>
#include <fs.h>
#include <vnode.h>
#include <thread.h>

#define MAX_FILE_FILETAB 20

struct File{
	struct vnode* vnode;
	int flags;
	off_t offset;
	struct lock *f_lock;
};

int sys_open(const_userptr_t filename, int flags, int mode, int*);
int sys_read(int filehandle, void *buf, size_t size,int*);
int sys_write(int filehandle, void *buf, size_t size,int*);
int sys_lseek(int filehandle, off_t pos, int code, int* retval);
int sys_close(int filehandle,int*);
int sys_dup2(int filehandle, int newhandle,int*);
int fdesc_init(struct thread *);
int thread_fd_opencon(void);
int sys_chdir(char *fname, int *retval);
int sys___getcwd(char *fname, size_t buflen, int *retval);
struct vnode* get_filetab(int fd);
int sfs_stat(struct vnode *v, struct stat *statbuf);

//file system calls
int sys_mkdir(userptr_t pathname, int32_t mode, int32_t *retval);
int sys_rmdir(userptr_t pathname, int32_t *retval);
int sys_remove(userptr_t pathname, int32_t *retval);
int sys_getdirentry(int32_t fd, userptr_t buf, int32_t buflen, int32_t *retval);
int sys_rename(userptr_t old, userptr_t new, int32_t *retval);
int sfs_getdirentry(struct vnode *v, struct uio *uio);
int sys_fstat(int32_t fd, userptr_t statbuf, int32_t *retval);


#endif /*_FILE_H_*/
