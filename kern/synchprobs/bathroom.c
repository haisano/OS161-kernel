/*
 * Copyright (c) 2001, 2002, 2009
 *      The President and Fellows of Harvard College.
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
 * Driver code for bathroom problem
 */
#include <types.h>
#include <lib.h>
#include <wchan.h>
#include <clock.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

#define REST 0
#define BOY 1
#define GIRL 2

#define NPEOPLE 100

static struct semaphore *br_sem;
static struct lock *br_lock;
static struct cv *boy_cv;
static struct cv *girl_cv;
static volatile int service_mode = REST;
static volatile int boy_waiting = 0;
static volatile int girl_waiting = 0;
static struct semaphore *donesem;

static
void
inititems(void)
{
	if (br_sem==NULL) {
		br_sem = sem_create("bathroom sem", 3);
		if (br_sem == NULL) {
			panic("synchtest: sem_create failed\n");
		}
	}
		
	if (boy_cv==NULL) {
		boy_cv = cv_create("boy_cv");
		if (boy_cv == NULL) {
			panic("synchtest: cv_create failed\n");
		}
	}

	if (girl_cv==NULL) {
		girl_cv = cv_create("girl_cv");
		if (girl_cv == NULL) {
			panic("synchtest: cv_create failed\n");
		}
	}
	
	if (br_lock==NULL) {
		br_lock = lock_create("bathroom_lock");
		if (br_lock == NULL) {
			panic("synchtest: mating lock_create failed\n");
		}
	}

	if (donesem==NULL) {
		donesem = sem_create("donesem", 0);
		if (donesem == NULL) {
			panic("synchtest: sem_create failed\n");
		}
	}
}

static
void
shower()
{
        // The thread enjoys a refreshing shower!
        //clocksleep(1);
}

static
void
boy(void *p, unsigned long which)
{
        (void)p;
        kprintf("boy #%ld starting\n", which);
		lock_acquire(br_lock);
		if(br_sem->sem_count == 0 || service_mode == GIRL || boy_waiting < girl_waiting)
		{
			boy_waiting++;
			cv_wait(boy_cv,br_lock);
			boy_waiting--;
		}
		lock_release(br_lock);
		P(br_sem);		
		service_mode = BOY;
        shower();
		kprintf("Boy #%ld: # in br-%d service_mode-%d\n",which,3-br_sem->sem_count,service_mode);
		V(br_sem);
		if(br_sem->sem_count == 3)
			service_mode = REST;
		lock_acquire(br_lock);
		if(boy_waiting > girl_waiting)
			cv_broadcast(boy_cv,br_lock);
		else if(service_mode != BOY)
			cv_broadcast(girl_cv,br_lock);
		lock_release(br_lock);
        //Implement synchronization
		V(donesem);
}

static
void
girl(void *p, unsigned long which)
{
        (void)p;
		kprintf("girl #%ld starting\n", which);
		lock_acquire(br_lock);
		if(br_sem->sem_count == 0 || service_mode == BOY || girl_waiting < boy_waiting)
		{
			girl_waiting++;
			cv_wait(girl_cv,br_lock);
			girl_waiting--;
		}
		lock_release(br_lock);
		P(br_sem);		
		service_mode = GIRL;
        shower();
		kprintf("Girl #%ld: # in br-%d service_mode-%d\n",which,3-br_sem->sem_count,service_mode);
		V(br_sem);
		lock_acquire(br_lock);
		if(br_sem->sem_count == 3)
			service_mode = REST;
		if(girl_waiting > boy_waiting)
			cv_broadcast(girl_cv,br_lock);
		else if(service_mode != GIRL)
			cv_broadcast(boy_cv,br_lock);
		lock_release(br_lock);
        // Implement synchronization
		V(donesem);
}

// Change this function as necessary
int
bathroom(int nargs, char **args)
{

        int i, err=0;
       
        (void)nargs;
        (void)args;
		inititems();
        for (i = 0; i < NPEOPLE; i++) {
                switch(i % 2) {
                        case 0:
                        err = thread_fork("Boy Thread",
                                          boy, NULL, i, NULL);
                        break;
                        case 1:
                        err = thread_fork("Girl Thread",
                                          girl, NULL, i, NULL);
                        break;
                }
                if (err) {
                        panic("bathroom: thread_fork failed: %s)\n",
                                strerror(err));
                }
        }
		
		for (i=0; i<NPEOPLE; i++) {
			P(donesem);
		}
	
	return 0;
}
