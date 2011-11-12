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

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "opt-synchprobs.h"

/*
 * Time-related definitions.
 *
 * hardclock() is called on every CPU HZ times a second, possibly only
 * when the CPU is not idle, for scheduling.
 *
 * timerclock() is called on one CPU once a second to allow simple
 * timed operations. (This is a fairly simpleminded interface.)
 *
 * gettime() may be used to fetch the current time of day.
 * getinterval() computes the time from time1 to time2.
 *
 * XXX we have struct timespec now, let's use it.
 */

/* hardclocks per second */
#if OPT_SYNCHPROBS
/* Make synchronization more exciting :) */
#define HZ  10000
#else
/* More realistic value */
#define HZ  100
#endif

void hardclock_bootstrap(void);

void hardclock(void);
void timerclock(void);

void gettime(time_t *seconds, uint32_t *nanoseconds);

void getinterval(time_t secs1, uint32_t nsecs,
                 time_t secs2, uint32_t nsecs2,
                 time_t *rsecs, uint32_t *rnsecs);

/*
 * clocksleep() suspends execution for the requested number of seconds,
 * like userlevel sleep(3). (Don't confuse it with wchan_sleep.)
 */
void clocksleep(int seconds);


#endif /* _CLOCK_H_ */
