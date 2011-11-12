/*
 * Copyright (c) 2001, 2002, 2009
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
 * Driver code for whale mating problem
 */
#include <types.h>
#include <lib.h>
#include <wchan.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

#define NMATING 10

#define FREE 0

static struct lock *match_lock;
static struct lock *female_lock;
static struct lock *male_lock;
static struct lock *mate_lock;
//static struct cv *male_cv;
//static struct cv *female_cv;
static struct cv *mate_cv;
static struct semaphore *mate_sem;
static struct semaphore *donesem;
static volatile int male_which = FREE;
static volatile int female_which = FREE;
static volatile int match_which = FREE;

static
void
inititems(void)
{
	
	if (mate_lock==NULL) {
		mate_lock = lock_create("mate_lock");
		if (mate_lock == NULL) {
			panic("synchtest: mate lock_create failed\n");
		}
	}

	if (male_lock==NULL) {
		male_lock = lock_create("male_lock");
		if (male_lock == NULL) {
			panic("synchtest: male lock_create failed\n");
		}
	}

	if (female_lock==NULL) {
		female_lock = lock_create("female_lock");
		if (female_lock == NULL) {
			panic("synchtest: female lock_create failed\n");
		}
	}
	if (match_lock==NULL) {
		match_lock = lock_create("match_lock");
		if (match_lock == NULL) {
			panic("synchtest: match lock_create failed\n");
		}
	}
	if (mate_cv==NULL) {
		mate_cv = cv_create("mate_cv");
		if (mate_cv == NULL) {
			panic("synchtest: mate cv_create failed\n");
		}
	}
	if (mate_sem==NULL) {
		mate_sem = sem_create("mate_sem", 3);
		if (mate_sem == NULL) {
			panic("synchtest: sem_create failed\n");
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
male(void *p, unsigned long which)
{
	(void)p;
	kprintf("male whale #%ld starting\n", which);
	lock_acquire(male_lock);
	P(mate_sem);
	male_which = which;
	lock_acquire(mate_lock);
	if(mate_sem->sem_count != 0)
		cv_wait(mate_cv,mate_lock);
	else
		cv_broadcast(mate_cv,mate_lock);
	lock_release(mate_lock);
	kprintf("male whale #%ld mating with female #%d and matchamker #%d\n", which, female_which, match_which);
	V(mate_sem);
	lock_acquire(mate_lock);
	if(mate_sem->sem_count != 3)
		cv_wait(mate_cv,mate_lock);
	else
		cv_broadcast(mate_cv,mate_lock);
	lock_release(mate_lock);
	kprintf("male whale #%ld finished mating\n", which);
	lock_release(male_lock);
	V(donesem);
	// Implement this function 
}

static
void
female(void *p, unsigned long which)
{
	(void)p;
	kprintf("female whale #%ld starting\n", which);
	lock_acquire(female_lock);
	P(mate_sem);
	female_which = which;
	lock_acquire(mate_lock);
	if(mate_sem->sem_count != 0)
		cv_wait(mate_cv,mate_lock);
	else
		cv_broadcast(mate_cv,mate_lock);
	lock_release(mate_lock);
	kprintf("female whale #%ld mating with male #%d and matchamker #%d\n", which, male_which, match_which);
	V(mate_sem);
	lock_acquire(mate_lock);
	if(mate_sem->sem_count != 3)
		cv_wait(mate_cv,mate_lock);
	else
		cv_broadcast(mate_cv,mate_lock);
	lock_release(mate_lock);
	kprintf("female whale #%ld finished mating\n", which);
	lock_release(female_lock);
	V(donesem);
	// Implement this function 
}

static
void
matchmaker(void *p, unsigned long which)
{
	(void)p;
	kprintf("match whale #%ld starting\n", which);
	lock_acquire(match_lock);
	P(mate_sem);
	match_which = which;
	lock_acquire(mate_lock);
	if(mate_sem->sem_count != 0)
		cv_wait(mate_cv,mate_lock);
	else
		cv_broadcast(mate_cv,mate_lock);
	lock_release(mate_lock);
	kprintf("matchmaker whale #%ld helping male #%d and female #%d mate\n", which, male_which, female_which);
	V(mate_sem);
	lock_acquire(mate_lock);
	if(mate_sem->sem_count != 3)
		cv_wait(mate_cv,mate_lock);
	else
		cv_broadcast(mate_cv,mate_lock);
	lock_release(mate_lock);
	lock_release(match_lock);
	kprintf("matchmaker whale #%ld finished helping mating\n", which);
	V(donesem);	// Implement this function 
}


// Change this function as necessary
int
whalemating(int nargs, char **args)
{

	int i, j, err=0;
	
	(void)nargs;
	(void)args;
	inititems();

	for (i = 0; i < 3; i++) {
		for (j = 0; j < NMATING; j++) {
			switch(i) {
			    case 0:
				err = thread_fork("Male Whale Thread",
						  male, NULL, j, NULL);
				break;
			    case 1:
				err = thread_fork("Female Whale Thread",
						  female, NULL, j, NULL);
				break;
			    case 2:
				err = thread_fork("Matchmaker Whale Thread",
						  matchmaker, NULL, j, NULL);
				break;
			}
			if (err) {
				panic("whalemating: thread_fork failed: %s)\n",
				      strerror(err));
			}
		}
	}
	
	for (i=0; i<3*NMATING; i++) {
		P(donesem);
	}
	
	kprintf("Whalemating successful - expect some baby whales soon\n");
	return 0;
}
