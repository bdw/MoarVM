/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "src/jit/x86_64.dasc".
*/

#line 1 "src/jit/x86_64.dasc"
#include "moar.h"
#include <dasm_proto.h>
#include <dasm_x86.h>
#include "emit.h"

#define OFFSET_ENV offsetof(MVMFrame, env)
#define OFFSET_ARGS offsetof(MVMFrame, args)
#define OFFSET_WORK offsetof(MVMFrame, work)
#define JIT_FRAME_SIZE 16

//|.arch x64
#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif
#line 12 "src/jit/x86_64.dasc"
//|.actionlist actions
static const unsigned char actions[50] = {
  85,72,137,229,72,129,252,236,239,255,72,137,125,252,248,72,137,117,252,240,
  72,139,142,233,255,72,129,196,239,93,195,255,72,185,237,237,252,255,209,255,
  72,139,77,252,240,72,139,137,233,255
};

#line 13 "src/jit/x86_64.dasc"

/* The 'work' registers that MVM supplies will be referenced a lot.
 * So the rcx register will be set up to hold the work space base. */
//|.define WORK, rcx

/* Also, since I'm using static register allocation (for now, that is), 
 * I will assign a single register for all valued operations */
//|.define VALUE, rdx
#define VALUE_REG 3

const unsigned char * MVM_jit_actions(void) {
    return actions;
}

static const MVMJitCpuReg x64_cpureg[] = {
    {  0, 0, "rax", 0 },
    {  1, 1, "rbx", 0 },
    {  2, 0, "rcx", 0 },
    {  3, 0, "rdx", 0 },
    {  4, 0, "rsi", 0 },
    {  5, 0, "rdi", 0 },
    {  6, 1, "rbp", 0 },
    {  7, 1, "rsp", 0 },
    {  8, 0, "r8",  0 },
    {  9, 0, "r9",  0 },
    { 10, 0, "r10", 0 },
    { 11, 0, "r11", 0 },
    { 12, 0, "r12", 0 },
    { 13, 0, "r13", 0 },
    { 14, 0, "r14", 0 },
    { 15, 0, "r15", 0 },
};

const MVMJitCpuReg * MVM_jit_cpureg(int * num_cpureg_out) {
    void * new_cpureg_space = malloc(sizeof(x64_cpureg));
    *num_cpureg_out = sizeof(x64_cpureg) / sizeof(MVMJitCpuReg);
    /* every invocant will get its own space */
    return memcpy(new_cpureg_space, x64_cpureg, sizeof(x64_cpureg));
}

/* A function prologue is always the same in x86 / x64, becuase
 * we do not provide variable arguments, instead arguments are provided
 * via a frame. All JIT entry points receive prologue */
void MVM_jit_emit_prologue(MVMThreadContext *tc, dasm_State **Dst) {
    /* set up our C call frame, i.e. allocate stack space*/
    //| push rbp
    //| mov rbp, rsp
    //| sub rsp, JIT_FRAME_SIZE
    dasm_put(Dst, 0, JIT_FRAME_SIZE);
#line 61 "src/jit/x86_64.dasc"

    //| mov [rbp-8], rdi              // thread context
    //| mov [rbp-16], rsi             // mvm frame
    //| mov WORK, [rsi + OFFSET_WORK] // work register base
    dasm_put(Dst, 10, OFFSET_WORK);
#line 65 "src/jit/x86_64.dasc"
}

/* And a function epilogue is also always the same */
void MVM_jit_emit_epilogue(MVMThreadContext *tc, dasm_State **Dst) {
    //| add rsp, JIT_FRAME_SIZE
    //| pop rbp
    //| ret
    dasm_put(Dst, 25, JIT_FRAME_SIZE);
#line 72 "src/jit/x86_64.dasc"
}


void MVM_jit_emit_c_call(MVMThreadContext *tc, MVMJitCallC * call_spec, dasm_State **Dst) {
    int i;
    if (call_spec->has_vargs) {
        MVM_exception_throw_adhoc(tc, "JIT can't handle varargs yet");
    }
    /* first, add arguments */
    for (i = 0; i < call_spec->num_args; i++) {
        /* just kidding */
        MVM_exception_throw_adhoc(tc, "JIT can't handle arguments yet");
    }
    /* Set up and emit the call. I re-use the work pointer register
     * because it has to be restored anyway and does not normally
     * participate in argument passing, so it is 'free'. Also, rax
     * is unavailable as it has to hold the number of arguments on the
     * stack. I've moved this below the argument setup as the work
     * register pointer might be needed there. */
    //| mov64 WORK, (uintptr_t)call_spec->func_ptr
    //| call WORK
    dasm_put(Dst, 32, (unsigned int)((uintptr_t)call_spec->func_ptr), (unsigned int)(((uintptr_t)call_spec->func_ptr)>>32));
#line 93 "src/jit/x86_64.dasc"
    /* Restore the work register pointer */
    //| mov WORK, [rbp-16]             // load the mvm frame
    //| mov WORK, [WORK + OFFSET_WORK] // load the work register pointer
    dasm_put(Dst, 40, OFFSET_WORK);
#line 96 "src/jit/x86_64.dasc"
}
