/* lstack.c, local stack stuff.
 * Written by Alfons Hoogervorst, 1997
 */
#ifndef __LSTACK_H
#define __LSTACK_H
#include <platform.h>

/* Stack stuff */
typedef struct
{
    int sp, x;
    char st[1];
} stkhdr;

void push_val(stkhdr FAR* s, LPCVOID p);
void push_data(stkhdr FAR* s, LPCVOID p, int dat_len);

/* LSTACK_DECLARE - declares an lstack. Remember to put an LSTACK_INIT 
 * into the code stream. */
#define LSTACK_DECLARE(name, size) \
    struct { \
        int sp, t;\
        char st[size];\
    } name

#define LSTACK_INIT(name) \
    name.sp = sizeof(name.st)

/* Pushes data of any size */
#define LSTACK_PUSH_DATA(stk, p, p_len) \
    push_data((LPVOID)&(stk), (p), (p_len))

/* Pushes DWORD sized data, value or whatever */
#define LSTACK_PUSH_DWORD(stk, dw) \
    push_val((LPVOID)&(stk), (LPVOID)dw)

/* Pushes WORD sized data, value or whatever */
#define LSTACK_PUSH_WORD(stk, w) \
    push_val((LPVOID)&(stk), (LPVOID)w)

/* Number of bytes currently occupying stack */
#define LSTACK_OCCUPIED(stk) \
    ((sizeof ((stk).st)) - (stk).sp)

/* Current stack pointer */
#define LSTACK_CURRENT(stk) \
    (LPVOID)(&stk.st[stk.sp])

#endif /* __LSTACK_H */
