/* regdosz.c, registry access from within a DOS program. This module should
 * be pretty close to portable to a lot of platforms, memory models and
 * compilers. Hence the Z (because it's the answer to Y). If you can't
 * get this working with another compiler, send me a note.
 *
 * The problem I wrestled with was passing parameters from a C function to
 * system code (registry service functions). I used some very wicked
 * inline assembly stuff, until I realized that the lstack.c macros provide
 * just the things I needed. Put the parameters in a local stack, and then
 * blit the contents into the memory space of the system... How easy.
 *
 * By Alfons Hoogervorst / Alliterated Software Development, 1997-1998.
 */

/* Some modifications by Richard Dawe <rd5718@bristol.ac.uk>, while modifying
   the library for incorporation into the libsocket library by Indrek Mandre
   <indrek@warp.edu.ee>. These modifications are indicated by comments starting
   "RD:" - if more than one line is changed, then the changes are enclosed in
   starting and ending comments.

   The changes I have made are:

   - Transparent usage of Reg* functions, i.e. regdos_startup() called
     automatically.

   - regdos_cleanup() called on exit automatically.
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>     /* RD: Need this for use of atexit() */
#include <string.h>
#include <assert.h>

#include "ahxtyps.h"
#include "dosx.h"
#include "regdos.h"
#include "lstack.h"
#include "vwin32.h"

/* RD: Changed this so it's assigned zero before usage */
static int been_here_before = 0;            /* Module available flag */

/* RD: regdos_cleanup2 for use with atexit() - stops the compiler complaining
   about regdos_cleanup()'s return value. This can be used for all
   platforms. */
int regdos_cleanup (void);

void regdos_cleanup2(void) { while (regdos_cleanup() > 0) {;} }
/* RD: End */

/* VWIN32_PROC - We need different representations for the vxd procedure
 * of VWIN32 to match which VxD entry point we're calling. */
typedef LPVOID_RM           VWIN32_PROC;

static VWIN32_PROC vwin32_entry_point;  /* This is our entry point */

/* Paragraph alignment macros */
#define NR_OF_PARAS(s)          (((s)+15)/16)
#define ALIGN_PARA(s)           (NR_OF_PARAS(s)*16)

/* REGDOS_DATA_SIZE - Segment for registry function data */
#define REGDOS_DATA_SIZE        ALIGN_PARA(10 * 1024)

/* REGDOS_STACK_SIZE - Segment for stack */
#define REGDOS_STACK_SIZE       ALIGN_PARA(256)
#define REGDOS_SEGMENT_SIZE     REGDOS_DATA_SIZE + REGDOS_STACK_SIZE

/* Now, since the sizes of both DATA and STACK blocks are paragraph aligned
 * they can all have a SEG:0000 address. Quite a convenience: we just
 * allocate one big block. */
#define REGDOS_DATA_SEG_INC     0
#define REGDOS_STACK_SEG_INC    NR_OF_PARAS(REGDOS_DATA_SIZE)

#define REGDOS_DATA_OFF_INC     0
#define REGDOS_STACK_OFF_INC    REGDOS_DATA_SIZE

static LPVOID_X data;
static LPVOID_X stack;

	/* Transfer buffer stuff */
static int      tbx_free;
static LPVOID_X tbx_top;

	/* Stack stuff */
static LPVOID_X tbx_stack_low, tbx_stack_high;

static
int free_mem(void)
{
	if ( VALID_PM(data.pm) ) dosx_dos_free(data);
	return 1;
}

static int
alloc_mem(void)
{
	if ( 0 == dosx_dos_alloc(&data, REGDOS_SEGMENT_SIZE)
		 && VALID_PM(data.pm) )
	{
/* These are NO OPS, I know */
		data.rm.seg  += REGDOS_DATA_SEG_INC;
		data.pm.off  += REGDOS_DATA_OFF_INC;

		stack         = data;
		stack.rm.seg += REGDOS_STACK_SEG_INC;
		stack.pm.off += REGDOS_STACK_OFF_INC;
		return 1;
	}
	return 0;
}

/***/

static void tbx_start(void)
{
	tbx_top  = data;
	tbx_free = REGDOS_DATA_SIZE;
}

/***/

static void tbx_end(void)
{
	tbx_start();
}

/***/

static LPVOID_X tbx_copy(LPCVOID data, unsigned data_len)
{
	LPVOID_X result = tbx_top;

	if ( tbx_free < data_len )
		result.valid = 0;
	else
	{
		dosx_far_near_copy(tbx_top.pm, data, data_len);
		tbx_top.pm.off += data_len;
		tbx_top.rm.off += data_len;
		tbx_free       -= data_len;
	}
	return result;
}

/***/

static LPVOID_X tbx_reserve(unsigned data_len)
{
	LPVOID_X result = tbx_top;

	assert(tbx_free >= data_len);
	if ( tbx_free < data_len )
	{
		result.valid = 0;
	}
	else
	{
		tbx_top.pm.off += data_len;
		tbx_top.rm.off += data_len;
		tbx_free       -= data_len;
	}
	return result;
}

/* tbx_stack_ptr() - Returns the pointer to stack bottom if the
 * stack is filled with size bytes. */
static LPVOID_X tbx_stack_ptr (unsigned size)
{
	LPVOID_X result = tbx_stack_high;
	result.pm.off  -= size;
	result.rm.off  -= size;
	return result;
}

/***/

/* get_vwin32_proc() - This function is quite essential. When compiled for
 * DPMI protected mode, it gets the real mode entry point of VWIN32 using
 * DPMI Issue Real Mode Interrupt. This real mode entry point is then
 * called using DPMI Call Real Mode Procedure With Far Return Frame.
 *
 * For DOS targets, get_vwin32_proc() gets the entry point using
 * get_vxd_entry_point() (get_vxd_proc()) implemented in the compiler
 * dependent _getvxdp.c
 */
static VWIN32_PROC
get_vwin32_proc(void)
{
	DOSX_REGS   dr;
	VWIN32_PROC rm;

	memset(&dr, 0, sizeof (dr));

	dr.r.x.ax = 0x1684;
	dr.r.x.bx = VWIN32_VXD_ID;
	dosx_int_real(0x2f, &dr);

	rm.seg = dr.s.es;
	rm.off = dr.r.x.di;

	return rm;
}

/***/


int
regdos_startup(void)
{
	if ( !been_here_before )
	{
        /* RD: Install atexit() handler */
        atexit(regdos_cleanup2);

		vwin32_entry_point = get_vwin32_proc();

		if ( !alloc_mem() ) return 0;

		dosx_far_set(stack.pm, 0xCC, REGDOS_STACK_SIZE);
		dosx_far_set(data.pm,  0xCC, REGDOS_DATA_SIZE);

		tbx_free = REGDOS_DATA_SIZE;
		tbx_top  = data;

		tbx_stack_high = tbx_stack_low = stack;
		tbx_stack_high.rm.off += (REGDOS_STACK_SIZE - 4);
		tbx_stack_high.pm.off += (REGDOS_STACK_SIZE - 4);
	}
	return ++been_here_before;
}

/***/

int
regdos_cleanup(void)
{
	if ( !been_here_before ) return 0;
/* Not terribly well... */
	been_here_before--;
	if ( been_here_before == 0 )
	{
		free_mem();
		return 0;
	}
	return been_here_before;
}

/***/


typedef LPVOID_X REGDOS_VAR;

	/* Declares a variable which points to data in real mode memory */
#   define DECLARE_REGDOS_VAR(name) \
	REGDOS_VAR name;

#   define REGDOS_CALL_START(params) \
{\
	LPVOID_X temp_x; \
	LSTACK_DECLARE(stx, params * sizeof(DWORD)); \
	LSTACK_INIT(stx); \
    tbx_start(); \
    memset(&temp_x, 0, sizeof(temp_x));  /* RD: Added (see below) */

    /* RD: I added the memset line to keep DJGPP compiler happy. Otherwise it
           believes temp_x is unused. I think this may be the case sometimes,
           but it is irritating to have the compiler bomb out with -Werror
           set, so this alleviates this problem. */

	/* Copy value parameter to stack */
#   define REGDOS_SET_VAL(val_var, val_var_size) \
	LSTACK_PUSH_DATA(stx, &val_var, val_var_size);

	/* Copy ptr data to transfer buffer, and then put the pointer on the
	 * stack. */
#   define REGDOS_SET_PTR(val_var, val_var_size) \
	if ( val_var != NULL ) \
	{ \
		temp_x = tbx_copy(val_var, val_var_size); \
		LSTACK_PUSH_DATA(stx, &temp_x.rm, sizeof (temp_x.rm)); \
	} \
	else  \
	{ \
		REGDOS_SET_VAL(val_var, sizeof(LPVOID_RM));\
	}

	/* Reserves memory for a buffer like variable. Get the data with
	 * REGDOS_GET_VAR() */
#   define REGDOS_SET_BUF(r, l, size) \
	r = tbx_reserve(size); \
	LSTACK_PUSH_DATA(stx, &r.rm, sizeof (r.rm));

	/* Reserve room for the var parameter, push its address on the stack,
	 * and return the "pointer" to the reserved memory. This pointer is used
	 * in a subsequent REGDOS_GET_VAR() call. The memory starting at ptr
	 * will have the data copied by the VxD. */
#   define REGDOS_SET_VAR(ptr, var, var_size) \
	if ( var != NULL ) ptr = tbx_copy(var, var_size); \
	else ptr = tbx_reserve(var_size); \
	LSTACK_PUSH_DATA(stx, &ptr.rm, sizeof (ptr.rm));

	/* Gets the data from a VAR parameter. */
#   define REGDOS_GET_VAR(var, ptr, var_size) \
	if ( var != NULL ) dosx_near_far_copy(var, ptr.pm, var_size);

	/* Copies stack data to the transfer buffer, and makes call into
	 * registry VxD. */
#   define REGDOS_CALL(result, opcode) \
	result = reg_call(opcode, LSTACK_CURRENT(stx), LSTACK_OCCUPIED(stx));

#   define REGDOS_CALL_END(params) \
	tbx_end(); \
}

/* definition of function:
 *
 * LONG reg_call(UINT opcode, UINT params, LPCDWORD stack_ptr);
 *
 * IN  opcode       = registry service opcode (REG_xxx)
 * IN  params       = number of byte params pushed onto stack
 * IN  stack_ptr    = pointer to last byte (least significant byte) of last
 *                    parameter in stack. Remember that the stack grows
 *                    bottom-top.
 * OUT RESULT       = error code of registry VxD (in DX:AX)
 */

static LONG
reg_call(unsigned opcode, LPVOID stack_ptr, unsigned params)
{
    DOSX_REGS dr;
    LONG lresult;
    LPVOID_X tx;

    memset(&dr, 0, sizeof (dr));

/* Copy the stack data to real mode memory */
    tx = tbx_stack_ptr(params);
    dosx_far_near_copy(tx.pm, stack_ptr, params);

/* Params to thunk */
    dr.r.x.ax   = opcode;

/* Other inits */
    dr.s.ds     = data.rm.seg;
    dr.s.ss     = tx.rm.seg;
    dr.r.x.bp   = tx.rm.off - sizeof(DWORD) - sizeof(WORD);
    dr.s.sp     = dr.r.x.bp; /* Point somewhere, as long if it's <= bp. */

    dosx_call_far_real(vwin32_entry_point, &dr);
    lresult = MAKELONG(dr.r.x.ax, dr.r.x.dx);
    return lresult;
}

/***/

LONG REGDOS_PROC
__RegCreateKey(HKEY hKey, LPCSTR lpszKeyName, HKEY FAR* Result)
{
	DECLARE_REGDOS_VAR(hkResult)
	LONG lResult;

    /* RD: Call start-up for transparency */
    regdos_startup();

	assert(hKey);
	assert(Result);
	assert(lpszKeyName && *lpszKeyName);
	REGDOS_CALL_START(3)
		REGDOS_SET_VAL(hKey, sizeof (hKey))
		REGDOS_SET_PTR(lpszKeyName, strlen(lpszKeyName) + 1)
		REGDOS_SET_BUF(hkResult, Result, sizeof(HKEY))
		REGDOS_CALL(lResult, OPCODE_REG_CREATE_KEY)
		REGDOS_GET_VAR(Result, hkResult, sizeof(HKEY))
	REGDOS_CALL_END(3)
	return lResult;
}

/***/

LONG REGDOS_PROC
__RegOpenKey(HKEY hk, LPCSTR key_name, HKEY FAR* result)
{
   DECLARE_REGDOS_VAR(hkResult)
   LONG lResult;

   /* RD: Call start-up for transparency */
   regdos_startup();

   assert(hk);
   assert(result);
   if ( key_name == NULL )
   {
	   *result = hk;
	   return ERROR_SUCCESS;
   }
   REGDOS_CALL_START(3)
	   REGDOS_SET_VAL(hk, sizeof (hk))
	   REGDOS_SET_PTR(key_name, strlen(key_name) + 1)
	   REGDOS_SET_BUF(hkResult, result, sizeof(HKEY))
	   REGDOS_CALL(lResult,  OPCODE_REG_OPEN_KEY)
	   REGDOS_GET_VAR(result,  hkResult, sizeof(HKEY))
   REGDOS_CALL_END(3)
   return lResult;
}

/***/

LONG REGDOS_PROC
__RegQueryValue (HKEY hk, LPCSTR key_name, LPSTR res_string, LPLONG res_len)
{
   LONG Result;
   DECLARE_REGDOS_VAR(rres_string)
   DECLARE_REGDOS_VAR(rres_len)

   /* RD: Call start-up for transparency */
   regdos_startup();

   assert(hk);
/* res_string may be NULL, but res_len mustn't */
   if ( !(!res_string && res_len) ) return !ERROR_SUCCESS;
   REGDOS_CALL_START(4)
	   REGDOS_SET_VAL(hk, sizeof (hk))
	   REGDOS_SET_PTR(key_name, strlen(key_name) + 1)
/* DOSX_FIXME: check length, should not exceed TBX_BUFFER_SIZE */
	   REGDOS_SET_BUF(rres_string, res_string, (unsigned)*res_len)
	   REGDOS_SET_VAR(rres_len, res_len, sizeof(LONG))
	   REGDOS_CALL(Result, OPCODE_REG_QUERY_VALUE)
/* Must copy result length first! */
	   REGDOS_GET_VAR(res_len, rres_len, sizeof(LONG))
/* FIXME: long -> unsigned */
	   REGDOS_GET_VAR(res_string, rres_string, (unsigned)*res_len)
   REGDOS_CALL_END(4)
   return Result;
}

/***/

LONG REGDOS_PROC
__RegCloseKey(HKEY hk)
{
   LONG Result;

   /* RD: Call start-up for transparency */
   regdos_startup();

   assert(hk);
   REGDOS_CALL_START(1)
	   REGDOS_SET_VAL(hk, sizeof (hk))
	   REGDOS_CALL(Result, OPCODE_REG_CLOSE_KEY)
   REGDOS_CALL_END(1)
   return Result;
}

/***/

LONG REGDOS_PROC
__RegDeleteKey(HKEY Key, LPCSTR SubKey)
{
	LONG Result;

    /* RD: Call start-up for transparency */
    regdos_startup();

	REGDOS_CALL_START(2)
		REGDOS_SET_VAL(Key, sizeof (Key))
		REGDOS_SET_PTR(SubKey, strlen(SubKey) + 1)
		REGDOS_CALL(Result, OPCODE_REG_DELETE_KEY)
	REGDOS_CALL_END(2)
	return Result;
}

/***/

LONG REGDOS_PROC
__RegDeleteValue(HKEY Key, LPCSTR Value)
{
	LONG Result;

    /* RD: Call start-up for transparency */
    regdos_startup();

	REGDOS_CALL_START(2)
		REGDOS_SET_VAL(Key, sizeof (Key))
		REGDOS_SET_PTR(Value, strlen(Value) + 1)
		REGDOS_CALL(Result, OPCODE_REG_DELETE_VALUE)
	REGDOS_CALL_END(2)
	return Result;
}

/***/

LONG REGDOS_PROC
__RegQueryValueEx(HKEY hk, LPCSTR key_name, LPDWORD reserved,
   LPDWORD data_type, LPBYTE data, LPDWORD data_len)
{
   LONG Result;
   DECLARE_REGDOS_VAR(rdata_type)
   DECLARE_REGDOS_VAR(rdata_len)
   DECLARE_REGDOS_VAR(rdata)

   /* RD: Call start-up for transparency */
   regdos_startup();

   assert(hk);
/* FUTURE FIXME: reserved is NULL in current Win32 API impls.
 * If this changes, also change the corresponding REGDOS_ stack param.
 * See REGDOS_xxx table below. */
   assert(key_name != NULL);
   assert(reserved == NULL);
   assert(data_type != NULL);
   assert(data_len != NULL);
   
   REGDOS_CALL_START(6)
	   REGDOS_SET_VAL(hk, sizeof (hk))
	   REGDOS_SET_PTR(key_name, strlen(key_name) + 1)
	   REGDOS_SET_VAL(reserved, sizeof (reserved))
	   REGDOS_SET_VAR(rdata_type, data_type, sizeof(DWORD))
	   REGDOS_SET_BUF(rdata, data, (unsigned)*data_len)
	   REGDOS_SET_VAR(rdata_len, data_len, sizeof(DWORD))
	   REGDOS_CALL(Result, OPCODE_REG_QUERY_VALUE_EX)
/* Copy changed stuff back. Data len first! */
	   REGDOS_GET_VAR(data_len, rdata_len, sizeof(DWORD))
	   REGDOS_GET_VAR(data, rdata, (unsigned)*data_len)
	   REGDOS_GET_VAR(data_type, rdata_type, sizeof(DWORD))
   REGDOS_CALL_END(6)
   return Result;
}

/***/

LONG REGDOS_PROC
__RegEnumKey(HKEY hKey, DWORD SubkeyIndex, LPSTR KeyName, DWORD KeyNameSize)
{
	LONG Result;
	DECLARE_REGDOS_VAR(rKeyName)

    /* RD: Call start-up for transparency */
    regdos_startup();
    
	assert(hKey);
	REGDOS_CALL_START(4)
	   REGDOS_SET_VAL(hKey, sizeof (hKey))
	   REGDOS_SET_VAL(SubkeyIndex, sizeof (SubkeyIndex))
	   REGDOS_SET_BUF(rKeyName, KeyName, KeyNameSize)
	   REGDOS_SET_VAL(KeyNameSize, sizeof (KeyNameSize))
	   REGDOS_CALL(Result, OPCODE_REG_ENUM_KEY)
/* Copy returned name to protected mode memory */
	   REGDOS_GET_VAR(KeyName, rKeyName, dosx_far_strlen(rKeyName.pm) + 1)
	REGDOS_CALL_END(4)
	return Result;
}

/***/

LONG REGDOS_PROC
__RegEnumValue(HKEY hKey, DWORD SubkeyIndex, LPSTR ValueString,
   LPDWORD ValueStringSize, LPDWORD Reserved, LPDWORD TypePtr,
   LPBYTE Data, LPDWORD DataSize)
{
	LONG Result;
	DECLARE_REGDOS_VAR(rValueString)
	DECLARE_REGDOS_VAR(rValueStringSize)
	DECLARE_REGDOS_VAR(rTypePtr)
	DECLARE_REGDOS_VAR(rData)
	DECLARE_REGDOS_VAR(rDataSize)

    /* RD: Call start-up for transparency */
    regdos_startup();

	assert(hKey);
	assert(ValueStringSize && *ValueStringSize);
	assert(DataSize && *DataSize);
/* FUTURE FIXME: I'm pushing reserved as a DOWRD value on the stack,
 * because it ought to be 0 on current Win32 implementations. Change
 * the REGDOS_xxx table if the situation changes. */
	REGDOS_CALL_START(8)
	   REGDOS_SET_VAL(hKey, sizeof (hKey))
	   REGDOS_SET_VAL(SubkeyIndex, sizeof (SubkeyIndex))
	   REGDOS_SET_BUF(rValueString, ValueString, *ValueStringSize)
	   REGDOS_SET_VAR(rValueStringSize, ValueStringSize, sizeof(DWORD))
	   REGDOS_SET_VAL(Reserved, sizeof (Reserved))
	   REGDOS_SET_VAR(rTypePtr, TypePtr, sizeof(DWORD))
	   REGDOS_SET_BUF(rData, Data, *DataSize)
	   REGDOS_SET_VAR(rDataSize, DataSize, sizeof(DWORD))
	   REGDOS_CALL(Result, OPCODE_REG_ENUM_VALUE)
/* <Sigh> Get the var parameters. Datalens first. */
	   REGDOS_GET_VAR(DataSize, rDataSize, sizeof(DWORD))
	   REGDOS_GET_VAR(ValueStringSize, rValueStringSize, sizeof(DWORD))
/* Get the retrieved data */
	   REGDOS_GET_VAR(Data, rData, *DataSize)
	   REGDOS_GET_VAR(ValueString, rValueString, *ValueStringSize)
	   REGDOS_GET_VAR(TypePtr, rTypePtr, sizeof(DWORD))
	REGDOS_CALL_END(8)
	return Result;
}

/***/

LONG REGDOS_PROC
__RegSetValueEx(HKEY hKey, LPCSTR ValueName, DWORD Reserved,
   DWORD ValueType, LPBYTE Data, DWORD DataLen)
{
	LONG Result;

    /* RD: Call start-up for transparency */
    regdos_startup();

	assert(hKey);
	REGDOS_CALL_START(6)
	   REGDOS_SET_VAL(hKey, sizeof (hKey))
	   REGDOS_SET_PTR(ValueName, strlen(ValueName) + 1)
	   REGDOS_SET_VAL(Reserved, sizeof (Reserved))
	   REGDOS_SET_VAL(ValueType, sizeof (ValueType))
	   REGDOS_SET_PTR(Data, DataLen)
	   REGDOS_SET_VAL(DataLen, sizeof (DataLen))
	   REGDOS_CALL(Result, OPCODE_REG_SET_VALUE_EX)
	REGDOS_CALL_END(6)
	return Result;
}

#ifdef TEST

int main (void)
{
	HKEY hKey, hKeyCreate;
	char Buffer[255], Values[255];
	unsigned long BufSize = sizeof Buffer, ValueSize, Count;
	unsigned long BufType = REG_SZ;
	long error;

	regdos_startup();
	printf("RegDos (TEST) - Enumerates CurrentVersion keys and str values\n");
	fflush(stdout);

/* Start with easy data */
	if ( ERROR_SUCCESS != (error = RegOpenKey(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion",
			&hKey)) )
	{
		printf("Error RegOpenKey(), %ld (%lx)", error, error);
		return -1;
	}

	if ( ERROR_SUCCESS != RegQueryValueEx(hKey, "RegisteredOwner",
			NULL, &BufType, (void FAR*)Buffer, &BufSize) )
	{
		printf("Error RegQueryValue(\"RegisteredOwner\")");
		return -2;
	} else printf("Registered owner: \"%s\"\n", Buffer);

	BufSize = sizeof Buffer;
	if ( ERROR_SUCCESS != RegQueryValueEx(hKey, "RegisteredOrganization",
			NULL, &BufType, (void FAR*)Buffer, &BufSize) )
	{
		printf("Error RegQueryValue(\"RegisteredOrganization\")");
		return -3;
	} else printf("Registered organization: \"%s\"\n", Buffer);

/* Now test the enumeration functions */
	printf("Enumerating keys in LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\n");
	Count = 0;
	while ( (error = RegEnumKey(hKey, Count, Buffer, sizeof Buffer) == ERROR_SUCCESS) )
	{
		Count++;
		printf("Key: %s\n", Buffer);
		fflush(stdout);
	}

	printf("Enumerating values in LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\n");
	Count = 0;
	do
	{
		BufSize = sizeof Buffer;
		ValueSize = sizeof Values;
		BufType = REG_SZ;
		error = RegEnumValue(hKey, Count, Buffer, &BufSize, (void FAR*)NULL,
			&BufType, (void FAR*)Values, &ValueSize);
		if ( error != ERROR_SUCCESS ) break;
		Buffer[BufSize] = 0;
		if ( BufType == REG_SZ ) printf("%s = \"%s\"\n", Buffer, Values);
		else
			printf("%s (Data type: %lu)\n", Buffer, BufType);
		fflush(stdout);
		Count++;
	} while ( 1 );

#define SET_TEXT "I Should Be Deleted!"
	if ( ERROR_SUCCESS != (error = RegSetValueEx(hKey,
			(char FAR*)"DeleteMeASAP", 0, REG_SZ,
			(void FAR*)(char FAR*)SET_TEXT,
			strlen(SET_TEXT) + 1)) )
	{
		printf("Error setting value %ld\n", error);
	}

	if ( ERROR_SUCCESS != RegCloseKey(hKey) )
	{
		printf("Error closing key");
		return -4;
	}

/* Open a key */
#define MY_KEY          "Acronyms Rendering Software Engineers"
	RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE", &hKey);
	if ( ERROR_SUCCESS != RegOpenKey(hKey, MY_KEY, &hKeyCreate) )
	{
		printf("Creating key\n");
		if ( ERROR_SUCCESS != RegCreateKey(hKey, MY_KEY, &hKeyCreate) )
		{
			printf("Error RegCreateKey()");
			RegCloseKey(hKey);
			return -1;
		}
		RegCloseKey(hKeyCreate);
	}
	else
	{
		printf("Deleting key\n");
		RegDeleteKey(hKey, MY_KEY);
		RegCloseKey(hKey);
	}
	RegCloseKey(hKey);

	regdos_cleanup();

    return(1);
}
#endif /* TEST */
