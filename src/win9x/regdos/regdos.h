/*
 * regdos.h, Accesses the registry from a DOS box.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997.
 */
#ifndef __REGDOS_H
#define __REGDOS_H

#include "ahxtyps.h"

#ifndef ERROR_SUCCESS
#   define ERROR_SUCCESS   0L
#endif

#ifndef ERROR_MORE_DATA
#   define ERROR_MORE_DATA 234L
#endif

/* For RegEnumValue() and RegEnumKey() */
#ifndef ERROR_NO_MORE_ITEMS
#   define ERROR_NO_MORE_ITEMS 259L
#endif

#ifndef ERROR_CANTREAD
#   define ERROR_CANTREAD 1012L
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef LPVOID HKEY;

#define __HKEY_CLASSES_ROOT               ((HKEY)0x80000000)
#define __HKEY_CURRENT_USER               ((HKEY)0x80000001)
#define __HKEY_LOCAL_MACHINE              ((HKEY)0x80000002)
#define __HKEY_USERS                      ((HKEY)0x80000003)
#define __HKEY_PERFORMANCE_DATA           ((HKEY)0x80000004)
#define __HKEY_CURRENT_CONFIG             ((HKEY)0x80000005)

#define HKEY_CLASSES_ROOT                 __HKEY_CLASSES_ROOT
#define HKEY_CURRENT_USER                 __HKEY_CURRENT_USER
#define HKEY_LOCAL_MACHINE                __HKEY_LOCAL_MACHINE
#define HKEY_USERS                        __HKEY_USERS
#define HKEY_PERFORMANCE_DATA             __HKEY_PERFORMANCE_DATA
#define HKEY_CURRENT_CONFIG               __HKEY_CURRENT_CONFIG

/* Reg types */
#define REG_NONE                          (0)
#define REG_SZ                            (1)   /* Unicode */
#define REG_EXPAND_SZ                     (2)   /*  '   ' but with environment
                                                   variable expansion */
#define REG_BINARY                        (3)   /* Binary */
#define REG_DWORD                         (4)   /* 32 bit number */
#define REG_DWORD_LITTLE_ENDIAN           (4)
#define REG_DWORD_BIG_ENDIAN              (5)   /* 32-bit number, big end */

#define REGDOS_PROC

LONG REGDOS_PROC __RegOpenKey  (HKEY, LPCSTR, HKEY FAR*);
#define RegOpenKey __RegOpenKey

LONG REGDOS_PROC __RegCreateKey(HKEY, LPCSTR, HKEY FAR*);
#define RegCreateKey __RegCreateKey

LONG REGDOS_PROC __RegCloseKey (HKEY);
#define RegCloseKey __RegCloseKey

LONG REGDOS_PROC __RegDeleteKey (HKEY, LPCSTR);
#define RegDeleteKey __RegDeleteKey

LONG REGDOS_PROC __RegDeleteValue (HKEY, LPCSTR);
#define RegDeleteValue __RegDeleteValue

LONG REGDOS_PROC __RegQueryValue (HKEY, LPCSTR, LPSTR, LPLONG);
#define RegQueryValue __RegQueryValue

LONG REGDOS_PROC __RegQueryValueEx (HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE,
				    LPDWORD);
#define RegQueryValueEx __RegQueryValueEx

LONG REGDOS_PROC __RegEnumKey(HKEY, DWORD, LPSTR, DWORD);
#define RegEnumKey __RegEnumKey

LONG REGDOS_PROC __RegEnumValue(HKEY, DWORD, LPSTR, LPDWORD, LPDWORD, LPDWORD,
				LPBYTE, LPDWORD);
#define RegEnumValue __RegEnumValue

LONG REGDOS_PROC __RegSetValueEx(HKEY, LPCSTR, DWORD, DWORD, LPBYTE, DWORD);
#define RegSetValueEx __RegSetValueEx

int regdos_startup (void);
int regdos_cleanup (void);

#ifdef __cplusplus
}
#endif

#endif /* __REGDOS_H */
