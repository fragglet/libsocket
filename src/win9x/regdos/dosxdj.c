/*
    Richard Dawe (libsocket): This file is part of RegDos & therefore covered
    by its copyright!
*/

/* dosxdjgpp.c, DOS eXtender functions using DPMI exclusively.
 * Not all functions were tested yet. Only those I needed very badly.
 *
 * NOTE: DJGPP specific.
 */

/*
 * By George Foot <george.foot@merton.oxford.ac.uk>, 1998.
 */

/* 01-15-98 * (gf) * Caveat implementor!  I wasn't trying too hard when
 *                   I wrote some of these functions; my first goal was to
 *                   get the registry interface to work in djgpp.  This
 *                   doesn't need some of the functions so they could well
 *                   be wrong.
 */

/*
 * Some modifications by Richard Dawe <rd5718@bristol.ac.uk> on 1998-2-6 when
 * incorporating Regdos into libsocket for Indrek Mandre <indrek@warp.edu.ee>.
 */

#ifndef __DJGPP__
#error dosxdjgp.c should only be used for djgpp compilations
#endif


#include <assert.h>
#include <dpmi.h>
#include <go32.h>
#include <stdlib.h>
#include <sys/farptr.h>
#include <sys/movedata.h>

#include "dosx.h"

/* All functions return 0 if they are succesful. If an error occurred
 * they return a value != 0.
 *
 * I'm still confused by the DPMI specs. Some interrupt list seem to
 * differ on whether a particular DPMI version returns an error in
 * register AX... So, if an error occurs, and AX turns up empty,
 * my code returns __LINE__. Set the high bit to distinguish our
 * errors.
 *
 * (gf): Since I'm using djgpp's dpmi interface functions and they 
 *       don't return meaningful error codes, I've used Alfons' 
 *       DPMI_ERROR system almost everywhere.
 */

#define DPMI_ERROR (int)(((DWORD)__LINE__) | 0x80000000UL)

/* (gf): DJGPP already has a structure with the registers in the DPMI
 *       format so I could probably simply cast the pointers here; for
 *       safety I've done it this way though.  I'll test it like this
 *       and when it works I'll try the cast.  The final method will 
 *       be whatever Alfons prefers of course ;).
 */

static void __gf_copy_registers_to_djgpp_format (DOSX_REGS *dr, __dpmi_regs *r) {
    #define __gf_copy(reg) r->d.##reg = dr->r.e.##reg
    __gf_copy (edi);
    __gf_copy (esi);
    __gf_copy (ebp);
    __gf_copy (ebx);
    __gf_copy (edx);
    __gf_copy (ecx);
    __gf_copy (eax);
    #undef __gf_copy

    r->d.res = dr->r.e.reserved;
    r->x.flags = dr->flags;

    #define __gf_copy(reg) r->x.##reg = dr->s.##reg
    __gf_copy (es);
    __gf_copy (ds);
    __gf_copy (fs);
    __gf_copy (gs);
    __gf_copy (ip);
    __gf_copy (cs);
    __gf_copy (sp);
    __gf_copy (ss);
    #undef __gf_copy 
}

static void __gf_copy_registers_from_djgpp_format (__dpmi_regs *r, DOSX_REGS *dr) {
    #define __gf_copy(reg) dr->r.e.##reg = r->d.##reg
    __gf_copy (edi);
    __gf_copy (esi);
    __gf_copy (ebp);
    __gf_copy (ebx);
    __gf_copy (edx);
    __gf_copy (ecx);
    __gf_copy (eax);
    #undef __gf_copy

    dr->r.e.reserved = r->d.res;
    dr->flags = r->x.flags;

    #define __gf_copy(reg) dr->s.##reg = r->x.##reg
    __gf_copy (es);
    __gf_copy (ds);
    __gf_copy (fs);
    __gf_copy (gs);
    __gf_copy (ip);
    __gf_copy (cs);
    __gf_copy (sp);
    __gf_copy (ss);
    #undef __gf_copy 
}

int dosx_int(int intno, DOSX_REGS FAR* dr) {
    __dpmi_regs r;
    __gf_copy_registers_to_djgpp_format (dr, &r);
    __dpmi_int (intno, &r);
    __gf_copy_registers_from_djgpp_format (&r, dr);
    return 0;
}

void dosx_vm_idle(void) {
    __dpmi_yield();
}

int dosx_dos_alloc(LPVOID_X FAR* news, unsigned siz) {
    int seg, sel;

    if ( news == NULL ) return DPMI_ERROR;
    news->valid = 0;

    seg = __dpmi_allocate_dos_memory ((siz + 15) / 16, &sel);
    if (seg == -1) return DPMI_ERROR;

    news->pm.sel = sel;
    news->pm.off = 0;

    news->rm.seg = seg;
    news->rm.off = 0;

    news->valid = 1;
    return 0;
}

int dosx_dos_free(LPVOID_X old) {
    if (!old.valid) return 0x8022;
    return (__dpmi_free_dos_memory (old.pm.sel) == -1) ? DPMI_ERROR : 0;
}

/* RD: Modified to allocate 'count' selectors, like other modules for other
       compilers; this has the side effect of preventing a compiler error
       with dosx.h */

/* RD: Added 2nd param */
int dosx_alloc_selector(SELECTOR FAR* sel, unsigned count) {
    int _sel;
    if (!sel) return DPMI_ERROR;
    *sel = _sel = __dpmi_allocate_ldt_descriptors (count);  /* RD: 1 -> count */
    return (_sel == -1) ? DPMI_ERROR : 0;
}

int dosx_free_selector(SELECTOR sel) {
    return (__dpmi_free_ldt_descriptor (sel) == -1) ? DPMI_ERROR : 0;
}

int dosx_alloc_alias(SELECTOR FAR* alias, SELECTOR copy) {
    int _alias;
    if (!alias) return DPMI_ERROR;
    *alias = _alias = __dpmi_create_alias_descriptor (copy);
    return (_alias == -1) ? DPMI_ERROR : 0;
}

SELECTOR dosx_current_ds(void) {
    return _my_ds();
}

SELECTOR dosx_current_cs(void) {
    return _my_cs();
}

SELECTOR dosx_current_ss(void) {
    return _my_ss();
}

int dosx_get_selector_base(SELECTOR sel, DWORD FAR* lin) {
    if (!lin) return DPMI_ERROR;
    return (__dpmi_get_segment_base_address (sel, lin) == -1) ? DPMI_ERROR : 0;
}

int dosx_set_selector_base(SELECTOR sel, DWORD lin) {
    if (!sel) return 0x8022;
    return (__dpmi_set_segment_base_address (sel, lin) == -1) ? DPMI_ERROR : 0;
}

int dosx_get_descriptor(SELECTOR sel, DESCRIPTOR FAR* des) {
    if (!sel) return 0x8022;
    if (!des) return DPMI_ERROR;
    return (__dpmi_get_descriptor (sel, des) == -1) ? DPMI_ERROR : 0;
}

int dosx_set_descriptor(SELECTOR sel, CONST DESCRIPTOR FAR* des) {
    if (!sel) return 0x8022;
    if (!des) return DPMI_ERROR;
    return (__dpmi_set_descriptor (sel, (void *)des) == -1) ? DPMI_ERROR : 0;
}

int dosx_set_selector_limit(SELECTOR sel, DWORD lim) {
    return (__dpmi_set_segment_limit (sel, lim) == -1) ? DPMI_ERROR : 0;
}

int dosx_get_selector_limit(SELECTOR sel, DWORD FAR* lim) {
    int result;
    DWORD limit = 0;
    DESCRIPTOR des;
    if ( lim == NULL || sel == 0 ) return DPMI_ERROR;
    result = dosx_get_descriptor(sel, &des); 
    assert(result == 0);
    if ( result == 0 )
    {
        limit = ((DWORD)(des.d.limitrts_hi & 0x0f) << 16UL) +
            (DWORD)des.d.limit_lo;
        if ( des.d.limitrts_hi & 0x80 ) limit *= 4096;
    }
    *lim = limit;
    return result;
}

int dosx_call_far_real(LPVOID_RM proc, DOSX_REGS FAR* dr) {
    __dpmi_regs r;
    int x;

    if ((!dr) || ((!proc.seg) && (!proc.off))) return DPMI_ERROR;
    __gf_copy_registers_to_djgpp_format (dr,&r);
    r.x.cs = proc.seg;
    r.x.ip = proc.off;
    x = __dpmi_simulate_real_mode_procedure_retf (&r);
    __gf_copy_registers_from_djgpp_format (&r,dr);
    return (x == -1) ? DPMI_ERROR : 0;
}

int dosx_int_real(int intno, DOSX_REGS FAR* dr) {
    __dpmi_regs r;
    int x;

    if (!dr) return DPMI_ERROR;
    __gf_copy_registers_to_djgpp_format (dr,&r);
    x = __dpmi_simulate_real_mode_interrupt (intno, &r);
    __gf_copy_registers_from_djgpp_format (&r,dr);
    return (x == -1) ? DPMI_ERROR : 0;
}

void dosx_far_near_copy(LPVOID_PM dst, LPCVOID src, unsigned count) {
    movedata (_my_ds(), (int)src, dst.sel, dst.off, count);
}

void dosx_near_far_copy(LPVOID dst, LPVOID_PM src, unsigned count) {
    movedata (src.sel, src.off, _my_ds(), (int)dst, count);
}

unsigned dosx_far_strlen(LPVOID_PM str) {
    unsigned x = str.off;
    _farsetsel (str.sel);
    while (_farnspeekb(x)) x++;
    return x - str.off;
}

void dosx_far_set(LPVOID_PM ptr, int value, unsigned size) {
    unsigned x;
    _farsetsel (ptr.sel);
    for (x = ptr.off; x < ptr.off + size; x++) _farnspokeb (x, value);
}

LPVOID_PM dosx_far_address_of(LPCVOID p) {
    LPVOID_PM result;
    result.sel = _my_ds();
    result.off = (int)p;
    return result;
}

DWORD dosx_linear_address_of(LPCVOID p) {
    DWORD base;
    dosx_get_selector_base(_my_ds(), &base);
    return base + (int)p;
}

/* dosx_map_linear() - Maps a linear address to a SEL:OFF32 pointer (FWORD).
 * The returned LPVOID_PM should be freed with dosx_unmap_linear() (or
 * by freeing the selector in the structure) */
int dosx_map_linear(LPVOID_PM FAR* res_ptr, DWORD linear) {
    LPVOID_PM p;
    int result;
    if ( res_ptr == NULL ) return DPMI_ERROR;
 /* FIXME: Assuming ds has an OK limit */
    result = dosx_alloc_alias(&p.sel, dosx_current_ds());
    assert(result == 0);
    result = dosx_set_selector_base(p.sel, linear);
    assert(result == 0);
    if ( result != 0 )
    {
        dosx_free_selector(p.sel);
        return DPMI_ERROR;
    }
    p.off = 0;
    *res_ptr = p;
    return 0;
}

int dosx_unmap_linear(LPVOID_PM p) {
    return dosx_free_selector(p.sel);
}

/* dosx_map_addr16() - Phoney, phoney, phoney. Equivalent to dosx_map_linear(),
 * but returns a 16 bit protected mode address. (But in a handy form.) */
DWORD dosx_map_addr16(LPCVOID address) {
    LPVOID_PM pm_addr;
    int result = dosx_map_linear(&pm_addr, dosx_linear_address_of(address));
    if ( result == 0 )
        return MAKELONG(pm_addr.off, pm_addr.sel);
    else return 0;
}

int dosx_unmap_addr16(DWORD dw) {
    return dosx_free_selector(HIWORD(dw));
}

/* dosx_make_selector16() - Twiggles a 32 bit selector in a 16 bit one.
 * Will affect instructions in the associated segment. */
int dosx_make_selector16(SELECTOR s) {
    int result;
    DESCRIPTOR four_seasons;
    result = dosx_get_descriptor(s, &four_seasons);
    assert(result == 0);
    if ( result != 0 ) return DPMI_ERROR;
 /* 2nd bit of 1st nibble of 6th byte of our pizza */
 /* (For hungry programmers: 6th bit of 6th byte) */
    four_seasons.b[6] &= ~0x40;
    result = dosx_set_descriptor(s, &four_seasons);
    assert(result == 0);
    return result;
}

/* dosx_presto_chango_selector() - Data selector -> code selector,
 * code selector -> data selector. Tribute to nifty Windows 3.x function,
 * which had to be exported as "ChangeSelector", but for some reason
 * was exported as "PrestoChangoSelector". */
int dosx_presto_chango_selector(SELECTOR s) {
    int result = 0;
    DESCRIPTOR four_seasons;
    result = dosx_get_descriptor(s, &four_seasons);
    assert(result == 0);
    if ( result != 0 ) return DPMI_ERROR;
 /* Flip bit 3 of type bits in byte 5:
  *
  *       7 65 4 321 0       A = Accessed
  *      +-+--+-+---+-+    TYP = Segment type
  *      |P|PL|S|TYP|A|  (D)PL = Descriptor Privilege Level
  *      +-+--+-+---+-+      P = Present
  *
  * For your information the possible values of TYP:
  *
  *  000 - read only
  *  001 - read/write
  *  010 - read-only, expand down
  *  011 - read-write, expand down
  *  100 - execute-only
  *  101 - execute/read
  *  110 - execute-only, conforming
  *  111 - execute/read-only, conforming
  */
    four_seasons.b[5] ^= 0x8; /* 1000b */
    result = dosx_set_descriptor(s, &four_seasons);
    assert(result == 0);
    return result;
}

int dosx_alloc_copy(SELECTOR FAR* dst, SELECTOR src) {
    int result;
    SELECTOR s;
    DESCRIPTOR four_seasons;
    if ( dst == 0 ) return DPMI_ERROR;
    result = dosx_get_descriptor(src, &four_seasons);
    assert(result == 0);
    if ( result != 0 ) return DPMI_ERROR;
    result = dosx_alloc_alias(&s, src);
    assert(result == 0);
    if ( result != 0 ) return DPMI_ERROR;
 /* FIXME: clean up this one. :) */
    if ( 0 != (result = dosx_set_descriptor(s, &four_seasons)) )
    {
        dosx_free_selector(s);
        return DPMI_ERROR;
    }
    *dst = s;
    return 0;
}

int dosx_lock_memory(LPVOID_PM address, unsigned size) {
    __dpmi_meminfo info;
    DWORD      d;

    dosx_get_selector_base(address.sel, &d);

    info.size = size;
    info.address = d + address.off;

    return (__dpmi_lock_linear_region (&info) == -1) ? DPMI_ERROR : 0;
}

int dosx_unlock_memory(LPVOID_PM address, unsigned size) {
    __dpmi_meminfo info;
    DWORD      d;

    dosx_get_selector_base(address.sel, &d);

    info.size = size;
    info.address = d + address.off;

    return (__dpmi_unlock_linear_region (&info) == -1) ? DPMI_ERROR : 0;
}
