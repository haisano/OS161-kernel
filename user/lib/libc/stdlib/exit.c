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

#include <stdlib.h>
#include <unistd.h>

/*
 * C standard function: exit process.
 */

void
exit(int code)
{
	/*
	 * In a more complicated libc, this would call functions registered
	 * with atexit() before calling the syscall to actually exit.
	 */

	_exit(code);
}

/*
 * The mips gcc we were using in 2001, and probably other versions as
 * well, knows more than is healthy: it knows without being told that
 * exit and _exit don't return.
 * 
 * This causes it to make foolish optimizations that cause broken
 * things to happen if _exit *does* return, as it does in the base
 * system (because it's unimplemented) and may also do if someone has
 * a bug.
 *
 * The way it works is that if _exit returns, execution falls into
 * whatever happens to come after exit(), with the registers set up in
 * such a way that when *that* function returns it actually ends up
 * calling itself again. This causes weird things to happen.
 *
 * This function has no purpose except to trap that
 * circumstance. Looping doing nothing is not entirely optimal, but
 * there's not much we *can* do, and it's better than jumping around
 * wildly as would happen if this function were removed.
 *
 * If you change this to loop calling _exit(), gcc "helpfully"
 * optimizes the loop away and the behavior reverts to that previously
 * described.
 */

void __exit_hack(void);  /* avoid gcc warning */

void
__exit_hack(void)
{
	volatile int blah = 1;
	while (blah) {}
}
