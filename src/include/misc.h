#ifndef	MISC_H
#define	MISC_H

/*
 *  Copyright (C) 2003-2021  Anders Gavare.  All rights reserved.
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
 *  Misc. definitions for GXemul.
 */


#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

/*
 *  ../../config.h contains #defines set by the configure script. Some of these
 *  might reduce speed of the emulator, so don't enable them unless you
 *  need them.
 */

#include "../../config.h"

#define	COPYRIGHT_MSG	"Copyright (C) 2003-2021  Anders Gavare"

// The recommended way to add a specific message to the startup banner or
// about box is to use the SECONDARY_MSG. This should end with a newline
// character, unless it is completely empty.
//
// Example:  "Modified by XYZ to include support for machine type UVW.\n"
//
#define	SECONDARY_MSG	""


#ifdef NO_C99_PRINTF_DEFINES
/*
 *  This is a SUPER-UGLY HACK which happens to work on some machines.
 *  The correct solution is to upgrade your compiler to C99.
 */
#ifdef NO_C99_64BIT_LONGLONG
#define	PRIi8		"i"
#define	PRIi16		"i"
#define	PRIi32		"i"
#define	PRIi64		"lli"
#define	PRIx8		"x"
#define	PRIx16		"x"
#define	PRIx32		"x"
#define	PRIx64		"llx"
#else
#define	PRIi8		"i"
#define	PRIi16		"i"
#define	PRIi32		"i"
#define	PRIi64		"li"
#define	PRIx8		"x"
#define	PRIx16		"x"
#define	PRIx32		"x"
#define	PRIx64		"lx"
#endif
#endif


#ifdef NO_MAP_ANON
#ifdef mmap
#undef mmap
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
static void *no_map_anon_mmap(void *addr, size_t len, int prot, int flags,
	int nonsense_fd, off_t offset)
{
	void *p;
	int fd = open("/dev/zero", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Could not open /dev/zero\n");
		exit(1);
	}

	printf("addr=%p len=%lli prot=0x%x flags=0x%x nonsense_fd=%i "
	    "offset=%16lli\n", addr, (long long) len, prot, flags,
	    nonsense_fd, (long long) offset);

	p = mmap(addr, len, prot, flags, fd, offset);

	printf("p = %p\n", p);

	/*  TODO: Close the descriptor?  */
	return p;
}
#define mmap no_map_anon_mmap
#endif


/*  tmp dir to use if the TMPDIR environment variable isn't set:  */
#define	DEFAULT_TMP_DIR		"/tmp"


struct cpu;
struct emul;
struct machine;
struct memory;

enum Endianness
{
	BigEndian = 0,
	LittleEndian
};

#define	NO_BYTE_ORDER_OVERRIDE		-1
#define	EMUL_UNDEFINED_ENDIAN		0
#define	EMUL_LITTLE_ENDIAN		1
#define	EMUL_BIG_ENDIAN			2

#define	SWAP32(x)	    ((((x) & 0xff000000) >> 24) | (((x)&0xff) << 24) | \
			     (((x) & 0xff0000) >> 8) | (((x) & 0xff00) << 8))
#define	SWAP16(x)	    ((((x) & 0xff00) >> 8) | (((x)&0xff) << 8))

#ifdef HOST_LITTLE_ENDIAN
#define	LE16_TO_HOST(x)	    (x)
#define	BE16_TO_HOST(x)	    (SWAP16(x))
#else
#define	LE16_TO_HOST(x)	    (SWAP16(x))
#define	BE16_TO_HOST(x)	    (x)
#endif

#ifdef HOST_LITTLE_ENDIAN
#define	LE32_TO_HOST(x)	    (x)
#define	BE32_TO_HOST(x)	    (SWAP32(x))
#else
#define	LE32_TO_HOST(x)	    (SWAP32(x))
#define	BE32_TO_HOST(x)	    (x)
#endif

#ifdef HOST_LITTLE_ENDIAN
#define	LE64_TO_HOST(x)	    (x)
#define BE64_TO_HOST(x)	    (	(((x) >> 56) & 0xff) +			\
				((((x) >> 48) & 0xff) << 8) +		\
				((((x) >> 40) & 0xff) << 16) +		\
				((((x) >> 32) & 0xff) << 24) +		\
				((((x) >> 24) & 0xff) << 32) +		\
				((((x) >> 16) & 0xff) << 40) +		\
				((((x) >> 8) & 0xff) << 48) +		\
				(((x) & 0xff) << 56)  )
#else
#define	BE64_TO_HOST(x)	    (x)
#define LE64_TO_HOST(x)	    (	(((x) >> 56) & 0xff) +			\
				((((x) >> 48) & 0xff) << 8) +		\
				((((x) >> 40) & 0xff) << 16) +		\
				((((x) >> 32) & 0xff) << 24) +		\
				((((x) >> 24) & 0xff) << 32) +		\
				((((x) >> 16) & 0xff) << 40) +		\
				((((x) >> 8) & 0xff) << 48) +		\
				(((x) & 0xff) << 56)  )
#endif


#define	FAILURE(error_msg)					{	\
		char where_msg[400];					\
		snprintf(where_msg, sizeof(where_msg),			\
		    "%s, line %i, function %s().\n",			\
		    __FILE__, __LINE__, __func__);			\
        	fprintf(stderr, "\n%s, in %s\n", error_msg, where_msg);	\
		exit(1);						\
	}

#define	CHECK_ALLOCATION(ptr)					{	\
		if ((ptr) == NULL)					\
			FAILURE("Out of memory");			\
	}


/*  bootblock.c:  */
int load_bootblock(struct machine *m, struct cpu *cpu,
	int *n_loadp, char ***load_namesp);


/*  bootblock_apple.c:  */
int apple_load_bootblock(struct machine *m, struct cpu *cpu,
	int disk_id, int disk_type, int *n_loadp, char ***load_namesp);


/*  bootblock_iso9660.c:  */
int iso_load_bootblock(struct machine *m, struct cpu *cpu,
	int disk_id, int disk_type, int iso_type, unsigned char *buf,
	int *n_loadp, char ***load_namesp);


/*  dec_prom.c:  */
int decstation_prom_emul(struct cpu *cpu);


/*  debugmsg.c:  */
#define	SUBSYS_ALL		-1
#define	SUBSYS_STARTUP		0	/*  Startup messages  */
#define	SUBSYS_EMUL		1	/*  Emulation setup related  */
#define	SUBSYS_DISK		2	/*  Disk I/O related  */
#define	SUBSYS_NET		3	/*  Network related  */
#define	SUBSYS_MACHINE		4	/*  Machine related  */
#define	SUBSYS_DEVICE		5	/*  Device specific  */
#define	SUBSYS_CPU		6	/*  General CPU  */
#define	SUBSYS_MEMORY		7	/*  Memory related  */
#define	SUBSYS_EXCEPTION	8	/*  CPU exceptions  */
#define	SUBSYS_PROMEMUL		9	/*  PROM emulation related  */
#define	SUBSYS_X11		10	/*  X11 related  */
#define	VERBOSITY_ERROR		0
#define	VERBOSITY_WARNING	1
#define	VERBOSITY_INFO		2
#define	VERBOSITY_DEBUG		3
#define	ENOUGH_VERBOSITY(subsys,reqverb)	(debugmsg_current_verbosity[(subsys)] >= (reqverb))
extern int *debugmsg_current_verbosity;
void debug_indentation(int diff);
void debug(const char *fmt, ...);
void fatal(const char *fmt, ...);
void debugmsg_init();
int debugmsg_register_subsystem(const char* name);
void debugmsg_change_settings(char *subsystem_name, char *n);
void debugmsg_print_settings(const char *subsystem_name);
void debugmsg_add_verbosity_level(int subsystem, int verbosity_delta);
void debugmsg_set_verbosity_level(int subsystem, int verbosity);
void debugmsg_set_breakpoint_level(int subsystem, int level);
void debugmsg(int subsystem, const char *name, int verbosity_required, const char *fmt, ...);
void debugmsg_cpu(struct cpu* cpu, int subsystem, const char *name, int verbosity_required, const char *fmt, ...);


/*  dreamcast.c:  */
void dreamcast_machine_setup(struct machine *);
void dreamcast_emul(struct cpu *cpu);


/*  dreamcast_scramble.c:  */
void dreamcast_descramble(char *from, char *to);


/*  file.c:  */
int file_n_executables_loaded(void);
void file_load(struct machine *machine, struct memory *mem,
	char *filename, uint64_t *entrypointp,
	int arch, uint64_t *gpp, int *byte_order, uint64_t *tocp);


/*  luna88kprom.c:  */
void luna88kprom_init(struct machine *machine);
int luna88kprom_emul(struct cpu *cpu);


/*  misc.c:  */
void color_prompt();
void color_normal();
void color_error(bool bold);
void color_debugmsg_subsystem();
void color_debugmsg_name();
void color_banner();
void color_pc_indicator();
const char* color_symbol_ptr();
const char* color_normal_ptr();
uint64_t xorshift64star(uint64_t *state);
unsigned long long mystrtoull(const char *s, char **endp, int base);
int mymkstemp(char *templ);
#ifdef USE_STRLCPY_REPLACEMENTS
size_t mystrlcpy(char *dst, const char *src, size_t size);
size_t mystrlcat(char *dst, const char *src, size_t size);
#endif
void print_separator_line(void);
uint64_t size_to_mask(uint64_t size);


/*  mvmeprom.c:  */
void mvmeprom_init(struct machine *machine);
int mvmeprom_emul(struct cpu *cpu);


/*  ncd88kprom.c:  */
void ncd88kprom_init(struct machine *machine);
int ncd88kprom_emul(struct cpu *cpu);


/*  ps2_bios.c:  */
int playstation2_sifbios_emul(struct cpu *cpu);


/*  sh_ipl_g.c:  */
void sh_ipl_g_emul_init(struct machine *machine);
int sh_ipl_g_emul(struct cpu *);


/*  yamon.c:  */
void yamon_machine_setup(struct machine *machine, uint64_t env);
int yamon_emul(struct cpu *);


#endif	/*  MISC_H  */
