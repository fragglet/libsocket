#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <lsck/vxd.h>

int  VXD_ID     = 0x1235;
char VXD_NAME[] = "SOCK.VXD";
int  vxd_entry[2];

int main (int argc, char *argv[])
{
    int i, ret, ok;

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

    /* Create TCP socket */
    for (ok = 1, i = 0; i < 32; i++) {
        __asm__ __volatile__ (
                                "   lcall _vxd_entry       \n\
                                    jc 1f                  \n\
                                    xorl %%eax, %%eax      \n\
                                 1:"
                              : "=a" (ret)
                              : "a"  (1), "b" (6), "D" (i)
                              : "%eax", "cc"
                             );

    	if (ret != 0) {
            printf("(Socket %i) SOCK.VXD returned %i\n", i, ret);
	    ok = 0;
	}
    }

    if (ok) {
        printf("Waiting two minutes...\n");
	sleep(120);
    }

    return(EXIT_SUCCESS);
}
