#include "moar.h"
#include "platform/mmap.h"
#include <dasm_proto.h>
#include "emit.h"



MVMJitGraph * MVM_jit_try_make_jit_graph(MVMThreadContext *tc, MVMSpeshGraph *spesh) {
    /* right now i'm planning to only jit a few very simple things. */
    MVMSpeshBB * current_bb;
    MVMSpeshIns * current_ins;
    MVMJitGraph * jit_graph;
    MVMJitNode * current_node;
    if (spesh->num_bbs > 1) {
	return NULL;
    }
    /* set up the graph */
    jit_graph = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitGraph));
    current_node = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitNode));
    
    jit_graph->spesh = spesh;
    jit_graph->entry = current_node;
    current_bb = spesh->entry;
    current_ins = current_bb->first_ins;
    /* loop over all instructions. */
    while (current_ins) {
	switch(current_ins->info->opcode) {
	case MVM_OP_const_i64: {
	    /* build a graph of 
	     * store --> literal
	     *       \-> address 
	     * and insert it into the chain
	     */
	    MVMint32 reg = current_ins->operands[0].reg.i;
	    MVMint64 val = current_ins->operands[1].lit_i64;
	    MVMJitStore *st = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitStore));
	    MVMJitLiteral *lt = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitLiteral));
	    MVMJitAddress *ad = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitAddress));

	    lt->node.type = MVM_JIT_NODE_LITERAL;
	    lt->size = sizeof(MVMint64);
	    lt->val.i64 = val;

	    ad->node.type = MVM_JIT_NODE_ADDRESS;
	    ad->type = MVM_JIT_MOAR_REGISTER;
	    ad->offset = reg;

	    st->node.type = MVM_JIT_NODE_STORE;
	    st->value = (MVMJitNode*) lt;
	    st->dest = (MVMJitNode*) ad;

	    current_node->next = (MVMJitNode*)st;
	    current_node = (MVMJitNode*)st;
	    break;
	}
	case MVM_OP_add_i: {
	    /* build a graph of 
	     * store --> add --> load --> address
	     *       \       \-> load --> address
             *        \-> address
	     * and insert it into the graph */
	    MVMint32 reg_result = current_ins->operands[0].reg.i;
	    MVMint32 reg_a = current_ins->operands[1].reg.i;
	    MVMint32 reg_b = current_ins->operands[2].reg.i;
	    MVMJitStore *st = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitStore));
	    MVMJitArith *ar = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitArith));
	    MVMJitAddress *adr = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitAddress));
	    MVMJitAddress *ada = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitAddress));
	    MVMJitAddress *adb = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitAddress));
	    MVMJitLoad * lda = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitLoad));
	    MVMJitLoad * ldb = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitLoad));

	    ada->node.type = MVM_JIT_NODE_ADDRESS;
	    ada->type = MVM_JIT_MOAR_REGISTER;
	    ada->offset = reg_a;

	    lda->node.type = MVM_JIT_NODE_LOAD;
	    lda->source = (MVMJitNode*)ada;

	    adb->node.type = MVM_JIT_NODE_ADDRESS;
	    adb->type = MVM_JIT_MOAR_REGISTER;
	    adb->offset = reg_b;

	    ldb->node.type = MVM_JIT_NODE_LOAD;
	    ldb->source = (MVMJitNode*)adb;

	    ar->node.type = MVM_JIT_NODE_ARITH;
	    ar->op = MVM_JIT_OP_ADD_I64;
	    ar->first = (MVMJitNode*)lda;
	    ar->second = (MVMJitNode*)ldb;

	    adr->node.type = MVM_JIT_NODE_ADDRESS;
	    adr->type = MVM_JIT_MOAR_REGISTER;
	    adr->offset = reg_result;

	    st->node.type = MVM_JIT_NODE_STORE;
	    st->value = (MVMJitNode*)ar;
	    st->dest = (MVMJitNode*)adr;
	    
	    current_node->next = (MVMJitNode*)st;
	    current_node = (MVMJitNode*)st;
	}
	default: 
	    /* Can't compile this opcode */
	    return NULL;
	}
	current_ins = current_ins->next;
    }
    /* finish the graph */
    current_node->next = MVM_spesh_alloc(tc, spesh, sizeof(MVMJitNode));
    current_node = current_node->next;

    jit_graph->exit = current_node;
    return jit_graph;
}


MVMJitCode MVM_jit_compile_graph(MVMThreadContext *tc, MVMJitGraph *graph) {
    
    dasm_State *state;
    char * memory;
    size_t codesize;
    
    dasm_init(&state, 1);
    dasm_setup(&state, MVM_jit_actions());

    /* here we generate code */
    MVM_jit_emit_prologue(tc, &state);
    /* tumbleweed */
    MVM_jit_emit_epilogue(tc, &state);
    
    dasm_link(&state, &codesize);
    memory = MVM_platform_alloc_pages(codesize, MVM_PAGE_READ|MVM_PAGE_WRITE);
    dasm_encode(&state, memory);
    MVM_platform_set_page_mode(memory, codesize, MVM_PAGE_READ|MVM_PAGE_EXEC);

    return (MVMJitCode)memory;
}
