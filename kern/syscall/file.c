#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <lib.h>
#include <uio.h>
#include <thread.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <syscall.h>
#include <copyinout.h>

/*
 * Add your file-related functions here ...
 */
 #define MAX_SIZE 255

 struct file_descriptor glob_file_table[OPEN_MAX];
 struct lock *table_lock;

int f_des_init(struct thread *thisthread){
    int result;
    char temp_path[] = "con: ";
    if(thisthread->file_map[0] == NULL){
        struct vnode *v_node;
        thisthread->file_map[0] = (struct file_descriptor *)kmalloc(sizeof(struct file_descriptor));
        result = vfs_open(temp_path, O_RDONLY, 0664, &v_node);
        if(result){
    		kprintf("f_desc_init failed\n");
    		kfree(temp_path);
    		return EINVAL;
    	}
        thisthread->file_map[0]->v_node = v_node;
    	thisthread->file_map[0]->permission_flag = O_RDONLY;
    	thisthread->file_map[0]->off_set = 0;
    	thisthread->file_map[0]->file_lock = lock_create(temp_path);
    }

    if(thisthread->file_map[1] == NULL){
        struct vnode *v_node2;
        thisthread->file_map[1] = (struct file_descriptor *)kmalloc(sizeof(struct file_descriptor));
        result = vfs_open(temp_path, O_RDONLY, 0664, &v_node2);
        if(result){
    		kprintf("f_desc_init failed\n");
    		kfree(temp_path);
    		return EINVAL;
    	}
        thisthread->file_map[1]->v_node = v_node2;
    	thisthread->file_map[1]->permission_flag = O_WRONLY;
    	thisthread->file_map[1]->off_set = 0;
    	thisthread->file_map[1]->file_lock = lock_create(temp_path);
    }

    if(thisthread->file_map[2] == NULL){
        struct vnode *v_node3;
        thisthread->file_map[2] = (struct file_descriptor *)kmalloc(sizeof(struct file_descriptor));
        result = vfs_open(temp_path, O_RDONLY, 0664, &v_node3);
        if(result){
    		kprintf("f_desc_init failed\n");
    		kfree(temp_path);
    		return EINVAL;
    	}
        thisthread->file_map[2]->v_node = v_node3;
    	thisthread->file_map[2]->permission_flag = O_WRONLY;
    	thisthread->file_map[2]->off_set = 0;
    	thisthread->file_map[2]->file_lock = lock_create(temp_path);
    }
    return 0;
}

int sys_open(const char *filename, int flag, mode_t mode, int *return_value) {
	if (filename == NULL) {
		return ENOENT;
	}
	char path[PATH_MAX];
	size_t len = 0;

	copyinstr((const_userptr_t) filename, path, PATH_MAX, &len);
	int fd_index = 3;

	while(curthread->file_map[fd_index] != NULL) {
		if (fd_index == OPEN_MAX) {
			return EMFILE;
		}
		fd_index ++;
	}
	curthread->file_map[fd_index] = (struct file_descriptor*)kmalloc(sizeof(struct file_descriptor));

	int glob_table_index = 0;
	while (glob_file_table[glob_table_index].v_node != NULL) {
		if (glob_table_index == OPEN_MAX) {
			return EMFILE;
		}
		glob_table_index ++;
	}

	struct vnode *vnode_struct;
	int v_error = vfs_open((char *)filename, flag, mode, &vnode_struct);

	if (v_error) {
		*return_value = -1;
		return v_error;
	}

	// Create a new file struct to add into our global file table;
	glob_file_table[glob_table_index].v_node = vnode_struct;
	glob_file_table[glob_table_index].off_set = 0;
	glob_file_table[glob_table_index].file_lock = lock_create(filename);
	glob_file_table[glob_table_index].permission_flag = flag;

	/*
	for(int index = 0; file_table->file[index]; index ++){
		if (index > OPEN_MAX) {
			*return_value = -1;
			return EMFILE;
		}
	}
	file_table_file[index] = file_handler;
	*return_value = index;
	*/
	curthread->file_map[fd_index] = &glob_file_table[glob_table_index];
	*return_value = fd_index;
	return 1;
}

int sys_read(int fd, void * buf, size_t size, int32_t * retval){
    if (fd < 0 || fd >= OPEN_MAX){
        return ENFILE;
    }else if (curthread->file_map[fd] == NULL){
        return ENFILE;
    }

    struct file_descriptor *F_des = curthread->file_map[fd];

    if(! (F_des->permission_flag != O_RDONLY || F_des->permission_flag != O_RDWR) )
	{
		*retval = -1;
		return EINVAL;
	}
    struct uio readuio;
	struct iovec iov;
	char *buffer = (char*)kmalloc(size);

    lock_acquire(F_des->file_lock);
    uio_kinit(&iov,&readuio,(void*)buffer,size,F_des->off_set,UIO_READ);

    int read_result = VOP_READ(F_des->v_node, &readuio);
    if(read_result){
        kfree(buffer);
		lock_release(F_des->file_lock);
       	*retval = -1;
    	return read_result;
    }

    copyout((const void *)buffer, (userptr_t)buf, size);
    F_des->off_set = readuio.uio_offset;
    *retval = size - readuio.uio_resid;
    lock_release(F_des->file_lock);
    kfree(buffer);
    return 0;
}


int sys_write(int fd,void *buf, size_t size,int* retval){
    if (fd < 0 || fd >= OPEN_MAX){
        return ENFILE;
    }else if (curthread->file_map[fd] == NULL){
        return ENFILE;
    }

    struct file_descriptor *F_des = curthread->file_map[fd];

    if(! (F_des->permission_flag != O_WRONLY || F_des->permission_flag != O_RDWR) )
	{
		*retval = -1;
		return EINVAL;
	}
    struct uio writeuio;
	struct iovec iov;
	char *buffer = (char*)kmalloc(size);

    lock_acquire(F_des->file_lock);
    uio_kinit(&iov,&writeuio,(void*)buffer,size,F_des->off_set,UIO_WRITE);
    copyin((const_userptr_t) buf, buffer, size);
    int write_result = VOP_WRITE(F_des->v_node, &writeuio);
    if (write_result){
        kfree(buffer);
		lock_release(F_des->file_lock);
       	*retval = -1;
    	return write_result;
    }

    F_des->off_set = writeuio.uio_offset;
    lock_release(F_des->file_lock);
    kfree(buffer);
    *retval = size - writeuio.uio_resid;
    return 0;
}
