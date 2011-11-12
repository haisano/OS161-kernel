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

#ifndef _MIPS_CURRENT_H_
#define _MIPS_CURRENT_H_


/*
 * Macro for current thread, or alternatively current cpu.
 *
 * This file should only be included via <current.h> (q.v.)
 *
 * These are machine-dependent because on some platforms it is
 * better/easier to keep track of curcpu and make curthread be
 * curcpu->c_curthread, and on others to keep track of curthread and
 * make curcpu be curthread->t_cpu.
 *
 * Either way we don't want retrieving curthread or curcpu to be
 * expensive; digging around in system board registers and whatnot is
 * not a very good idea. So we want to keep either curthread or curcpu
 * on-chip somewhere in some fashion.
 *
 * There are various possible approaches; for example, one might use
 * the MMU on each CPU to map that CPU's cpu structure to a fixed
 * virtual address that's the same on all CPUs. Then curcpu can be a
 * constant. (But one has to remember to use curcpu->c_self as the
 * canonical form of the pointer anywhere that's visible to other
 * CPUs.) Another approach is to reserve a register to hold curthread.
 *
 * On mips there's an architectural issue that informs this choice:
 * there's no easy way to find the current cpu, the current thread, or
 * even the kernel stack of the current thread when entering the
 * kernel at trap time. (On most CPUs there's a canonical way to find
 * at least the stack.)
 *
 * Therefore we do the following:
 *
 * - We misuse a kernel-settable field of a nonessential MMU register
 *   to hold the CPU number.
 *
 * - On trap entry we use this number to index an array that gets us
 *   both the kernel stack and curthread.
 *
 * - We tell the compiler not to use the s7 register and keep
 *   curthread there.
 *
 * Note that if you want to change this scheme to use a different
 * register, or change to a different scheme, you need to touch three
 * places: here, the mips-specific kernel CFLAGS in the makefiles, and
 * the trap entry and return code.
 */

register struct thread *curthread asm("$23");	/* s7 register */
#undef __NEED_CURTHREAD
#define __NEED_CURCPU

/* For how we've defined it, curthread gets set first, then curcpu. */
#define INIT_CURCPU(cpu, thread) (curthread = (thread), curcpu = (cpu))

#endif /* _MIPS_CURRENT_H_ */
