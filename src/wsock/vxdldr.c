/*
 *  libsocket - BSD socket like library for DJGPP
 *  Copyright 1997, 1998 by Indrek Mandre
 *  Copyright 1997, 1998 by Richard Dawe
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Library General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Original code from Dan Hedlund's wsock library.
 */

#include <dpmi.h>
#include <string.h>

#include "winsock.h"
#include "lsckglob.h"
#include "farptrx.h"
#include <lsck/vxd.h>

#define VxdLdr_DeviceID     0x0027
#define VxdLdr_LoadDevice   1
#define VxdLdr_UnLoadDevice 2

int VxdLdrEntry [2] = {0, 0};

void VxdLdrInit (void)
{
  VxdGetEntry (VxdLdrEntry, VxdLdr_DeviceID);
}

int VxdLdrLoadDevice (char Device [])
{
  int Error;

  __dpmi_meminfo _dev;
  int dev;

  if (VxdLdrEntry [1] == 0) VxdLdrInit ();
  if (VxdLdrEntry [1] == 0) return -1;

  _dev.handle = 0;
  _dev.size = strlen (Device) + 1;
  _dev.address = 0;

  __dpmi_allocate_memory (&_dev);
  dev = __dpmi_allocate_ldt_descriptors (1);
  __dpmi_set_segment_base_address (dev, _dev.address);
  __dpmi_set_segment_limit (dev, _dev.size);

  _farpokex (dev, 0, Device, _dev.size);

  asm ("pushl   %%ds            \n\
        movw    %%cx, %%ds      \n\
        lcall   %%cs:_VxdLdrEntry \n\
        setc    %%al            \n\
        movzbl  %%al, %%eax     \n\
        popl    %%ds"
        : "=a" (Error)
        : "a" (VxdLdr_LoadDevice), "c" (dev), "d" (0));

  __dpmi_free_ldt_descriptor (dev);
  __dpmi_free_memory (_dev.handle);

  return Error;
}

int VxdLdrUnLoadDevice (char Device [])
{
  int Error;

  __dpmi_meminfo _dev;
  int dev;

  if (VxdLdrEntry [1] == 0) VxdLdrInit ();
  if (VxdLdrEntry [1] == 0) return -1;

  _dev.handle = 0;
  _dev.size = strlen (Device) + 1;
  _dev.address = 0;

  __dpmi_allocate_memory (&_dev);
  dev = __dpmi_allocate_ldt_descriptors (1);
  __dpmi_set_segment_base_address (dev, _dev.address);
  __dpmi_set_segment_limit (dev, _dev.size);

  asm ("pushl   %%ds            \n\
        movw    %%cx, %%ds      \n\
        lcall   %%cs:_VxdLdrEntry \n\
        setc    %%al            \n\
        movzbl  %%al, %%eax     \n\
        popl    %%ds"
        : "=a" (Error)
        : "a" (VxdLdr_UnLoadDevice), "c" (dev), "d" (0));

  __dpmi_free_ldt_descriptor (dev);
  __dpmi_free_memory (_dev.handle);

  return Error;
}
