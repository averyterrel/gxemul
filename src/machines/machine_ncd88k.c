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
 *  COMMENT: NCD 88K X-terminal
 *
 *  Some info about the 88K variants here:
 *
 *	https://web-docs.gsi.de/~kraemer/COLLECTION/NCD/ncd19c.html
 *	http://www.geekdot.com/ncd-x-terminal/
 *	http://www.bitsavers.org/pdf/ncd/
 *
 *
 *  TODO: Separate the original (?) NCD19c model from the MCXL model?
 *
 *
 *  According to https://techmonitor.ai/technology/ncd_announces_x_terminal_pricing_revaluation
 *  these were (some?) of the NCD X-terminal models, including their intented
 *  price back in the 90s:
 *
 *	Model		CPU		Price	RAM	Comment
 *	-----		---		-----	---	-------
 *	NCD19r		R3000		$2,900	4 MB
 *	NCD19		68020		$3,400
 *	NCD17cr		88100		$5,400	6 MB	color?
 *	NCD19g		88110+fp	$4,500	6 MB	grayscale?
 *
 *  No other sources I have found mention any 88110 variant though. :-(
 *
 *  http://linux-distributions.org/docs/xfaq/X_Terminal_List_-_Quarterly_posting[Q2_93].txt
 *  for example doesn't list any NCD 88110 models.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


MACHINE_SETUP(ncd88k)
{
	machine->machine_name = strdup("NCD 88K X-terminal");

	if (machine->emulated_hz == 0)
		machine->emulated_hz = 20000000;

	/*
	 *  Memory map, guessed from the sections in the Xncd19c binary that
	 *  are loaded (.text and .data), and from some of the memory accesses
	 *  performed when running the code in that binary.
	 *
	 *	0x00000000 .. ?			PROM?
	 *
	 *	0x01000000 .. 0x0100003c	Serial controller, unknown type
	 *					[byte access at multiples of 4]
	 *
	 *	0x014c0000			[byte access] Hardware device?
	 *					0x00 written here.
	 *
	 *	0x01800000			32-bit value 0x00000000 written here.
	 *
	 *	0x01d80000 .. 0x1d800003	[byte access on write, 32-bit
	 *					access on read?]
	 *					0x01d80000: 0x00, 0x80, and 0xff written here
	 *					0x01d80001: 0x60 written here.
	 *					0x01d80002: 0x00, 0x01, 0x02, 0x0a written here.
	 *					0x01d80003: 0xf3 written here.
	 *					Most commonly, 0x80 or 0xff is written
	 *					to 0x01d80000. Some form of
	 *					interrupt controller?
	 *
	 *	0x01e00001			0x01 (byte) written here.
	 *
	 *	0x04000000			RAM used for code (.text)
	 *					64 MB theoretical max?
	 *
	 *	0x08000000 .. 0x0defffff	RAM used for data (.data)
	 *					96 MB theoretical max?
	 *
	 *	0x0e000500 .. 0x0e1ffd0b	[32-bit words] Hardware device?
	 *					VRAM? Accessed as 2048-byte lines
	 *					where only the first 1280 are
	 *					probed...
	 *
	 *
	 *  Devices that are expected to be mapped somewhere:
	 *	NVRAM and/or controller for it
	 *	RAMDAC (for palette etc)
	 *	PS2 keyboard and serial mouse controller(s)
	 *	Speaker controller
	 *	Ethernet controller
	 *	Interrupt controller of some kind?
	 *	Memory controller?
	 *	Timers?
	 */

	int ram_code_in_MB = 8;
	int ram_data_in_MB = 16;

        machine->main_console_handle = (size_t)device_add(machine, "ncdserial addr=0x01000000");

	dev_ram_init(machine, 0x04000000, ram_code_in_MB * 1048576, DEV_RAM_RAM, 0, "ram04");
	if (ram_code_in_MB < 64) {
		int unavailable_size_in_MB = 64 - ram_code_in_MB;
		dev_ram_init(machine, 0x04000000 + ram_code_in_MB * 1048576, unavailable_size_in_MB * 1048576, DEV_RAM_FAIL, 0, "ram04unavail");
	}

	dev_ram_init(machine, 0x08000000, ram_data_in_MB * 1048576, DEV_RAM_RAM, 0, "ram08");
	// Hm, a DEV_RAM_FAIL for memory beyond the data RAM could be needed
	// too, but it doesn't seemt to be (?).

	if (machine->x11_md.in_use) {
		struct vfb_data* fb = dev_fb_init(machine, machine->memory,
		    0x0e000000, VFB_GENERIC, 1280, 1024, 2048, 1024, 8,
		    machine->machine_name);

		set_grayscale_palette(fb, 256);

		fb->rgb_palette[3 * 0 + 0] = 0x00;
		fb->rgb_palette[3 * 0 + 1] = 0x00;
		fb->rgb_palette[3 * 0 + 2] = 0x00;

		fb->rgb_palette[3 * 1 + 0] = 0xff;
		fb->rgb_palette[3 * 1 + 1] = 0xff;
		fb->rgb_palette[3 * 1 + 2] = 0xff;
	} else {
		debugmsg(SUBSYS_MACHINE, "video", VERBOSITY_WARNING,
		    "You need to start the emulator with -X to enable "
		    "graphical framebuffer output!");
	}

	if (!machine->prom_emulation)
		return;

	ncd88kprom_init(machine);
}


MACHINE_DEFAULT_CPU(ncd88k)
{
	machine->cpu_name = strdup("88100");
}


MACHINE_DEFAULT_RAM(ncd88k)
{
	/*  The PROM is at memory offset 0, so not really RAM.
	    Size: Unknown.  */
	machine->physical_ram_in_mb = 1;
}


MACHINE_REGISTER(ncd88k)
{
	MR_DEFAULT(ncd88k, "NCD 88K X-terminal", MACHINE_NCD88K);

	machine_entry_add_alias(me, "ncd88k");

	me->set_default_ram = machine_default_ram_ncd88k;
}

