////////////////////////////////////////////////////////////////
// rune: Configuration

#define DK_DEBUG_PRINT_LEX      0
#define DK_DEBUG_PRINT_PARSE    0
#define DK_DEBUG_PRINT_CHECK    0
#define DK_DEBUG_PRINT_EMIT     0

////////////////////////////////////////////////////////////////
// rune: Errors

static arena *temp_arena = { 0 };

// TODO(rune): Think about a smarter way to report source code locations.
// currently most of the ast's memory usage is dk_loc structs.
typedef struct dk_loc dk_loc;
struct dk_loc {
    i64 row;
    i64 col;
    i64 pos;
    str file_name;
};

typedef struct dk_err dk_err;
struct dk_err {
    str msg;
    dk_loc loc;
    dk_err *next;
};

typedef struct dk_err_list dk_err_list;
struct dk_err_list {
    dk_err *first;
    dk_err *last;
    i64 count;
};

typedef struct dk_err_sink dk_err_sink;
struct dk_err_sink {
    dk_err_list err_list;
};

#define                 dk_tprint(...) dk_tprint_args(argsof(__VA_ARGS__))
static str              dk_tprint_args(args args);

static void dk_report_err(dk_err_sink *sink, dk_loc loc, str msg);
static void dk_print_err(dk_err *err, arena *arena);

////////////////////////////////////////////////////////////////
// rune: Bytecode

typedef enum dk_bc_opcode {
    DK_BC_OPCODE_NOP,

    DK_BC_OPCODE_LDI,
    DK_BC_OPCODE_LDL,
    DK_BC_OPCODE_STL,
    DK_BC_OPCODE_POP,
    DK_BC_OPCODE_DUP,

    DK_BC_OPCODE_ADD,
    DK_BC_OPCODE_SUB,
    DK_BC_OPCODE_UMUL,
    DK_BC_OPCODE_IMUL,
    DK_BC_OPCODE_UDIV,
    DK_BC_OPCODE_IDIV,

    DK_BC_OPCODE_FADD,
    DK_BC_OPCODE_FSUB,
    DK_BC_OPCODE_FMUL,
    DK_BC_OPCODE_FDIV,

    DK_BC_OPCODE_AND,
    DK_BC_OPCODE_OR,
    DK_BC_OPCODE_NOT,

    DK_BC_OPCODE_EQ,
    DK_BC_OPCODE_GT,
    DK_BC_OPCODE_LT,

    DK_BC_OPCODE_FEQ,
    DK_BC_OPCODE_FGT,
    DK_BC_OPCODE_FLT,

    DK_BC_OPCODE_CALL,
    DK_BC_OPCODE_RET,
    DK_BC_OPCODE_BR,

    DK_BC_OPCODE_I2F,
    DK_BC_OPCODE_F2I,

    DK_BC_OPCODE_COUNT,
} dk_bc_opcode;

typedef enum dk_bc_operand_kind {
    DK_BC_OPERAND_KIND_NONE,
    DK_BC_OPERAND_KIND_IMM,
    DK_BC_OPERAND_KIND_LOC,
    DK_BC_OPERAND_KIND_SYM,
    DK_BC_OPERAND_KIND_POS,

    DK_BC_OPERAND_KIND_COUNT,
} dk_bc_operand_kind;

typedef union dk_bc_inst_prefix dk_bc_inst_prefix;
union dk_bc_inst_prefix {
    struct { u8 opcode : 6, operand_size : 2; };
    struct { u8 u8; };
};

typedef struct dk_bc_inst dk_bc_inst;
struct dk_bc_inst {
    u8 opcode;
    u64 operand;
};

typedef struct dk_bc_opcode_info dk_bc_opcode_info;
struct dk_bc_opcode_info {
    str name;
    dk_bc_operand_kind operand_kind;
};

static readonly dk_bc_opcode_info dk_bc_opcode_infos[DK_BC_OPCODE_COUNT] = {
#if 1 // English mnemonics
    [DK_BC_OPCODE_NOP] =   { STR("nop"),                                 },
    [DK_BC_OPCODE_LDI]   = { STR("ldi"),      DK_BC_OPERAND_KIND_IMM     },
    [DK_BC_OPCODE_LDL]   = { STR("ldl"),      DK_BC_OPERAND_KIND_LOC     },
    [DK_BC_OPCODE_POP]   = { STR("pop"),                                 },
    [DK_BC_OPCODE_STL]   = { STR("stl"),      DK_BC_OPERAND_KIND_LOC     },
    [DK_BC_OPCODE_DUP]   = { STR("dup"),                                 },

    [DK_BC_OPCODE_ADD]   = { STR("add"),                                 },
    [DK_BC_OPCODE_SUB]   = { STR("sub"),                                 },
    [DK_BC_OPCODE_UMUL]  = { STR("umul"),                                },
    [DK_BC_OPCODE_IMUL]  = { STR("imul"),                                },
    [DK_BC_OPCODE_UDIV]  = { STR("udiv"),                                },
    [DK_BC_OPCODE_IDIV]  = { STR("idiv"),                                },

    [DK_BC_OPCODE_FADD]  = { STR("fadd"),                                },
    [DK_BC_OPCODE_FSUB]  = { STR("fsub"),                                },
    [DK_BC_OPCODE_FMUL]  = { STR("fmul"),                                },
    [DK_BC_OPCODE_FDIV]  = { STR("fdiv"),                                },

    [DK_BC_OPCODE_AND]   = { STR("and"),                                 },
    [DK_BC_OPCODE_OR]    = { STR("or"),                                  },
    [DK_BC_OPCODE_NOT]   = { STR("not"),                                 },

    [DK_BC_OPCODE_EQ]    = { STR("eq"),                                  },
    [DK_BC_OPCODE_GT]    = { STR("gt"),                                  },
    [DK_BC_OPCODE_LT]    = { STR("lt"),                                  },

    [DK_BC_OPCODE_FEQ]   = { STR("feq"),                                 },
    [DK_BC_OPCODE_FGT]   = { STR("fgt"),                                 },
    [DK_BC_OPCODE_FLT]   = { STR("flt"),                                 },

    [DK_BC_OPCODE_CALL]  = { STR("call"),     DK_BC_OPERAND_KIND_SYM     },
    [DK_BC_OPCODE_RET]   = { STR("ret"),                                 },
    [DK_BC_OPCODE_BR]    = { STR("br"),       DK_BC_OPERAND_KIND_POS     },

    [DK_BC_OPCODE_I2F]   = { STR("i2f"),                                 },
    [DK_BC_OPCODE_F2I]   = { STR("f2i"),                                 },
#else
    [DK_BC_OPCODE_NOP] =   { STR("nop"),                                 },
    [DK_BC_OPCODE_LDI]   = { STR("ilu"),      DK_BC_OPERAND_KIND_IMM     }, // Indlæs umiddelbar
    [DK_BC_OPCODE_LDL]   = { STR("ill"),      DK_BC_OPERAND_KIND_LOC     }, // Indlæs lokal
    [DK_BC_OPCODE_POP]   = { STR("tag"),                                 },
    [DK_BC_OPCODE_STL]   = { STR("gel"),      DK_BC_OPERAND_KIND_LOC     }, // Gem lokal

    [DK_BC_OPCODE_ADD]   = { STR("plus"),                                 },
    [DK_BC_OPCODE_SUB]   = { STR("minus"),                                 },
    [DK_BC_OPCODE_UMUL]  = { STR("ugange"),                                },
    [DK_BC_OPCODE_IMUL]  = { STR("igange"),                                },
    [DK_BC_OPCODE_UDIV]  = { STR("udel"),                                },
    [DK_BC_OPCODE_IDIV]  = { STR("idel"),                                },

    [DK_BC_OPCODE_FADD]  = { STR("fplus"),                                },
    [DK_BC_OPCODE_FSUB]  = { STR("fminus"),                                },
    [DK_BC_OPCODE_FMUL]  = { STR("fgange"),                                },
    [DK_BC_OPCODE_FDIV]  = { STR("fdel"),                                },

    [DK_BC_OPCODE_AND]   = { STR("or"),                                 },
    [DK_BC_OPCODE_OR]    = { STR("elr"),                                  },
    [DK_BC_OPCODE_NOT]   = { STR("ikke"),                                 },

    [DK_BC_OPCODE_CALL]  = { STR("kald"),     DK_BC_OPERAND_KIND_SYM     },
    [DK_BC_OPCODE_RET]   = { STR("tilbage"),                                 },
    [DK_BC_OPCODE_BR]    = { STR("gren"),     DK_BC_OPERAND_KIND_POS     },
#endif
};

typedef struct dk_bc_symbol dk_bc_symbol;
struct dk_bc_symbol {
    i64 id;
    i64 size;
    i64 pos;
};

////////////////////////////////////////////////////////////////
// rune: Number spelling

typedef struct dk_num_spelling dk_num_spelling;
struct dk_num_spelling {
    str spelling;
    i64 val;
};

static readonly dk_num_spelling dk_digit_nums[] = {
    { STR("nul"),   0 },
    { STR("en"),    1 },
    { STR("et"),    1 },
    { STR("to"),    2 },
    { STR("tre"),   3 },
    { STR("fire"),  4 },
    { STR("fem"),   5 },
    { STR("seks"),  6 },
    { STR("syv"),   7 },
    { STR("otte"),  8 },
    { STR("ni"),    9 },
};

static readonly dk_num_spelling dk_tens_nums[] = {
    { STR("ti"),        10 },

    { STR("elleve"),    11 },
    { STR("tolv"),      12 },
    { STR("tretten"),   13 },
    { STR("fjorten"),   14 },
    { STR("femten"),    15 },
    { STR("seksten"),   16 },
    { STR("sytten"),    17 },
    { STR("atten"),     18 },
    { STR("nitten"),    19 },

    { STR("tyve"),      20 },
    { STR("tredive"),   30 },
    { STR("fyrre"),     40 },
    { STR("halvtreds"), 50 },
    { STR("tres"),      60 },
    { STR("halvfjers"), 70 },
    { STR("firs"),      80 },
    { STR("halvfems"),  90 },
};

static readonly dk_num_spelling dk_mult_nums[] = {
    { STR("hundrede"),             100 },
    { STR("hundred"),              100 },
    { STR("tusinde"),             1000 },
    { STR("tusind"),              1000 },
    { STR("millioner"),        1000000 },
    { STR("million"),          1000000 },
    { STR("milliarder"),    1000000000 },
    { STR("milliard"),      1000000000 },
};

static dk_num_spelling *dk_chop_num_spelling(str *s, dk_num_spelling *spellings, i64 spellings_count);
static bool             dk_chop_str_maybe(str *s, str chop);
static i64              dk_number_from_str(str s);

////////////////////////////////////////////////////////////////
// rune: Tokenization

typedef enum dk_literal_kind {
    DK_LITERAL_KIND_NONE,
    DK_LITERAL_KIND_INT,
    DK_LITERAL_KIND_FLOAT,
    DK_LITERAL_KIND_BOOL,

    DK_LITERAL_KIND_COUNT,
} dk_literal_kind;

typedef struct dk_literal dk_literal;
struct dk_literal {
    dk_literal_kind kind;
    union {
        i64 int_;
        f64 float_;
        bool bool_;
    };
};

typedef enum dk_token_kind {
    DK_TOKEN_KIND_EOF,

    // rune: Punctuation
    DK_TOKEN_KIND_DOT,
    DK_TOKEN_KIND_COMMA,
    DK_TOKEN_KIND_PAREN_OPEN,
    DK_TOKEN_KIND_PAREN_CLOSE,

    // rune: Generic tokens
    DK_TOKEN_KIND_LITERAL,
    DK_TOKEN_KIND_WORD,
    DK_TOKEN_KIND_COMMENT,

    DK_TOKEN_KIND_COUNT,
} dk_token_kind;

typedef struct dk_keyword_spelling dk_keyword_spelling;
struct dk_keyword_spelling {
    dk_token_kind keyword;
    str spelling;
};

typedef struct dk_token dk_token;
struct dk_token {
    dk_loc loc;
    dk_literal literal;
    str text;
    dk_token_kind kind;
    dk_token *next;
};

typedef struct dk_token_list dk_token_list;
struct dk_token_list {
    dk_token *first;
    dk_token *last;
    i64 count;
};

typedef struct dk_tokenizer dk_tokenizer;
struct dk_tokenizer {
    str src;
    dk_loc loc;
    u8 peek0;
    u8 peek1;
};

static str           dk_str_from_token_kind(dk_token_kind a);

static void          dk_tokenizer_eat(dk_tokenizer *t);
static void          dk_tokenizer_init(dk_tokenizer *t, str src, i64 pos);

static bool          dk_next_token(dk_tokenizer *t, dk_token *token);

static dk_token_list dk_token_list_from_str(str s, arena *arena);

////////////////////////////////////////////////////////////////
// rune: Forward declarations

typedef struct dk_pattern       dk_pattern;
typedef struct dk_pattern_part  dk_pattern_part;
typedef struct dk_clause        dk_clause;
typedef struct dk_clause_part   dk_clause_part;
typedef struct dk_expr          dk_expr;
typedef struct dk_stmt          dk_stmt;
typedef struct dk_func          dk_func;
typedef struct dk_type          dk_type;
typedef struct dk_symbol        dk_symbol;
typedef struct dk_local         dk_local;

////////////////////////////////////////////////////////////////
// rune: Patterns

typedef struct dk_pattern_part dk_pattern_part;
struct dk_pattern_part {
    str name;
    str type_name;
    dk_type *type;
    dk_pattern_part *next;
};

typedef struct dk_pattern_part_list dk_pattern_part_list;
struct dk_pattern_part_list {
    dk_pattern_part *first;
    dk_pattern_part *last;
    i64 count;
};

typedef struct dk_pattern dk_pattern;
struct dk_pattern {
    dk_pattern_part_list parts;
};

static dk_pattern_part *dk_pattern_push_part(dk_pattern *p, arena *arena);
static dk_pattern_part *dk_pattern_push_word(dk_pattern *p, arena *arena, str name);
static dk_pattern_part *dk_pattern_push_type(dk_pattern *p, arena *arena, str name, str type_name);

////////////////////////////////////////////////////////////////
// rune: Clause types

typedef struct dk_clause_part_list dk_clause_part_list;
struct dk_clause_part_list {
    dk_clause_part *first;
    dk_clause_part *last;
};

typedef struct dk_clause_list dk_clause_list;
struct dk_clause_list {
    dk_clause *first;
    dk_clause *last;
};

typedef enum dk_clause_part_kind {
    DK_CLAUSE_PART_KIND_NONE,
    DK_CLAUSE_PART_KIND_WORD,
    DK_CLAUSE_PART_KIND_LIST,
    DK_CLAUSE_PART_KIND_LITERAL,

    DK_CLAUSE_PART_KIND_COUNT,
} dk_clause_part_kind;

typedef enum dk_clause_part_flags {
    DK_CLAUSE_PART_FLAG_DEN_DET = 1,
} dk_clause_part_flags;

typedef struct dk_clause_part dk_clause_part;
struct dk_clause_part {
    dk_clause_part_kind kind;
    dk_clause_part_flags flags;
    union {
        str word;
        dk_clause_list list;
        dk_literal literal;
    };
    dk_clause_part *next;

    dk_token *token;
};

typedef struct dk_clause dk_clause;
struct dk_clause {
    dk_clause_part_list parts;
    dk_clause *next;

    dk_token *token;
};

////////////////////////////////////////////////////////////////
// rune: Statement types.

typedef struct dk_stmt_list dk_stmt_list;
struct dk_stmt_list {
    dk_stmt *first;
    dk_stmt *last;
};

typedef enum dk_stmt_kind {
    DK_STMT_KIND_NONE,
    DK_STMT_KIND_EXPR,
    DK_STMT_KIND_DECL,
    DK_STMT_KIND_RETURN,
    DK_STMT_KIND_IF,
    DK_STMT_KIND_WHILE,

    DK_STMT_KIND_COUNT,
} dk_stmt_kind;

typedef struct dk_stmt dk_stmt;
struct dk_stmt {
    dk_stmt_kind kind;
    dk_clause_list clauses;
    str name;
    str type_name;
    dk_expr *expr;
    dk_stmt_list then;
    dk_stmt_list else_;
    dk_stmt *next;
};

////////////////////////////////////////////////////////////////
// rune: Local variable types

typedef enum dk_local_flags {
    DK_LOCAL_FLAG_ARG = 1,
} dk_local_flags;

typedef struct dk_local dk_local;
struct dk_local {
    str name;
    i64 off;
    dk_local_flags flags;
    dk_type *type;
    dk_local *next;
};

typedef struct dk_local_list dk_local_list;
struct dk_local_list {
    dk_local *first;
    dk_local *last;
    i64 count;
};

////////////////////////////////////////////////////////////////
// rune: Function types

typedef enum dk_func_kind {
    DK_FUNC_KIND_NONE,
    DK_FUNC_KIND_USER,
    DK_FUNC_KIND_OPCODE,
    DK_FUNC_KIND_NATIVE,
    DK_FUNC_KIND_ASSIGN,

    DK_FUNC_KIND_COUNT,
} dk_func_kind;

typedef struct dk_func dk_func;
struct dk_func {
    dk_pattern pattern;
    dk_stmt_list stmts;

    dk_func_kind kind;
    dk_bc_opcode opcode; // TODO(rune): Cleanup

    str      type_name;
    dk_type *type;

    dk_local_list locals;
    u32 symbol_id;

    dk_func *next;
};

typedef struct dk_func_list dk_func_list;
struct dk_func_list {
    dk_func *first;
    dk_func *last;
};

////////////////////////////////////////////////////////////////
// rune: Expression types

typedef struct dk_expr_list dk_expr_list;
struct dk_expr_list {
    dk_expr *first;
    dk_expr *last;
};

typedef enum dk_expr_kind {
    DK_EXPR_KIND_NONE,
    DK_EXPR_KIND_FUNC,
    DK_EXPR_KIND_LITERAL,
    DK_EXPR_KIND_LOCAL,
    DK_EXPR_KIND_LIST,
    DK_EXPR_KIND_ASSIGN,

    DK_EXPR_KIND_COUNT,
} dk_expr_kind;

typedef struct dk_expr dk_expr;
struct dk_expr {
    dk_expr_kind kind;
    union {
        struct { dk_func *func; dk_expr_list func_args; };
        dk_literal literal;
        dk_local *local;
        dk_expr_list list;
    };
    dk_type *type;
    dk_expr *next;
};

////////////////////////////////////////////////////////////////
// rune: Type types

typedef struct dk_type_list dk_type_list;
struct dk_type_list {
    dk_type *first;
    dk_type *last;
};

typedef struct dk_type dk_type;
struct dk_type {
    str name;
    i64 size;
    dk_type *next;
};

////////////////////////////////////////////////////////////////
// rune: Parse tree types

typedef struct dk_tree dk_tree;
struct dk_tree {
    dk_type_list types;
    dk_func_list funcs;
};

////////////////////////////////////////////////////////////////
// rune: Parser

typedef struct dk_parser dk_parser;
struct dk_parser {
    dk_token *peek;
    arena *arena;
};

// rune: Initialization
static void dk_parser_init(dk_parser *p, dk_token_list tokens, arena *arena);

// rune: Token peek and token consumption
static dk_token *dk_peek_token(dk_parser *p);
static bool      dk_peek_token_kind(dk_parser *p, dk_token_kind kind);
static bool      dk_peek_token_text(dk_parser *p, str text);

static dk_token *dk_eat_token(dk_parser *p);
static dk_token *dk_eat_token_kind(dk_parser *p, dk_token_kind kind);
static dk_token *dk_eat_token_text(dk_parser *p, str text);

static bool      dk_eat_token_kind_maybe(dk_parser *p, dk_token_kind kind);
static bool      dk_eat_token_text_maybe(dk_parser *p, str text);

// rune: Parsing
static dk_clause *         dk_parse_clause(dk_parser *p);
static dk_clause_list      dk_parse_clause_list(dk_parser *p);
static dk_stmt *           dk_parse_stmt(dk_parser *p);
static dk_stmt_list        dk_parse_stmt_list(dk_parser *p);
static dk_func *           dk_parse_func(dk_parser *p);
static dk_func_list        dk_parse_func_list(dk_parser *p);
static dk_tree *           dk_parse_tree(dk_parser *p);

////////////////////////////////////////////////////////////////
// rune: Semantic analysis

typedef struct dk_checker dk_checker;
struct dk_checker {
    dk_tree *tree;

    dk_type *builtin_int;
    dk_type *builtin_float;
    dk_type *builtin_bool;
    dk_type *builtin_any;
    dk_local_list locals;


    arena *arena;

    u32 symbol_id_counter;
};

static dk_expr *dk_check_clause(dk_checker *c, dk_clause *clause);
static dk_expr *dk_check_clause_list(dk_checker *c, dk_clause_list clauses);
static void     dk_check_stmt(dk_checker *c, dk_stmt *stmt);
static void     dk_check_stmt_list(dk_checker *c, dk_stmt_list stmts);
static void     dk_check_func_sig(dk_checker *c, dk_func *func);
static void     dk_check_func_body(dk_checker *c, dk_func *func);
static void     dk_check_tree(dk_tree *tree, arena *arena);

////////////////////////////////////////////////////////////////
// rune: Dynamic buffer

typedef struct dk_buffer dk_buffer;
struct dk_buffer {
    u8 *data;
    i64 capacity;
    i64 size;
};

static void *   dk_buffer_push(dk_buffer *buffer, i64 size);
static u8 *     dk_buffer_push_u8(dk_buffer *buffer, u8 a);
static u16 *    dk_buffer_push_u16(dk_buffer *buffer, u16 a);
static u32 *    dk_buffer_push_u32(dk_buffer *buffer, u32 a);
static u64 *    dk_buffer_push_u64(dk_buffer *buffer, u64 a);
#define         dk_buffer_push_struct(buffer, T) ((T *)dk_buffer_push((buffer), sizeof(T)))

static void *   dk_buffer_pop(dk_buffer *buffer, i64 size);
static u8       dk_buffer_pop_u8(dk_buffer *buffer);
static u16      dk_buffer_pop_u16(dk_buffer *buffer);
static u32      dk_buffer_pop_u32(dk_buffer *buffer);
static u64      dk_buffer_pop_u64(dk_buffer *buffer);
#define         dk_buffer_pop_struct(buffer, T) ((T *)dk_buffer_pop((buffer), sizeof(T)))

static void *   dk_buffer_get(dk_buffer *buffer, i64 pos, i64 size);
static u8 *     dk_buffer_get_u8(dk_buffer *buffer, i64 pos);
static u16 *    dk_buffer_get_u16(dk_buffer *buffer, i64 pos);
static u32 *    dk_buffer_get_u32(dk_buffer *buffer, i64 pos);
static u64 *    dk_buffer_get_u64(dk_buffer *buffer, i64 pos);
#define         dk_buffer_get_struct(buffer, pos, T) ((T *)dk_buffer_get((buffer), pos))

static void *   dk_buffer_read(dk_buffer *buffer, i64 *pos, i64 size);
static u8       dk_buffer_read_u8(dk_buffer *buffer, i64 *pos);
static u16      dk_buffer_read_u16(dk_buffer *buffer, i64 *pos);
static u32      dk_buffer_read_u32(dk_buffer *buffer, i64 *pos);
static u64      dk_buffer_read_u64(dk_buffer *buffer, i64 *pos);
#define         dk_buffer_read_struct(buffer, pos, T) ((T *)dk_buffer_read(buffer, pos, sizeof(T)))

////////////////////////////////////////////////////////////////
// rune: Bytecode emit

typedef struct dk_program dk_program;
struct dk_program {
    dk_buffer head;
    dk_buffer body;
};

typedef struct dk_emitter dk_emitter;
struct dk_emitter {
    dk_buffer head;
    dk_buffer body;
};

// rune: Low-level emit helpers.
static u8 * dk_emit_u8(dk_emitter *e, u8 a);
static u16 *dk_emit_u16(dk_emitter *e, u16 a);
static u32 *dk_emit_u32(dk_emitter *e, u32 a);
static u64 *dk_emit_u64(dk_emitter *e, u64 a);
static void dk_emit_inst1(dk_emitter *e, dk_bc_opcode opcode);
static void dk_emit_inst2(dk_emitter *e, dk_bc_opcode opcode, u64 operand);

// rune: Emit tree.
static void dk_emit_symbol(dk_emitter *e, u32 id, u32 size);
static void dk_emit_literal(dk_emitter *e, dk_literal literal);
static void dk_emit_expr(dk_emitter *e, dk_expr *expr);
static void dk_emit_stmt_list(dk_emitter *e, dk_stmt_list stmts);
static void dk_emit_tree(dk_emitter *e, dk_tree *tree);

////////////////////////////////////////////////////////////////
// rune: Runtime

static str dk_run_program(dk_program program, arena *output_arena);

////////////////////////////////////////////////////////////////
// rune: Debug print

static void dk_print_level(i64 level);
static void dk_print_literal(dk_literal literal, i64 level);
static void dk_print_token(dk_token *token, i64 level);
static void dk_print_clause_part(dk_clause_part *part, i64 level);
static void dk_print_clause_part_list(dk_clause_part_list parts, i64 level);
static void dk_print_clause(dk_clause *clause, i64 level);
static void dk_print_clause_list(dk_clause_list clauses, i64 level);
static void dk_print_stmt(dk_stmt *stmt, i64 level);
static void dk_print_stmt_list(dk_stmt_list stmts, i64 level);
static void dk_print_pattern_part(dk_pattern_part *part, i64 level);
static void dk_print_pattern(dk_pattern pattern, i64 level);
static void dk_print_func(dk_func *func, i64 level);
static void dk_print_func_list(dk_func_list funcs, i64 level);
static void dk_print_type(dk_type *type, i64 level);
static void dk_print_type_list(dk_type_list types, i64 level);
static void dk_print_expr(dk_expr *expr, i64 level);
static void dk_print_expr_list(dk_expr_list exprs, i64 level);
static void dk_print_tree(dk_tree *tree, i64 level);
static void dk_print_local(dk_local *local, i64 level);
static void dk_print_local_list(dk_local_list locals, i64 level);
static void dk_print_program(dk_program program);
