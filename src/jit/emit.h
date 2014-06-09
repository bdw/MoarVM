/* Declarations for architecture-specific codegen stuff */
const unsigned char * MVM_jit_actions(void);
const MVMJitCpuReg * MVM_jit_cpureg(int * num_cpureg_out);
void MVM_jit_emit_prologue(MVMThreadContext *tc, dasm_State **Dst);
void MVM_jit_emit_epilogue(MVMThreadContext *tc, dasm_State **Dst);
void MVM_jit_emit_c_call(MVMThreadContext *tc, MVMJitCallC *call_spec, dasm_State **Dst);
