/*
 * Copyright (c) 2009
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

/* Make sure to build out-of-line versions of spinlock inline functions */
#define SPINLOCK_INLINE   /* empty */

#include <types.h>
#include <lib.h>
#include <cpu.h>
#include <spl.h>
#include <spinlock.h>
#include <current.h>	/* for curcpu */

/*
 * Spinlocks.
 */


/*
 * Initialize spinlock.
 */
void
spinlock_init(struct spinlock *lk)
{
	spinlock_data_set(&lk->lk_lock, 0);
	lk->lk_holder = NULL;
}

/*
 * Clean up spinlock.
 */
void
spinlock_cleanup(struct spinlock *lk)
{
	KASSERT(lk->lk_holder == NULL);
	KASSERT(spinlock_data_get(&lk->lk_lock) == 0);
}

/*
 * Get the lock.
 *
 * First disable interrupts (otherwise, if we get a timer interrupt we
 * might come back to this lock and deadlock), then use a machine-level
 * atomic operation to wait for the lock to be free.
 */
void
spinlock_acquire(struct spinlock *lk)
{
	struct cpu *mycpu;

	splraise(IPL_NONE, IPL_HIGH);

	/* this must work before curcpu initialization */
	if (CURCPU_EXISTS()) {
		mycpu = curcpu->c_self;
		if (lk->lk_holder == mycpu) {
			panic("Deadlock on spinlock %p\n", lk);
		}
	}
	else {
		mycpu = NULL;
	}

	while (1) {
		/*
		 * Do test-test-and-set, that is, read first before
		 * doing test-and-set, to reduce bus contention.
		 *
		 * Test-and-set is a machine-level atomic operation
		 * that writes 1 into the lock word and returns the
		 * previous value. If that value was 0, the lock was
		 * previously unheld and we now own it. If it was 1,
		 * we don't.
		 */
		if (spinlock_data_get(&lk->lk_lock) != 0) {
			continue;
		}
		if (spinlock_data_testandset(&lk->lk_lock) != 0) {
			continue;
		}
		break;
	}

	lk->lk_holder = mycpu;
}

/*
 * Release the lock.
 */
void
spinlock_release(struct spinlock *lk)
{
	/* this must work before curcpu initialization */
	if (CURCPU_EXISTS()) {
		KASSERT(lk->lk_holder == curcpu->c_self);
	}

	lk->lk_holder = NULL;
	spinlock_data_set(&lk->lk_lock, 0);
	spllower(IPL_HIGH, IPL_NONE);
}

/*
 * Check if the current cpu holds the lock.
 */ 
bool
spinlock_do_i_hold(struct spinlock *lk)
{
	if (!CURCPU_EXISTS()) {
		return true;
	}

	/* Assume we can read lk_holder atomically enough for this to work */
	return (lk->lk_holder == curcpu->c_self);
}
