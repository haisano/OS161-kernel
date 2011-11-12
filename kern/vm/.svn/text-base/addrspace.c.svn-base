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
#include <addrspace.h>
#include <vm.h>
#include <bitmap.h>
#include <mainbus.h>
#include <vfs.h>
#include <vnode.h>
#include <current.h>
#include <synch.h>
#include <kern/fcntl.h>
#include <stat.h>
#include <uio.h>
#include <spl.h>
#include <generic/random.h>
#include <clock.h>
#include <mips/trapframe.h>

#define ISSWAPPED(x) ((x) & 0x00000080)
#define SET_SWAPPED(x) ((x) | 0x00000080)
#define ISDIRTY(x) ((x) & 0x00000400)
#define SET_DIRTY(x) ((x) | 0x00000400)
#define ISVALID(x) ((x) & 0x00000200)
#define SET_VALID(x) ((x) | 0x00000200)
#define ISKERNEL(x) ((x) & 0x00000001)
#define SET_KERNEL(x) ((x) | 0x00000001)

#define USE_LRU1 


struct vnode *swap_file;
struct page_table_entry *coremap;
struct page_table_entry *coreswap;
int myvm_fl=0;
uint32_t coremap_size;
uint32_t coreswap_size;
uint32_t coremap_start;

struct lock *vm_lock; 

void
vm_bootstrap(void)
{
	char fname[] = "lhd0raw:";
	vm_lock = lock_create("vm_lock");
	int result = vfs_open(fname, 0, O_RDWR , &swap_file);
	if(result)
    		panic("Virtual Memory: Swap space could not be created \n");
	alloc = (struct alloc_status*)kmalloc(64*sizeof(struct alloc_status));	
	coreswap_init();
	coremap_init();	
	myvm_fl=1;
}

void coremap_init() {
	int v_size = ( mainbus_ramsize()-(ram_stealmem(0) + PAGE_SIZE) )/PAGE_SIZE;
	mem_map = bitmap_create(v_size);
	coremap = (struct page_table_entry*)kmalloc(v_size * sizeof(struct page_table_entry));
	coremap_start = ram_stealmem(0);
	for(int i = 0; i < v_size; i++) {
		coremap[i].paddr = (coremap_start + (i * PAGE_SIZE));
	}
	coremap_size = v_size;
	kprintf("COREMAP INIT: %d %d\n",coremap_start,coremap_size);
}

void coreswap_init() {
	struct stat temp;
	VOP_STAT(swap_file, &temp);
	coreswap_size = temp.st_size/PAGE_SIZE;
	swap_map = bitmap_create(coreswap_size);
	coreswap = (struct page_table_entry*)kmalloc(coreswap_size * sizeof(struct page_table_entry));
	int coreswap_start=0;
	kprintf("COREMAP INIT: %d %d\n",coremap_start,coremap_size);
	for(int i = 0; i < temp.st_size/PAGE_SIZE; i++) {
		coreswap[i].paddr = (coreswap_start + (i * PAGE_SIZE));
	}
	kprintf("COREMAP INIT: %d %d\n",coremap_start,coremap_size);
}


/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */


/*
 * This routine handles a TLB FAULT
 * It searches the page table for the virtual address
 * If the swapped bit is not set, then pass that entry to TLB and all done
 * If swapped bit is set, then (call page fault handler)
 * 
 * PAGE FAULT HANDLER
 * 	- find a free page
 * 	- swap in the free page to memory
 */

uint32_t find_lru_page () {
	int spl=splhigh();
	uint32_t lfu_page;
	#if USE_LRU==0
	repeat:
		lfu_page = random()%coremap_size;
		if(ISKERNEL(coremap[lfu_page].paddr))
			goto repeat;
	#else
	time_t secs; uint32_t nanosecs;
	gettime(&secs, &nanosecs);
	uint32_t max=secs,maxn=nanosecs;
	for(int i=0;i< (int)coremap_size;i++){
		if(  !(ISKERNEL(coremap[i].paddr))
		     && (coremap[i].counter <= max)
		     && (coremap[i].ncounter <= maxn){
			max=coremap[i].counter;
			maxn = coremap[i].ncounter;
			lfu_page=i;
		}
	}
	#endif
	splx(spl);
	if(coremap[lfu_page].vaddr > USERSPACETOP)
		panic("VM LRU PAGE: SWAPPING OUT KERNEL PAGE");
	return(coremap[lfu_page].paddr);
}



/*
 *  insert and remove entry into coremap
 */


uint32_t coremap_insert_pid (uint32_t vaddr, uint32_t paddr, int pid) {
	int spl=splhigh();
	paddr = paddr & PAGE_FRAME;
	int map_index = (paddr - coremap_start) / PAGE_SIZE;
	KASSERT( (coremap[ map_index ].paddr & PAGE_FRAME) == paddr );
	coremap[ map_index ].vaddr = vaddr;
	if(vaddr > USERSPACETOP)
		coremap[ map_index ].paddr = SET_VALID(paddr)|SET_DIRTY(paddr)|SET_KERNEL(paddr);
	else
		coremap[ map_index ].paddr = SET_VALID(paddr)|SET_DIRTY(paddr);
	coremap[ map_index ].counter = 0;
	coremap[ map_index ].pid = pid;
	if(!bitmap_isset(mem_map,map_index))
		bitmap_mark(mem_map, map_index);
	splx(spl);
	return 0;
}

uint32_t coremap_insert (uint32_t vaddr, uint32_t paddr) {
	int spl=splhigh();
	paddr = paddr & PAGE_FRAME;
	int map_index = (paddr - coremap_start) / PAGE_SIZE;
	KASSERT( (coremap[ map_index ].paddr & PAGE_FRAME) == paddr );
	coremap[ map_index ].vaddr = vaddr;
	if(vaddr > USERSPACETOP)
		coremap[ map_index ].paddr = SET_VALID(paddr)|SET_DIRTY(paddr)|SET_KERNEL(paddr);
	else
		coremap[ map_index ].paddr = SET_VALID(paddr)|SET_DIRTY(paddr);
	coremap[ map_index ].counter = 0;
	coremap[ map_index ].pid = curthread->pid;
	if(!bitmap_isset(mem_map,map_index))
		bitmap_mark(mem_map, map_index);
	splx(spl);
	return 0;
}


uint32_t coremap_remove (uint32_t paddr) {
	int spl=splhigh();
	paddr = paddr & PAGE_FRAME;
	int map_index = (paddr - coremap_start) / PAGE_SIZE;
	KASSERT( (coremap[ map_index ].paddr & PAGE_FRAME) == (paddr & PAGE_FRAME) );
	coremap[ map_index ].vaddr = 0;
	coremap[ map_index ].paddr = paddr & PAGE_FRAME;
	coremap[ map_index ].counter = 0;
	coremap[ map_index ].pid = 0;
	bitmap_unmark(mem_map, map_index);
	splx(spl);	
	return 0;
}

uint32_t coremap_remove_pid(int pid) {
	for(int i=0; i < (int)coremap_size; i++) {
		if(coremap[i].pid == pid) {
			coremap_remove(coremap[i].paddr);
		}
	}
	return 0;
}

/*
 *  insert and remove entry into coreswap
 */
uint32_t coreswap_insert (uint32_t vaddr, uint32_t offset, int pid) {
	(void)pid;
	int map_index = (offset & PAGE_FRAME) / PAGE_SIZE;
	KASSERT( (coreswap[ map_index ].paddr & PAGE_FRAME) == offset );
	int spl=splhigh();
	coreswap[ map_index ].vaddr = vaddr;
	coreswap[ map_index ].counter = 0;
	coreswap[ map_index ].pid = curthread->pid;
	if(!bitmap_isset(swap_map,map_index))
		bitmap_mark(swap_map, map_index);
	splx(spl);
	return 0;
}

uint32_t coreswap_remove (uint32_t offset) {
	int map_index = (offset & PAGE_FRAME) / PAGE_SIZE;
	KASSERT( (coreswap[ map_index ].paddr & PAGE_FRAME) == offset );
	int spl=splhigh();
	coreswap[ map_index ].vaddr = 0;
	coreswap[ map_index ].paddr = offset;
	coreswap[ map_index ].counter = 0;
	coreswap[ map_index ].pid = 0;	
	bitmap_unmark(swap_map, map_index);
	splx(spl);
	return 0;
}


/*
 *  This is the main function for VM
 *  Search the mem_map to find a free page
 *  if nothing is free, Use the LRU routine to get a free page
 *  	Use the coremap to find the LRU page
 *	Mark the entries in the corresponding page table
 *	swap out the page
 *	return the free page
 */


uint32_t get_free_page() {
	int spl=splhigh();
	unsigned index;
	int result = bitmap_alloc(mem_map, &index);
	if(result) {
		uint32_t paddr = find_lru_page();
		if( ISVALID(paddr) ) {
			splx(spl);
			uint32_t offset = get_free_offset();
			swap_out(offset, paddr);
			//mark entry into swap_table
			/* possiblity of race condition */
			no_page_write++;
			return paddr;
		}
		panic("VM PANIC: LRU ERROR");
	}
	//add a check for ISVALID
	splx(spl);
	return (coremap[index].paddr);
}

uint32_t get_free_offset() {
	int spl=splhigh();
	unsigned index;
	int result = bitmap_alloc(swap_map, &index);
	if(result) {
		splx(spl);
		kprintf("VM: Out of Swap Space ----- Killing Current Thread");
		sys__exit(0);
	}
	splx(spl);
	return (index*PAGE_SIZE);
}


void swap_in(uint32_t paddr, uint32_t offset) {
	KASSERT(paddr >= coremap_start);
	int spl=splhigh();
	struct iovec swap_iov;
	struct uio swap_uio;
	uio_kinit(&swap_iov, &swap_uio, (void*)PADDR_TO_KVADDR(paddr & PAGE_FRAME), PAGE_SIZE, offset, UIO_READ);
	coreswap_remove(offset);
	splx(spl);
	//kprintf("SWAP IN: 0x%x %d %d\n",coreswap[(offset)/PAGE_SIZE].vaddr,paddr,offset);
	int result=VOP_READ(swap_file, &swap_uio);
	if(result) {
		panic("VM: SWAP in Failed");
	}
}

void swap_out(uint32_t offset, uint32_t paddr) {
	KASSERT(paddr >= coremap_start);
	int spl=splhigh();
	struct iovec swap_iov;
	struct uio swap_uio;
	uio_kinit(&swap_iov, &swap_uio, (void*)PADDR_TO_KVADDR(paddr & PAGE_FRAME), PAGE_SIZE, offset, UIO_WRITE);
	coreswap_insert(coremap[(paddr-coremap_start)/PAGE_SIZE].vaddr,offset,coremap[(paddr-coremap_start)/PAGE_SIZE].pid);
	splx(spl);
	//kprintf("SWAP OUT: 0x%x %d %d\n",coremap[(paddr-coremap_start)/PAGE_SIZE].vaddr,paddr,offset);
	int result=VOP_WRITE(swap_file, &swap_uio);
		if(result) {
		panic("VM: SWAP in Failed");
	}
	tlb_invalidate(paddr);
}


uint32_t find_pgtbl_pid(uint32_t vir_addr, int pid) {
	int i;
	int spl=splhigh();	
	for(i=0; i < (int)coremap_size; i++) {
		if(coremap[i].vaddr == vir_addr && (coremap[i].pid == pid || coremap[i].pid == 0)) {
			if(ISVALID(coremap[i].paddr)) {
			  //coremap[i].counter++;
				splx(spl);
				no_page++;
				return coremap[i].paddr;
			}
		}
		if(coremap[i].vaddr == vir_addr){
			splx(spl);
			panic("FIND PAGE TBL: address present but not matching pid 0x%x\n 0x%x %d %d \n",vir_addr,coremap[i].paddr,coremap[i].pid,curthread->pid);//debug
		}
	}
	no_page++;
	splx(spl);
	return( page_fault_handler(vir_addr) );

}

uint32_t find_pgtbl(uint32_t vir_addr) {
	int i;
	int spl=splhigh();	
	for(i=0; i < (int)coremap_size; i++) {
		if(coremap[i].vaddr == vir_addr && (coremap[i].pid == curthread->pid || coremap[i].pid == 0)) {
			if(ISVALID(coremap[i].paddr)) {
			  //coremap[i].counter++;
				splx(spl);
				no_tlb++;
				return coremap[i].paddr;
			}
		}
		if(coremap[i].vaddr == vir_addr){
			splx(spl);
			panic("FIND PAGE TBL: address present but not matching pid 0x%x\n 0x%x %d %d \n",vir_addr,coremap[i].paddr,coremap[i].pid,curthread->pid);//debug
		}
	}
	no_page++;
	splx(spl);
	return( page_fault_handler(vir_addr) );

}


uint32_t find_swap_table(uint32_t vir_addr) {
	int i;
	//kprintf("VM SWAP IN: 0x%x \n",vir_addr);
	int spl=splhigh();
	for(i=0; i < (int)coreswap_size; i++) {
		if(coreswap[i].vaddr == vir_addr && (coreswap[i].pid == curthread->pid || coreswap[i].pid == curthread->t_addrspace->pid)) {
		  //coreswap[i].counter++;
				splx(spl);
				no_page_write++;
				//kprintf("VM SWAP IN: 0x%x 0x%x \n",vir_addr,coreswap[i].paddr);
				return(coreswap[i].paddr);
		}
		if(coreswap[i].vaddr == vir_addr) {
			splx(spl);
			panic("FIND SWAP TBL: address present but not matching pid 0x%x\n 0x%x %d %d \n",vir_addr,coreswap[i].paddr,coreswap[i].pid,curthread->pid);//debug
		}
	}
	splx(spl);
	coredump();
	panic("VM: Illegal Address Exception, I'll Die now 0x%x!!\n\n",vir_addr);
	return 0;
}

void coredump() {
	
	print_stats();
	kprintf("\n---------------------------------coredump----------------------------------\n");	
	//dump the coremap and coreswap structure
	for(int i=0; i < (int)coremap_size; i++)
	{
		kprintf("| 0x%x = 0x%x |",coremap[i].vaddr,coremap[i].paddr);
	}
	kprintf("\n---------------------------------coredump----------------------------------\n");
	for(int i=0; i < (int)coreswap_size; i++)
	{
		kprintf("| 0x%x |",coreswap[i].vaddr);
	}
	kprintf("\n\n");

}


/*
 * The page faults means we need to swap in the page from DISK
 * We need to set {vaddr + OFFSET} and the swapped and valid flags in coremap
 * The counter for LFU needs to be reset and the PID needs to be set
 */

uint32_t tlb_fault_handler(uint32_t vir_addr) {
	uint32_t paddr = find_pgtbl(vir_addr & PAGE_FRAME);
	if( ISVALID(paddr) ) {
		time_t secs; uint32_t nanosecs;
		gettime(&secs, &nanosecs);
		int map_index = ((paddr & PAGE_FRAME)-coremap_start) / PAGE_SIZE;
		coremap[ map_index ].counter = (uint32_t)secs;
		coremap[ map_index ].ncounter = (uint32_t)nanosecs;
		return (paddr & PAGE_FRAME);
	}
	panic("VM: invalid page table fault 0x%x",vir_addr);
	return 0;
}

uint32_t page_fault_handler(uint32_t vir_addr) {
	uint32_t offset = find_swap_table(vir_addr);
	uint32_t paddr = get_free_page();
	swap_in(paddr,offset);
	paddr = SET_VALID(paddr);
	paddr = SET_SWAPPED(paddr);
	//int map_index = (paddr - coremap_start) / PAGE_SIZE;
	int spl=splhigh();
	coremap_insert_pid(vir_addr, paddr, coreswap[(offset & PAGE_FRAME)/PAGE_SIZE].pid);
	splx(spl);
	return paddr;	
}

/* Routine to Allocate a Single page */

uint32_t alloc_page(uint32_t vir_addr, int pid) {
	uint32_t paddr = get_free_page();
	//kprintf("Vm Alloc: 0x%x\n",vir_addr);
	paddr = SET_VALID(paddr);
	if( vir_addr >= MIPS_KSEG0)
		paddr = SET_KERNEL(paddr);
	coremap_insert_pid(vir_addr,paddr,pid);
	return paddr;
}

/* Routine to allocate n contiguous pages */

uint32_t alloc_npages(int n, int pid) {
	int spl=splhigh();
	unsigned index=0,count=0;
	unsigned swap_index=0,swap_count=0;
	unsigned best_count=0, best_index=0;

	if(n==1) {
		uint32_t paddr = get_free_page();
		paddr = SET_VALID(paddr);
		paddr = SET_KERNEL(paddr);
		coremap_insert( PADDR_TO_KVADDR(paddr), paddr);
		splx(spl);
		return (paddr & PAGE_FRAME);
	}

	/* find a big enough hole */
	for(int i = 0 ; i < (int)coremap_size; i++, count ++) {
		if( !(ISKERNEL(coremap[i].paddr)) ) {
			if( swap_count == 0) 
				swap_index = i;
			swap_count++;
			if ((int)swap_count >= n)
			{
				if(best_count > count) {
					best_count = count;
					best_index = swap_index;
				}
				swap_count = 0;
			}
		}
		else {
			swap_count = 0;
		}
		if( bitmap_isset(mem_map, i) )	{
			if((int)count == n) {
				break; 
			}
			continue;
		}
		else {
			index = i;
			count = 0;
		}
	
	}
	if((int)count < n) {
		if((int)best_count < n) {
			splx(spl);
			return 0;
		}
		else {
			/* elaborate plan to swap out the required number of contiguous pages
			 * if i am clever i should kick this process out and tell it take a hike
			 * but i am only a machine and have no emotions to act upon */
			for(int i=best_index; i < (int)(best_index + best_count); i++) {
				uint32_t check_paddr = coremap[i].paddr;
				//the page is valid
				if( ISVALID(check_paddr) ) {
					uint32_t offset = get_free_offset();
					//mark entry into swap_table
					splx(spl);
					swap_out(offset,check_paddr);
					/* possiblity of race condition */
					coremap_remove(check_paddr);
				}
			}
			index = best_index;	
		}
	}
	for(int i=index ; i < (int)(index + n); i++) {
		coremap_insert_pid(PADDR_TO_KVADDR(coremap[i].paddr), coremap[i].paddr, pid);
	}
	splx(spl);
	kprintf("VM Alloc N PAGES: 0x%x\n",coremap[index].paddr);
	return (coremap[index].paddr & PAGE_FRAME);
}


