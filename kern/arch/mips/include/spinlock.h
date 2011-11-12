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

#ifndef _MIPS_SPINLOCK_H_
#define _MIPS_SPINLOCK_H_

#include <cdefs.h>


/* Type of value needed to actually spin on */
typedef unsigned spinlock_data_t;

/* Initializer for use by SPINLOCK_INITIALIZER */
#define SPINLOCK_DATA_INITIALIZER	0

/* Atomic operations on spinlock_data_t */
void spinlock_data_set(volatile spinlock_data_t *sd, unsigned val);
spinlock_data_t spinlock_data_get(volatile spinlock_data_t *sd);
spinlock_data_t spinlock_data_testandset(volatile spinlock_data_t *sd);

////////////////////////////////////////////////////////////

SPINLOCK_INLINE
void
spinlock_data_set(volatile spinlock_data_t *sd, unsigned val)
{
	*sd = val;
}

SPINLOCK_INLINE
spinlock_data_t
spinlock_data_get(volatile spinlock_data_t *sd)
{
	return *sd;
}

SPINLOCK_INLINE
spinlock_data_t
spinlock_data_testandset(volatile spinlock_data_t *sd)
{
	spinlock_data_t x;
	spinlock_data_t y;

	/*
	 * Test-and-set using LL/SC.
	 *
	 * Load the existing value into X, and use Y to store 1.
	 * After the SC, Y contains 1 if the store succeeded,
	 * 0 if it failed.
	 *
	 * On failure, return 1 to pretend that the spinlock
	 * was already held.
	 */

	y = 1;
	__asm volatile(
		".set push;"		/* save assembler mode */
		".set mips32;"		/* allow MIPS32 instructions */
		".set volatile;"	/* avoid unwanted optimization */
		"ll %0, 0(%2);"		/*   x = *sd */
		"sc %1, 0(%2);"		/*   *sd = y; y = success? */
		".set pop"		/* restore assembler mode */
		: "=r" (x), "+r" (y) : "r" (sd));
	if (y == 0) {
		return 1;
	}
	return x;
}


#endif /* _MIPS_SPINLOCK_H_ */
