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
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR ncdserialEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *   
 *
 *  COMMENT: Serial controller used in NCD 88K machines.
 *
 *  TODO: Once I know what device this really is, it may turn out to be
 *  one of the already implemented ones...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


struct ncdserial_data {
	int		console_handle;
};


DEVICE_ACCESS(ncdserial)
{
	struct ncdserial_data *d = (struct ncdserial_data *) extra;

	switch (relative_addr) {

	case 0x24:
		data[0] = 0x04;
		break;

	case 0x2c:
		if (writeflag == MEM_WRITE) {
			console_putchar(d->console_handle, data[0]);
		}
		break;

	case 0x38:
	case 0x3c:
		break;

	default:
		debug("[ ncdserial: unimplemented %s offset 0x%x",
		    writeflag == MEM_WRITE? "write to" : "read from",
		    (int) relative_addr);
		if (writeflag == MEM_WRITE)
			debug(": 0x%x", (int)data[0]);
		debug(" ]\n");
		// exit(1);
	}

	return 1;
}


DEVINIT(ncdserial)
{
	struct ncdserial_data *d;
	char *name3;
	size_t nlen;

	CHECK_ALLOCATION(d = (struct ncdserial_data *) malloc(sizeof(struct ncdserial_data)));
	memset(d, 0, sizeof(struct ncdserial_data));

	nlen = strlen(devinit->name) + 10;
	if (devinit->name2 != NULL)
		nlen += strlen(devinit->name2) + 10;
	CHECK_ALLOCATION(name3 = (char *) malloc(nlen));
	if (devinit->name2 != NULL && devinit->name2[0])
		snprintf(name3, nlen, "%s [%s]", devinit->name, devinit->name2);
	else
		snprintf(name3, nlen, "%s", devinit->name);

	d->console_handle = console_start_slave(devinit->machine, name3, 1);

	memory_device_register(devinit->machine->memory, name3,
	    devinit->addr, 0x40, dev_ncdserial_access, d,
	    DM_DEFAULT, NULL);

	/*  NOTE: Ugly cast into pointer  */
	devinit->return_ptr = (void *)(size_t)d->console_handle;
	return 1;
}

