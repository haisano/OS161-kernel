 	
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
#include <spl.h>
#include <spinlock.h>
#include <thread.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>
#include <syscall.h>

/*
 * Dumb MIPS-only "VM system" that is intended to only be just barely
 * enough to struggle off the ground. You should replace all of this
 * code while doing the VM assignment. In fact, starting in that
 * assignment, this file is not included in your kernel!
 */

/* under dumbvm, always have 48k of user stack */
#define STACKPAGES    12
#define DUMBVM_STACKPAGES 12
/*
 * Wrap rma_stealmem in a spinlock.
 */
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
static int al_index;
//static int tlb_index;

static
paddr_t
getppages(unsigned long npages)
{
	paddr_t addr;
	spinlock_acquire(&stealmem_lock);
	addr = ram_stealmem(npages);
	spinlock_release(&stealmem_lock);
	return addr;
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t 
alloc_kpages(int npages)
{
	paddr_t pa;
	if(myvm_fl==0)
		pa=getppages(npages);				
	else
		pa=alloc_npages(npages,curthread->pid);
		if(npages>1)
			enter_status(PADDR_TO_KVADDR(pa),npages);
	if (pa==0) {
		return 0;
	}

	DEBUG(DB_VM, "dumbvm: alloc_kpages: %d\n", pa);

	return PADDR_TO_KVADDR(pa);
}

void enter_status(vaddr_t vaddr,int no_pages)
{
	alloc[al_index].vaddr=vaddr;
	alloc[al_index].npages=no_pages;
	al_index++;
}

void 
free_kpages(vaddr_t addr)
{
	/* nothing - leak the memory. */
	int index=check_in_alloc_status(addr);
	if(index!=0)
	{	
		for(int i=0;i < (int)alloc[index].npages; i++)	{	
			coremap_remove( (addr+(i*PAGE_SIZE))-MIPS_KSEG0);		
		}	
		alloc[index].npages=0;
		alloc[index].vaddr=0;
		for(int i=index;i < (int)al_index;i++) {
			alloc[index]=alloc[index+1];
		}
	}
}


int check_in_alloc_status(vaddr_t vaddr)
{	
	int i;
	for(i=0;i<al_index;i++)
	{
		if(alloc[i].vaddr==vaddr)
			return i;

	}
	return 0;
}

void tlb_invalidate(paddr_t paddr)
{
	uint32_t ehi,elo,i;
	for (i=0; i<NUM_TLB; i++) {
		tlb_read(&ehi, &elo, i);
		if ((elo & 0xfffff000) == (paddr & 0xfffff000))	{
			tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);		
		}
	}
}


void
vm_tlbshootdown_all(void)
{
	panic("dumbvm tried to do tlb shootdown?!\n");
}

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("dumbvm tried to do tlb shootdown?!\n");
}

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	//vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	paddr_t paddr;
	int i;
	uint32_t ehi, elo;
	struct addrspace *as;
	int spl;

	faultaddress &= PAGE_FRAME;

	DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);
	
	switch (faulttype) {
	    case VM_FAULT_READONLY:
		/* We always create pages read-write, so we can't get this */
		panic("dumbvm: got VM_FAULT_READONLY\n");
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		break;
	    default:
		return EINVAL;
	}

	as = curthread->t_addrspace;
	if (as == NULL) {
		/*
		 * No address space set up. This is probably a kernel
		 * fault early in boot. Return EFAULT so as to panic
		 * instead of getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

	/* Assert that the address space has been set up properly. */
	KASSERT(as->as_vbase1 != 0);
	KASSERT(as->as_npages1 != 0);
	KASSERT(as->as_vbase2 != 0);
	KASSERT(as->as_npages2 != 0);
	KASSERT((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	KASSERT((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);

	/*vbase1 = as->as_vbase1;
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;*/
	
	if(faultaddress <= MIPS_KSEG0)
	{
		
	  /*if((faultaddress > vbase1 && faultaddress < vtop1)||
	     (faultaddress > vbase2 && faultaddress < vtop2)||
	     (faultaddress > stackbase && faultaddress < stacktop)) {*/  

		paddr = tlb_fault_handler(faultaddress);
		if(paddr==0) {
			panic("VM: faultaddress not found %x\n",faultaddress);
		}
		/* make sure it's page-aligned */
		KASSERT((paddr & PAGE_FRAME) == paddr);

		/* Disable interrupts on this CPU while frobbing the TLB. */
		spl = splhigh();
		/* Check for Invalid Entries in the TLB
 		 * if none found replace a random entry
 		 */ 
		for (i=0; i<NUM_TLB; i++) {
			tlb_read(&ehi, &elo, i);
			if (elo & TLBLO_VALID)	{
				continue;
			}
			ehi = faultaddress;
			paddr = paddr & PAGE_FRAME;
			elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
			DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
			tlb_write(ehi, elo, i);
			splx(spl);
			return 0;
		}
    	      	ehi = faultaddress;
    		elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		tlb_random(ehi,elo);
		splx(spl);
		kprintf("VM_FAULT: TLB REplacement occurred 0x%x\n",faultaddress);
	}
	else
	{
		kprintf("VM_FAULT: Kernel Page 0x%x\n",faultaddress);
		return 0;
	}
	
	//return EFAULT;
	return 0;
}

struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		return NULL;
	}

	as->as_vbase1 = 0;
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	as->as_npages2 = 0;
	as->heaptop=0;
	as->heapbase=0;
	as->pid=curthread->pid;
	return as;

}

void
as_destroy(struct addrspace *as)
{
	kfree(as);
}

void
as_activate(struct addrspace *as)
{
	int i, spl;

	(void)as;
	/* Disable interrupts on this CPU while frobbing the TLB. */
	spl = splhigh();

	for (i=0; i<NUM_TLB; i++) {
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable)
{
	size_t npages; 
	/* Align the region. First, the base... */
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;
	/* ...and now the length. */
	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;
	npages = sz / PAGE_SIZE;
	/* We don't use these - all pages are read-write */
	(void)readable;
	(void)writeable;
	(void)executable;

	if(as->as_vbase1 == 0)
	{
		as->as_vbase1=vaddr;
		as->as_npages1=npages;
		as->heapbase= as->as_vbase2 + (as->as_npages2*PAGE_SIZE);
		as->heaptop=as->heapbase;
		return 0;
	}
	/*Data */

	if(as->as_vbase2 == 0)
	{
		as->as_vbase2=vaddr;
		as->as_npages2=npages;
		as->heapbase= as->as_vbase2 + (as->as_npages2*PAGE_SIZE);
		as->heaptop=as->heapbase;
		return 0;
		
	}


	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;
}

static
void
as_zero_region(paddr_t paddr, unsigned npages)
{

	(void)paddr;
	(void)npages;
	//bzero((void *)PADDR_TO_KVADDR(paddr), npages * PAGE_SIZE);
}

int
as_prepare_load(struct addrspace *as)
{

	uint32_t i;
	for(i=0;i<as->as_npages1;i++)
	{		
		paddr_t temp;
		if(myvm_fl==0)
			temp=getppages(1);	
		else
			temp=alloc_page(as->as_vbase1+i*PAGE_SIZE,as->pid);				
		if(temp == 0)
			return ENOMEM;		
		as_zero_region(temp,1);						
	}
	
	for(i=0;i<as->as_npages2;i++)
	{	
		paddr_t temp;
		if(myvm_fl==0)
			temp=getppages(1);	
		else
			temp=alloc_page(as->as_vbase2+i*PAGE_SIZE,as->pid);			
		if(temp == 0)
			return ENOMEM;		
		as_zero_region(temp,1);						
	}
	for(i = STACKPAGES ;i > 0; i--){
		paddr_t temp;
		if(myvm_fl == 0) 
			temp=getppages(1);	
		else
			temp=alloc_page(USERSTACK-(i*PAGE_SIZE),as->pid);	
		if(temp == 0)
			return ENOMEM;		
		as_zero_region(temp,1);
	}
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	(void)as;
	
	*stackptr = USERSTACK;
	return 0;
}

int as_copy(struct addrspace *old, struct addrspace **ret,int pid)
{	
	struct addrspace *new;
	uint32_t i;
	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}
	new->as_vbase1 = old->as_vbase1;
	new->as_npages1 = old->as_npages1;
	new->as_vbase2 = old->as_vbase2;
	new->as_npages2 = old->as_npages2;
	new->heapbase = new->as_vbase2 + (new->as_npages2*PAGE_SIZE);
	new->heaptop = new->heapbase;

	new->pid = curthread->pid;
	if (as_prepare_load(new)) {
		as_destroy(new);
		return ENOMEM;
	}

	for(i=0;i<old->as_npages1;i++)	{	
		memmove((void *) (PADDR_TO_KVADDR(find_pgtbl_pid(new->as_vbase1+(i*PAGE_SIZE), pid)) ) , (const void *)old->as_vbase1+(i*PAGE_SIZE),PAGE_SIZE);
	}  	
	for(i=0;i<old->as_npages2;i++)
	{		
		memmove((void *) (PADDR_TO_KVADDR(find_pgtbl_pid(new->as_vbase2+(i*PAGE_SIZE), pid)) ), (const void *)old->as_vbase2+(i*PAGE_SIZE),PAGE_SIZE);		
	}
	for(i = STACKPAGES; i > 0; i++) {
		memmove((void *) (PADDR_TO_KVADDR(find_pgtbl_pid(USERSTACK-(i*PAGE_SIZE), pid)) ), (const void *)USERSTACK-(i*PAGE_SIZE),PAGE_SIZE);	
	}  
    	for(;new->heaptop < old->heaptop;new->heaptop+=PAGE_SIZE) {
        	memmove((void *) (PADDR_TO_KVADDR(find_pgtbl_pid(new->heaptop, pid))), (const void *)new->heaptop,PAGE_SIZE);
    	}
	*ret=new;
	return 0;
}


int sbrk(int size, int *retval)
{
	struct addrspace *as;
	size_t no_pages;
	uint32_t i;
	as = curthread->t_addrspace;
	vaddr_t old_heaptop;
	old_heaptop=as->heaptop;
	if(size==0) {
		*retval = old_heaptop;
		return 0;
	}
	if(size<0) {
		*retval = -1;
		return EINVAL;
	}
	
	no_pages=(size/PAGE_SIZE)+1;
	
	if( (as->heaptop+(no_pages*PAGE_SIZE)) > (USERSTACK+(STACKPAGES*PAGE_SIZE)) ) {
		*retval = -1;
		return EINVAL;
	}

	for(i=0;i<no_pages;i++)
	{
		paddr_t temp;	
		if(myvm_fl==0)
			temp=getppages(1);	
		else
			temp=alloc_page(as->heaptop+(i*PAGE_SIZE),as->pid);			
		if(temp == 0) {
			*retval = -1;
			return ENOMEM;		
		}
	}
	as->heaptop=as->heaptop+(no_pages*PAGE_SIZE);
	*retval = old_heaptop;
	return 0;
}

void print_stats()
{
	kprintf("\n\n----------------------------------------------------VIRTUAL MEMORY STATS---------------------------------------------\n\n");
	kprintf("TLB FAULTS:%d\n",no_tlb);
	kprintf("PAGE FAULTS:%d\n",no_page);
	kprintf("PAGE FAULTS WITH SYNCHRONOUS WRITE:%d\n",no_page_write);
	kprintf("\n");
}

