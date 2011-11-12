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

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
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

#endif

#include "disk.h"

static
uint32_t
dumpsb(void)
{
	struct sfs_super sp;
	diskread(&sp, SFS_SB_LOCATION);
	if (SWAPL(sp.sp_magic) != SFS_MAGIC) {
		errx(1, "Not an sfs filesystem");
	}
	sp.sp_volname[sizeof(sp.sp_volname)-1] = 0;
	printf("Volume name: %-40s  %u blocks\n", sp.sp_volname, 
	       SWAPL(sp.sp_nblocks));

	return SWAPL(sp.sp_nblocks);
}

static
void
dodirblock(uint32_t block)
{
	struct sfs_dir sds[SFS_BLOCKSIZE/sizeof(struct sfs_dir)];
	int nsds = SFS_BLOCKSIZE/sizeof(struct sfs_dir);
	int i;

	diskread(&sds, block);

	printf("    [block %u]\n", block);
	for (i=0; i<nsds; i++) {
		uint32_t ino = SWAPL(sds[i].sfd_ino);
		if (ino==SFS_NOINO) {
			printf("        [free entry]\n");
		}
		else {
			sds[i].sfd_name[SFS_NAMELEN-1] = 0; /* just in case */
			printf("        %u %s\n", ino, sds[i].sfd_name);
		}
	}
}

static
void
dumpdir(uint32_t ino)
{
	struct sfs_inode sfi;
	uint32_t ib[SFS_DBPERIDB];
	int nentries, i;
	uint32_t block, nblocks=0;

	diskread(&sfi, ino);

	nentries = SWAPL(sfi.sfi_size) / sizeof(struct sfs_dir);
	if (SWAPL(sfi.sfi_size) % sizeof(struct sfs_dir) != 0) {
		warnx("Warning: dir size is not a multiple of dir entry size");
	}
	printf("Directory %u: %d entries\n", ino, nentries);

	for (i=0; i<SFS_NDIRECT; i++) {
		block = SWAPL(sfi.sfi_direct[i]);
		if (block) {
			dodirblock(block);
			nblocks++;
		}
	}
	if (SWAPL(sfi.sfi_indirect)) {
		diskread(&ib, SWAPL(sfi.sfi_indirect));
		for (i=0; i<SFS_DBPERIDB; i++) {
			block = SWAPL(ib[i]);
			if (block) {
				dodirblock(block);
				nblocks++;
			}
		}
	}
	printf("    %u blocks in directory\n", nblocks);
}

static
void
dumpbits(uint32_t fsblocks)
{
	uint32_t nblocks = SFS_BITBLOCKS(fsblocks);
	uint32_t i, j;
	char data[SFS_BLOCKSIZE];

	printf("Freemap: %u blocks (%u %u %u)\n", nblocks, SFS_BITMAPSIZE(fsblocks), fsblocks, SFS_BLOCKBITS);

	for (i=0; i<nblocks; i++) {
		diskread(data, SFS_MAP_LOCATION+i);
		for (j=0; j<SFS_BLOCKSIZE; j++) {
			printf("%02x", (unsigned char)data[j]);
			if (j%32==31) {
				printf("\n");
			}
		}
	}
	printf("\n");
}

int
main(int argc, char **argv)
{
	uint32_t nblocks;

#ifdef HOST
	hostcompat_init(argc, argv);
#endif

	if (argc!=2) {
		errx(1, "Usage: dumpsfs device/diskfile");
	}

	opendisk(argv[1]);
	nblocks = dumpsb();
	dumpbits(nblocks);
	dumpdir(SFS_ROOT_LOCATION);

	closedisk();

	return 0;
}
