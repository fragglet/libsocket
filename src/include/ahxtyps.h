/* ahxtyps.h, portable Windows style type definitions.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997.
 */
#ifndef __AHXTYPS_H
#define __AHXTYPS_H

#if defined(PF_MSDOS_32) || defined(PF_MSDOS) || defined(PF_MSDOS_16)

#   if !defined(CONST)
#       define CONST const
#   endif

#define __DEFPTRS(p) \
    typedef p FAR*          LP ## p; \
    typedef CONST p FAR*    LPC ## p;\
    typedef p NEAR*         NP ## p; \
    typedef CONST p NEAR*   NPC ## p;\
    typedef p*              P ## p; \
    typedef CONST p*        PC ## p

/* Some Windows style types */
/* 0x0A11FAB2 - There's a problem with pointers. WSOCK.VXD wants 16 bit
 * pointers. WSOCK.H now uses PTR1616 where necessary. */

/* VOID */
typedef void            VOID;
__DEFPTRS(VOID);

/* BYTE, WORD and DWORD are machine dependent data types */

/* BYTE is a 8 bit data type */
typedef unsigned char   BYTE;
__DEFPTRS(BYTE);

/* WORD is a 16 bit data type */
typedef unsigned short  WORD;
__DEFPTRS(WORD);

/* DWORD is a 32 bit data type */
typedef unsigned long   DWORD;
__DEFPTRS(DWORD);

/* char string data type */
typedef char            STR;
typedef char            CHAR;
__DEFPTRS(STR);
__DEFPTRS(CHAR);

/* boolean data type */
typedef int             BOOL;
__DEFPTRS(BOOL);

#define FALSE           0
#define TRUE            1

/* native integer data type */
typedef int             INT;
__DEFPTRS(INT);

/* native short integer data type */
typedef short int       SHORT;
__DEFPTRS(SHORT);

/* native unsigned integer data type */
typedef unsigned        UINT;
__DEFPTRS(UINT);

/* FIXME: a LONG is a long data type, but on intel specific targets,
 * it's assumed to be 32 bits. Blame IBM and Microsoft. */
/* native signed long data type */
typedef long            LONG;
__DEFPTRS(LONG);


#ifndef MAKEWORD
#   define MAKEWORD(lo, hi) \
((((WORD)(lo)) & 0xff) | (((WORD)(hi)) << 8))
#endif

/* FIXME: See definition of LONG. This macro should be called MAKEDWORD() */
#ifndef MAKELONG
#   define MAKELONG(lo, hi) \
((((DWORD)(lo)) & 0xffffUL) | ((((DWORD)(hi)) & 0xffffUL) << 16UL))
#endif

#ifndef MAKELP
#   define MAKELP(sel, off)     (LPVOID)MAKELONG(off, sel)
#endif

#ifndef SELECTOROF
#   define SELECTOROF(p)        HIWORD(p)
#endif

#ifndef LOBYTE
#   define LOBYTE(w)            (((WORD)(w)) & 0xff)
#endif

#ifndef HIBYTE
#   define HIBYTE(w)           ((((WORD)(w)) >> 8) & 0xff)
#endif

#ifndef LOWORD
#   define LOWORD(l)            ((WORD)((l) & 0xFFFF))
#endif

#ifndef HIWORD
#   define HIWORD(l)        (WORD)((((DWORD)(l)) & 0xFFFF0000UL) >> 16UL)
#endif

#endif /* platforms */

#endif /* __AHXTYPS_H */
