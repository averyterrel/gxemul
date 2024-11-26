/*
 *  Copyright (C) 2021  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  COMMENT: NCD 88K PROM emulation
 *
 *  For emulation of the system calls provided by the NCD 88K X-terminal ROM
 *  firmware.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "console.h"
#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


/*
 *  ncd88kprom_init():
 */
void ncd88kprom_init(struct machine *machine)
{
	struct cpu *cpu = machine->cpus[0];

        /*
         *  If the VBR is not set (i.e. it is 0x00000000), then the vector
         *  at 0x400 is the user trap vector (0x80) which seems to be used for
         *  basic system calls by Xncd19c.
         *
         *  The following instructions seem to be used:
         *
         *	tb0	0,r0,0x80	<-- basic syscalls?
         *	tb0	0,r0,0x81
         *	tb0	0,r0,0x82
         *	tb0	0,r0,0x1f7
         */

	uint32_t vbr = cpu->cd.m88k.cr[M88K_CR_VBR];

        store_32bit_word(cpu, vbr + 8 * 0x80, M88K_PROM_INSTR);
}


/*
 *  ncd88kprom_emul():
 *
 *  Input:
 *	r2 = function?
 *	r3 = first argument (for functions that take arguments)
 *
 *  Output:
 *	?
 */
int ncd88kprom_emul(struct cpu *cpu)
{
	int func = cpu->cd.m88k.r[2];

	switch (func) {
	case 1:
		debugmsg(SUBSYS_PROMEMUL, "luna88k", VERBOSITY_DEBUG, "putchar 0x%02x", cpu->cd.m88k.r[3]);
		// Note: cpu->machine->main_console_handle is the serial port.
		console_putchar(0, cpu->cd.m88k.r[3]);
		break;

	default:
		cpu_register_dump(cpu->machine, cpu, 1, 0);
		cpu_register_dump(cpu->machine, cpu, 0, 1);
		debugmsg(SUBSYS_PROMEMUL, "ncd88k", VERBOSITY_ERROR, "unimplemented function 0x%" PRIx32, func);
		cpu->running = 0;
		return 0;
	}

	/*  Perform an 'rte' instruction, basically:  */
	m88k_stcr(cpu, cpu->cd.m88k.cr[M88K_CR_EPSR], M88K_CR_PSR, 1);
	cpu->pc = cpu->cd.m88k.cr[M88K_CR_SNIP] & M88K_NIP_ADDR;

	return 1;
}

