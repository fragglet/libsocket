/* lstack.c, local stack stuff.
 * Written by Alfons Hoogervorst, 1997
 */
#include "lstack.h"

void push_val(stkhdr FAR* s, LPCVOID p)
{
    push_data(s, &p, sizeof(DWORD));
}

/* if you pass value params, pass the correct size :) */
void push_data(stkhdr FAR* s, LPCVOID p, int dat_len)
{
    int tl = dat_len;
/* This is not terribly well... */
    for ( ; tl; tl-- )
        s->st[--(s->sp)] = ((LPCSTR)p)[tl - 1];
}
