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
 * Test code for kmalloc.
 */
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>
#include <test.h>

/*
 * Test kmalloc; allocate ITEMSIZE bytes NTRIES times, freeing
 * somewhat later.
 *
 * The total of ITEMSIZE * NTRIES is intended to exceed the size of
 * available memory.
 *
 * mallocstress does the same thing, but from NTHREADS different
 * threads at once.
 */

#define NTRIES   1200
#define ITEMSIZE  997
#define NTHREADS  8

static
void
mallocthread(void *sm, unsigned long num)
{
	struct semaphore *sem = sm;
	void *ptr;
	void *oldptr=NULL;
	void *oldptr2=NULL;
	int i;

	for (i=0; i<NTRIES; i++) {
		ptr = kmalloc(ITEMSIZE);
		if (ptr==NULL) {
			if (sem) {
				kprintf("thread %lu: kmalloc returned NULL\n",
					num);
				V(sem);
				return;
			}
			kprintf("kmalloc returned null; test failed.\n");
			return;
		}
		if (oldptr2) {
			kfree(oldptr2);
		}
		oldptr2 = oldptr;
		oldptr = ptr;
	}
	if (oldptr2) {
		kfree(oldptr2);
	}
	if (oldptr) {
		kfree(oldptr);
	}
	if (sem) {
		V(sem);
	}
}

int
malloctest(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	kprintf("Starting kmalloc test...\n");
	mallocthread(NULL, 0);
	kprintf("kmalloc test done\n");

	return 0;
}

int
mallocstress(int nargs, char **args)
{
	struct semaphore *sem;
	int i, result;

	(void)nargs;
	(void)args;

	sem = sem_create("mallocstress", 0);
	if (sem == NULL) {
		panic("mallocstress: sem_create failed\n");
	}

	kprintf("Starting kmalloc stress test...\n");

	for (i=0; i<NTHREADS; i++) {
		result = thread_fork("mallocstress",
				     mallocthread, sem, i,
				     NULL);
		if (result) {
			panic("mallocstress: thread_fork failed: %s\n",
			      strerror(result));
		}
	}

	for (i=0; i<NTHREADS; i++) {
		P(sem);
	}

	sem_destroy(sem);
	kprintf("kmalloc stress test done\n");

	return 0;
}
