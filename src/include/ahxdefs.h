/* ahxdefs.h, compiler / system specific definitions.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997-1998.
 *
 * Contributions by:
 *
 * (gf)         = George Foot <george.foot@merton.oxford.ac.uk>
 * JV>          = Jacob Verhoeks <jacob@duoware.demon.nl>
 * 0x0a11fab2   = Alfons Hoogervorst <alfons@hoogervorst.demon.nl>
 */

#ifndef __AHXDEFS_H
#define __AHXDEFS_H

/* __BEGIN_EXTERN_C, __END_EXTERN_C, __EXTERN C - Language independent
 * externals */
#ifdef __cplusplus
#   define __BEGIN_EXTERN_C    extern "C" {
#   define __END_EXTERN_C      }
#   define __EXTERN_C          extern "C"
#else
#   define __BEGIN_EXTERN_C
#   define __END_EXTERN_C
#   define __EXTERN_C          extern
#endif


/* PACK_ME - Several structure members need to be byte packed. Define
 * your compilers stuff here. Example useage in headers:
 *
 *       typedef struct
 *       {
 *          int     useful          PACK_ME;
 *          BYTE    unuseful        PACK_ME;
 *          LONG    very_useful     PACK_ME;
 *       } I_USED_TO_BE_UNPACKED; */

#if defined(__SC__) || defined(__BORLANDC__) || defined(__WATCOMC__)
#   define PACK_ME
/* 01-15-98 (gf) */
#elif defined(__DJGPP__)
#   define PACK_ME      __attribute__((__packed__))
#else
#   define PACK_ME
#endif


/* INLINE - If your C compiler has an extended keyword for making functions
 * inline, set macro INLINE to that keyword. (Symantec and Microsoft have
 * the __inline keyword). Example usage:
 *
 *     static INLINE int inline_function(void)
 *     {
 *          return 1;
 *     } */

#if !defined(INLINE)
#   if defined(__SC__)
#       define INLINE __inline
#   else
#       define INLINE
#   endif
#endif


/* Define functions and macros that help porting the int packages.
 * You can choose to write an entire new DOSX module, or use dosxgenc.c.
 * In the case of the latter, you should define the following macros. */
#if defined(__SC__) || defined(__BORLANDC__) || defined(__PACIFIC__)
/* Borland C++ and Symantec C++ are mimicking MS C */
#   define INT86       int86
#   define INT86X      int86x
#   define WREGS(r)    r.x
#   define EREGS(r)    r.e
#   define CFLAG(r)    r.x.cflag
#elif defined(__WATCOMC__)
/* Watcom does it a little bit odd. */
#   define WREGS(r)    r.w
#   define EREGS(r)    r.x
#   define CFLAG(r)    r.x.cflag
#   if defined(PF_MSDOS_32)
#       define INT86       int386
#       define INT86X      int386x
#   elif defined(PF_MSDOS) || defined(PF_MSDOS_16)
#       define INT86       int86
#       define INT86X      int86x
#   endif
#endif


/* Calling conventions, far and near pointers. These stuff are only
 * for _MSDOS && _WIN16 type of things. */

#if defined(PF_MSDOS_32)
/* NO FAR NEAR FOR 32 BIT DOS EXTENDERS! */
#elif defined(PF_WINDOWS)
/* For Windows platforms, FAR and NEAR will be defined later on */
#   define AHX_SKIP_CC
#   define AHX_SKIP_FAR_NEAR
#elif defined(PF_MSDOS_16)
#   define AHX_SUPP_FAR_NEAR
#elif defined(PF_MSDOS)
#   define AHX_SUPP_CC
#   define AHX_SUPP_FAR_NEAR
/* For Pacific C (large model) we undefine it :) */
#   if defined(__PACIFIC__) && defined(__LARGE__)
#       undef AHX_SUPP_CC
#       undef AHX_SUPP_FAR_NEAR
#   endif
#endif /* PF_MSDOS_32 */


/* FAR and NEAR pointers. For DOS applications these definitions can
 * make code slightly more efficient. However, we don't want these
 * things for Win32 and _EXTENDED_DOS. Use AHX_SKIP_ to skip definitions
 * entirely. Use AHX_SUPP_ to define macros to the special keywords.
 * If you omit AHX_SUPP_ and AHX_SKIP_, the macros will be dummies. */

#if defined(AHX_SKIP_CC)
#   undef AHX_SKIP_CC
#else
#   if defined(AHX_SUPP_CC)
#       ifndef PASCAL
#           define PASCAL   __pascal
#       endif
#       ifndef CDECL
#           define CDECL    __cdecl
#       endif
#       undef AHX_SUPP_CC
#   else
#       ifndef PASCAL
#           define PASCAL
#       endif
#       ifndef CDECL
#           define CDECL
#       endif
#   endif
#endif

#if defined(AHX_SKIP_FAR_NEAR)
#   undef AHX_SKIP_FAR_NEAR
#else
#   if defined(AHX_SUPP_FAR_NEAR)
#       ifndef FAR
#           define FAR __far
#       endif
#       ifndef NEAR
#           define NEAR __near
#       endif
#       undef AHX_SUPP_FAR_NEAR
#   else
#       ifndef FAR
#           define FAR
#       endif
#       ifndef NEAR
#           define NEAR
#       endif
#   endif
#endif


/* SIGNAL_PROC - Some compilers need a special calling convention for a
 * signal() function. */

#ifdef __TURBOC__
#   define SIGNAL_PROC CDECL
#elif __BORLANDC__ >= 0x0400
#   define SIGNAL_PROC _USERENTRY
#elif defined(__SC__)
#   define SIGNAL_PROC
#else
#   define SIGNAL_PROC
#endif /* __TURBOC__ */


/* ATEXIT_PROC - Some compilers need a special calling convention for an
 * atexit() function. */


/* 01-15-98 * JV> Changed by Jacob Verhoeks to add support for BC 3.1
 */
#if defined(__BORLANDC__)
#  if defined(PF_MSDOS_32) /* use _USERENTRY only in 32 bit version */
#     define ATEXIT_PROC     _USERENTRY
#  else   /* BC 3.1 dos */
#     define ATEXIT_PROC
#  endif /* PF_MSDOS_32 */
#else
#   define ATEXIT_PROC
#endif /* __BORLANDC__ */


#endif /* __AHXDEFS_H */
