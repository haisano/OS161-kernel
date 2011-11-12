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

//struct File *fp[20];
//fp=(struct File*)malloc(sizeof(struct File));
//fp->open=true;


/* a simple init function for testing the file functions*/

//struct File *fdesc[20];

struct vnode* get_filetab(int fd) {
	return curthread->fdesc[fd]->vnode;
}

int sys_open(const_userptr_t filename, int flags, int mode, int *retval){

    int result,fd;
	size_t len;
	struct vnode *vn; 
	char *fname = (char*)kmalloc(sizeof(char)*PATH_MAX);
	if(fname == NULL)
		return ENOMEM;
	//copy the string to kernel space
	copyinstr(filename, fname, PATH_MAX, &len);
	
	// filedescriptor after stderr 
  	for(fd = 3; fd < MAX_FILE_FILETAB; fd++) 
	{
    		if(curthread->fdesc[fd] == NULL) 
			break;
	}
	if(fd == MAX_FILE_FILETAB)
	{
		return ENFILE;
	}
      	curthread->fdesc[fd] = (struct File *)kmalloc(sizeof(struct File*));
	if(curthread->fdesc[fd] == NULL)
	{
        	return ENFILE; 
      	} 
	  	
  	result = vfs_open(fname, flags,mode, &vn);
	curthread->fdesc[fd]->vnode = vn;
	if(result)
    		return result;
	
	curthread->fdesc[fd]->flags = flags;
	curthread->fdesc[fd]->offset = 0;
	curthread->fdesc[fd]->f_lock=lock_create(fname);	
	*retval = fd;
	kfree(fname);
	return 0;
}


int fdesc_init(struct thread *newthread)
{
	if(newthread->fdesc[0] == NULL){
	int result1;
	struct vnode *v1;	
	newthread->fdesc[0] = (struct File *)kmalloc(sizeof(struct File));
	char temp1[]="con:";
	result1 = vfs_open(temp1, O_RDONLY, 0664, &v1);
	if(result1){
		kprintf("fdesc_init failed\n");
		kfree(temp1);
		return EINVAL;
	}
	newthread->fdesc[0]->vnode = v1;
	newthread->fdesc[0]->flags = O_RDONLY;
	newthread->fdesc[0]->offset = 0;	
	newthread->fdesc[0]->f_lock = lock_create(temp1);
	}

	if(newthread->fdesc[1] == NULL){
	struct vnode *v2;	
	newthread->fdesc[1] = (struct File *)kmalloc(sizeof(struct File));
	char temp2[]="con:";	
	int result2 = vfs_open(temp2, O_WRONLY, 0664, &v2);
	if(result2) {
		kprintf("fdesc_init failed\n");
		kfree(temp2);
		return EINVAL;
	}
	newthread->fdesc[1]->vnode=v2;
	newthread->fdesc[1]->flags = O_WRONLY;
	newthread->fdesc[1]->offset = 0;
	newthread->fdesc[1]->f_lock = lock_create(temp2);
	}

	if(newthread->fdesc[2] == NULL){
	struct vnode *v3;	
	newthread->fdesc[2] = (struct File *)kmalloc(sizeof(struct File));
	char temp3[]="con:";
	int result3 = vfs_open(temp3, O_WRONLY, 0664, &v3);
	if(result3) {
		kprintf("fdesc_init failed\n");
		kfree(temp3);
		return EINVAL;
	}
	newthread->fdesc[2]->vnode = v3;
	newthread->fdesc[2]->flags = O_WRONLY;
	newthread->fdesc[2]->offset=0;
	newthread->fdesc[2]->f_lock = lock_create(temp3);
	}
	return 0;
}

/* read
 * read from a filehandle to a buffer of specified size
 */

int sys_read(int filehandle, void *buf, size_t size,int* retval){

        lock_acquire(curthread->fdesc[filehandle]->f_lock);
	if(filehandle < 0 || filehandle > MAX_FILE_FILETAB){
		*retval = -1;
		return EBADF;
	}
	if(curthread->fdesc[filehandle] == NULL)
	{
		*retval = -1;
		return EBADF;
	}
	if(! (curthread->fdesc[filehandle]->flags != O_RDONLY || curthread->fdesc[filehandle]->flags != O_RDWR) )
	{
		*retval = -1;
		return EINVAL;
	}
	struct uio readuio;
	struct iovec iov;
	char *buffer = (char*)kmalloc(size);
	//readuio = (struct uio*)kmalloc(sizeof(struct uio));
		
 	uio_kinit(&iov,&readuio,(void*)buffer,size,curthread->fdesc[filehandle]->offset,UIO_READ);	
	
  	int result=VOP_READ(curthread->fdesc[filehandle]->vnode, &readuio);
	if (result) {
		kfree(buffer);
		lock_release(curthread->fdesc[filehandle]->f_lock);
	       	*retval = -1;  
        	return result;
	}
	//if(filehandle > 3)
        	curthread->fdesc[filehandle]->offset= readuio.uio_offset;
	//else
	//copyoutstr((const char*)buffer,(userptr_t)buf,sizeof(buffer),&size1);
	//	curthread->fdesc[filehandle]->offset= 1;
	copyout((const void *)buffer, (userptr_t)buf, size);
        *retval=size - readuio.uio_resid;
	kfree(buffer);
	//kfree(readuio->uio_iov);
       //kfree(readuio);
	lock_release(curthread->fdesc[filehandle]->f_lock);
	return 0;
}

/* write
 * write to a filehandle from a buffer of specified size
 */

int sys_write(int filehandle,void *buf, size_t size,int* retval){
	
	if(filehandle < 0 || filehandle > MAX_FILE_FILETAB){
		*retval = -1;
		return EBADF;
	}	
	if(curthread->fdesc[filehandle] == NULL)
	{
		*retval = -1;
		return EBADF;
	}
	if(! (curthread->fdesc[filehandle]->flags != O_WRONLY || curthread->fdesc[filehandle]->flags!=O_RDWR) )
	{
		*retval = -1;
		return EINVAL;
	}
	lock_acquire(curthread->fdesc[filehandle]->f_lock);
       
	struct uio writeuio;
	struct iovec iov;
	size_t size1;
	char *buffer = (char*)kmalloc(size);
	copyinstr((userptr_t)buf,buffer,strlen(buffer),&size1);
	
	uio_kinit(&iov, &writeuio, (void*) buffer, size, curthread->fdesc[filehandle]->offset, UIO_WRITE);
	int result=VOP_WRITE(curthread->fdesc[filehandle]->vnode, &writeuio);
        if (result) {
		kfree(buffer);
		lock_release(curthread->fdesc[filehandle]->f_lock);
		*retval = -1; 
                return result;
        }
	curthread->fdesc[filehandle]->offset = writeuio.uio_offset;
	*retval = size - writeuio.uio_resid;
	kfree(buffer);
	lock_release(curthread->fdesc[filehandle]->f_lock);
        return 0;
}

int sys_lseek(int filehandle, off_t pos, int code,int *retval){

	off_t offset;
	struct stat tmp;
	int result;
	
	if(filehandle < 0 || filehandle > MAX_FILE_FILETAB){
		*retval = -1;
		return EBADF;
	}
	if(curthread->fdesc == NULL)
	{
		*retval = -1;
		return EBADF;
	}
	struct File* fd = curthread->fdesc[filehandle];
	lock_acquire(curthread->fdesc[filehandle]->f_lock);

	//actual seek occurs 
	switch(code) {
		case SEEK_SET:
		offset = pos;
		break;

		case SEEK_CUR:
		offset = fd->offset + pos;
		break;

		case SEEK_END:
      		result = VOP_STAT(fd->vnode, &tmp);
		if(result)
        		return result;
		offset = tmp.st_size + pos;
		break;

		default:
		lock_release(curthread->fdesc[filehandle]->f_lock);
		return EINVAL;
	}

	if(offset < 0) {
		lock_release(curthread->fdesc[filehandle]->f_lock);
		return EINVAL;
	}

	result = VOP_TRYSEEK(fd->vnode, offset);
	if(result)
		return result;

	// All done, update offset
	*retval = fd->offset = offset;
	lock_release(curthread->fdesc[filehandle]->f_lock);
	return 0;
}

int sys_close(int filehandle,int* retval){

       if(filehandle < 0 || filehandle > MAX_FILE_FILETAB){
		*retval = -1;
		return EBADF;
	}
	if(curthread->fdesc[filehandle] == NULL){
	        *retval = -1; 
                return EBADF;
       }
	lock_acquire(curthread->fdesc[filehandle]->f_lock);
	
	if(curthread->fdesc[filehandle]->vnode != NULL)
		vfs_close(curthread->fdesc[filehandle]->vnode);
		
	if(curthread->fdesc[filehandle]->vnode->vn_refcount == 0) {
		if(curthread->fdesc[filehandle]->vnode != NULL) 
			kfree(curthread->fdesc[filehandle]->vnode);
		kfree(curthread->fdesc[filehandle]);
	}
	
	else {
        curthread->fdesc[filehandle] = NULL;
	}
       	*retval = 0;
	lock_release(curthread->fdesc[filehandle]->f_lock); 
        return 0;
}


int sys_dup2(int filehandle, int newhandle, int* retval){
        if(filehandle < 0 || newhandle < 0){
          *retval = -1;
                return EBADF;
        }
        if(filehandle >= MAX_FILE_FILETAB || newhandle >= MAX_FILE_FILETAB){
           *retval = -1;
           return EBADF;
        }
        if(curthread->fdesc[newhandle] != NULL){                            
                if(sys_close(filehandle,retval))
                	return EBADF;
        }
	lock_acquire(curthread->fdesc[filehandle]->f_lock);
        curthread->fdesc[newhandle]->flags = curthread->fdesc[filehandle]->flags;
        curthread->fdesc[newhandle]->offset = curthread->fdesc[filehandle]->offset;
        curthread->fdesc[newhandle]->vnode = curthread->fdesc[filehandle]->vnode;
	*retval = newhandle;
	lock_release(curthread->fdesc[filehandle]->f_lock);
        return 0;
}


int sys___getcwd(char *fname, size_t buflen, int *retval)
{

	struct iovec iov;
	struct uio readuio; 
	//to do
	char *name = (char*)kmalloc(buflen);
	uio_kinit(&iov, &readuio, name, buflen-1, 0, UIO_READ);
	
	int result = vfs_getcwd(&readuio);
	if(result)
	{
		*retval = -1;
		return result;
	}
	//null terminate
	name[buflen-1-readuio.uio_resid] = 0;
	size_t size;
	copyoutstr((const void *)name, (userptr_t)fname, buflen, &size);
	*retval = buflen-readuio.uio_resid;
	kfree(name);
	return 0;
}

int sys_chdir(char *fname, int *retval)
{
	char *name = (char*)kmalloc(sizeof(char)*PATH_MAX);
	size_t len;
	int s = splhigh();
	copyinstr((userptr_t)fname, name, PATH_MAX, &len);
	int result = vfs_chdir(name);
	if(result)
	{
		splx(s);
		return result;
	}
	*retval = 0;
	kfree(name);
	splx(s);
	return 0;
}	
