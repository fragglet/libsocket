/* ahxplatf.h, definitions for compilers / platforms.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997-1998.
 *
 * Contributions by:
 *
 * (gf)         = George Foot <george.foot@merton.oxford.ac.uk>
 * JV>          = Jacob Verhoeks <jacob@duoware.demon.nl>
 * 0x0a11fab2   = Alfons Hoogervorst <alfons@hoogervorst.demon.nl>
 */
#ifndef __AHXPLATF_H
#define __AHXPLATF_H

/* PLATFORM SELECTION MACROS. A compiler should define the
   following macros. For other platforms, define a macro
   yourself. Porters for OS-9 could perhaps use _OUTER_SPACE_OS

        PLATFORM             MACROS
        ----------------------------------------------------
        MS-DOS               PF_MSDOS
        Windows 3.x          PF_WIN16
        Windows NT, 95       PF_WIN32
        Windows              PF_WINDOWS
        Protected mode       PF_MSDOS_32, PF_MSDOS_16
        targets


        Special macros

        Intel 80x86          _INTEL_80x86
        Thread safe
            libraries        _MT
        Win DLL target       _DLL
*/

/* Borland && RSXNTDJ: merged these two because they use the same
 * defines. */
#if defined(__BORLANDC__) || defined(__TURBOC__) || defined(__RSXNT__)

#   if (__BORLANDC__ < 0x0300) || (__TURBOC__ < 0x0300)
#       error BCC / TCC versions < 3.0 not supported.
#   endif

#   ifndef _INTEL_80x86
#       define _INTEL_80x86
#   endif

#   if defined(__DLL__) && !defined(_DLL)
#       define _DLL
#   endif

#   if defined(__MT__) && !defined(_MT)
#       define _MT
#   endif

/* Now do the platform things. Windows first to catch oddities in Borland
 * and RSXNTDJ. */

#   if defined(_Windows) || defined(__RSXNT__)

#       if defined(__WIN32__)
#           if !defined(PF_WINDOWS)
#               define PF_WINDOWS 32
#           endif
#           if !defined(_WIN32)
#               define _WIN32
#           endif
#           if !defined(WIN32)
#               define WIN32
#           endif
#           if !defined(PF_WIN32)
#               define PF_WIN32
#           endif
#       else
#           if !defined(PF_WINDOWS)
#               define PF_WINDOWS 16
#           endif
#           if !defined(PF_WIN16)
#               define PF_WIN16
#           endif
#           if !defined(WIN16)
#               define WIN16
#           endif
#           if !defined(_WIN16)
#               define _WIN16
#           endif
#       endif /* __WIN32__ */

#   elif defined(__MSDOS__)

/* Don't know about any Borland Powerpacks. Assuming 16 bit real mode
 * things. */
#       if !defined(_MSDOS)
#           define _MSDOS
#       endif
#       if !defined(MSDOS)
#           define MSDOS
#       endif
#       if !defined(PF_MSDOS)
#           define PF_MSDOS
#       endif

#   endif /* platforms */

#endif /* Borland && RSXNT compilers */



/* WARNING: When testing for a target platform, test for a specific
 * platform first. For example, _MSDOS is defined for most _EXTENDED_DOS
 * targets. (That's a compiler specific problem.) */

/* Symantec compilers */
#if defined(__SC__)

#   define SIG_BYTES   0xB2, 0xFA, 0x11, 0x0A
#   define SIG_WORDS   0xFAB2, 0x0A11
#   define SIG_ULONG   0x0A11FAB2UL

#   if defined(_M_I86) || defined(M_I86) && !defined(_INTEL_80x86)
#       define _INTEL_80x86
#   endif

/* Protected mode */
#   if defined(DOS386)
#       ifndef _EXTENDED_DOS
#           define _EXTENDED_DOS
#       endif
#       ifndef EXTENDED_DOS
#           define EXTENDED_DOS
#       endif
#       ifndef PF_MSDOS_32
#           define PF_MSDOS_32
#       endif

/* Win32 */
#   elif defined(_WIN32) || defined(WIN32)

#       if !defined(_WIN32)
#          define _WIN32
#       endif
#       if !defined(WIN32)
#          define WIN32
#       endif
#       if !defined(PF_WIN32)
#          define PF_WIN32
#       endif
#       if !defined(PF_WINDOWS)
#          define PF_WINDOWS    32
#       endif
#       if !defined(_WINDOWS)
#          define _WINDOWS
#       endif
/* Symantec's libraries are thread safe */
#       ifndef _MT
#          define _MT
#       endif

#   elif defined(_WINDOWS) && (defined(_MSDOS) || defined(MSDOS))

#       ifndef PF_WIN16
#          define PF_WIN16
#       endif
#       ifndef PF_WINDOWS
#          define PF_WINDOWS 16
#       endif

#   elif defined(_MSDOS) || defined(MSDOS)
#       ifndef _MSDOS
#           define _MSDOS
#       endif
#       ifndef MSDOS
#           define MSDOS
#       endif
#       ifndef PF_MSDOS
#           define PF_MSDOS
#       endif

#   endif /* Platforms */

#endif /* __SC__ */



#if defined(__WATCOMC__)

#   ifndef _INTEL_80x86
#       define _INTEL_80x86
#   endif /* _INTEL_80x86 */

/* Paltforms for __WATCOMC__ */
#   if defined(__WINDOWS__) || defined(__NT__)
#       if defined(__NT__)
#           ifndef _WIN32
#               define _WIN32
#           endif
#           ifndef WIN32
#               define WIN32
#           endif
#           ifndef PF_WIN32
#               define PF_WIN32
#           endif
#           ifndef _MT
#               define _MT
#           endif
#       else
#           ifndef _WIN16
#               define _WIN16
#           endif
#           ifndef WIN16
#               define WIN16
#           endif
#           ifndef PF_WIN16
#               define PF_WIN16
#           endif
#       endif
#   ifndef PF_WINDOWS
#       define PF_WINDOWS
#   endif
#   elif defined(__386__)

#       if !defined(_EXTENDED_DOS)
#           define _EXTENDED_DOS
#       endif
#       if !defined(EXTENDED_DOS)
#           define EXTENDED_DOS
#       endif
#       ifndef PF_MSDOS_32
#           define PF_MSDOS_32
#       endif

#   elif defined(__DOS__)
#       if !defined(_MSDOS)
#           define _MSDOS
#       endif
#       if !defined(MSDOS)
#           define MSDOS
#       endif
#       if !defined(PF_MSDOS)
#           define PF_MSDOS
#       endif

#   endif /* platform */

#endif /* __WATCOMC__ */


/* 01-15-98 * (gf)       * Might be worth checking that they're not using
 *                         RSXNTDJ here; but I don't know anything about
 *                         RSXNTDJ.
 * 01-16-98 * 0x0a11fab2 * Added &&  !defined(__RSXNT__)
 */
#if defined(__DJGPP__) && !defined(__RSXNT__)

#   ifndef _INTEL_80x86
#       define _INTEL_80x86
#   endif
#   if !defined(_EXTENDED_DOS)
#       define _EXTENDED_DOS
#   endif
#   if !defined(EXTENDED_DOS)
#       define EXTENDED_DOS
#   endif
#   ifndef PF_MSDOS_32
#       define PF_MSDOS_32
#   endif

#endif /* __DJGPP__ */


/* 01-19-98 * JV> Added MS Visual C
 */
#if defined(_MSC_VER)

#   if defined(_M_I86) || defined(M_I86) && !defined(_INTEL_80x86)
#       define _INTEL_80x86
#   endif

/* Win32 */
#   if defined(_WIN32) || defined(WIN32)

#       if !defined(_WIN32)
#          define _WIN32
#       endif
#       if !defined(WIN32)
#          define WIN32
#       endif
#       if !defined(PF_WIN32)
#          define PF_WIN32
#       endif
#       if !defined(PF_WINDOWS)
#          define PF_WINDOWS    32
#       endif
#       if !defined(_WINDOWS)
#          define _WINDOWS
#       endif
/* MS's libraries are thread safe */
#       ifndef _MT
#          define _MT
#       endif

#   elif defined(_WINDOWS) && (defined(_MSDOS) || defined(MSDOS))

#       ifndef PF_WIN16
#          define PF_WIN16
#       endif
#       ifndef PF_WINDOWS
#          define PF_WINDOWS 16
#       endif

#   endif /* Platforms */

#endif /* _MSC_VER */


/* Pacific / Hi-Tech C. CantbelieveIreallymanagedtodothis. :) */
#if defined(__PACIFIC__) && defined(i8086)
/* Do memory model definitions */
#   if defined(LARGE_MODEL)
#       ifndef __LARGE__
#           define __LARGE__
#       endif
#   elif defined(SMALL_MODEL)
#       ifndef __SMALL__
#           define __SMALL__
#       endif
#   else
#       error Pacific C: Mixed-Up Memory Model
#   endif /* Pacific C memory model */
/* Processor */
#   ifndef _INTEL_80x86
#       define _INTEL_80x86
#   endif
/* Platform supported */
#   ifndef PF_MSDOS
#       define PF_MSDOS
#   endif

#   ifndef _MSDOS
#       define _MSDOS
#   endif

#   ifndef MSDOS
#       define MSDOS
#   endif

#endif /* __PACIFIC__ && i8086 */


#endif /* __AHXPLATF_H */
