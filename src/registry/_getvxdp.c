/* _getvxdp.c, extra definitions. Should be included by getvxdp.c.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997-1998.
 */

#if defined(PF_MSDOS_32)

/* For both Symantec C++ and Watcom C++ the following works fine. */
#   if defined(__SC__) || defined(__WATCOMC__) || defined(__DJGPP__)

#include <dos.h>
VXD_PROC get_vxd_entry_point(unsigned short id)
{
    VXD_PROC a;
    union REGS r;
    struct SREGS s;
    segread(&s);
    s.es = 0;
    WREGS(r).di = 0;
    WREGS(r).ax = 0x1684;
    WREGS(r).bx = id;
    INT86X(0x2f, &r, &r, &s);
    a.sel = s.es;
    a.off = WREGS(r).di;
#if 0
    dbgprint("VxD(%d) proc @ %04.4x:%04.4x\n", id, s.es, WREGS(r).di);
#endif
    return a;
}
#   else
#       error Define get_vxd_entry_point() for your compiler (PF_MSDOS_32)
#   endif

#else /* PF_MSDOS */

#   if defined(__WATCOMC__)

VXD_PROC a11_get_vxd_entry_point(unsigned short);
#       pragma aux a11_get_vxd_entry_point = \
            "push es" \
            "xor di,di" \
            "mov es,di" \
            "mov ax,1684h" \
            "int 2fh" \
            "mov ax,di" \
            "mov dx,es" \
            "pop es" \
            parm [bx] value [dx ax] modify [di si];
VXD_PROC get_vxd_entry_point(unsigned short id)
{
    VXD_PROC p = a11_get_vxd_entry_point(id);
#if 0
    dbgprint("VxD(%d) proc @ %04.4x:%04.4x\n", id, HIWORD((DWORD)p), LOWORD((DWORD)p));
#endif
    return p;
}

#   elif defined(__SC__) || defined(__TURBOC__)

/* This one should work fine with Symantec and Borland. In this case
 * inline assembly is preferable, because Symantec's RTL seems to
 * filter INT 2FHs. */
VXD_PROC get_vxd_entry_point(unsigned short id)
{
    VXD_PROC ep;
    __asm push ds
    __asm push es
    __asm push di
    __asm push si

    __asm xor di,di
    __asm mov es,di
    __asm mov bx,id
    __asm mov ax,0x1684
    __asm int 0x2f

    __asm mov WORD PTR [ep],di
    __asm mov WORD PTR [ep+2],es

g_v_e_p_cleanup:
    __asm pop si
    __asm pop di
    __asm pop es
    __asm pop ds
    return ep;
}

#   elif defined(__PACIFIC__)

/* NOTE: turn off "no return" warnings */
VXD_PROC get_vxd_entry_point(unsigned short id)
{
    asm("push ds");
    asm("push es");
    asm("push si");
    asm("push di");
    asm("xor di,di");
    asm("mov es,di");
    asm("mov bx,dx");
    asm("mov ax,#01684h");
    asm("int #02fh");
    asm("mov dx,es");
    asm("mov ax,di");
    asm("pop di");
    asm("pop si");
    asm("pop es");
    asm("pop ds");
/* NOTE: __PACIFIC__ no return, set warning level to 3. */
}

#   else
#       error Unsupported compiler
#   endif /* a compiler */

#endif /* a platform */
