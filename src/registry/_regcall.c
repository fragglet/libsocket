/* _regcall.c
 * By Alfons Hoogervorst / Alliterated Software Development, 1997-1998.
 */

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

#if defined(__WATCOMC__)
/* This is the definition to match up with the inline assembly stub, so
 * Watcom knows what we mean. */
LONG a11_reg_call (unsigned opcode, LPCVOID stack_ptr, unsigned params);
#endif


/* For a change do the PF_MSDOS version first. */

#if defined(PF_MSDOS)

#   if defined(__SC__) || defined(__TURBOC__)

static LONG
reg_call(unsigned opcode, LPVOID stack_ptr, unsigned params)
{
    LONG res;

    __asm push ds           /* NOTE: I'm always saving these. */
    __asm push es
    __asm push si
    __asm push di

    __asm mov ax,opcode     /* set opcode, because we're using bp */
    __asm mov cx,params
    __asm sub sp,cx         /* space for the stack for registry call */
    __asm shr cx,1          /* number of words */
    __asm mov dx,ss
    __asm mov es,dx         /* set up destination */
    __asm mov di,sp
    __asm mov dx,ds         /* store ds for now */
    __asm lds si,stack_ptr  /* set up source */
    __asm cld
    __asm rep movsw         /* copy */
    __asm mov ds,dx         /* reset ds, so we can use an indirect call */

    __asm sub sp,4          /* VWIN32 requirement #1: reserve 4 bytes */
    __asm push bp           /* VWIN32 requirement #2: extra word */
    __asm mov bp,sp         /* VWIN32 requirement #3: SP == BP */

/* Make the call (Assumption: SS does not change. This makes sense.) */

    __asm call DWORD PTR vwin32_entry_point

    __asm pop bp            /* now we can use the arguments again */
    __asm add sp,4          /* get rid of req. #1 */
    __asm add sp,params     /* get rid of the stack we created */

/* We're back, save the result */
    __asm mov WORD PTR res,ax
    __asm mov WORD PTR res+2,dx

    __asm pop di
    __asm pop si
    __asm pop es
    __asm pop ds
    return res;
}

#   elif defined(__WATCOMC__)

/* Watcom C makes it really tough because of it uses registers as params.
 * This asm stub also needs to reload DS before the indirect call because
 * I encountered an intermittent bug in the Watcom code generator. */
#pragma aux a11_reg_call = \
    "push ds" \
    "push es" \
    "push bp"       /* we need to save CX somewhere! */ \
    "mov bp,sp" \
    "push cx"       /* <sigh> */ \
    "sub sp,cx"     /* make stack */ \
    "shr cx,1"      /* number of words to copy */ \
    "mov di,ss"     /* aww, use di to copy ss to es */ \
    "mov es,di"     /* setup destination */ \
    "mov di,sp" \
    "mov si,bx"     /* bx has offset of source */ \
    "push ds"       /* aww, need DS to do indirect call later */ \
    "mov ds,dx"     /* dx has source segment */ \
    "cld" \
    "rep movsw"     /* we can copy it now */ \
    "pop ds" \
    "sub sp,4"      /* align */ \
    "push bp" \
    "mov bp,sp" \
    "mov si,SEG vwin32_entry_point" /* BUGBUG? */ \
    "mov ds,si" \
    "callf [vwin32_entry_point]" \
    "pop bp"        /* restore bp */ \
    "add sp,[bp-2]" /* flush cx, align and some stack bytes */ \
    "add sp,6"      /* flush remnants of stack */ \
    "pop bp" \
    "pop es" \
    "pop ds" \
    parm [ax] [bx dx] [cx] value [dx ax] modify [si di];

/* Now the real function */
static LONG
reg_call(unsigned opcode,  LPVOID stack_ptr, unsigned params)
{
    return a11_reg_call(opcode, stack_ptr, params);
}

#   elif defined(__PACIFIC__)

/* Pacific C does it a little bit odd. If the first parameter is 16 bit,
 * then it's put in DX. All other parameters are pushed on the stack. */
static LONG
reg_call(unsigned opcode, LPVOID stack_ptr, unsigned params)
{
    /* stack_ptr @ [bp+6], params @ [bp+10] */
    asm("push ds");
    asm("push es");
    asm("push si");
    asm("push di");

    asm("mov ax,dx");       /* set opcode, because we're using bp */
    asm("mov cx,10[bp]");
    asm("sub sp,cx");       /* space for the stack for registry call */
    asm("shr cx,#1");       /* number of words */
    asm("mov dx,ss");
    asm("mov es,dx");       /* set up destination */
    asm("mov di,sp");
    asm("mov dx,ds");       /* store ds for now */
    asm("lds si,6[bp]");    /* set up source */
    asm("cld");
    asm("rep movsw");       /* copy */
    asm("mov ds,dx");       /* reset ds, so we can use an indirect call */
    asm("sub sp,#4");       /* VWIN32 requirement #1: reserve 4 bytes */
    asm("push bp");         /* VWIN32 requirement #2: extra word */
    asm("mov bp,sp");       /* VWIN32 requirement #3: SS == BP */

/* Make the call (assumption: SS does not change. This makes sense.) */
    asm("callf [_vwin32_entry_point]");

/* We're back, save the result */
    asm("pop bp");          /* we can use the arguments again */
    asm("add sp,#4");       /* get rid of req. #1 */
    asm("add sp,10[bp]");   /* get rid of the stack we created */

    asm("pop di");
    asm("pop si");
    asm("pop es");
    asm("pop ds");

/* NOTE: no return, set warning level to 3. */
}
#       else
#               error Unsupported compiler for PF_MSDOS
#   endif /* PF_MSDOS compiler */

#elif defined(PF_MSDOS_32) && !defined(AHX_DONT_SWITCH_TO_REAL)
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

#elif defined(PF_MSDOS_32) /* PF_MSDOS_32, native */

#   if defined(__SC__)
#   else
#       error Not supported compiler for PF_MSDOS_32 */
#   endif /* compiler */


#if 0
/* 0x0a11fab2 - This is pure speculation. Need to finish this, if you find
 * a flaw in the following logic, contact me:
 *
 * We want to call the 16 bit protected mode entry point from 32 bit mode.
 * Calling that entry point is not a problem, as long as it is done from
 * a 'native mode' segment. (With 'native mode' I mean: if we're running
 * in a 32 bit program, we should call the entry point from a 32 bit
 * segment.) However, VWIN32 requires parameters on the stack; that's
 * our problem.
 *
 * First of all, all parameters should be mapped to SEL:OFF16 addresses.
 * That's easy to do using either dosx_map_addr16(), or the MAPPTR32
 * functions.
 *
 * Secondly the SS:SP should be a SEL:OFF16 address too. We're calling
 * a 16 bit protected mode entry point, which wants 16 bit things.
 *
 * OK, here's what the following code intends to do.
 *
 * 1. The variables / arguments have already been mapped, and are pointed to
 *    by stack_ptr. This is probably hidden by the REGDOS_xxx macros.
 *
 * 2. Before we jump to the entry point, we create a 16 bit alias for
 *    ss. This alias maps a region of 64K (the max. of a 16 bit selector)
 *    of the 32 bit stack segment. Ofcourse, in such a way, that it
 *    encompasses the linear address that ESP is pointing to.
 *
 * This quite convenient because the specials struct is accessible using both
 * the ESP (when the 32 bit SS is active) and SP (when the 16 bit SS is
 * active). Moreover, we can actually set the SP to a FIXED address
 * (SPECIALS_ADDRESS). Why? Because we can set the linear base of the 16 bit
 * SS anywhere, as long as the 16 bit SS encompasses both ESP and the address
 * of specials.
 *
 * Now, does this work? I think it should, but I may be wrong.
 */

#define SPECIALS_ADDRESS    64178

static LONG
reg_call(unsigned opcode, LPVOID stack_ptr, unsigned params)
{
    LONG res = __LINE__;

/* SPECIALS - Consists of info that should be located at address
 * SPECIALS_ADDRESS. This one is allowed to be aligned. */
    typedef struct
    {
        unsigned params;
        SELECTOR ss16;
        WORD     old_ss;
        DWORD    old_ebp;
        DWORD    old_esp;
        DWORD    ssbase;
        DWORD    off_params_sp;
        DWORD    spill;         /* Always one DWORD too much */
    } SPECIALS;
    SPECIALS specials;
#define SPECIALS_SIZE sizeof(specials)

/* Preparations */
    specials.params = params;
    specials.proc   = vwin32_entry_point;
    dosx_get_selector_base(dosx_current_ss(), &specials.ssbase);
    dosx_alloc_copy(&specials.ss16, dosx_current_ss());
    dosx_set_selector_limit(specials.ss16, 0xFFFF);

/* Save fatals */
    __asm push es
    __asm push esi
    __asm push edi
    __asm mov specials.old_ss,ss
    __asm mov specials.old_esp,esp
    __asm mov specials.old_ebp,ebp

/* Map specials to address SPECIALS_ADDRESS in ss16 */
    __asm lea edx,specials
    __asm add edx,specials.ssbase
    __asm sub edx,SPECIALS_ADDRESS
    __asm mov eax,7
    __asm movzx ebx,specials.ss16
    __asm mov ecx,edx
    __asm shr ecx,16
    __asm int 31h
    __asm jc dpmi_error

/* DPMI saves registers, so we can re-use EDX. Now we need to calculate
 * where we put our registry parameters, in terms of the 16 bit SP.
 * Ideally, ESP should point at specials, but you never know what code
 * is inserted into the stream by the compiler. I *am* assuming that
 * ESP points to specials or some bytes below. */
    __asm mov ebx,esp
    __asm add ebx,specials.ssbase
    __asm sub ebx,params
    __asm sub edx,ebx           /* Sub this from SP to get params */
    __asm neg edx
    __asm add edx,SPECIALS_ADDRESS
    __asm mov specials.off_params_sp,edx

/* Now move the parameters to their destination proper. */
    __asm mov ecx,params
    __asm sub esp,ecx
    __asm shr ecx,2
    __asm cld
    __asm mov esi,stack_ptr
    __asm mov di,ss
    __asm mov es,di
    __asm mov edi,esp
    __asm rep movsd

    __asm mov ax,opcode
    __asm mov bx,specials.ss16
    __asm mov edx,specials.off_params_sp
    __asm cli
    __asm mov ss,bx
    __asm mov esp,edx
    __asm sti

/* Now we're there */
    __asm mov ebp,SPECIALS_ADDRESS
    __asm mov eax,[ebp].SPECIALS.opcode
    __asm mov edi,OFFSET vwin32_entry_point

/* Now conform to what VWIN32 requires: 6 bytes on stack, before jumping
 * into the deep. A far call takes up 4 bytes already.  */
    __asm push ebp          /* These are 4 bytes */
    __asm push bp
    __asm mov
    __asm db 0xff, 0x1f /* callf [edi] */




/* This is going to be insane, I know */
    __asm mov eax,esp            /* Assuming ESP <= m.sp32 */
    __asm mov edx,m.sp32
    __asm sub edx,eax
    __asm mov ax,m.sp16
    __asm sub ax,dx
    __asm mov m.sp16,ax

}
#endif /* 0 */


#else
#   error Unsupported platform
#endif

