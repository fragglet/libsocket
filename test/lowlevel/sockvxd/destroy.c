#include <stdio.h>
#include <stdlib.h>

#include <lsck/vxd.h>

int  VXD_ID     = 0x1235;
char VXD_NAME[] = "SOCK.VXD";
int  vxd_entry[2];

int main (int argc, char *argv[])
{
    int n = -1, ret;

    /* Get the socket fd to create */
    if (argc < 2) {
        printf("Syntax: %s <Integer, 0-31>\n", argv[0]);
	return(EXIT_FAILURE);
    }

    n = atoi(argv[1]);
    if ((n < 1) || (n > 31)) {
        printf("Syntax: %s <Integer, 0-31>\n", argv[0]);
	return(EXIT_FAILURE);
    }

    /* Load the VxD and get its entry point */
    if (VxdLdrLoadDevice (VXD_NAME) != 0) {
	printf("Failed to load SOCK.VXD\n");
    	return(EXIT_FAILURE);
    }

    vxd_entry[0] = vxd_entry[1] = 0;
    VxdGetEntry (vxd_entry, VXD_ID);
    if ((vxd_entry[0] == 0) && (vxd_entry[1] == 0)) {
	printf("Failed to obtain entry point of SOCK.VXD\n");
    	return(EXIT_FAILURE);
    }

    /* Destroy a socket */
    __asm__ __volatile__ (
                            "   lcall _vxd_entry       \n\
                                jc 1f                  \n\
                                xorl %%eax, %%eax      \n\
                             1:"
                          : "=a" (ret)
                          : "a"  (2), "D" (n)
                          : "%eax", "cc"
                         );

    if (ret != 0) printf("SOCK.VXD returned %i\n", ret);
    ret = (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

    return(ret);
}
