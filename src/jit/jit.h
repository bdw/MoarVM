/* Whats the basic idea here? For now, an MVMJitGraph is simply a
 * linear stream of compilable instructions. The difference between
 * this and MVMSpeshGraph is that it contains information to help 
 * allocate registers and select instructions. 
 *
 * This isn't even my final form. */
struct MVMJitGraph {
    MVMSpeshGraph * spesh;
    MVMJitNode * entry;
    MVMJitNode * exit;

    MVMJitCpuReg * cpu_regs;
    size_t num_cpu_reg;
};


/* A cpu register descriptor */
struct MVMJitCpuReg {
    const MVMuint8 reg_id;       // identifier for this register
    const MVMuint8 is_reserved;  // is this a reserved register
    const char * name;           // human readable name for dumping
    int last_use;                // last used in instruction 
};

typedef enum {
    MVM_JIT_NODE_STORE,
    MVM_JIT_NODE_LOAD,
    MVM_JIT_NODE_ADDRESS,
    MVM_JIT_NODE_LITERAL,
    MVM_JIT_NODE_ARITH,
    MVM_JIT_NODE_CALL_C,
} MVMJitNodeType;

typedef enum {
    MVM_JIT_CPU_REGISTER,
    MVM_JIT_CPU_FRAME,
    MVM_JIT_MOAR_REGISTER,
    MVM_JIT_MOAR_LEXICAL,
} MVMJitAddressType;

typedef enum {
    MVM_JIT_OP_ADD_I64,
    MVM_JIT_OP_SUB_I64,
    MVM_JIT_OP_MUL_I64,
    MVM_JIT_OP_DIV_I64,
} MVMJitArithOp;

struct MVMJitNode {
    MVMJitNodeType type;
    MVMJitNode * next; 
};

struct MVMJitAddress {
    MVMJitNode node;
    MVMJitAddressType type;
    MVMint32 offset;
};

struct MVMJitStore {
    MVMJitNode node;
    MVMJitNode * dest;
    MVMJitNode * value;
};

struct MVMJitLoad {
    MVMJitNode node;
    MVMJitNode * source;
};

struct MVMJitArith {
    MVMJitNode node;
    MVMJitArithOp op;
    MVMJitNode * first;
    MVMJitNode * second;
};


struct MVMJitLiteral {
    MVMJitNode node;
    size_t size;
    union {
	MVMint32  i32;
	MVMuint32 u32;
	MVMint64  i64;
	MVMuint64 u64;
    } val;
};

struct MVMJitCallC {
    MVMJitNode node;
    void * func_ptr; // what do we call
    MVMuint16 num_args; // how many arguments we pass
    MVMuint16 has_vargs; // does the receiver consider them variable
};


typedef void (*MVMJitCode)(MVMThreadContext *tc, MVMFrame *frame);

MVMJitGraph* MVM_jit_try_make_jit_graph(MVMThreadContext *tc, MVMSpeshGraph *spesh);
MVMJitCode MVM_jit_compile_graph(MVMThreadContext *tc, MVMJitGraph *graph);


