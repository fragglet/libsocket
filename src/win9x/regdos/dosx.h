/* dosx.h - Header for dos extender specific functions.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997
 */

#ifndef __DOSX_H
#define __DOSX_H

#include "ahxtyps.h"

/* Selector type */
typedef WORD SELECTOR;

/* LPVOID_PM - Protected mode pointer. */
typedef struct
{
    DWORD       off __attribute__((packed));
    SELECTOR    sel __attribute__((packed));
} LPVOID_PM;

/* Valid protected mode address */
#define VALID_PM(p) !(p.off == 0 && p.sel == 0)

/* LPVOID_RM - Real mode pointer */
typedef struct {
    WORD       off __attribute__((packed));
    WORD       seg __attribute__((packed));
} LPVOID_RM;

/* LPVOID_X - Combo pointer, addresses both real mode and protected mode */
typedef struct
{
    int        valid __attribute__((packed));
    LPVOID_PM  pm    __attribute__((packed));
    LPVOID_RM  rm    __attribute__((packed));
} LPVOID_X;

/* DOSX_REGS - Used by DOSX functions to pass registers to real mode
 * functions or interrupts. Has the same layout as the DPMI real mode
 * register data structure. I have splitted this stuff for DJGPP
 * porters. I know that packing structs for that compiler is a wee
 * bit troublesome. :-) */

typedef struct
{
    WORD    di                __attribute__((packed));
    WORD    hdi               __attribute__((packed));
    WORD    si                __attribute__((packed));
    WORD    hsi               __attribute__((packed));
    WORD    bp                __attribute__((packed));
    WORD    hbp               __attribute__((packed));
    DWORD   reserved          __attribute__((packed));
    WORD    bx                __attribute__((packed));
    WORD    hbx               __attribute__((packed));
    WORD    dx                __attribute__((packed));
    WORD    hdx               __attribute__((packed));
    WORD    cx                __attribute__((packed));
    WORD    hcx               __attribute__((packed));
    WORD    ax                __attribute__((packed));
    WORD    hax /* are us. */ __attribute__((packed));
} __ahx_word_regs;

typedef struct
{
    DWORD   edi      __attribute__((packed));
    DWORD   esi      __attribute__((packed));
    DWORD   ebp      __attribute__((packed));
    DWORD   reserved __attribute__((packed));
    DWORD   ebx      __attribute__((packed));
    DWORD   edx      __attribute__((packed));
    DWORD   ecx      __attribute__((packed));
    DWORD   eax      __attribute__((packed));
} __ahx_dword_regs;

typedef struct
{
    WORD    es __attribute__((packed));
    WORD    ds __attribute__((packed));
    WORD    fs __attribute__((packed));
    WORD    gs __attribute__((packed));
    WORD    ip __attribute__((packed));
    WORD    cs __attribute__((packed));
    WORD    sp __attribute__((packed));
    WORD    ss __attribute__((packed));
} __ahx_segment_regs;

typedef union
{
    __ahx_word_regs     x __attribute__((packed));
    __ahx_dword_regs    e __attribute__((packed));
} __ahx_regs;

/* Completion of DOSX_REGS structure */
typedef struct
{
    __ahx_regs          r     __attribute__((packed));
    WORD                flags __attribute__((packed));
    __ahx_segment_regs  s     __attribute__((packed));
#define RX r.x
#define RE r.e
} DOSX_REGS;

/* DESCRIPTOR - Here's the descriptor structure, also splitted up. */
typedef struct
{
    WORD limit_lo    __attribute__((packed));
    WORD base_lo     __attribute__((packed));
    BYTE base_hi     __attribute__((packed));
    BYTE rts_lo      __attribute__((packed));
    BYTE limitrts_hi __attribute__((packed));
    BYTE base_xhi    __attribute__((packed));
} __ahx_descr;

typedef union
{
    WORD        w[4] __attribute__((packed));
    BYTE        b[8] __attribute__((packed));
    __ahx_descr d    __attribute__((packed));
} DESCRIPTOR;

/* MAKELINEAR() - Converts real mode address to linear address */
#define MAKELINEAR(seg, off) \
    ((((DWORD)(seg)) << 4UL) + (DWORD)(off))

#ifdef __cplusplus
extern "C" {
#endif

int dosx_int (int, DOSX_REGS FAR*);
void dosx_vm_idle (void);

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

#ifdef __cplusplus
}
#endif

#endif /* __DOSX_H */
