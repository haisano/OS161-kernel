/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2009
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

#include <sys/types.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "support.h"
#include "kern/sfs.h"

#ifdef HOST
#include <netinet/in.h> // for arpa/inet.h
#include <arpa/inet.h>  // for ntohl
#include "hostcompat.h"
#define SWAPL(x) ntohl(x)
#define SWAPS(x) ntohs(x)

#else

#define SWAPL(x) (x)
#define SWAPS(x) (x)
#define NO_REALLOC
#define NO_QSORT

#endif

#include "disk.h"


#define EXIT_USAGE    4
#define EXIT_FATAL    3
#define EXIT_UNRECOV  2
#define EXIT_RECOV    1
#define EXIT_CLEAN    0

static int badness=0;

static
void
setbadness(int code)
{
	if (badness < code) {
		badness = code;
	}
}

////////////////////////////////////////////////////////////

static
void
swapsb(struct sfs_super *sp)
{
	sp->sp_magic = SWAPL(sp->sp_magic);
	sp->sp_nblocks = SWAPL(sp->sp_nblocks);
}

static
void
swapinode(struct sfs_inode *sfi)
{
	int i;

	sfi->sfi_size = SWAPL(sfi->sfi_size);
	sfi->sfi_type = SWAPS(sfi->sfi_type);
	sfi->sfi_linkcount = SWAPS(sfi->sfi_linkcount);

	for (i=0; i<SFS_NDIRECT; i++) {
		sfi->sfi_direct[i] = SWAPL(sfi->sfi_direct[i]);
	}

#ifdef SFS_NIDIRECT
	for (i=0; i<SFS_NIDIRECT; i++) {
		sfi->sfi_indirect[i] = SWAPL(sfi->sfi_indirect[i]);
	}
#else
	sfi->sfi_indirect = SWAPL(sfi->sfi_indirect);
#endif

#ifdef SFS_NDIDIRECT
	for (i=0; i<SFS_NDIDIRECT; i++) {
		sfi->sfi_dindirect[i] = SWAPL(sfi->sfi_dindirect[i]);
	}
#else
#ifdef HAS_DIDIRECT
	sfi->sfi_dindirect = SWAPL(sfi->sfi_dindirect);
#endif
#endif

#ifdef SFS_NTIDIRECT
	for (i=0; i<SFS_NTIDIRECT; i++) {
		sfi->sfi_tindirect[i] = SWAPL(sfi->sfi_tindirect[i]);
	}
#else
#ifdef HAS_TIDIRECT
	sfi->sfi_tindirect = SWAPL(sfi->sfi_tindirect);
#endif
#endif
}

static
void
swapdir(struct sfs_dir *sfd)
{
	sfd->sfd_ino = SWAPL(sfd->sfd_ino);
}

static
void
swapindir(uint32_t *entries)
{
	int i;
	for (i=0; i<SFS_DBPERIDB; i++) {
		entries[i] = SWAPL(entries[i]);
	}
}

static
void
swapbits(uint8_t *bits)
{
	/* nothing to do */
	(void)bits;
}

////////////////////////////////////////////////////////////

static
void *
domalloc(size_t len)
{
	void *x;
	x = malloc(len);
	if (x==NULL) {
		errx(EXIT_FATAL, "Out of memory");
	}
	return x;
}

////////////////////////////////////////////////////////////

typedef enum {
	B_SUPERBLOCK,	/* Block that is the superblock */
	B_BITBLOCK,	/* Block used by free-block bitmap */
	B_INODE,	/* Block that is an inode */
	B_IBLOCK,	/* Indirect (or doubly-indirect etc.) block */
	B_DIRDATA,	/* Data block of a directory */
	B_DATA,		/* Data block */
	B_TOFREE,	/* Block that was used but we are releasing */
	B_PASTEND,	/* Block off the end of the fs */
} blockusage_t;

static uint32_t nblocks, bitblocks;
static uint32_t uniquecounter = 1;

static unsigned long count_blocks=0, count_dirs=0, count_files=0;

////////////////////////////////////////////////////////////

static uint8_t *bitmapdata;
static uint8_t *tofreedata;

static
void
bitmap_init(uint32_t bitblocks)
{
	size_t i, mapsize = bitblocks * SFS_BLOCKSIZE;
	bitmapdata = domalloc(mapsize * sizeof(uint8_t));
	tofreedata = domalloc(mapsize * sizeof(uint8_t));
	for (i=0; i<mapsize; i++) {
		bitmapdata[i] = tofreedata[i] = 0;
	}
}

static
const char *
blockusagestr(blockusage_t how, uint32_t howdesc)
{
	static char rv[256];
	switch (how) {
	    case B_SUPERBLOCK: return "superblock";
	    case B_BITBLOCK: return "bitmap block";
	    case B_INODE: return "inode";
	    case B_IBLOCK: 
		snprintf(rv, sizeof(rv), "indirect block of inode %lu", 
			 (unsigned long) howdesc);
		break;
	    case B_DIRDATA:
		snprintf(rv, sizeof(rv), "directory data from inode %lu", 
			 (unsigned long) howdesc);
		break;
	    case B_DATA:
		snprintf(rv, sizeof(rv), "file data from inode %lu", 
			 (unsigned long) howdesc);
		break;
	    case B_TOFREE:
		assert(0);
		break;
	    case B_PASTEND:
		return "past the end of the fs";
	}
	return rv;
}

static
void
bitmap_mark(uint32_t block, blockusage_t how, uint32_t howdesc)
{
	unsigned index = block/8;
	uint8_t mask = ((uint8_t)1)<<(block%8);

	if (how == B_TOFREE) {
		if (tofreedata[index] & mask) {
			/* already marked to free once, ignore */
			return;
		}
		if (bitmapdata[index] & mask) {
			/* block is used elsewhere, ignore */
			return;
		}
		tofreedata[index] |= mask;
		return;
	}

	if (tofreedata[index] & mask) {
		/* really using the block, don't free it */
		tofreedata[index] &= ~mask;
	}

	if (bitmapdata[index] & mask) {
		warnx("Block %lu (used as %s) already in use! (NOT FIXED)",
		      (unsigned long) block, blockusagestr(how, howdesc));
		setbadness(EXIT_UNRECOV);
	}

	bitmapdata[index] |= mask;

	if (how != B_PASTEND) {
		count_blocks++;
	}
}

static
int
countbits(uint8_t val)
{
	uint8_t x;
	int ct=0;

	for (x=1; x; x<<=1) {
		if (val & x) ct++;
	}
	return ct;
}

static
void
reportbits(uint32_t bitblock, uint32_t byte, uint8_t val, const char *what)
{
	uint8_t x, y;
	uint32_t blocknum;

	for (x=1, y=0; x; x<<=1, y++) {
		if (val & x) {
			blocknum = bitblock*SFS_BLOCKBITS + byte*CHAR_BIT + y;
			warnx("Block %lu erroneously shown %s in bitmap",
			      (unsigned long) blocknum, what);
		}
	}
}

static
void
check_bitmap(void)
{
	uint8_t bits[SFS_BLOCKSIZE], *found, *tofree, tmp;
	uint32_t alloccount=0, freecount=0, i, j;
	int bchanged;

	for (i=0; i<bitblocks; i++) {
		diskread(bits, SFS_MAP_LOCATION+i);
		swapbits(bits);
		found = bitmapdata + i*SFS_BLOCKSIZE;
		tofree = tofreedata + i*SFS_BLOCKSIZE;
		bchanged = 0;

		for (j=0; j<SFS_BLOCKSIZE; j++) {
			/* we shouldn't have blocks marked both ways */
			assert((found[j] & tofree[j])==0);

			if (bits[j]==found[j]) {
				continue;
			}

			if (bits[j]==(found[j] | tofree[j])) {
				bits[j] = found[j];
				bchanged = 1;
				continue;
			}

			/* free the ones we're freeing */
			bits[j] &= ~tofree[j];

			/* are we short any? */
			if ((bits[j] & found[j]) != found[j]) {
				tmp = found[j] & ~bits[j];
				alloccount += countbits(tmp);
				if (tmp != 0) {
					reportbits(i, j, tmp, "free");
				}
			}

			/* do we have any extra? */
			if ((bits[j] & found[j]) != bits[j]) {
				tmp = bits[j] & ~found[j];
				freecount += countbits(tmp);
				if (tmp != 0) {
					reportbits(i, j, tmp, "allocated");
				}
			}

			bits[j] = found[j];
			bchanged = 1;
		}

		if (bchanged) {
			swapbits(bits);
			diskwrite(bits, SFS_MAP_LOCATION+i);
		}
	}

	if (alloccount > 0) {
		warnx("%lu blocks erroneously shown free in bitmap (fixed)",
		      (unsigned long) alloccount);
		setbadness(EXIT_RECOV);
	}
	if (freecount > 0) {
		warnx("%lu blocks erroneously shown used in bitmap (fixed)",
		      (unsigned long) freecount);
		setbadness(EXIT_RECOV);
	}
}

////////////////////////////////////////////////////////////

struct inodememory {
	uint32_t ino;
	uint32_t linkcount;	/* files only; 0 for dirs */
};

static struct inodememory *inodes = NULL;
static int ninodes=0, maxinodes=0;

static
void
addmemory(uint32_t ino, uint32_t linkcount)
{
	assert(ninodes <= maxinodes);
	if (ninodes == maxinodes) {
#ifdef NO_REALLOC
		int newmax = (maxinodes+1)*2;
		void *p = domalloc(newmax * sizeof(struct inodememory));
		if (inodes) {
			memcpy(p, inodes, ninodes);
			free(inodes);
		}
		inodes = p;
#else
		maxinodes = (maxinodes+1)*2;
		inodes = realloc(inodes, maxinodes * sizeof(uint32_t));
		if (inodes==NULL) {
			errx(EXIT_FATAL, "Out of memory");
		}
#endif
	}
	inodes[ninodes].ino = ino;
	inodes[ninodes].linkcount = linkcount;
}

/* returns nonzero if directory already remembered */
static
int
remember_dir(uint32_t ino, const char *pathsofar)
{
	int i;

	/* don't use this for now */
	(void)pathsofar;

	for (i=0; i<ninodes; i++) {
		if (inodes[i].ino==ino) {
			assert(inodes[i].linkcount==0);
			return 1;
		}
	}

	addmemory(ino, 0);

	return 0;
}

static
void
observe_filelink(uint32_t ino)
{
	int i;
	for (i=0; i<ninodes; i++) {
		if (inodes[i].ino==ino) {
			assert(inodes[i].linkcount>0);
			inodes[i].linkcount++;
			return;
		}
	}
	bitmap_mark(ino, B_INODE, ino);
	addmemory(ino, 1);
}

static
void
adjust_filelinks(void)
{
	struct sfs_inode sfi;
	int i;

	for (i=0; i<ninodes; i++) {
		if (inodes[i].linkcount==0) {
			/* directory */
			continue;
		}
		diskread(&sfi, inodes[i].ino);
		swapinode(&sfi);
		assert(sfi.sfi_type == SFS_TYPE_FILE);
		if (sfi.sfi_linkcount != inodes[i].linkcount) {
			warnx("File %lu link count %lu should be %lu (fixed)",
			      (unsigned long) inodes[i].ino,
			      (unsigned long) sfi.sfi_linkcount,
			      (unsigned long) inodes[i].linkcount);
			sfi.sfi_linkcount = inodes[i].linkcount;
			setbadness(EXIT_RECOV);
			swapinode(&sfi);
			diskwrite(&sfi, inodes[i].ino);
		}
		count_files++;
	}
}

////////////////////////////////////////////////////////////

static
int
checknullstring(char *buf, size_t maxlen)
{
	size_t i;
	for (i=0; i<maxlen; i++) {
		if (buf[i]==0) {
			return 0;
		}
	}
	buf[maxlen-1] = 0;
	return 1;
}

static
int
checkbadstring(char *buf)
{
	size_t i;
	int rv=0;

	for (i=0; buf[i]; i++) {
		if (buf[i]==':' || buf[i]=='/') {
			buf[i] = '_';
			rv = 1;
		}
	}
	return rv;
}

////////////////////////////////////////////////////////////

static
void
check_sb(void)
{
	struct sfs_super sp;
	uint32_t i;
	int schanged=0;

	diskread(&sp, SFS_SB_LOCATION);
	swapsb(&sp);
	if (sp.sp_magic != SFS_MAGIC) {
		errx(EXIT_UNRECOV, "Not an sfs filesystem");
	}

	assert(nblocks==0);
	assert(bitblocks==0);
	nblocks = sp.sp_nblocks;
	bitblocks = SFS_BITBLOCKS(nblocks);
	assert(nblocks>0);
	assert(bitblocks>0);

	bitmap_init(bitblocks);
	for (i=nblocks; i<bitblocks*SFS_BLOCKBITS; i++) {
		bitmap_mark(i, B_PASTEND, 0);
	}

	if (checknullstring(sp.sp_volname, sizeof(sp.sp_volname))) {
		warnx("Volume name not null-terminated (fixed)");
		setbadness(EXIT_RECOV);
		schanged = 1;
	}
	if (checkbadstring(sp.sp_volname)) {
		warnx("Volume name contains illegal characters (fixed)");
		setbadness(EXIT_RECOV);
		schanged = 1;
	}

	if (schanged) {
		swapsb(&sp);
		diskwrite(&sp, SFS_SB_LOCATION);
	}

	bitmap_mark(SFS_SB_LOCATION, B_SUPERBLOCK, 0);
	for (i=0; i<bitblocks; i++) {
		bitmap_mark(SFS_MAP_LOCATION+i, B_BITBLOCK, i);
	}
}

////////////////////////////////////////////////////////////

static
void
check_indirect_block(uint32_t ino, uint32_t *ientry, uint32_t *blockp,
		     uint32_t nblocks, uint32_t *badcountp, 
		     int isdir, int indirection)
{
	uint32_t entries[SFS_DBPERIDB];
	uint32_t i, ct;

	if (*ientry !=0) {
		diskread(entries, *ientry);
		swapindir(entries);
		bitmap_mark(*ientry, B_IBLOCK, ino);
	}
	else {
		for (i=0; i<SFS_DBPERIDB; i++) {
			entries[i] = 0;
		}
	}

	if (indirection > 1) {
		for (i=0; i<SFS_DBPERIDB; i++) {
			check_indirect_block(ino, &entries[i], 
					     blockp, nblocks, 
					     badcountp,
					     isdir,
					     indirection-1);
		}
	}
	else {
		assert(indirection==1);

		for (i=0; i<SFS_DBPERIDB; i++) {
			if (*blockp < nblocks) {
				if (entries[i] != 0) {
					bitmap_mark(entries[i],
						    isdir ? B_DIRDATA : B_DATA,
						    ino);
				}
			}
			else {
				if (entries[i] != 0) {
					(*badcountp)++;
					bitmap_mark(entries[i],
						    isdir ? B_DIRDATA : B_DATA,
						    ino);
					entries[i] = 0;
				}
			}
			(*blockp)++;
		}
	}

	ct=0;
	for (i=ct=0; i<SFS_DBPERIDB; i++) {
		if (entries[i]!=0) ct++;
	}
	if (ct==0) {
		if (*ientry != 0) {
			(*badcountp)++;
			bitmap_mark(*ientry, B_TOFREE, 0);
			*ientry = 0;
		}
	}
	else {
		assert(*ientry != 0);
		if (*badcountp > 0) {
			swapindir(entries);
			diskwrite(entries, *ientry);
		}
	}
}

/* returns nonzero if inode modified */
static
int
check_inode_blocks(uint32_t ino, struct sfs_inode *sfi, int isdir)
{
	uint32_t size, block, nblocks, badcount;

	badcount = 0;

	size = SFS_ROUNDUP(sfi->sfi_size, SFS_BLOCKSIZE);
	nblocks = size/SFS_BLOCKSIZE;

	for (block=0; block<SFS_NDIRECT; block++) {
		if (block < nblocks) {
			if (sfi->sfi_direct[block] != 0) {
				bitmap_mark(sfi->sfi_direct[block],
					    isdir ? B_DIRDATA : B_DATA, ino);
			}
		}
		else {
			if (sfi->sfi_direct[block] != 0) {
				badcount++;
				bitmap_mark(sfi->sfi_direct[block],
					    B_TOFREE, 0);
			}			
		}
	}

#ifdef SFS_NIDIRECT
	for (i=0; i<SFS_NIDIRECT; i++) {
		check_indirect_block(ino, &sfi->sfi_indirect[i], 
				     &block, nblocks, &badcount, isdir, 1);
	}
#else
	check_indirect_block(ino, &sfi->sfi_indirect, 
			     &block, nblocks, &badcount, isdir, 1);
#endif

#ifdef SFS_NDIDIRECT
	for (i=0; i<SFS_NDIDIRECT; i++) {
		check_indirect_block(ino, &sfi->sfi_dindirect[i], 
				     &block, nblocks, &badcount, isdir, 2);
	}
#else
#ifdef HAS_DIDIRECT
	check_indirect_block(ino, &sfi->sfi_dindirect, 
			     &block, nblocks, &badcount, isdir, 2);
#endif
#endif

#ifdef SFS_NTIDIRECT
	for (i=0; i<SFS_NTIDIRECT; i++) {
		check_indirect_block(ino, &sfi->sfi_tindirect[i], 
				     &block, nblocks, &badcount, isdir, 3);
	}
#else
#ifdef HAS_TIDIRECT
	check_indirect_block(ino, &sfi->sfi_tindirect, 
			     &block, nblocks, &badcount, isdir, 3);
#endif
#endif

	if (badcount > 0) {
		warnx("Inode %lu: %lu blocks after EOF (freed)", 
		     (unsigned long) ino, (unsigned long) badcount);
		setbadness(EXIT_RECOV);
		return 1;
	}

	return 0;
}

////////////////////////////////////////////////////////////

static
uint32_t
ibmap(uint32_t iblock, uint32_t offset, uint32_t entrysize)
{
	uint32_t entries[SFS_DBPERIDB];

	if (iblock == 0) {
		return 0;
	}

	diskread(entries, iblock);
	swapindir(entries);

	if (entrysize > 1) {
		uint32_t index = offset / entrysize;
		offset %= entrysize;
		return ibmap(entries[index], offset, entrysize/SFS_DBPERIDB);
	}
	else {
		assert(offset < SFS_DBPERIDB);
		return entries[offset];
	}
}

#define BMAP_ND   		SFS_NDIRECT
#define BMAP_D(sfi, x)		((sfi)->sfi_direct[(x)])

#ifdef SFS_NIDIRECT
#define BMAP_NI			SFS_NIDIRECT
#define BMAP_I(sfi, x)		((sfi)->sfi_indirect[(x)])
#else
#define BMAP_NI			1
#define BMAP_I(sfi, x)		((void)(x), (sfi)->sfi_indirect)
#endif

#ifdef SFS_NDIDIRECT
#define BMAP_NII		SFS_NDIDIRECT
#define BMAP_II(sfi, x)		((sfi)->sfi_dindirect[(x)])
#else
#ifdef HAS_DIDIRECT
#define BMAP_NII		1
#define BMAP_II(sfi, x)		((void)(x), (sfi)->sfi_dindirect)
#else
#define BMAP_NII		0
#define BMAP_II(sfi, x)		((void)(x), (void)(sfi), 0)
#endif
#endif

#ifdef SFS_NTIDIRECT
#define BMAP_NIII		SFS_NTIDIRECT
#define BMAP_III(sfi, x)	((sfi)->sfi_tindirect[(x)])
#else
#ifdef HAS_TIDIRECT
#define BMAP_NIII		1
#define BMAP_III(sfi, x)	((void)(x), (sfi)->sfi_tindirect)
#else
#define BMAP_NIII		0
#define BMAP_III(sfi, x)	((void)(x), (void)(sfi), 0)
#endif
#endif

#define BMAP_DMAX   BMAP_ND
#define BMAP_IMAX   (BMAP_DMAX+SFS_DBPERIDB*BMAP_NI)
#define BMAP_IIMAX  (BMAP_IMAX+SFS_DBPERIDB*BMAP_NII)
#define BMAP_IIIMAX (BMAP_IIMAX+SFS_DBPERIDB*BMAP_NIII)

#define BMAP_DSIZE	1
#define BMAP_ISIZE	(BMAP_DSIZE*SFS_DBPERIDB)
#define BMAP_IISIZE	(BMAP_ISIZE*SFS_DBPERIDB)
#define BMAP_IIISIZE	(BMAP_IISIZE*SFS_DBPERIDB)

static
uint32_t
dobmap(const struct sfs_inode *sfi, uint32_t fileblock)
{
	uint32_t iblock, offset;

	if (fileblock < BMAP_DMAX) {
		return BMAP_D(sfi, fileblock);
	}
	else if (fileblock < BMAP_IMAX) {
		iblock = (fileblock - BMAP_DMAX)/BMAP_ISIZE;
		offset = (fileblock - BMAP_DMAX)%BMAP_ISIZE;
		return ibmap(BMAP_I(sfi, iblock), offset, BMAP_DSIZE);
	}
	else if (fileblock < BMAP_IIMAX) {
		iblock = (fileblock - BMAP_IMAX)/BMAP_IISIZE;
		offset = (fileblock - BMAP_IMAX)%BMAP_IISIZE;
		return ibmap(BMAP_II(sfi, iblock), offset, BMAP_ISIZE);
	}
	else if (fileblock < BMAP_IIIMAX) {
		iblock = (fileblock - BMAP_IIMAX)/BMAP_IIISIZE;
		offset = (fileblock - BMAP_IIMAX)%BMAP_IIISIZE;
		return ibmap(BMAP_III(sfi, iblock), offset, BMAP_IISIZE);
	}
	return 0;
}

static
void
dirread(struct sfs_inode *sfi, struct sfs_dir *d, unsigned nd)
{
	const unsigned atonce = SFS_BLOCKSIZE/sizeof(struct sfs_dir);
	unsigned nblocks = SFS_ROUNDUP(nd, atonce) / atonce;
	unsigned i, j;

	for (i=0; i<nblocks; i++) {
		uint32_t block = dobmap(sfi, i);
		if (block!=0) {
			diskread(d + i*atonce, block);
			for (j=0; j<atonce; j++) {
				swapdir(&d[i*atonce+j]);
			}
		}
		else {
			warnx("Warning: sparse directory found");
			bzero(d + i*atonce, SFS_BLOCKSIZE);
		}
	}
}

static
void
dirwrite(const struct sfs_inode *sfi, struct sfs_dir *d, int nd)
{
	const unsigned atonce = SFS_BLOCKSIZE/sizeof(struct sfs_dir);
	unsigned nblocks = SFS_ROUNDUP(nd, atonce) / atonce;
	unsigned i, j, bad;

	for (i=0; i<nblocks; i++) {
		uint32_t block = dobmap(sfi, i);
		if (block!=0) {
			for (j=0; j<atonce; j++) {
				swapdir(&d[i*atonce+j]);
			}
			diskwrite(d + i*atonce, block);
		}
		else {
			for (j=bad=0; j<atonce; j++) {
				if (d[i*atonce+j].sfd_ino != SFS_NOINO ||
				    d[i*atonce+j].sfd_name[0] != 0) {
					bad = 1;
				}
			}
			if (bad) {
				warnx("Cannot write to missing block in "
				      "sparse directory (ERROR)");
				setbadness(EXIT_UNRECOV);
			}
		}
	}
}

////////////////////////////////////////////////////////////

static struct sfs_dir *global_sortdirs;
static
int
dirsortfunc(const void *aa, const void *bb)
{
	const int *a = (const int *)aa;
	const int *b = (const int *)bb;
	const struct sfs_dir *ad = &global_sortdirs[*a];
	const struct sfs_dir *bd = &global_sortdirs[*b];
	return strcmp(ad->sfd_name, bd->sfd_name);
}

#ifdef NO_QSORT
static
void
qsort(int *data, int num, size_t size, int (*f)(const void *, const void *))
{
	int i, j;
	(void)size;

	/* because I'm lazy, bubble sort */
	for (i=0; i<num-1; i++) {
		for (j=i+1; j<num; j++) {
			if (f(&data[i], &data[j]) < 0) {
				int tmp = data[i];
				data[i] = data[j];
				data[j] = tmp;
			}
		}
	}
}
#endif

static
void
sortdir(int *vector, struct sfs_dir *d, int nd)
{
	global_sortdirs = d;
	qsort(vector, nd, sizeof(int), dirsortfunc);
}

/* tries to add a directory entry; returns 0 on success */
static
int
dir_tryadd(struct sfs_dir *d, int nd, const char *name, uint32_t ino)
{
	int i;
	for (i=0; i<nd; i++) {
		if (d[i].sfd_ino==SFS_NOINO) {
			d[i].sfd_ino = ino;
			assert(strlen(name) < sizeof(d[i].sfd_name));
			strcpy(d[i].sfd_name, name);
			return 0;
		}
	}
	return -1;
}

static
int
check_dir_entry(const char *pathsofar, uint32_t index, struct sfs_dir *sfd)
{
	int dchanged = 0;

	if (sfd->sfd_ino == SFS_NOINO) {
		if (sfd->sfd_name[0] != 0) {
			setbadness(EXIT_RECOV);
			warnx("Directory /%s entry %lu has name but no file",
			      pathsofar, (unsigned long) index);
			sfd->sfd_name[0] = 0;
			dchanged = 1;
		}
	}
	else {
		if (sfd->sfd_name[0] == 0) {
			snprintf(sfd->sfd_name, sizeof(sfd->sfd_name),
				 "FSCK.%lu.%lu",
				 (unsigned long) sfd->sfd_ino,
				 (unsigned long) uniquecounter++);
			setbadness(EXIT_RECOV);
			warnx("Directory /%s entry %lu has file but "
			      "no name (fixed: %s)",
			      pathsofar, (unsigned long) index,
			      sfd->sfd_name);
			dchanged = 1;
		}
		if (checknullstring(sfd->sfd_name, sizeof(sfd->sfd_name))) {
			setbadness(EXIT_RECOV);
			warnx("Directory /%s entry %lu not "
			      "null-terminated (fixed)",
			      pathsofar, (unsigned long) index);
			dchanged = 1;
		}
		if (checkbadstring(sfd->sfd_name)) {
			setbadness(EXIT_RECOV);
			warnx("Directory /%s entry %lu contains invalid "
			      "characters (fixed)",
			      pathsofar, (unsigned long) index);
			dchanged = 1;
		}
	}
	return dchanged;
}

////////////////////////////////////////////////////////////

static
int
check_dir(uint32_t ino, uint32_t parentino, const char *pathsofar)
{
	struct sfs_inode sfi;
	struct sfs_dir *direntries;
	int *sortvector;
	uint32_t dirsize, ndirentries, maxdirentries, subdircount, i;
	int ichanged=0, dchanged=0, dotseen=0, dotdotseen=0;

	diskread(&sfi, ino);
	swapinode(&sfi);

	if (remember_dir(ino, pathsofar)) {
		/* crosslinked dir */
		return 1;
	}

	bitmap_mark(ino, B_INODE, ino);
	count_dirs++;

	if (sfi.sfi_size % sizeof(struct sfs_dir) != 0) {
		setbadness(EXIT_RECOV);
		warnx("Directory /%s has illegal size %lu (fixed)",
		      pathsofar, (unsigned long) sfi.sfi_size);
		sfi.sfi_size = SFS_ROUNDUP(sfi.sfi_size, 
					   sizeof(struct sfs_dir));
		ichanged = 1;
	}

	if (check_inode_blocks(ino, &sfi, 1)) {
		ichanged = 1;
	}

	ndirentries = sfi.sfi_size/sizeof(struct sfs_dir);
	maxdirentries = SFS_ROUNDUP(ndirentries, 
				    SFS_BLOCKSIZE/sizeof(struct sfs_dir));
	dirsize = maxdirentries * sizeof(struct sfs_dir);
	direntries = domalloc(dirsize);
	sortvector = domalloc(ndirentries * sizeof(int));

	dirread(&sfi, direntries, ndirentries);
	for (i=ndirentries; i<maxdirentries; i++) {
		direntries[i].sfd_ino = SFS_NOINO;
		bzero(direntries[i].sfd_name, sizeof(direntries[i].sfd_name));
	}

	for (i=0; i<ndirentries; i++) {
		if (check_dir_entry(pathsofar, i, &direntries[i])) {
			dchanged = 1;
		}
		sortvector[i] = i;
	}

	sortdir(sortvector, direntries, ndirentries);

	/* don't use ndirentries-1 here in case ndirentries == 0 */
	for (i=0; i+1<ndirentries; i++) {
		struct sfs_dir *d1 = &direntries[sortvector[i]];
		struct sfs_dir *d2 = &direntries[sortvector[i+1]];
		assert(d1 != d2);

		if (d1->sfd_ino == SFS_NOINO) {
			continue;
		}

		if (!strcmp(d1->sfd_name, d2->sfd_name)) {
			if (d1->sfd_ino == d2->sfd_ino) {
				setbadness(EXIT_RECOV);
				warnx("Directory /%s: Duplicate entries for "
				      "%s (merged)",
				      pathsofar, d1->sfd_name);
				d1->sfd_ino = SFS_NOINO;
				d1->sfd_name[0] = 0;
			}
			else {
				snprintf(d1->sfd_name, sizeof(d1->sfd_name),
					 "FSCK.%lu.%lu",
					 (unsigned long) d1->sfd_ino,
					 (unsigned long) uniquecounter++);
				setbadness(EXIT_RECOV);
				warnx("Directory /%s: Duplicate names %s "
				      "(one renamed: %s)",
				      pathsofar, d2->sfd_name, d1->sfd_name);
			}
			dchanged = 1;
		}
	}

	for (i=0; i<ndirentries; i++) {
		if (!strcmp(direntries[i].sfd_name, ".")) {
			if (direntries[i].sfd_ino != ino) {
				setbadness(EXIT_RECOV);
				warnx("Directory /%s: Incorrect `.' entry "
				      "(fixed)", pathsofar);
				direntries[i].sfd_ino = ino;
				dchanged = 1;
			}
			assert(dotseen==0); /* due to duplicate checking */
			dotseen = 1;
		}
		else if (!strcmp(direntries[i].sfd_name, "..")) {
			if (direntries[i].sfd_ino != parentino) {
				setbadness(EXIT_RECOV);
				warnx("Directory /%s: Incorrect `..' entry "
				      "(fixed)", pathsofar);
				direntries[i].sfd_ino = parentino;
				dchanged = 1;
			}
			assert(dotdotseen==0); /* due to duplicate checking */
			dotdotseen = 1;
		}
	}

	if (!dotseen) {
		if (dir_tryadd(direntries, ndirentries, ".", ino)==0) {
			setbadness(EXIT_RECOV);
			warnx("Directory /%s: No `.' entry (added)",
			      pathsofar);
			dchanged = 1;
		}
		else if (dir_tryadd(direntries, maxdirentries, ".", ino)==0) {
			setbadness(EXIT_RECOV);
			warnx("Directory /%s: No `.' entry (added)",
			      pathsofar);
			ndirentries++;
			dchanged = 1;
			sfi.sfi_size += sizeof(struct sfs_dir);
			ichanged = 1;
		}
		else {
			setbadness(EXIT_UNRECOV);
			warnx("Directory /%s: No `.' entry (NOT FIXED)",
			      pathsofar);
		}
	}

	if (!dotdotseen) {
		if (dir_tryadd(direntries, ndirentries, "..", parentino)==0) {
			setbadness(EXIT_RECOV);
			warnx("Directory /%s: No `..' entry (added)",
			      pathsofar);
			dchanged = 1;
		}
		else if (dir_tryadd(direntries, maxdirentries, "..", 
				    parentino)==0) {
			setbadness(EXIT_RECOV);
			warnx("Directory /%s: No `..' entry (added)",
			      pathsofar);
			ndirentries++;
			dchanged = 1;
			sfi.sfi_size += sizeof(struct sfs_dir);
			ichanged = 1;
		}
		else {
			setbadness(EXIT_UNRECOV);
			warnx("Directory /%s: No `..' entry (NOT FIXED)",
			      pathsofar);
		}
	}

	subdircount=0;
	for (i=0; i<ndirentries; i++) {
		if (!strcmp(direntries[i].sfd_name, ".")) {
			/* nothing */
		}
		else if (!strcmp(direntries[i].sfd_name, "..")) {
			/* nothing */
		}
		else if (direntries[i].sfd_ino == SFS_NOINO) {
			/* nothing */
		}
		else {
			char path[strlen(pathsofar)+SFS_NAMELEN+1];
			struct sfs_inode subsfi;

			diskread(&subsfi, direntries[i].sfd_ino);
			swapinode(&subsfi);
			snprintf(path, sizeof(path), "%s/%s", 
				 pathsofar, direntries[i].sfd_name);

			switch (subsfi.sfi_type) {
			    case SFS_TYPE_FILE:
				if (check_inode_blocks(direntries[i].sfd_ino,
						       &subsfi, 0)) {
					swapinode(&subsfi);
					diskwrite(&subsfi, 
						  direntries[i].sfd_ino);
				}
				observe_filelink(direntries[i].sfd_ino);
				break;
			    case SFS_TYPE_DIR:
				if (check_dir(direntries[i].sfd_ino,
					      ino,
					      path)) {
					setbadness(EXIT_RECOV);
					warnx("Directory /%s: Crosslink to "
					      "other directory (removed)",
					      path);
					direntries[i].sfd_ino = SFS_NOINO;
					direntries[i].sfd_name[0] = 0;
					dchanged = 1;
				}
				else {
					subdircount++;
				}
				break;
			    default:
				setbadness(EXIT_RECOV);
				warnx("Object /%s: Invalid inode type "
				      "(removed)", path);
				direntries[i].sfd_ino = SFS_NOINO;
				direntries[i].sfd_name[0] = 0;
				dchanged = 1;
				break;
			}
		}
	}

	if (sfi.sfi_linkcount != subdircount+2) {
		setbadness(EXIT_RECOV);
		warnx("Directory /%s: Link count %lu should be %lu (fixed)",
		      pathsofar, (unsigned long) sfi.sfi_linkcount,
		      (unsigned long) subdircount+2);
		sfi.sfi_linkcount = subdircount+2;
		ichanged = 1;
	}

	if (dchanged) {
		dirwrite(&sfi, direntries, ndirentries);
	}

	if (ichanged) {
		swapinode(&sfi);
		diskwrite(&sfi, ino);
	}

	free(direntries);
	free(sortvector);

	return 0;
}


static
void
check_root_dir(void)
{
	struct sfs_inode sfi;
	diskread(&sfi, SFS_ROOT_LOCATION);
	swapinode(&sfi);

	switch (sfi.sfi_type) {
	    case SFS_TYPE_DIR:
		break;
	    case SFS_TYPE_FILE:
		warnx("Root directory inode is a regular file (fixed)");
		goto fix;
	    default:
		warnx("Root directory inode has invalid type %lu (fixed)",
		      (unsigned long) sfi.sfi_type);
	    fix:
		setbadness(EXIT_RECOV);
		sfi.sfi_type = SFS_TYPE_DIR;
		swapinode(&sfi);
		diskwrite(&sfi, SFS_ROOT_LOCATION);
		break;
	}

	check_dir(SFS_ROOT_LOCATION, SFS_ROOT_LOCATION, "");
}

////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
#ifdef HOST
	hostcompat_init(argc, argv);
#endif

	if (argc!=2) {
		errx(EXIT_USAGE, "Usage: sfsck device/diskfile");
	}

	assert(sizeof(struct sfs_super)==SFS_BLOCKSIZE);
	assert(sizeof(struct sfs_inode)==SFS_BLOCKSIZE);
	assert(SFS_BLOCKSIZE % sizeof(struct sfs_dir) == 0);

	opendisk(argv[1]);

	check_sb();
	check_root_dir();
	check_bitmap();
	adjust_filelinks();

	closedisk();

	warnx("%lu blocks used (of %lu); %lu directories; %lu files",
	      count_blocks, (unsigned long) nblocks, count_dirs, count_files);

	switch (badness) {
	    case EXIT_USAGE:
	    case EXIT_FATAL:
	    default:
		/* not supposed to happen here */
		assert(0);
		break;
	    case EXIT_UNRECOV:
		warnx("WARNING - unrecoverable errors. Maybe try again?");
		break;
	    case EXIT_RECOV:
		warnx("Caution - filesystem modified. Run again for luck.");
		break;
	    case EXIT_CLEAN:
		break;
	}

	return badness;
}
