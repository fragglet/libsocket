/* ahxcfg.h, configuration stuff.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997-1998.
 * AHX is short for Accumulative Header. (X added because it sounded nicer)
 */

#ifndef __AHXCFG_H

/* Allow new options stuff */
#define AHX_OPTIONS

/* Allow DNS lookup */
#define AHX_DNS

/* Debug / tracing off */
/*#define NODEBUG*/
/*#define NDEBUG*/

/* Choose whenever possible for the compiler native things. */
/* #define AHX_COMPILER_NATIVE */

/* This setting acknowledges that you're are working with a
 * PC compiler which knows about the C calling conventions.
 * With AHX_RESTRICTIVE_C, some modules will export their
 * functions as CDECL. */
/*#define AHX_RESTRICTIVE_C*/


/****************************************************************************/


/*** DON'T TOUCH ANYTHING BELOW ***/

#if defined(AHX_RESTRICTIVE_C)
#   if defined(PF_MSDOS)
#       if defined(__SC__) || defined(__BORLANDC__) || defined(__PACIFIC__) || defined(__WATCOMC__)
#       else
#           error Unsupported compiler for AHX_RESTRICTIVE_C
#       endif
#   endif
#endif

/* __DNS__ - Portability with older code */
#if defined(AHX_DNS)
#   define __DNS__
#endif

#endif /* __AHXCFG_H */
