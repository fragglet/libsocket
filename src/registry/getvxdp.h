/* getvxdp.h, returns the entry point of a VxD. The implementation is
 * very compiler specific, and therefore deferred to an external file
 * called _getvxdp.c.
 * By Alfons Hoogervorst / Alliterated Software Development, 1997-1998.
 */

#ifndef __GETVXDP_H

#ifdef PF_MSDOS_32
typedef LPVOID_PM       VXD_PROC;
#define VALID_VXD_PROC(p) VALID_PM(p)
#else
typedef LPVOID          VXD_PROC;
#define VALID_VXD_PROC(p) ((p) != NULL)
#endif

#define get_vxd_proc get_vxd_entry_point

__BEGIN_EXTERN_C
VXD_PROC get_vxd_entry_point (unsigned short);
__END_EXTERN_C

#endif /* __GETVXDP_H */

