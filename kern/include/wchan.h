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

#include <threadlist.h>
#include <spinlock.h>

#ifndef _WCHAN_H_
#define _WCHAN_H_

/*
 * Wait channel.
 */


/* Wait channel. */
struct wchan {
	const char *wc_name;		/* name for this channel */
	struct threadlist wc_threads;	/* list of waiting threads */
	struct spinlock wc_lock;	/* lock for mutual exclusion */
};

/*
 * Create a wait channel. Use NAME as a symbolic name for the channel.
 * NAME should be a string constant; if not, the caller is responsible
 * for freeing it after the wchan is destroyed.
 */
struct wchan *wchan_create(const char *name);

/*
 * Destroy a wait channel. Must be empty and unlocked.
 */
void wchan_destroy(struct wchan *wc);

/*
 * Return nonzero if there are no threads sleeping on the channel.
 * This is meant to be used only for diagnostic purposes.
 */
bool wchan_isempty(struct wchan *wc);

/*
 * Lock and unlock the wait channel.
 */
void wchan_lock(struct wchan *wc);
void wchan_unlock(struct wchan *wc);

/*
 * Go to sleep on a wait channel. The current thread is suspended
 * until awakened by someone else, at which point this function
 * returns.
 *
 * The channel must be locked, and will have been *unlocked* upon
 * return.
 */
void wchan_sleep(struct wchan *wc);

/*
 * Wake up one thread, or all threads, sleeping on a wait channel.
 * The queue should not already be locked.
 *
 * The current implementation is FIFO but this is not promised by the
 * interface.
 */
void wchan_wakeone(struct wchan *wc);
void wchan_wakeall(struct wchan *wc);


#endif /* _WCHAN_H_ */
