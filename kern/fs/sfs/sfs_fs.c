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
 * SFS filesystem
 *
 * Filesystem-level interface routines.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <array.h>
#include <bitmap.h>
#include <uio.h>
#include <vfs.h>
#include <device.h>
#include <sfs.h>
#include <synch.h>

/* Shortcuts for the size macros in kern/sfs.h */
#define SFS_FS_BITMAPSIZE(sfs)  SFS_BITMAPSIZE((sfs)->sfs_super.sp_nblocks)
#define SFS_FS_BITBLOCKS(sfs)   SFS_BITBLOCKS((sfs)->sfs_super.sp_nblocks)

/*
 * Routine for doing I/O (reads or writes) on the free block bitmap.
 * We always do the whole bitmap at once; writing individual sectors
 * might or might not be a worthwhile optimization.
 *
 * The free block bitmap consists of SFS_BITBLOCKS 512-byte sectors of
 * bits, one bit for each sector on the filesystem. The number of
 * blocks in the bitmap is thus rounded up to the nearest multiple of
 * 512*8 = 4096. (This rounded number is SFS_BITMAPSIZE.) This means
 * that the bitmap will (in general) contain space for some number of
 * invalid sectors that are actually beyond the end of the disk
 * device. This is ok. These sectors are supposed to be marked "in
 * use" by mksfs and never get marked "free".
 *
 * The sectors used by the superblock and the bitmap itself are
 * likewise marked in use by mksfs.
 */

static
int
sfs_mapio(struct sfs_fs *sfs, enum uio_rw rw)
{
	uint32_t j, mapsize;
	char *bitdata;
	int result;

	/* Number of blocks in the bitmap. */
	mapsize = SFS_FS_BITBLOCKS(sfs);

	/* Pointer to our bitmap data in memory. */
	bitdata = bitmap_getdata(sfs->sfs_freemap);
	
	/* For each sector in the bitmap... */
	for (j=0; j<mapsize; j++) {

		/* Get a pointer to its data */
		void *ptr = bitdata + j*SFS_BLOCKSIZE;

		/* and read or write it. The bitmap starts at sector 2. */ 
		if (rw == UIO_READ) {
			result = sfs_rblock(sfs, ptr, SFS_MAP_LOCATION+j);
		}
		else {
			result = sfs_wblock(sfs, ptr, SFS_MAP_LOCATION+j);
		}

		/* If we failed, stop. */
		if (result) {
			return result;
		}
	}
	return 0;
}

/*
 * Sync routine. This is what gets invoked if you do FS_SYNC on the
 * sfs filesystem structure.
 */

static
int
sfs_sync(struct fs *fs)
{
	struct sfs_fs *sfs; 
	unsigned i, num;
	int result;

	vfs_biglock_acquire();

	/*
	 * Get the sfs_fs from the generic abstract fs.
	 *
	 * Note that the abstract struct fs, which is all the VFS
	 * layer knows about, is actually a member of struct sfs_fs.
	 * The pointer in the struct fs points back to the top of the
	 * struct sfs_fs - essentially the same object. This can be a
	 * little confusing at first.
	 *
	 * The following diagram may help:
	 *
	 *     struct sfs_fs        <-------------\
         *           :                            |
         *           :   sfs_absfs (struct fs)    |   <------\
         *           :      :                     |          |
         *           :      :  various members    |          |
         *           :      :                     |          |
         *           :      :  fs_data  ----------/          |
         *           :      :                             ...|...
         *           :                                   .  VFS  .
         *           :                                   . layer . 
         *           :   other members                    .......
         *           :                                    
         *           :
	 *
	 * This construct is repeated with vnodes and devices and other
	 * similar things all over the place in OS/161, so taking the
	 * time to straighten it out in your mind is worthwhile.
	 */

	sfs = fs->fs_data;

	/* Go over the array of loaded vnodes, syncing as we go. */
	num = vnodearray_num(sfs->sfs_vnodes);
	for (i=0; i<num; i++) {
		struct vnode *v = vnodearray_get(sfs->sfs_vnodes, i);
		VOP_FSYNC(v);
	}

	/* If the free block map needs to be written, write it. */
	if (sfs->sfs_freemapdirty) {
		result = sfs_mapio(sfs, UIO_WRITE);
		if (result) {
			vfs_biglock_release();
			return result;
		}
		sfs->sfs_freemapdirty = false;
	}

	/* If the superblock needs to be written, write it. */
	if (sfs->sfs_superdirty) {
		result = sfs_wblock(sfs, &sfs->sfs_super, SFS_SB_LOCATION);
		if (result) {
			vfs_biglock_release();
			return result;
		}
		sfs->sfs_superdirty = false;
	}

	vfs_biglock_release();
	return 0;
}

/*
 * Routine to retrieve the volume name. Filesystems can be referred
 * to by their volume name followed by a colon as well as the name
 * of the device they're mounted on.
 */
static
const char *
sfs_getvolname(struct fs *fs)
{
	struct sfs_fs *sfs = fs->fs_data;
	const char *ret;

	vfs_biglock_acquire();
	ret = sfs->sfs_super.sp_volname;
	vfs_biglock_release();

	return ret;
}

/*
 * Unmount code.
 *
 * VFS calls FS_SYNC on the filesystem prior to unmounting it.
 */
static
int
sfs_unmount(struct fs *fs)
{
	struct sfs_fs *sfs = fs->fs_data;

	vfs_biglock_acquire();
	
	/* Do we have any files open? If so, can't unmount. */
	if (vnodearray_num(sfs->sfs_vnodes) > 0) {
		vfs_biglock_release();
		return EBUSY;
	}

	/* We should have just had sfs_sync called. */
	KASSERT(sfs->sfs_superdirty == false);
	KASSERT(sfs->sfs_freemapdirty == false);

	/* Once we start nuking stuff we can't fail. */
	vnodearray_destroy(sfs->sfs_vnodes);
	bitmap_destroy(sfs->sfs_freemap);
	
	/* The vfs layer takes care of the device for us */
	(void)sfs->sfs_device;

	/* Destroy the fs object */
	kfree(sfs);

	/* nothing else to do */
	vfs_biglock_release();
	return 0;
}

/*
 * Mount routine.
 *
 * The way mount works is that you call vfs_mount and pass it a
 * filesystem-specific mount routine. Said routine takes a device and
 * hands back a pointer to an abstract filesystem. You can also pass
 * a void pointer through.
 *
 * This organization makes cleanup on error easier. Hint: it may also
 * be easier to synchronize correctly; it is important not to get two
 * filesystem with the same name mounted at once, or two filesystems
 * mounted on the same device at once.
 */

static
int
sfs_domount(void *options, struct device *dev, struct fs **ret)
{
	int result;
	struct sfs_fs *sfs;
	int i;

	vfs_biglock_acquire();

	/* We don't pass any options through mount */
	(void)options;

	/*
	 * Make sure our on-disk structures aren't messed up
	 */
	KASSERT(sizeof(struct sfs_super)==SFS_BLOCKSIZE);
	KASSERT(sizeof(struct sfs_inode)==SFS_BLOCKSIZE);
	KASSERT(SFS_BLOCKSIZE % sizeof(struct sfs_dir) == 0);

	/*
	 * We can't mount on devices with the wrong sector size.
	 *
	 * (Note: for all intents and purposes here, "sector" and
	 * "block" are interchangeable terms. Technically a filesystem
	 * block may be composed of several hardware sectors, but we
	 * don't do that in sfs.)
	 */
	if (dev->d_blocksize != SFS_BLOCKSIZE) {
		vfs_biglock_release();
		return ENXIO;
	}

	/* Allocate object */
	sfs = kmalloc(sizeof(struct sfs_fs));
	if (sfs==NULL) {
		vfs_biglock_release();
		return ENOMEM;
	}

	/* Allocate array */
	sfs->sfs_vnodes = vnodearray_create();
	if (sfs->sfs_vnodes == NULL) {
		kfree(sfs);
		vfs_biglock_release();
		return ENOMEM;
	}

	/* Set the device so we can use sfs_rblock() */
	sfs->sfs_device = dev;

	/* Load superblock */
	result = sfs_rblock(sfs, &sfs->sfs_super, SFS_SB_LOCATION);
	if (result) {
		vnodearray_destroy(sfs->sfs_vnodes);
		kfree(sfs);
		vfs_biglock_release();
		return result;
	}

	/* Make some simple sanity checks */

	if (sfs->sfs_super.sp_magic != SFS_MAGIC) {
		kprintf("sfs: Wrong magic number in superblock "
			"(0x%x, should be 0x%x)\n", 
			sfs->sfs_super.sp_magic,
			SFS_MAGIC);
		vnodearray_destroy(sfs->sfs_vnodes);
		kfree(sfs);
		vfs_biglock_release();
		return EINVAL;
	}
	
	if (sfs->sfs_super.sp_nblocks > dev->d_blocks) {
		kprintf("sfs: warning - fs has %u blocks, device has %u\n",
			sfs->sfs_super.sp_nblocks, dev->d_blocks);
	}

	/* Ensure null termination of the volume name */
	sfs->sfs_super.sp_volname[sizeof(sfs->sfs_super.sp_volname)-1] = 0;

	/* Load free space bitmap */
	sfs->sfs_freemap = bitmap_create(SFS_FS_BITMAPSIZE(sfs));
	if (sfs->sfs_freemap == NULL) {
		vnodearray_destroy(sfs->sfs_vnodes);
		kfree(sfs);
		vfs_biglock_release();
		return ENOMEM;
	}
	result = sfs_mapio(sfs, UIO_READ);
	if (result) {
		bitmap_destroy(sfs->sfs_freemap);
		vnodearray_destroy(sfs->sfs_vnodes);
		kfree(sfs);
		vfs_biglock_release();
		return result;
	}

		sfs->sfs_vnodes_lock=lock_create("vnodes lock");
		
		if(sfs->sfs_vnodes_lock == NULL) 
        {
		bitmap_destroy(sfs->sfs_freemap);
		vnodearray_destroy(sfs->sfs_vnodes);
		kfree(sfs);
		return ENOMEM;
     	}
     	
     	sfs->sfs_bitmap_lock = lock_create("bitmap lock");
        if(sfs->sfs_bitmap_lock == NULL) 
        {
                lock_destroy(sfs->sfs_vnodes_lock);
		bitmap_destroy(sfs->sfs_freemap);
		vnodearray_destroy(sfs->sfs_vnodes);
		kfree(sfs);
		return ENOMEM;
	}
	
	sfs->sfs_bmap_lock = lock_create("bmap lock");
        if(sfs->sfs_bmap_lock == NULL) 
        {
                lock_destroy(sfs->sfs_vnodes_lock);
                lock_destroy(sfs->sfs_bitmap_lock);
		bitmap_destroy(sfs->sfs_freemap);
		vnodearray_destroy(sfs->sfs_vnodes);
		kfree(sfs);
		return ENOMEM;
	}
	
	sfs->sfs_bcache_lock = lock_create("bcache lock");
        if(sfs->sfs_bcache_lock == NULL) 
        {
                lock_destroy(sfs->sfs_bmap_lock);
                lock_destroy(sfs->sfs_vnodes_lock);
                lock_destroy(sfs->sfs_bitmap_lock);
		bitmap_destroy(sfs->sfs_freemap);
		vnodearray_destroy(sfs->sfs_vnodes);
		kfree(sfs);
		return ENOMEM;
	}


	for(i = 0; i < BUFFER_SIZE; i++)
        {
            sfs->sfs_rcache[i].inode_no = -1; 
            sfs->sfs_rcache[i].block_no = -1; 
        }



	/* Set up abstract fs calls */
	sfs->sfs_absfs.fs_sync = sfs_sync;
	sfs->sfs_absfs.fs_getvolname = sfs_getvolname;
	sfs->sfs_absfs.fs_getroot = sfs_getroot;
	sfs->sfs_absfs.fs_unmount = sfs_unmount;
	sfs->sfs_absfs.fs_data = sfs;

	/* the other fields */
	sfs->sfs_superdirty = false;
	sfs->sfs_freemapdirty = false;

	/* Hand back the abstract fs */
	*ret = &sfs->sfs_absfs;

	vfs_biglock_release();
	return 0;
}

/*
 * Actual function called from high-level code to mount an sfs.
 */

int
sfs_mount(const char *device)
{
	return vfs_mount(device, NULL, sfs_domount);
}
