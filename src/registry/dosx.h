/* dosx.h - Header for dos extender specific functions.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997
 */
#ifndef __DOSX_H
#define __DOSX_H

#include <platform.h>

/* Selector type */
typedef WORD SELECTOR;

/* NOTE: packing on */
#include <ahxpack1.h>

/* LPVOID_PM - Protected mode pointer. */
typedef struct
{
#ifdef PF_MSDOS_32
    DWORD       off             PACK_ME;
#elif defined(PF_MSDOS) || defined(PF_MSDOS_16)
    WORD        off             PACK_ME;
#endif
    SELECTOR    sel             PACK_ME;
} LPVOID_PM;

/* Valid protected mode address */
#define VALID_PM(p) !(p.off == 0 && p.sel == 0)

/* LPVOID_RM - Real mode pointer */
typedef struct {
    WORD       off              PACK_ME;
    WORD       seg              PACK_ME;
} LPVOID_RM;

/* LPVOID_X - Combo pointer, addresses both real mode and protected mode */
typedef struct
{
    int        valid            PACK_ME;
    LPVOID_PM  pm               PACK_ME;
    LPVOID_RM  rm               PACK_ME;
} LPVOID_X;

/* DOSX_REGS - Used by DOSX functions to pass registers to real mode
 * functions or interrupts. Has the same layout as the DPMI real mode
 * register data structure. I have splitted this stuff for DJGPP
 * porters. I know that packing structs for that compiler is a wee
 * bit troublesome. :-) */

typedef struct
{
    WORD    di                  PACK_ME;
    WORD    hdi                 PACK_ME;
    WORD    si                  PACK_ME;
    WORD    hsi                 PACK_ME;
    WORD    bp                  PACK_ME;
    WORD    hbp                 PACK_ME;
    DWORD   reserved            PACK_ME;
    WORD    bx                  PACK_ME;
    WORD    hbx                 PACK_ME;
    WORD    dx                  PACK_ME;
    WORD    hdx                 PACK_ME;
    WORD    cx                  PACK_ME;
    WORD    hcx                 PACK_ME;
    WORD    ax                  PACK_ME;
    WORD    hax /* are us. */   PACK_ME;
} __ahx_word_regs;

typedef struct
{
    DWORD   edi                 PACK_ME;
    DWORD   esi                 PACK_ME;
    DWORD   ebp                 PACK_ME;
    DWORD   reserved            PACK_ME;
    DWORD   ebx                 PACK_ME;
    DWORD   edx                 PACK_ME;
    DWORD   ecx                 PACK_ME;
    DWORD   eax                 PACK_ME;
} __ahx_dword_regs;

typedef struct
{
    WORD    es                  PACK_ME;
    WORD    ds                  PACK_ME;
    WORD    fs                  PACK_ME;
    WORD    gs                  PACK_ME;
    WORD    ip                  PACK_ME;
    WORD    cs                  PACK_ME;
    WORD    sp                  PACK_ME;
    WORD    ss                  PACK_ME;
} __ahx_segment_regs;

typedef union
{
    __ahx_word_regs     x       PACK_ME;
    __ahx_dword_regs    e       PACK_ME;
} __ahx_regs;

/* Completion of DOSX_REGS structure */
typedef struct
{
    __ahx_regs          r       PACK_ME;
    WORD                flags   PACK_ME;
    __ahx_segment_regs  s       PACK_ME;
#define RX r.x
#define RE r.e
} DOSX_REGS;

/* DESCRIPTOR - Here's the descriptor structure, also splitted up. */
typedef struct
{
    WORD limit_lo               PACK_ME;
    WORD base_lo                PACK_ME;
    BYTE base_hi                PACK_ME;
    BYTE rts_lo                 PACK_ME;
    BYTE limitrts_hi            PACK_ME;
    BYTE base_xhi               PACK_ME;
} __ahx_descr;

typedef union
{
    WORD        w[4]            PACK_ME;
    BYTE        b[8]            PACK_ME;
    __ahx_descr d;
} DESCRIPTOR;

/* NOTE: packing off */
#include <ahxpackn.h>

/* MAKELINEAR() - Converts real mode address to linear address */
#define MAKELINEAR(seg, off) \
    ((((DWORD)(seg)) << 4UL) + (DWORD)(off))

__BEGIN_EXTERN_C
int dosx_int (int, DOSX_REGS FAR*);
void dosx_vm_idle (void);

#if defined(PF_MSDOS_32)
int dosx_dos_alloc (LPVOID_X FAR*, unsigned);
int dosx_dos_free (LPVOID_X);
unsigned dosx_selector_inc (void);
int dosx_alloc_selector (SELECTOR FAR*, unsigned);
int dosx_free_selector (SELECTOR);
int dosx_alloc_alias (SELECTOR FAR*, SELECTOR);
SELECTOR dosx_current_ds (void);
SELECTOR dosx_current_cs (void);
SELECTOR dosx_current_ss (void);
int dosx_get_selector_base (SELECTOR, DWORD FAR*);
int dosx_set_selector_base (SELECTOR, DWORD);
int dosx_get_descriptor (SELECTOR, DESCRIPTOR FAR*);
int dosx_set_descriptor (SELECTOR, CONST DESCRIPTOR FAR*);
int dosx_set_selector_limit (SELECTOR, DWORD);
int dosx_get_selector_limit (SELECTOR, DWORD FAR*);
int dosx_call_far_real (LPVOID_RM, DOSX_REGS FAR*);
int dosx_int_real (int, DOSX_REGS FAR*);
void dosx_far_near_copy (LPVOID_PM, LPCVOID, unsigned);
void dosx_near_far_copy (LPVOID, LPVOID_PM, unsigned);
unsigned dosx_far_strlen (LPVOID_PM);
void dosx_far_set (LPVOID_PM, int, unsigned);
LPVOID_PM dosx_far_address_of (LPCVOID);
DWORD dosx_linear_address_of (LPCVOID);
int dosx_map_linear (LPVOID_PM FAR*, DWORD);
int dosx_unmap_linear (LPVOID_PM);
DWORD dosx_map_addr16 (LPCVOID);
int dosx_unmap_addr16 (DWORD);
#define dosx_change_selector dosx_presto_chango_selector
int dosx_presto_chango_selector (SELECTOR);
int dosx_make_selector16 (SELECTOR);
int dosx_alloc_copy (SELECTOR FAR*, SELECTOR);
int dosx_lock_memory (LPVOID_PM, unsigned);
int dosx_unlock_memory (LPVOID_PM, unsigned);

#endif /* _EXTENDED_DOS */

__END_EXTERN_C

#endif /* __DOSX_H */
