////////////////////////////////////////////////////////////////
// rune: Char flags

typedef enum char_flags {
    CHAR_FLAG_CONTROL    = (1 << 0),
    CHAR_FLAG_PRINTABLE  = (1 << 1),
    CHAR_FLAG_WHITESPACE = (1 << 2),
    CHAR_FLAG_UPPER      = (1 << 3),
    CHAR_FLAG_LOWER      = (1 << 4),
    CHAR_FLAG_LETTER     = (1 << 5),
    CHAR_FLAG_DIGIT      = (1 << 6),
    CHAR_FLAG_HEXDIGIT   = (1 << 7),
    CHAR_FLAG_PUNCT      = (1 << 8),
    CHAR_FLAG_UNDERSCORE = (1 << 9),
    CHAR_FLAG_NON_ASCII  = (1 << 10),

    CHAR_FLAG_WORD       = CHAR_FLAG_LETTER|CHAR_FLAG_DIGIT|CHAR_FLAG_UNDERSCORE,
    CHAR_FLAG_WORDSEP    = CHAR_FLAG_PUNCT|CHAR_FLAG_WHITESPACE,
} char_flags;

static char_flags u32_get_char_flags(u32 c);
static char_flags u8_get_char_flags(u8 c);

////////////////////////////////////////////////////////////////
// rune: Ascii

static bool u8_is_letter(u8 c);
static bool u8_is_digit(u8 c);
static bool u8_is_upper(u8 c);
static bool u8_is_lower(u8 c);
static bool u8_is_punct(u8 c);
static bool u8_is_printable(u8 c);
static bool u8_is_whitespace(u8 c);

static bool u32_is_letter(u32 c);
static bool u32_is_digit(u32 c);
static bool u32_is_upper(u32 c);
static bool u32_is_lower(u32 c);
static bool u32_is_punct(u32 c);
static bool u32_is_printable(u32 c);
static bool u32_is_whitespace(u32 c);

static u8 u8_to_lower(u8 c);
static u8 u8_to_upper(u8 c);

static u32 u32_to_lower(u32 c);
static u32 u32_to_upper(u32 c);

////////////////////////////////////////////////////////////////
// rune: String operations

static str str_from_cstr(char *cstr);
static bool str_eq(str a, str b);
static bool str_eq_nocase(str a, str b);

static i64 str_idx_of_u8(str s, u8 c);
static i64 str_idx_of_str(str a, str b);
static i64 str_idx_of_last_u8(str s, u8 c);
static i64 str_idx_of_last_str(str a, str b);

static str substr_idx(str s, i64 idx);
static str substr_len(str s, i64 idx, i64 len);
static str substr_range(str s, i64_range r);

static str str_chop_left(str *s, i64 idx);
static str str_chop_right(str *s, i64 idx);
static str str_chop_by_delim(str *s, str delim);

static str str_left(str s, i64 len);
static str str_right(str s, i64 len);
static str str_trim(str s);
static str str_trim_left(str s);
static str str_trim_right(str s);
static bool str_starts_with_u8(str s, u8 c);
static bool str_starts_with_str(str s, str prefix);
static bool str_ends_with_u8(str s, u8 c);
static bool str_ends_with_str(str s, str suffix);

typedef struct { str v[3]; } str_x3;
static str_x3 str_split_x3(str s, i64 a, i64 b);

////////////////////////////////////////////////////////////////
// rune: String list

typedef struct str_node str_node;
struct str_node {
    str v;
    str_node *next;
};

typedef struct str_list str_list;
struct str_list {
    str_node *first;
    str_node *last;
    i64 count;
    i64 total_len;
};

static str  str_list_push(str_list *list, arena *arena, str s);
static str  str_list_push_fmt_args(str_list *list, arena *arena, args args);
#define     str_list_push_fmt(list, arena, ...) (str_list_push_fmt_args((list), (arena), argsof(__VA_ARGS__)))
static str  str_list_push_front(str_list *list, arena *arena, str s);
static void str_list_join(str_list *list, str_list append);
static str  str_list_concat(str_list *list, arena *arena);
static str  str_list_concat_sep(str_list *list, arena *arena, str sep);

////////////////////////////////////////////////////////////////
// rune: String array

typedef struct str_array str_array;
struct str_array {
    str *v;
    i64 count;
};

static str_array str_array_reserve(i64 count, arena *arena);
static str_array str_array_from_list(str_list list, arena *arena);

////////////////////////////////////////////////////////////////
// rune: Comparison

static i32 str_cmp_case(str a, str b);
static i32 str_cmp_nocase(str a, str b);

////////////////////////////////////////////////////////////////
// rune: Sorting

static void str_sort_case(str_array array);
static void str_sort_nocase(str_array array);

////////////////////////////////////////////////////////////////
// rune: Widestrings

static bool wstr_eq(wstr a, wstr b);

////////////////////////////////////////////////////////////////
// rune: Fuzzy matching

typedef struct fuzzy_match_node fuzzy_match_node;
struct fuzzy_match_node {
    i64_range range;
    fuzzy_match_node *next;
};

typedef struct fuzzy_match_list fuzzy_match_list;
struct fuzzy_match_list {
    fuzzy_match_node *first;
    i64 count;
};

static void             fuzzy_match_list_insert_after(fuzzy_match_list *list, fuzzy_match_node *after, i64_range range, arena *arena);
static fuzzy_match_list fuzzy_match_list_from_str(str haystack, str_list needles, arena *arena);
