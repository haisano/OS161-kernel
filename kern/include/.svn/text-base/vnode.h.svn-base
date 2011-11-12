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

#ifndef _VNODE_H_
#define _VNODE_H_


struct uio;
struct stat;

/*
 * A struct vnode is an abstract representation of a file.
 *
 * It is an interface in the Java sense that allows the kernel's
 * filesystem-independent code to interact usefully with multiple sets
 * of filesystem code.
 */

/*
 * Abstract low-level file.
 *
 * Note: vn_fs may be null if the vnode refers to a device.
 *
 * vn_opencount is managed using VOP_INCOPEN and VOP_DECOPEN by
 * vfs_open() and vfs_close(). Code above the VFS layer should not
 * need to worry about it.
 */
struct vnode {
	int vn_refcount;                /* Reference count */
	int vn_opencount;

	struct fs *vn_fs;               /* Filesystem vnode belongs to */

	void *vn_data;                  /* Filesystem-specific data */

	const struct vnode_ops *vn_ops; /* Functions on this vnode */
	
	struct lock* vn_rwlock; 
	struct lock* vn_createlock;
	struct lock* vn_countlock;

};

/*
 * Abstract operations on a vnode.
 *
 * These are used in the form VOP_FOO(vnode, args), which are macros
 * that expands to vnode->vn_ops->vop_foo(vnode, args). The operations
 * "foo" are:
 *
 *    vop_open        - Called on *each* open() of a file. Can be used to
 *                      reject illegal or undesired open modes. Note that
 *                      various operations can be performed without the
 *                      file actually being opened.
 *                      The vnode need not look at O_CREAT, O_EXCL, or 
 *                      O_TRUNC, as these are handled in the VFS layer.
 *
 *                      VOP_OPEN should not be called directly from above
 *                      the VFS layer - use vfs_open() to open vnodes.
 *                      This maintains the open count so VOP_CLOSE can be
 *                      called at the right time.
 *
 *    vop_close       - To be called on *last* close() of a file.
 *
 *                      VOP_CLOSE should not be called directly from above
 *                      the VFS layer - use vfs_close() to close vnodes
 *                      opened with vfs_open().
 *
 *    vop_reclaim     - Called when vnode is no longer in use. Note that
 *                      this may be substantially after vop_close is
 *                      called.
 *
 *****************************************
 *
 *    vop_read        - Read data from file to uio, at offset specified
 *                      in the uio, updating uio_resid to reflect the
 *                      amount read, and updating uio_offset to match.
 *                      Not allowed on directories or symlinks.
 *
 *    vop_readlink    - Read the contents of a symlink into a uio.
 *                      Not allowed on other types of object.
 *
 *    vop_getdirentry - Read a single filename from a directory into a
 *                      uio, choosing what name based on the offset
 *                      field in the uio, and updating that field.
 *                      Unlike with I/O on regular files, the value of
 *                      the offset field is not interpreted outside
 *                      the filesystem and thus need not be a byte
 *                      count. However, the uio_resid field should be
 *                      handled in the normal fashion.
 *                      On non-directory objects, return ENOTDIR.
 *
 *    vop_write       - Write data from uio to file at offset specified
 *                      in the uio, updating uio_resid to reflect the
 *                      amount written, and updating uio_offset to match.
 *                      Not allowed on directories or symlinks.
 *
 *    vop_ioctl       - Perform ioctl operation OP on file using data
 *                      DATA. The interpretation of the data is specific
 *                      to each ioctl.
 *
 *    vop_stat        - Return info about a file. The pointer is a 
 *                      pointer to struct stat; see kern/stat.h.
 *
 *    vop_gettype     - Return type of file. The values for file types
 *                      are in kern/stattypes.h.
 *
 *    vop_tryseek     - Check if seeking to the specified position within
 *                      the file is legal. (For instance, all seeks
 *                      are illegal on serial port devices, and seeks
 *                      past EOF on files whose sizes are fixed may be
 *                      as well.)
 *
 *    vop_fsync       - Force any dirty buffers associated with this file
 *                      to stable storage.
 *
 *    vop_mmap        - Map file into memory. If you implement this
 *                      feature, you're responsible for choosing the
 *                      arguments for this operation.
 *
 *    vop_truncate    - Forcibly set size of file to the length passed
 *                      in, discarding any excess blocks.
 *
 *    vop_namefile    - Compute pathname relative to filesystem root
 *                      of the file and copy to the specified
 *                      uio. Need not work on objects that are not
 *                      directories.
 *
 *****************************************
 *
 *    vop_creat       - Create a regular file named NAME in the passed
 *                      directory DIR. If boolean EXCL is true, fail if
 *                      the file already exists; otherwise, use the
 *                      existing file if there is one. Hand back the
 *                      vnode for the file as per vop_lookup.
 *
 *    vop_symlink     - Create symlink named NAME in the passed directory,
 *                      with contents CONTENTS.
 *
 *    vop_mkdir       - Make directory NAME in the passed directory PARENTDIR.
 *
 *    vop_link        - Create hard link, with name NAME, to file FILE
 *                      in the passed directory DIR.
 *
 *    vop_remove      - Delete non-directory object NAME from passed 
 *                      directory. If NAME refers to a directory,
 *                      return EISDIR. If passed vnode is not a
 *                      directory, return ENOTDIR.
 *
 *    vop_rmdir       - Delete directory object NAME from passed 
 *                      directory.
 *
 *    vop_rename      - Rename file NAME1 in directory VN1 to be
 *                      file NAME2 in directory VN2.
 *
 *****************************************
 *
 *    vop_lookup      - Parse PATHNAME relative to the passed directory
 *                      DIR, and hand back the vnode for the file it
 *                      refers to. May destroy PATHNAME. Should increment
 *                      refcount on vnode handed back.
 *
 *    vop_lookparent  - Parse PATHNAME relative to the passed directory
 *                      DIR, and hand back (1) the vnode for the
 *                      parent directory of the file it refers to, and
 *                      (2) the last component of the filename, copied
 *                      into kernel buffer BUF with max length LEN. May
 *                      destroy PATHNAME. Should increment refcount on
 *                      vnode handed back.
 */

#define VOP_MAGIC	0xa2b3c4d5

struct vnode_ops {
	unsigned long vop_magic;	/* should always be VOP_MAGIC */

	int (*vop_open)(struct vnode *object, int flags_from_open);
	int (*vop_close)(struct vnode *object);
	int (*vop_reclaim)(struct vnode *vnode);


	int (*vop_read)(struct vnode *file, struct uio *uio);
	int (*vop_readlink)(struct vnode *link, struct uio *uio);
	int (*vop_getdirentry)(struct vnode *dir, struct uio *uio);
	int (*vop_write)(struct vnode *file, struct uio *uio);
	int (*vop_ioctl)(struct vnode *object, int op, userptr_t data);
	int (*vop_stat)(struct vnode *object, struct stat *statbuf);
	int (*vop_gettype)(struct vnode *object, mode_t *result);
	int (*vop_tryseek)(struct vnode *object, off_t pos);
	int (*vop_fsync)(struct vnode *object);
	int (*vop_mmap)(struct vnode *file /* add stuff */);
	int (*vop_truncate)(struct vnode *file, off_t len);
	int (*vop_namefile)(struct vnode *file, struct uio *uio);


	int (*vop_creat)(struct vnode *dir, 
			 const char *name, bool excl, mode_t mode,
			 struct vnode **result);
	int (*vop_symlink)(struct vnode *dir, 
			   const char *contents, const char *name);
	int (*vop_mkdir)(struct vnode *parentdir, 
			 const char *name, mode_t mode);
	int (*vop_link)(struct vnode *dir, 
			const char *name, struct vnode *file);
	int (*vop_remove)(struct vnode *dir, 
			  const char *name);
	int (*vop_rmdir)(struct vnode *dir,
			 const char *name);

	int (*vop_rename)(struct vnode *vn1, const char *name1, 
			  struct vnode *vn2, const char *name2);

	
	int (*vop_lookup)(struct vnode *dir, 
			  char *pathname, struct vnode **result);
	int (*vop_lookparent)(struct vnode *dir,
			      char *pathname, struct vnode **result,
			      char *buf, size_t len);
};

#define __VOP(vn, sym) (vnode_check(vn, #sym), (vn)->vn_ops->vop_##sym)

#define VOP_OPEN(vn, flags)             (__VOP(vn, open)(vn, flags))
#define VOP_CLOSE(vn)                   (__VOP(vn, close)(vn))
#define VOP_RECLAIM(vn)                 (__VOP(vn, reclaim)(vn))

#define VOP_READ(vn, uio)               (__VOP(vn, read)(vn, uio))
#define VOP_READLINK(vn, uio)           (__VOP(vn, readlink)(vn, uio))
#define VOP_GETDIRENTRY(vn, uio)        (__VOP(vn,getdirentry)(vn, uio))
#define VOP_WRITE(vn, uio)              (__VOP(vn, write)(vn, uio))
#define VOP_IOCTL(vn, code, buf)        (__VOP(vn, ioctl)(vn,code,buf))
#define VOP_STAT(vn, ptr) 	        	(__VOP(vn, stat)(vn, ptr))
#define VOP_GETTYPE(vn, result)         (__VOP(vn, gettype)(vn, result))
#define VOP_TRYSEEK(vn, pos)            (__VOP(vn, tryseek)(vn, pos))
#define VOP_FSYNC(vn)                   (__VOP(vn, fsync)(vn))
#define VOP_MMAP(vn /*add stuff */)     (__VOP(vn, mmap)(vn /*add stuff */))
#define VOP_TRUNCATE(vn, pos)           (__VOP(vn, truncate)(vn, pos))
#define VOP_NAMEFILE(vn, uio)           (__VOP(vn, namefile)(vn, uio))

#define VOP_CREAT(vn,nm,excl,mode,res)  (__VOP(vn, creat)(vn,nm,excl,mode,res))
#define VOP_SYMLINK(vn, name, content)  (__VOP(vn, symlink)(vn, name, content))
#define VOP_MKDIR(vn, name, mode)       (__VOP(vn, mkdir)(vn, name, mode))
#define VOP_LINK(vn, name, vn2)         (__VOP(vn, link)(vn, name, vn2))
#define VOP_REMOVE(vn, name)            (__VOP(vn, remove)(vn, name))
#define VOP_RMDIR(vn, name)             (__VOP(vn, rmdir)(vn, name))
#define VOP_RENAME(vn1,name1,vn2,name2)(__VOP(vn1,rename)(vn1,name1,vn2,name2))

#define VOP_LOOKUP(vn, name, res)       (__VOP(vn, lookup)(vn, name, res))
#define VOP_LOOKPARENT(vn,nm,res,bf,ln) (__VOP(vn,lookparent)(vn,nm,res,bf,ln))

/*
 * Consistency check
 */
void vnode_check(struct vnode *, const char *op);

/*
 * Reference count manipulation (handled above filesystem level)
 */
void vnode_incref(struct vnode *);
void vnode_decref(struct vnode *);

#define VOP_INCREF(vn) 			vnode_incref(vn)
#define VOP_DECREF(vn) 			vnode_decref(vn)

/*
 * Open count manipulation (handled above filesystem level)
 *
 * VOP_INCOPEN is called by vfs_open. VOP_DECOPEN is called by vfs_close.
 * Neither of these should need to be called from above the vfs layer.
 */
void vnode_incopen(struct vnode *);
void vnode_decopen(struct vnode *);

#define VOP_INCOPEN(vn) 		vnode_incopen(vn)
#define VOP_DECOPEN(vn) 		vnode_decopen(vn)

/*
 * Vnode initialization (intended for use by filesystem code)
 * The reference count is initialized to 1.
 */
int vnode_init(struct vnode *, const struct vnode_ops *ops,
	       struct fs *fs, void *fsdata);

#define VOP_INIT(vn, ops, fs, data)     vnode_init(vn, ops, fs, data)

/*
 * Vnode final cleanup (intended for use by filesystem code)
 * The reference count is asserted to be 1.
 */
void vnode_cleanup(struct vnode *);

#define VOP_CLEANUP(vn)			vnode_cleanup(vn)


#endif /* _VNODE_H_ */
