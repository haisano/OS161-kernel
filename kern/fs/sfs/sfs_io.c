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

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <vfs.h>
#include <device.h>
#include <sfs.h>

////////////////////////////////////////////////////////////
//
// Basic block-level I/O routines
//
// Note: sfs_rblock is used to read the superblock
// early in mount, before sfs is fully (or even mostly)
// initialized, and so may not use anything from sfs
// except sfs_device.

int
sfs_rwblock(struct sfs_fs *sfs, struct uio *uio)
{
	int result;
	int tries=0;
	struct sfs_vnode;// *sv = v->vn_data;
//	KASSERT(vfs_biglock_do_i_hold());

	DEBUG(DB_SFS, "sfs: %s %llu\n", 
	      uio->uio_rw == UIO_READ ? "read" : "write",
	      uio->uio_offset / SFS_BLOCKSIZE);

		  /*  lock_acquire(sfs->sfs_bcache_lock);
			for(i=0;i<BUFFER_SIZE;i++)
			{
				if( (sfs->sfs_rcache[i].inode_no == (int)sv->sv_ino) &&
				    (sfs->sfs_rcache[i].block_no == (int)diskblock) )
				{
						// Match Found 
					if(uio->uio_rw=="read")	
						result=memcpy(uio->uio_iov->iov_kbase,sfs->sfs_bcache[i]->cache,uio->uio_iov->iov_len);
					else
						result=memcpy(sfs->sfs_bcache[i]->cache,uio->uio_iov->uio_kbase,uio->uio_iov->iov_len);
						
					lock_release(sfs->sfs_bcache_lock);
					return 0;
				}
				
				else if((cache_slot==-1) &&(sfs->sfs_rcache[i].inode_no==-1))
  				{
  				 		// Free slot found in Cache 
  				 		cache_slot=i;
  				}
  			}*/
  		



 retry:
	result = sfs->sfs_device->d_io(sfs->sfs_device, uio);
	if (result == EINVAL) {
		/*
		 * This means the sector we requested was out of range,
		 * or the seek address we gave wasn't sector-aligned,
		 * or a couple of other things that are our fault.
		 */
		panic("sfs: d_io returned EINVAL\n");
	}
	if (result == EIO) {
		if (tries == 0) {
			tries++;
			kprintf("sfs: block %llu I/O error, retrying\n",
				uio->uio_offset / SFS_BLOCKSIZE);
			goto retry;
		}
		else if (tries < 10) {
			tries++;
			goto retry;
		}
		else {
			kprintf("sfs: block %llu I/O error, giving up after "
				"%d retries\n",
				uio->uio_offset / SFS_BLOCKSIZE, tries);
		}
	}
	
/*	if result==0)
	{
			if(cache_slot!=-1)
			{
				result=memcpy(sfs->sfs_bcache[cache_slot]->cache,uio->uio_iov->uio_kbase-SFS_BLOCKSIZE,uio->uio_iov->iov_len);
				sfs->sfs_bcache[cache_slot]->inode_no=uio(uio_offset-SFS_BLOCKSIZE);
				sfs->sfs_bcache[cache_slot]->block_no=((uio->uio_offset-SFS_BLOCKSIZE)/SFS_BLOCKSIZE);
				lock_release(sfs->sfs_bcache_lock);
				return 0;
			}
			else
			{
				// Buffer is Full - Replacement 
				cache_slot=(random() % BUFFER_SIZE);
				// Flush the buffer from a random slot
				result=memcpy((uio->uio_iov->iov_kbase-SFS_BLOCKSIZE),sfs->sfs_bcache[cache_slot]->cache,uio->uio_iov->iov_len);
				result=memcpy(sfs->sfs_bcache[cache_slot]->cache,uio->uio_iov->uio_kbase-SFS_BLOCKSIZE,uio->uio_iov->iov_len);	
				lock_release(sfs->sfs_bcache_lock);
				return 0;
			}		
			
	}*/
	//lock_release(sfs->sfs_bcache_lock);
	return result;

}

int
sfs_rblock(struct sfs_fs *sfs, void *data, uint32_t block)
{
	struct iovec iov;
	struct uio ku;

	SFSUIO(&iov, &ku, data, block, UIO_READ);
	return sfs_rwblock(sfs, &ku);
}

int
sfs_wblock(struct sfs_fs *sfs, void *data, uint32_t block)
{
	struct iovec iov;
	struct uio ku;

	SFSUIO(&iov, &ku, data, block, UIO_WRITE);
	return sfs_rwblock(sfs, &ku);
}
