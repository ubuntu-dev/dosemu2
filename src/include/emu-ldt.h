/*
 * emu-ldt.h
 *
 * Definitions of structures used with the modify_ldt system call.
 */

#ifndef _EMU_LDT_H
#define _EMU_LDT_H

#include "Asm/ldt.h"

/* check */
/* #if !defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN) */
/* #error Augh */
/* #endif */

/* Maximum number of {GLI}DT entries supported. */
#define LGDT_ENTRIES	8192
#define IDT_ENTRIES	256
/* The size of each {GLI}DT entry _in_memory_ */
#define LGDT_ENTRY_SIZE	8
#define IDT_ENTRY_SIZE	8

#define MAX_SELECTORS	LDT_ENTRIES

/*
 *   segment descriptors - little endian
 *   Memory layout
 *
 *  00-15	limit 15-0
 *  16-39	base  23-0
 *  40-43	type
 *  44		DT
 *  45-46	DPL
 *  47		P
 *  48-51	limit 19-16
 *  52		vf
 *  53		r
 *  54		DB
 *  55		G
 *  56-63	base 31-24
 *
 * limit mask 000h0000.0000llll
 * base  mask hh0000mm.llll0000
 * flags mask 00f0ff00.00000000
 *
 */
typedef struct descriptor {
	unsigned limit_lo  : 16;	/* b0,b1 */
	unsigned base_lo   : 16;	/* b2,b3 */
	unsigned base_mid  : 8;		/* b4 */
	unsigned type      : 4;		/* b5 */
	unsigned S         : 1;
	unsigned DPL       : 2;
	unsigned present   : 1;
	unsigned limit_hi  : 4;		/* b6 */
	unsigned AVL       : 1;
	unsigned unused    : 1;
	unsigned DB        : 1;
	unsigned gran      : 1;
	unsigned base_hi   : 8;		/* b7 */
} Descriptor;

typedef struct gatedesc {
	unsigned offs_lo   : 16;	/* b0,b1 */
	unsigned seg       : 16;	/* b2,b3 */
	unsigned unused    : 8;		/* b4: b0-4 reserved, b5-7=0 */
	unsigned type      : 4;		/* b5 */
	unsigned S         : 1;
	unsigned DPL       : 2;
	unsigned present   : 1;
	unsigned offs_hi   : 16;	/* b6,b7 */
} Gatedesc;

typedef struct {
	unsigned short prev_task;	/* 00 */
	unsigned short dummy1;
	unsigned long esp0;		/* 04 */
	unsigned short ss0;
	unsigned short __ss0h;
	unsigned long esp1;		/* 0c */
	unsigned short ss1;
	unsigned short __ss1h;
	unsigned long esp2;		/* 14 */
	unsigned short ss2;
	unsigned short __ss2h;
	unsigned long cr3;		/* 1c */
	unsigned long eip;		/* 20 */
	unsigned long eflags;		/* 24 */
	unsigned long eax;		/* 28 */
	unsigned long ecx;
	unsigned long edx;		/* 30 */
	unsigned long ebx;
	unsigned long esp;		/* 38 */
	unsigned long ebp;
	unsigned long esi;		/* 40 */
	unsigned long edi;
	unsigned short es, __esh;	/* 48 */
	unsigned short cs, __csh;
	unsigned short ss, __ssh;	/* 50 */
	unsigned short ds, __dsh;
	unsigned short fs, __fsh;
	unsigned short gs, __gsh;
	unsigned short LDT, __ldth;	/* 60 */
	unsigned short tflag;		/* 64 */
	unsigned short IOmapbase;	/* 66 */
} Task;

extern Descriptor *GDT;
extern Descriptor *LDT;
extern Gatedesc   *IDT;

int emu_modify_ldt(int func, void *ptr, unsigned long bytecount);
unsigned short GetSelectorXfer(unsigned short w);

#endif
