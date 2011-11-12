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

#ifndef _THREAD_H_
#define _THREAD_H_

/*
 * Definition of a thread.
 *
 * Note: curthread is defined by <current.h>.
 */

#include <spinlock.h>
#include <threadlist.h>

struct addrspace;
struct cpu;
struct vnode;

/* get machine-dependent defs */
#include <machine/thread.h>
#include <mips/trapframe.h>


/* Size of kernel stacks; must be power of 2 */
#define STACK_SIZE 4096

/* Mask for extracting the stack base address of a kernel stackstruct thread *t_thr1,*temp_thread ;
		unsigned j;
		for (i=0; i<to_send; i++) {

			temp_thread=curcpu->c_runqueue.tl_head.tln_next->tln_self;
			t_thr1=curcpu->c_runqueue.tl_head.tln_next->tln_self;

			for (j=0; (t_thr1->t_listnode.tln_next) != NULL ; (t_thr1)=(t_thr1)->t_listnode.tln_next->tln_self)
			{  	
				j++;
				if(((t_thr1)->priority) < temp_thread->priority)
					temp_thread=t_thr1;
				if(j == curcpu->c_runqueue.tl_count)
					break;
			}
			threadlist_remove(&curcpu->c_runqueue,temp_thread);		
			threadlist_addhead(&victims,temp_thread);

	}
 pointer */
#define STACK_MASK  (~(vaddr_t)(STACK_SIZE-1))

/* Macro to test if two addresses are on the same kernel stack */
#define SAME_STACK(p1, p2)     (((p1) & STACK_MASK) == ((p2) & STACK_MASK))
#define MAX_FDESC 20
#define MAX_PID 300 


/* States a thread can be in. */
typedef enum {
	S_RUN,		/* running */
	S_READY,	/* ready to run */
	S_SLEEP,	/* sleeping */
	S_ZOMBIE,	/* zombie; exited but not yet deleted */
} threadstate_t;

/* Thread structure. */
struct thread {
	/*
	 * These go up front so they're easy to get to even if the
	 * debugger is messed up.
	 */
	char *t_name;			/* Name of this thread */
	const char *t_wchan_name;	/* Name of wait channel, if sleeping */
	threadstate_t t_state;		/* State this thread is in */

	/*
	 * Thread subsystem internal fields.
	 */
	struct thread_machdep t_machdep; /* Any machine-dependent goo */
	struct threadlistnode t_listnode; /* Link for run/sleep/zombie lists */
	char *t_stack;			/* Kernel-level stack */
	struct switchframe *t_context;	/* Saved register context (on stack) */
	struct cpu *t_cpu;		/* CPU thread runs on */

	/*
	 * Interrupt state fields.
	 *
	 * t_in_interrupt is true if current execution is in an
	 * interrupt handler, which means the thread's normal context
	 * of execution is stopped somewhere in the middle of doing
	 * something else. This makes assorted operations unsafe.
	 *
	 * See notes in spinlock.c regarding t_curspl and t_iplhigh_count.
	 *
	 * Exercise for the student: why is this material per-thread
	 * rather than per-cpu or global?
	 */
	bool t_in_interrupt;		/* Are we in an interrupt? */
	int t_curspl;			/* Current spl*() state */
	int t_iplhigh_count;		/* # of times IPL has been raised */

	/*
	 * Public fields
	 */
	int priority;
	/* VM */
	struct addrspace *t_addrspace;	/* virtual address space */

	/* VFS */
	struct vnode *t_cwd;		/* current working directory */

	/* add more here as needed */
	struct File *fdesc[MAX_FDESC]; //file descriptor (make look to make an array of 20 FDs)
	//remember to deallocate any FD's for this process
	pid_t pid;   //pid for the process need to set it with the process descriptor table	
	pid_t ppid; //the parent PID;
};

struct wait_List{
	struct wchan *wait_chan_exit;
	pid_t pid;
	int status;
	struct wait_List *wl_next;
};

struct pid_List{
	pid_t pid;
	struct pid_List *next;
};

/* Call once during system startup to allocate data structures. */
void thread_bootstrap(void);

/* Call late in system startup to get secondary CPUs running. */
void thread_start_cpus(void);

/* Call during panic to stop other threads in their tracks */
void thread_panic(void);

/* Call during system shutdown to offline other CPUs. */
void thread_shutdown(void);

/*
 * Make a new thread, which will start executing at "func". The "data"
 * arguments (one pointer, one number) are passed to the function. The
 * current thread is used as a prototype for creating the new one. If
 * "ret" is non-null, the thread structure for the new thread is
 * handed back. (Note that using said thread structure from the parent
 * thread should be done only with caution, because in general the
 * child thread might exit at any time.) Returns an error code.
 */
int thread_fork(const char *name, 
                void (*func)(void *, unsigned long),
                void *data1, unsigned long data2, 
                struct thread **ret);

/*
 * Cause the current thread to exit.
 * Interrupts need not be disabled.
 */
void thread_exit(void);

/*
 * Cause the current thread to yield to the next runnable thread, but
 * itself stay runnable.
 * Interrupts need not be disabled.
 */
void thread_yield(void);

/*
 * Reshuffle the run queue. Called from the timer interrupt.
 */
void schedule(void);

/*
 * Potentially migrate ready threads to other CPUs. Called from the
 * timer interrupt.
 */
void thread_consider_migration(void);

pid_t sys_fork(struct trapframe *, int *retval);

struct wait_List* alloc_wait_list(void);
int add_wait_list( struct wait_List *node);
int remove_wait_list(struct wait_List *node);
struct wait_List* get_waiting_pid(pid_t pid);
int sys__exit(int code); 
int sys_waitpid(pid_t pid, int *status, int options,int *retval);

struct pid_List* alloc_pid_table(void);
int pid_exists(pid_t pid);
pid_t add_pid_table(void);
int remove_pid_table(pid_t pid);
int sys_getpid(int *retval);
int sys_execv(const char *program, char **args, int *retval);

#endif /* _THREAD_H_ */
