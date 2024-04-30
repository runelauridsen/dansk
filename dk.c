////////////////////////////////////////////////////////////////
// rune: Errors

#define dk_tprint(...) dk_tprint_args(argsof(__VA_ARGS__))
static str dk_tprint_args(args args) {
    str ret = arena_print_args(temp_arena, args);
    return ret;
}

static dk_err *dk_err_list_add(dk_err_list *list, arena *arena) {
    dk_err *node = arena_push_struct(arena, dk_err);
    slist_push(list, node);
    list->count += 1;
    return node;
}

static void dk_report_err(dk_err_sink *sink, dk_loc loc, str msg) {
    dk_err *node = dk_err_list_add(&sink->err_list, temp_arena);
    node->msg = msg;
    node->loc = loc;
}

static void dk_print_err(dk_err *err, arena *arena) {
    str_list list = { 0 };
    str line = str("");
    i64 line_min = 0;
    i64 line_max = 0;
    {
        // TODO(rune): Hardcoded file name.
        err->loc.file_name = str("W:/dansk/scratch.dk");

        // TODO(rune): Don't re-read the file every time. Maybe add some kind of file-system-cache-thing?
        struct arena *temp = arena_create_default();
        str file_data = os_read_entire_file(err->loc.file_name, temp, null);
        line_min = err->loc.pos;

        bool found_non_whitespace = false;
        while (line_min > 0) {
            if (line_min < file_data.len) {
                if (u8_is_whitespace(file_data.v[line_min]) == false) {
                    found_non_whitespace = true;
                }

                if (found_non_whitespace && file_data.v[line_min - 1] == '\r') {
                    break;
                }

                if (found_non_whitespace && file_data.v[line_min - 1] == '\n') {
                    break;
                }
            }
            line_min--;
        }

        line_max = line_min;
        while (line_min < file_data.len) {
            if (!u8_is_whitespace(file_data.v[line_min])) break;
            line_min++;
        }

        while (line_max < file_data.len) {
            if (file_data.v[line_max] == '\r') break;
            if (file_data.v[line_max] == '\n') break;
            line_max++;
        }
        line = substr_range(file_data, i64_range(line_min, line_max));
        line = arena_copy_str(arena, line);

        arena_destroy(temp);
    }

    // rune: Error message.
    str_list_push_fmt(&list, arena, "Compilation error: %\n", err->msg);

    // rune: Source code location.
    str_list_push_fmt(&list, arena, "\n");
    str_list_push(&list, arena, str(ANSI_FG_GRAY));
    str loc_str = str_list_push_fmt(&list, arena, "%()(%:%) ", err->loc.file_name, err->loc.row + 1, err->loc.col + 1);
    str_list_push(&list, arena, str(ANSI_FG_DEFAULT));

    // rune: Source code text with error marker.
    i64 pad_len = err->loc.pos - line_min + loc_str.len;
    str pad     = arena_push_str(arena, pad_len, ' ');
    str_list_push_fmt(&list, arena, "%\n", line);
    str_list_push(&list, arena, str(ANSI_FG_BRIGHT_RED));
    str_list_push_fmt(&list, arena, "%^\n", pad);
    str_list_push(&list, arena, str(ANSI_FG_DEFAULT));

    str ret = str_list_concat(&list, arena);
    println(ret);
}

static dk_err_sink *dk_global_err = null; // TODO(rune): Remove global.

////////////////////////////////////////////////////////////////
// rune: Number spelling

static dk_num_spelling *dk_chop_num_spelling(str *s, dk_num_spelling *spellings, i64 spellings_count) {
    for_narray (dk_num_spelling, it, spellings, spellings_count) {
        if (str_starts_with_str(*s, it->spelling)) {
            str_chop_left(s, it->spelling.len);
            return it;
        }
    }

    return null;
}

static bool dk_chop_str_maybe(str *s, str chop) {
    str a = str_left(*s, chop.len);
    if (str_eq_nocase(a, chop)) {
        str_chop_left(s, chop.len);
        return true;
    } else {
        return false;
    }
}

static i64 dk_number_from_str(str s) {
    i64 a = 0;
    i64 b = 0;
    i64 c = 0;
    i64 last_mult = 0;
    i64 word_count = 0;

    while (s.len > 0) {
        while (dk_chop_str_maybe(&s, str("-")) || dk_chop_str_maybe(&s, str("og"))) {
        }

        dk_num_spelling *tens = dk_chop_num_spelling(&s, dk_tens_nums, countof(dk_tens_nums));
        if (tens) {
            a += tens->val;
            word_count += 1;
            continue;
        }

        dk_num_spelling *digit = dk_chop_num_spelling(&s, dk_digit_nums, countof(dk_digit_nums));
        if (digit) {
            a += digit->val;
            word_count += 1;
            continue;
        }

        dk_num_spelling *mult = dk_chop_num_spelling(&s, dk_mult_nums, countof(dk_mult_nums));
        if (mult) {
            if (last_mult >= mult->val) {
                c += b;
                b = 0;
            }

            if (b == 0 && a == 0) a = 1;

            b += a;
            b *= mult->val;
            a = 0;

            last_mult = mult->val;
            word_count += 1;
            continue;
        }

        break;
    }

    c += a;
    c += b;

    if (s.len > 0 || word_count == 0) {
        c = -1;
    }

    return c;
}

////////////////////////////////////////////////////////////////
// rune: Tokenizer

static dk_literal dk_make_literal_int(i64 v) { return (dk_literal) { .kind = DK_LITERAL_KIND_INT, .int_ = v }; }
static dk_literal dk_make_literal_float(f64 v) { return (dk_literal) { .kind = DK_LITERAL_KIND_FLOAT, .float_ = v }; }
static dk_literal dk_make_literal_bool(bool v) { return (dk_literal) { .kind = DK_LITERAL_KIND_BOOL, .bool_ = v }; }

static void dk_tokenizer_eat(dk_tokenizer *t) {
    t->loc.pos += 1;
    t->loc.col += 1;

    if (t->peek0 == '\n') {
        t->loc.col = 0;
        t->loc.row += 1;
    }

    t->peek0 = (t->loc.pos + 0 < t->src.len) ? (t->src.v[t->loc.pos + 0]) : ('\0');
    t->peek1 = (t->loc.pos + 1 < t->src.len) ? (t->src.v[t->loc.pos + 1]) : ('\0');
}

static void dk_tokenizer_init(dk_tokenizer *t, str src, i64 pos) {
    t->src     = src;
    t->loc.pos = pos - 1;
    dk_tokenizer_eat(t);
}

static str dk_str_from_token_kind(dk_token_kind a) {
    switch (a) {
        case DK_TOKEN_KIND_EOF:             return str("DK_TOKEN_KIND_EOF");
        case DK_TOKEN_KIND_DOT:             return str("DK_TOKEN_KIND_DOT");
        case DK_TOKEN_KIND_COMMA:           return str("DK_TOKEN_KIND_COMMA");
        case DK_TOKEN_KIND_PAREN_OPEN:      return str("DK_TOKEN_KIND_PAREN_OPEN");
        case DK_TOKEN_KIND_PAREN_CLOSE:     return str("DK_TOKEN_KIND_PAREN_CLOSE");
        case DK_TOKEN_KIND_LITERAL:         return str("DK_TOKEN_KIND_LITERAL");
        case DK_TOKEN_KIND_WORD:            return str("DK_TOKEN_KIND_WORD");
        case DK_TOKEN_KIND_COMMENT:         return str("DK_TOKEN_KIND_COMMENT");
        default:                            return str("INVALID");
    }
}

static bool dk_next_token(dk_tokenizer *t, dk_token *token) {
    zero_struct(token);

    // rune: Eat whitespace
    while (u8_get_char_flags(t->peek0) & CHAR_FLAG_WHITESPACE) {
        dk_tokenizer_eat(t);
    }

    dk_token_kind kind = 0;
    dk_loc begin_loc = t->loc;

    // rune: Punctuation token
    if (u8_is_punct(t->peek0)) {
        u8 c = t->peek0;
        dk_tokenizer_eat(t);

        switch (c) {
            case '.': kind = DK_TOKEN_KIND_DOT;         break;
            case ',': kind = DK_TOKEN_KIND_COMMA;       break;
            case '(': kind = DK_TOKEN_KIND_PAREN_OPEN;  break;
            case ')': kind = DK_TOKEN_KIND_PAREN_CLOSE; break;
        }

        str s = substr_len(t->src, begin_loc.pos, t->loc.pos - begin_loc.pos);
        if (kind == 0) {
            dk_report_err(dk_global_err, t->loc, dk_tprint("Invalid operator"));
        }
    }

    // rune: Word/keyword token
    else if (u8_is_letter(t->peek0) || t->peek0 == '_') {
        while (u8_is_letter(t->peek0) || u8_is_digit(t->peek0) || t->peek0 == '_' || t->peek0 == '-' || t->peek0 >= 128) {
            dk_tokenizer_eat(t);
        }

        str s = substr_len(t->src, begin_loc.pos, t->loc.pos - begin_loc.pos);

        static str false_spellings[] = { STR("falsk"), STR("falskt") }; // TODO(rune): Cleanup
        static str true_spellings[] = { STR("sand"), STR("sandt") };    // TODO(rune): Cleanup

        // rune: Bool literal false
        if (kind == 0) {
            for_sarray (str, it, false_spellings) {
                if (str_eq_nocase(s, *it)) {
                    kind = DK_TOKEN_KIND_LITERAL;
                    token->literal = dk_make_literal_bool(false);
                    break;
                }
            }
        }

        // rune: Bool literal true
        if (kind == 0) {
            for_sarray (str, it, true_spellings) {
                if (str_eq_nocase(s, *it)) {
                    kind = DK_TOKEN_KIND_LITERAL;
                    token->literal = dk_make_literal_bool(true);
                    break;
                }
            }
        }

        // rune: Spelled numbers
        if (kind == 0) {
            i64 spelled_value = dk_number_from_str(s);
            if (spelled_value != -1) {
                kind = DK_TOKEN_KIND_LITERAL;
                token->literal = dk_make_literal_int(spelled_value);
            }
        }

        // rune: Then it's just a regular word
        if (kind == 0) {
            kind = DK_TOKEN_KIND_WORD;
        }

        // rune: Continue comment until newline.
        if (str_eq_nocase(s, str("bemærk"))) {
            kind = DK_TOKEN_KIND_COMMENT;
            while (t->peek0 != '\n') {
                dk_tokenizer_eat(t);
            }
        }
    }

    // rune: Number token
    else if (u8_is_digit(t->peek0)) {
        bool got_dec_sep = false;

        while (u8_is_digit(t->peek0) || (t->peek0 == ',' && u8_is_digit(t->peek1))) {
            if (t->peek0 == ',') {
                got_dec_sep = true;
            }
            dk_tokenizer_eat(t);
        }

        str s = substr_len(t->src, begin_loc.pos, t->loc.pos - begin_loc.pos);
        if (got_dec_sep) {
            // rune: Parse float.
            f64 value = 0.0;
            bool fmt_ok = false;
            char temp[256];
            if (s.len + 1 < sizeof(temp)) {
                memcpy(temp, s.v, s.len);
                temp[s.len] = '\0';
                for_n (i64, i, s.len) {
                    if (temp[i] == ',') temp[i] = '.';
                }

                char *end_ptr = temp;
                value = strtod(temp, &end_ptr);

                if (end_ptr == temp + s.len) {
                    fmt_ok = true;
                }
            }

            if (fmt_ok) {
                kind = DK_TOKEN_KIND_LITERAL;
                token->literal = dk_make_literal_float(value);
            } else {
                dk_report_err(dk_global_err, t->loc, str("Invalid floating point literal.")); // TODO(rune): Better error message.
            }
        } else {
            // rune: Parse integer.
            // TODO(rune): Check for overflow
            kind = DK_TOKEN_KIND_LITERAL;
            i64 value = 0;
            for_n (i64, i, s.len) {
                u8 c = s.v[i];
                value *= 10;
                value += c - '0';
            }
            token->literal = dk_make_literal_int(value);
        }
    }

    token->kind    = kind;
    token->text    = substr_len(t->src, begin_loc.pos, t->loc.pos - begin_loc.pos);
    token->loc     = begin_loc;

    return kind != 0;
}

static dk_token_list dk_token_list_from_str(str s, arena *arena) {
    dk_token_list list = { 0 };
    dk_token token = { 0 };

    dk_tokenizer tokenizer = { 0 };
    dk_tokenizer_init(&tokenizer, s, 0);
    while (dk_next_token(&tokenizer, &token)) {
        dk_token *node = arena_push_struct(arena, dk_token);
        *node = token;

        slist_push(&list, node);
        list.count++;
    }

    dk_token *eof_node = arena_push_struct(arena, dk_token);
    eof_node->kind = DK_TOKEN_KIND_EOF;
    eof_node->loc = tokenizer.loc;
    slist_push(&list, eof_node);

    return list;
}

////////////////////////////////////////////////////////////////
// rune: Patterns

static dk_pattern_part *dk_pattern_push_part(dk_pattern *p, arena *arena) {
    dk_pattern_part *part = arena_push_struct(arena, dk_pattern_part);

    slist_push(&p->parts, part);
    p->parts.count++;

    return part;
}

static dk_pattern_part *dk_pattern_push_word(dk_pattern *p, arena *arena, str name) {
    dk_pattern_part *part = dk_pattern_push_part(p, arena);
    part->name = name;
    return part;
}

static dk_pattern_part *dk_pattern_push_type(dk_pattern *p, arena *arena, str name, str type_name) {
    dk_pattern_part *part = dk_pattern_push_part(p, arena);
    part->name = name;
    part->type_name = type_name;
    return part;
}

////////////////////////////////////////////////////////////////
// rune: Parser

// rune: Initialization
static void dk_parser_init(dk_parser *p, dk_token_list tokens, arena *arena) {
    p->arena = arena;
    p->peek = tokens.first;
    if (p->peek && p->peek->kind == DK_TOKEN_KIND_COMMENT) {
        dk_eat_token(p);
    }
}

// rune: Token peek and token consumption

static dk_token *dk_peek_token(dk_parser *p) {
    return p->peek;
}

static bool dk_peek_token_kind(dk_parser *p, dk_token_kind kind) {
    bool ret = p->peek->kind == kind;
    return ret;
}

static bool dk_peek_token_text(dk_parser *p, str text) {
    bool ret = str_eq_nocase(p->peek->text, text);
    return ret;
}

static dk_token *dk_eat_token(dk_parser *p) {
    dk_token *eaten = p->peek;
    do {
        p->peek = p->peek->next;
    } while (p->peek && p->peek->kind == DK_TOKEN_KIND_COMMENT);

    if (p->peek == null) {
        static readonly dk_token null_token = { 0 };
        p->peek = &null_token;
    }

#if DK_DEBUG_PRINT_LEX
    dk_print_token(eaten, 0);
#endif

    return eaten;
}

static dk_token *dk_eat_token_kind(dk_parser *p, dk_token_kind kind) {
    dk_token *eaten = dk_eat_token(p);
    if (eaten->kind != kind) {
        dk_report_err(dk_global_err, eaten->loc, dk_tprint(
            "Unexpected token\n"
            "    Wanted: %\n"
            "    Given:  %\n",
            dk_str_from_token_kind(kind),
            dk_str_from_token_kind(eaten->kind)));
    }
    return eaten;
}

static dk_token *dk_eat_token_text(dk_parser *p, str text) {
    dk_token *eaten = dk_eat_token(p);
    if (!str_eq_nocase(eaten->text, text)) {
        str wanted = text;
        str given = eaten->text;
        if (eaten->kind != DK_TOKEN_KIND_WORD) {
            given = dk_str_from_token_kind(eaten->kind);
        }

        dk_report_err(dk_global_err, eaten->loc, dk_tprint(
            "Unexpected token\n"
            "    Wanted: %\n"
            "    Given:  %\n",
            wanted,
            given));
    }
    return eaten;
}

static bool dk_eat_token_kind_maybe(dk_parser *p, dk_token_kind kind) {
    bool ret = false;
    if (p->peek->kind == kind) {
        dk_eat_token(p);
        ret = true;
    }
    return ret;
}

static bool dk_eat_token_text_maybe(dk_parser *p, str text) {
    bool ret = false;
    if (str_eq_nocase(p->peek->text, text)) {
        dk_eat_token(p);
        ret = true;
    }
    return ret;
}

// rune: Parsing

static dk_clause *dk_parse_clause(dk_parser *p) {
    dk_clause *clause = arena_push_struct(p->arena, dk_clause);
    clause->token = p->peek;
    while (p->peek->kind != 0) {
        dk_clause_part *part = null;
        switch (p->peek->kind) {
            case DK_TOKEN_KIND_WORD: {
                part       = arena_push_struct(p->arena, dk_clause_part);
                part->kind = DK_CLAUSE_PART_KIND_WORD;
                part->word = dk_eat_token(p)->text;
            } break;

            case DK_TOKEN_KIND_LITERAL: {
                part          = arena_push_struct(p->arena, dk_clause_part);
                part->kind    = DK_CLAUSE_PART_KIND_LITERAL;
                part->literal = dk_eat_token(p)->literal;
            } break;

            case DK_TOKEN_KIND_PAREN_OPEN: {
                dk_eat_token_kind(p, DK_TOKEN_KIND_PAREN_OPEN);
                while (p->peek->kind != 0 && p->peek->kind != DK_TOKEN_KIND_PAREN_CLOSE) {
                    part       = arena_push_struct(p->arena, dk_clause_part);
                    part->kind = DK_CLAUSE_PART_KIND_LIST;
                    part->list = dk_parse_clause_list(p);
                }
                dk_eat_token_kind(p, DK_TOKEN_KIND_PAREN_CLOSE);
            } break;
        }

        if (part) {
            slist_push(&clause->parts, part);
        } else {
            break;
        }
    }

    if (clause->parts.first == null) {
        assert(false); // TODO(rune): Better error reporting.
    }

    return clause;
}

static dk_clause_list dk_parse_clause_list(dk_parser *p) {
    dk_clause_list clauses = { 0 };
    while (1) {
        dk_clause *clause = dk_parse_clause(p);
        slist_push(&clauses, clause);

        if (dk_eat_token_kind_maybe(p, DK_TOKEN_KIND_COMMA)) {
            dk_eat_token_text_maybe(p, str("og"));
        } else {
            break;
        }
    }
    return clauses;
}


static dk_stmt *dk_parse_stmt(dk_parser *p) {
    dk_stmt *stmt = arena_push_struct(p->arena, dk_stmt);

    // rune: Declaration statement
    if (dk_peek_token_text(p, str("lad"))) {
        dk_eat_token(p);
        dk_token *name_token = dk_eat_token_kind(p, DK_TOKEN_KIND_WORD);
        dk_eat_token_text(p, str("være"));
        dk_eat_token_text_maybe(p, str("en"));
        dk_eat_token_text_maybe(p, str("et"));
        dk_token *type_token = dk_eat_token_kind(p, DK_TOKEN_KIND_WORD);
        dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);

        stmt->kind      = DK_STMT_KIND_DECL;
        stmt->name      = name_token->text;
        stmt->type_name = type_token->text;
    }

    // rune: If statement
    else if (dk_peek_token_text(p, str("hvis"))) {
        dk_eat_token(p);
        stmt->kind = DK_STMT_KIND_IF;
        stmt->clauses = dk_parse_clause_list(p);

        dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);
        stmt->then = dk_parse_stmt_list(p);

        if (dk_peek_token_text(p, str("ellers"))) {
            dk_eat_token(p);
            dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);

            stmt->else_ = dk_parse_stmt_list(p);
        }
    }

    // TODO(rune): While statement
    else if (dk_peek_token_text(p, str("imens")) || dk_peek_token_text(p, str("imedens"))) {
        dk_eat_token(p);
        stmt->kind = DK_STMT_KIND_WHILE;
        stmt->clauses = dk_parse_clause_list(p);

        dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);
        stmt->then = dk_parse_stmt_list(p);
    }

    // rune: Return statement
    else if (dk_peek_token_text(p, str("tilbagegiv"))) {
        dk_eat_token(p);
        stmt->kind = DK_STMT_KIND_RETURN;
        stmt->clauses = dk_parse_clause_list(p);
        dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);
    }

    // rune: Clause list statement
    else {
        stmt->clauses = dk_parse_clause_list(p);
        stmt->kind = DK_STMT_KIND_EXPR;
        dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);
    }

    return stmt;
}


// TODO(rune): Couldn't this just be part of the normal parsing step?
// TODO(rune): Couldn't this just be part of the normal parsing step?
// TODO(rune): Couldn't this just be part of the normal parsing step?

static void dk_fixup_den_det_clause_list(dk_clause_list *list) {
    dk_clause *head = list->first;
    dk_clause *prev = null;
    dk_clause *curr = list->first;
    while (curr != null) {
        dk_clause_part *part = curr->parts.first;
        while (part != null) {
            switch (part->kind) {
                case DK_CLAUSE_PART_KIND_WORD: {
                    if (str_eq_nocase(part->word, str("det")) ||
                        str_eq_nocase(part->word, str("den"))) {

                        part->kind = DK_CLAUSE_PART_KIND_LIST;
                        part->flags |= DK_CLAUSE_PART_FLAG_DEN_DET;
                        part->list.first = head;
                        part->list.last  = prev ? prev : head;

                        prev->next = null;

                        list->first = curr;

                        head = curr;
                    }
                } break;

                case DK_CLAUSE_PART_KIND_LIST: {
                    dk_fixup_den_det_clause_list(&part->list);
                } break;
            }

            part = part->next;
        }

        prev = curr;
        curr = curr->next;
    }
}

static void dk_fixup_den_det(dk_stmt_list stmts) {
    for_list (dk_stmt, stmt, stmts) {
        dk_fixup_den_det_clause_list(&stmt->clauses);
    }
}

static dk_stmt_list dk_parse_stmt_list(dk_parser *p) {
    static readonly str begin_spelling = STR("goddag");
    static readonly str end_spelling = STR("farvel");

    dk_stmt_list stmts = { 0 };
    dk_eat_token_text(p, begin_spelling);
    dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);
    while (p->peek->kind != 0) {
        if (dk_peek_token_text(p, end_spelling)) {
            break;
        }

        dk_stmt *stmt = dk_parse_stmt(p);
        slist_push(&stmts, stmt);
    }
    dk_eat_token_text(p, end_spelling);
    dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);

    dk_fixup_den_det(stmts);
    return stmts;
}

static dk_func *dk_parse_func(dk_parser *p) {
    dk_func *func = arena_push_struct(p->arena, dk_func);
    func->kind = DK_FUNC_KIND_USER;

    static readonly str return_spelling = STR("tilbagegiver");

    // rune: Pattern
    dk_pattern pattern = { 0 };
    while (p->peek->kind != 0) {
        if (dk_peek_token_text(p, return_spelling)) {
            break;
        }

        if (p->peek->kind == DK_TOKEN_KIND_WORD) {
            dk_token *name_token = dk_eat_token_kind(p, DK_TOKEN_KIND_WORD);

            dk_pattern_push_word(&pattern, p->arena, name_token->text);
        } else if (p->peek->kind == DK_TOKEN_KIND_PAREN_OPEN) {
            dk_eat_token_kind(p, DK_TOKEN_KIND_PAREN_OPEN);
            dk_token *name_token = dk_eat_token_kind(p, DK_TOKEN_KIND_WORD);
            dk_eat_token_text(p, str("som"));
            dk_token *type_token = dk_eat_token_kind(p, DK_TOKEN_KIND_WORD);
            dk_eat_token_kind(p, DK_TOKEN_KIND_PAREN_CLOSE);

            dk_pattern_push_type(&pattern, p->arena, name_token->text, type_token->text);
        } else {
            break;
        }
    }
    func->pattern = pattern;

    // rune: Return type
    dk_eat_token_text(p, return_spelling);
    func->type_name = dk_eat_token_kind(p, DK_TOKEN_KIND_WORD)->text;
    dk_eat_token_kind(p, DK_TOKEN_KIND_DOT);

    // rune: Statements
    func->stmts = dk_parse_stmt_list(p);

    return func;
}

static dk_tree *dk_parse_tree(dk_parser *p) {
    dk_tree *tree = arena_push_struct(p->arena, dk_tree);

    while (p->peek->kind != 0 && dk_global_err->err_list.count == 0) {
        // rune: Visibilty
        dk_eat_token_text(p, str("offentlig")); // TODO(rune): Parse visibilty properly.

        // rune: Function declaration
        if (dk_eat_token_text_maybe(p, str("funktion"))) {
            dk_func *func = dk_parse_func(p);
            slist_push(&tree->funcs, func);
        }

        // rune: Type declaration
        // TODO(rune): Type declaration
    }

#if DK_DEBUG_PRINT_PARSE
    print(ANSI_FG_BRIGHT_MAGENTA);
    print("==== PARSED TREE ====\n");
    dk_print_tree(tree, 0);
    print("\n");
#endif

    return tree;
}

////////////////////////////////////////////////////////////////
// rune: Sematic analysis

static str dk_str_from_pattern(dk_pattern pattern, arena *arena) {
    str_list list = { 0 };
    for_list (dk_pattern_part, part, pattern.parts) {
        if (part->type != null) {
            str_list_push_fmt(&list, arena, "<%>", part->type->name);
        } else if (part->type_name.len > 0) {
            str_list_push_fmt(&list, arena, "<%>", part->type_name);
        } else {
            str_list_push(&list, arena, part->name);
        }
    }
    str s = str_list_concat_sep(&list, arena, str(" "));
    return s;
}

static dk_type *dk_resolve_type(dk_checker *c, str name) {
    dk_type *ret = null;
    for_list (dk_type, type, c->tree->types) {
        if (str_eq_nocase(type->name, name)) {
            ret = type;
            break;
        }
    }

    if (ret == null) {
        // TODO(rune): Better error reporting.
        assert(false && "Unresolved identifier.");
    }

    return ret;
}

static dk_local *dk_resolve_local(dk_checker *c, str name) {
    dk_local *ret = null;
    for_list (dk_local, local, c->locals) {
        if (str_eq_nocase(local->name, name)) {
            ret = local;
            break;
        }
    }

    return ret;
}

static dk_func *dk_resolve_func(dk_checker *c, dk_pattern want_pattern, dk_loc loc) {
    dk_func *ret = null;
    for_list (dk_func, func, c->tree->funcs) {
        if (func->pattern.parts.count == want_pattern.parts.count) {
            dk_pattern_part *want = want_pattern.parts.first;
            dk_pattern_part *cand = func->pattern.parts.first;

            bool match = true;
            for_n (i64, i, want_pattern.parts.count) {
                if (want->type) {
                    if (cand->type != c->builtin_any) {
                        match &= str_eq_nocase(want->type->name, cand->type_name);
                    }
                } else {
                    match &= str_eq_nocase(want->name, cand->name);
                }

                if (match == false) {
                    break;
                }

                want = want->next;
                cand = cand->next;
            }

            if (match) {
                ret = func;
                break;
            }
        }
    }

    if (ret == null) {

        // TODO(rune): Better error reporting.
        str msg =  dk_tprint(
            "Unresolved function\n"
            "   Pattern: %\n",
            dk_str_from_pattern(want_pattern, temp_arena)
        );

        dk_report_err(dk_global_err, loc, msg);

        static readonly dk_type dk_null_type = { 0 };
        static readonly dk_func dk_null_func = { .type = &dk_null_type };
        ret = &dk_null_func;
    }

    return ret;
}

static dk_local *dk_push_local(dk_checker *c, str name, dk_type *type) {
    // TODO(rune): Check for duplicate symbol name.

    dk_local *local = arena_push_struct(c->arena, dk_local);
    local->name = name;
    local->type = type;
    local->off  = c->locals.count;

    slist_push(&c->locals, local);
    c->locals.count++;

    return local;
}

static dk_expr *dk_check_clause(dk_checker *c, dk_clause *clause) {
    // rune: Check arguments
    dk_pattern want_pattern = { 0 };
    dk_expr_list args = { 0 };
    for_list (dk_clause_part, part, clause->parts) {
        dk_expr *arg = null;
        switch (part->kind) {
            case DK_CLAUSE_PART_KIND_WORD: {
                dk_local *local = dk_resolve_local(c, part->word);
                if (local) {
                    arg = arena_push_struct(c->arena, dk_expr);
                    arg->kind = DK_EXPR_KIND_LOCAL;
                    arg->local = local;
                    arg->type = local->type;
                }
            } break;

            case DK_CLAUSE_PART_KIND_LIST: {
                arg = dk_check_clause_list(c, part->list);
            } break;

            case DK_CLAUSE_PART_KIND_LITERAL: {
                arg = arena_push_struct(c->arena, dk_expr);
                arg->kind = DK_EXPR_KIND_LITERAL;
                arg->literal = part->literal;

                switch (part->literal.kind) {
                    case DK_LITERAL_KIND_INT:   arg->type = c->builtin_int;     break;
                    case DK_LITERAL_KIND_FLOAT: arg->type = c->builtin_float;   break;
                    case DK_LITERAL_KIND_BOOL:  arg->type = c->builtin_bool;    break;
                    default:                    assert(false && "Invalid literal kind.");
                }
            } break;

            default: {
                assert(false && "Invalid clause part kind.");
            } break;
        }

        if (arg) {
            assert(arg->type != null);
            slist_push(&args, arg);

            // TODO(rune): Cleanup
            dk_pattern_part *arg_part = dk_pattern_push_part(&want_pattern, c->arena);
            arg_part->type = arg->type;
        } else {
            assert(part->kind == DK_CLAUSE_PART_KIND_WORD);
            dk_pattern_push_word(&want_pattern, c->arena, part->word);
        }
    }

    dk_expr *expr = null;
    if ((clause->parts.first == clause->parts.last) &&
        (args.first == args.last) &&
        (args.first != null)) {
        expr = args.first;
    } else {
        dk_func *func = dk_resolve_func(c, want_pattern, clause->token->loc);
        if (func->kind == DK_FUNC_KIND_ASSIGN) {
            // rune: Assignment e.g. "Gem det i A"
            expr = arena_push_struct(c->arena, dk_expr);
            expr->kind = DK_EXPR_KIND_ASSIGN;
            expr->type = c->builtin_int; // TODO(rune): Void type
            expr->func_args = args;

            // TODO(rune): Check valid lvalue

            dk_expr *rvalue = args.first;
            dk_expr *lvalue = args.last;
            if (lvalue->type != rvalue->type) {
                dk_report_err(dk_global_err, clause->token->loc, dk_tprint(
                    "Type mismtach\n"
                    "    Wanted: %\n"
                    "    Given:  %\n",
                    lvalue->type->name,
                    rvalue->type->name)
                );
            }

        } else {
            // rune: Normal function call
            expr = arena_push_struct(c->arena, dk_expr);
            expr->kind = DK_EXPR_KIND_FUNC;
            expr->func = func;
            expr->type = func->type;
            expr->func_args = args;
        }
    }


    assert(expr->type);
    return expr;
}

static dk_expr *dk_check_clause_list(dk_checker *c, dk_clause_list clauses) {
    dk_expr *expr = null;

    if (clauses.first == clauses.last) {
        expr = dk_check_clause(c, clauses.first);
    } else {
        expr = arena_push_struct(c->arena, dk_expr);
        expr->kind = DK_EXPR_KIND_LIST;
        for_list (dk_clause, subclause, clauses) {
            dk_expr *subexpr = dk_check_clause(c, subclause);
            slist_push(&expr->list, subexpr);
        }
        expr->type = expr->list.last->type;
    }

    assert(expr->type);
    return expr;
}

static void dk_check_stmt(dk_checker *c, dk_stmt *stmt) {
    switch (stmt->kind) {
        case DK_STMT_KIND_DECL: {
            dk_type *type = dk_resolve_type(c, stmt->type_name);
            dk_push_local(c, stmt->name, type);
        } break;

        case DK_STMT_KIND_EXPR:
        case DK_STMT_KIND_IF:
        case DK_STMT_KIND_WHILE:
        case DK_STMT_KIND_RETURN: {
            stmt->expr = dk_check_clause_list(c, stmt->clauses);;
            dk_check_stmt_list(c, stmt->then);
            dk_check_stmt_list(c, stmt->else_);
        } break;

        default: {
            assert(false && "Invalid stmt kind.");
        } break;
    }
}

static void dk_check_stmt_list(dk_checker *c, dk_stmt_list stmts) {
    for_list (dk_stmt, stmt, stmts) {
        dk_check_stmt(c, stmt);
    }
}

static void dk_check_func_sig(dk_checker *c, dk_func *func) {
    // rune: Symbol id
    if (func->kind == DK_FUNC_KIND_USER) {
        func->symbol_id = ++c->symbol_id_counter;
    }

    // rune: Return type
    func->type = dk_resolve_type(c, func->type_name);

    // rune: Arguments
    for_list (dk_pattern_part, part, func->pattern.parts) {
        if (part->type_name.len > 0) {
            part->type = dk_resolve_type(c, part->type_name);
        }
    }
}

static void dk_check_func_body(dk_checker *c, dk_func *func) {
    // TODO(rune): Cleanup
    dk_local_list restore_locals = c->locals;
    c->locals.first = null;
    c->locals.last  = null;

    assert(func->type); // NOTE(rune): Shuold've already been checked with dk_check_func_sig().

    // rune: Arguments
    for_list (dk_pattern_part, part, func->pattern.parts) {
        if (part->type_name.len > 0) {
            dk_local *local = dk_push_local(c, part->name, part->type);
            local->flags |= DK_LOCAL_FLAG_ARG;
        }
    }

    // rune: Body
    dk_check_stmt_list(c, func->stmts);

    // TODO(rune): Cleanup
    func->locals = c->locals;
    c->locals = restore_locals;
}

static dk_pattern dk_pattern_from_str(str s, arena *arena) {
    dk_pattern pattern = { 0 };
    str_list words = str_split_by_whitespace(s, arena);
    for_list (str_node, node, words) {
        dk_pattern_part *part = dk_pattern_push_part(&pattern, arena);

        i64 idx = str_idx_of_u8(node->v, ':');
        if (idx == -1) {
            part->name = node->v;
        } else {
            part->name      = substr_len(node->v, 0, idx);
            part->type_name = substr_idx(node->v, idx + 1);
        }
    }
    return pattern;
}

static dk_func *dk_func_make_opcode(dk_bc_opcode opcode, str pattern, str return_type_name, arena *arena) {
    dk_func *func   = arena_push_struct(arena, dk_func);
    func->type_name = return_type_name;
    func->pattern   = dk_pattern_from_str(pattern, arena);
    func->kind      = DK_FUNC_KIND_OPCODE;
    func->opcode    = opcode;
    return func;
}

static void dk_check_tree(dk_tree *tree, arena *arena) {
    dk_checker c = { 0 };
    c.tree = tree;
    c.arena = arena;

    // rune: Builtin types
    c.builtin_int          = arena_push_struct(c.arena, dk_type);
    c.builtin_int->name    = str("heltal");
    c.builtin_int->size    = 8;
    slist_push(&tree->types, c.builtin_int);

    c.builtin_float        = arena_push_struct(c.arena, dk_type);
    c.builtin_float->name  = str("flyder");
    c.builtin_float->size  = 8;
    slist_push(&tree->types, c.builtin_float);

    c.builtin_bool         = arena_push_struct(c.arena, dk_type);
    c.builtin_bool->name   = str("påstand"); // TODO(rune): Cleanup string contants.
    c.builtin_bool->size   = 1;
    slist_push(&tree->types, c.builtin_bool);

    // NOTE(rune): Special type used be builtin expressions such as "Gem det i A", where "det" and "A" can be any types.
    c.builtin_any         = arena_push_struct(c.arena, dk_type);
    c.builtin_any->name   = str("$any");
    c.builtin_any->size   = 1;
    slist_push(&tree->types, c.builtin_any);

    // rune: Opcode instrinsics

    {
        typedef struct dk_intrinsic dk_intrinsic;
        struct dk_intrinsic {
            dk_bc_opcode opcode;
            str pattern;
            str return_type;
        };

        static readonly dk_intrinsic intrinsics[] = {
            // rune: Arithmetic
            { DK_BC_OPCODE_ADD,  STR("læg A:heltal sammen med B:heltal"),   STR("heltal")  },
            { DK_BC_OPCODE_FADD, STR("læg A:flyder sammen med B:flyder"),   STR("flyder")  },
            { DK_BC_OPCODE_SUB,  STR("træk A:heltal fra B:heltal"),         STR("heltal")  },
            { DK_BC_OPCODE_FSUB, STR("træk A:flyder fra B:flyder"),         STR("flyder")  },
            { DK_BC_OPCODE_IMUL, STR("gang A:heltal med B:heltal"),         STR("heltal")  },
            { DK_BC_OPCODE_FMUL, STR("gang A:flyder med B:flyder"),         STR("flyder")  },
            { DK_BC_OPCODE_IDIV, STR("del A:heltal med B:heltal"),          STR("heltal")  },
            { DK_BC_OPCODE_FDIV, STR("del A:flyder med B:flyder"),          STR("flyder")  },

            // rune: Comparison
            { DK_BC_OPCODE_LT,   STR("A:heltal er mindre end B:heltal"),    STR("påstand") },
            { DK_BC_OPCODE_FLT,  STR("A:flyder er mindre end B:flyder"),    STR("påstand") },
            { DK_BC_OPCODE_GT,   STR("A:heltal er større end B:heltal"),    STR("påstand") },
            { DK_BC_OPCODE_FGT,  STR("A:flyder er større end B:flyder"),    STR("påstand") },
            { DK_BC_OPCODE_EQ,   STR("A:heltal er lig med B:heltal"),       STR("påstand") },
            { DK_BC_OPCODE_FEQ,  STR("A:flyder er lig med B:flyder"),       STR("påstand") },

            // rune: Boolean
            { DK_BC_OPCODE_OR,   STR("enten A:påstand eller B:påstand"),    STR("påstand") },
            { DK_BC_OPCODE_AND,  STR("både A:påstand og B:påstand"),        STR("påstand") },
            { DK_BC_OPCODE_NOT,  STR("ikke A:påstand"),                     STR("påstand") },

            // rune: Casts
            { DK_BC_OPCODE_F2I,  STR("støb A:flyder som heltal"),           STR("heltal")  },
            { DK_BC_OPCODE_I2F,  STR("støb A:heltal som flyder"),           STR("flyder")  },
        };

        for_sarray (dk_intrinsic, it, intrinsics) {
            dk_func *func = dk_func_make_opcode(it->opcode, it->pattern, it->return_type, c.arena);
            slist_push(&tree->funcs, func);
        }
    }

    // rune: Special case expressions
    {
        dk_func *func   = arena_push_struct(c.arena, dk_func);
        func->pattern   = dk_pattern_from_str(str("gem src:$any i dst:$any"), c.arena);
        func->type_name = c.builtin_int->name;
        func->kind      = DK_FUNC_KIND_ASSIGN;

        slist_push(&tree->funcs, func);
    }

    // rune: Native functions
    {
        dk_func *func   = arena_push_struct(c.arena, dk_func);
        func->pattern   = dk_pattern_from_str(str("print A:heltal"), c.arena);
        func->type_name = c.builtin_int->name;
        func->symbol_id = 0xdeadbeef;
        func->kind      = DK_FUNC_KIND_NATIVE;

        slist_push(&tree->funcs, func);
    }
    {
        dk_func *func   = arena_push_struct(c.arena, dk_func);
        func->pattern   = dk_pattern_from_str(str("print A:flyder"), c.arena);
        func->type_name = c.builtin_int->name;
        func->symbol_id = 0xdeadbeef + 1;
        func->kind      = DK_FUNC_KIND_NATIVE;

        slist_push(&tree->funcs, func);
    }
    {
        dk_func *func   = arena_push_struct(c.arena, dk_func);
        func->pattern   = dk_pattern_from_str(str("print A:påstand"), c.arena);
        func->type_name = c.builtin_int->name;
        func->symbol_id = 0xdeadbeef + 2;
        func->kind      = DK_FUNC_KIND_NATIVE;

        slist_push(&tree->funcs, func);
    }

    // rune: Types
    for_list (dk_type, type, tree->types) {
        if (dk_global_err->err_list.count > 0) break;

        // TODO(rune): Type check types
    }

    // rune: Check functions signatures
    for_list (dk_func, func, tree->funcs) {
        if (dk_global_err->err_list.count > 0) break;
        dk_check_func_sig(&c, func);
    }

    // rune: Check functions statements
    for_list (dk_func, func, tree->funcs) {
        if (dk_global_err->err_list.count > 0) break;
        dk_check_func_body(&c, func);
    }

#if DK_DEBUG_PRINT_CHECK
    print(ANSI_FG_BRIGHT_MAGENTA);
    print("==== CHECKED TREE ====\n");
    dk_print_tree(tree, 0);
    print("\n");
#endif
}

////////////////////////////////////////////////////////////////
// rune: Dynamic buffer

static void *dk_buffer_push(dk_buffer *buffer, i64 size) {
    if (buffer->size + size > buffer->capacity) {
        i64 next_size = buffer->capacity * 2;
        if (next_size == 0) {
            next_size = kilobytes(4);
        }

        buffer->data = heap_realloc(buffer->data, next_size);
        buffer->capacity = next_size;
    }

    assert(buffer->size + size <= buffer->capacity);
    void *ret = buffer->data + buffer->size;
    buffer->size += size;
    return ret;
}

static void *dk_buffer_pop(dk_buffer *buffer, i64 size) {
    assert(size <= buffer->size); // TODO(rune): Error handling.
    buffer->size -= size;
    void *ret = buffer->data + buffer->size;
    return ret;
}

static void *dk_buffer_get(dk_buffer *buffer, i64 pos, i64 size) {
    assert(pos + size <= buffer->size); // TODO(rune): Error handling.
    void *ret = buffer->data + pos;
    return ret;
}

static void *dk_buffer_read(dk_buffer *buffer, i64 *pos, i64 size) {
    assert(*pos + size <= buffer->size); // TODO(rune): Error handling.
    void *ret = buffer->data + *pos;
    *pos += size;
    return ret;
}

static u8 *dk_buffer_push_u8(dk_buffer *buffer, u8 a) { u8 *b = dk_buffer_push(buffer, sizeof(u8)); *b = a; return b; }
static u16 *dk_buffer_push_u16(dk_buffer *buffer, u16 a) { u16 *b = dk_buffer_push(buffer, sizeof(u16)); *b = a; return b; }
static u32 *dk_buffer_push_u32(dk_buffer *buffer, u32 a) { u32 *b = dk_buffer_push(buffer, sizeof(u32)); *b = a; return b; }
static u64 *dk_buffer_push_u64(dk_buffer *buffer, u64 a) { u64 *b = dk_buffer_push(buffer, sizeof(u64)); *b = a; return b; }

static u8 dk_buffer_pop_u8(dk_buffer *buffer) { return *(u8 *)dk_buffer_pop(buffer, sizeof(u8)); }
static u16 dk_buffer_pop_u16(dk_buffer *buffer) { return *(u16 *)dk_buffer_pop(buffer, sizeof(u16)); }
static u32 dk_buffer_pop_u32(dk_buffer *buffer) { return *(u32 *)dk_buffer_pop(buffer, sizeof(u32)); }
static u64 dk_buffer_pop_u64(dk_buffer *buffer) { return *(u64 *)dk_buffer_pop(buffer, sizeof(u64)); }

static u8 *dk_buffer_get_u8(dk_buffer *buffer, i64 pos) { return (u8 *)dk_buffer_get(buffer, pos, sizeof(u8)); }
static u16 *dk_buffer_get_u16(dk_buffer *buffer, i64 pos) { return (u16 *)dk_buffer_get(buffer, pos, sizeof(u16)); }
static u32 *dk_buffer_get_u32(dk_buffer *buffer, i64 pos) { return (u32 *)dk_buffer_get(buffer, pos, sizeof(u32)); }
static u64 *dk_buffer_get_u64(dk_buffer *buffer, i64 pos) { return (u64 *)dk_buffer_get(buffer, pos, sizeof(u64)); }

static u8 dk_buffer_read_u8(dk_buffer *buffer, i64 *pos) { return *(u8 *)dk_buffer_read(buffer, pos, sizeof(u8)); }
static u16 dk_buffer_read_u16(dk_buffer *buffer, i64 *pos) { return *(u16 *)dk_buffer_read(buffer, pos, sizeof(u16)); }
static u32 dk_buffer_read_u32(dk_buffer *buffer, i64 *pos) { return *(u32 *)dk_buffer_read(buffer, pos, sizeof(u32)); }
static u64 dk_buffer_read_u64(dk_buffer *buffer, i64 *pos) { return *(u64 *)dk_buffer_read(buffer, pos, sizeof(u64)); }

////////////////////////////////////////////////////////////////
// rune: Emit

static u8 *dk_emit_u8(dk_emitter *e, u8 a) { return dk_buffer_push_u8(&e->body, a); }
static u16 *dk_emit_u16(dk_emitter *e, u16 a) { return dk_buffer_push_u16(&e->body, a); }
static u32 *dk_emit_u32(dk_emitter *e, u32 a) { return dk_buffer_push_u32(&e->body, a); }
static u64 *dk_emit_u64(dk_emitter *e, u64 a) { return dk_buffer_push_u64(&e->body, a); }

static void dk_emit_inst1(dk_emitter *e, dk_bc_opcode opcode) {
    assert(dk_bc_opcode_infos[opcode].operand_kind == DK_BC_OPERAND_KIND_NONE);

    dk_bc_inst_prefix prefix = { .opcode = opcode };
    dk_emit_u8(e, prefix.u8);
}

static void dk_emit_inst2(dk_emitter *e, dk_bc_opcode opcode, u64 operand) {
    assert(dk_bc_opcode_infos[opcode].operand_kind != DK_BC_OPERAND_KIND_NONE);

    dk_bc_inst_prefix prefix = { .opcode = opcode };
    if (operand <= U8_MAX) {
        prefix.operand_size = 0;
        dk_emit_u8(e, prefix.u8);
        dk_emit_u8(e, (u8)operand);
    } else if (operand <= U16_MAX) {
        prefix.operand_size = 1;
        dk_emit_u8(e, prefix.u8);
        dk_emit_u16(e, (u16)operand);
    } else if (operand <= U32_MAX) {
        prefix.operand_size = 2;
        dk_emit_u8(e, prefix.u8);
        dk_emit_u32(e, (u32)operand);
    } else {
        prefix.operand_size = 3;
        dk_emit_u8(e, prefix.u8);
        dk_emit_u64(e, (u64)operand);
    }
}

static void dk_emit_symbol(dk_emitter *e, u32 id, u32 size) {
    dk_bc_symbol *symbol = dk_buffer_push_struct(&e->head, dk_bc_symbol);
    symbol->id   = id;
    symbol->size = size;
    symbol->pos  = e->body.size;
}

static void dk_emit_literal(dk_emitter *e, dk_literal literal) {
    switch (literal.kind) {
        case DK_LITERAL_KIND_INT:   dk_emit_inst2(e, DK_BC_OPCODE_LDI, u64(literal.int_));             break;
        case DK_LITERAL_KIND_FLOAT: dk_emit_inst2(e, DK_BC_OPCODE_LDI, u64_from_f64(literal.float_));    break;
        case DK_LITERAL_KIND_BOOL:  dk_emit_inst2(e, DK_BC_OPCODE_LDI, u64(literal.bool_));            break;
        default:                    assert(false && "Not implemented.");                               break;
    }
}

static void dk_emit_expr(dk_emitter *e, dk_expr *expr) {
    switch (expr->kind) {
        case DK_EXPR_KIND_LIST: {
            for_list (dk_expr, subexpr, expr->list) {
                dk_emit_expr(e, subexpr);
                if (subexpr != expr->list.last && subexpr->type->size > 0) {
                    dk_emit_inst1(e, DK_BC_OPCODE_POP);
                }
            }
        } break;

        case DK_EXPR_KIND_LITERAL: {
            dk_emit_literal(e, expr->literal);
        } break;

        case DK_EXPR_KIND_LOCAL: {
            dk_emit_inst2(e, DK_BC_OPCODE_LDL, expr->local->off);
        } break;

        case DK_EXPR_KIND_FUNC: {
            for_list (dk_expr, arg, expr->func_args) {
                dk_emit_expr(e, arg);
            }

            dk_func *func = expr->func;
            if (func->kind == DK_FUNC_KIND_OPCODE) {
                assert(dk_bc_opcode_infos[func->opcode].operand_kind == DK_BC_OPERAND_KIND_NONE);
                dk_emit_inst1(e, func->opcode);
            } else {
                dk_emit_inst2(e, DK_BC_OPCODE_CALL, func->symbol_id);
            }
        } break;

        case DK_EXPR_KIND_ASSIGN: {
            dk_expr *rvalue = expr->func_args.first;
            dk_expr *lvalue = expr->func_args.last;

            assert(lvalue->kind == DK_EXPR_KIND_LOCAL); // TODO(rune): Better lvalue handling

            dk_emit_expr(e, rvalue);
            dk_emit_inst1(e, DK_BC_OPCODE_DUP); // TODO(rune): When we have void type, we don't need this dup instruction.
            dk_emit_inst2(e, DK_BC_OPCODE_STL, lvalue->local->off);
        } break;

        default: {
            assert(false && "Invalid expr kind.");
        } break;
    }
}

static void dk_emit_stmt_list(dk_emitter *e, dk_stmt_list stmts) {
    for_list (dk_stmt, stmt, stmts) {
        switch (stmt->kind) {
            case DK_STMT_KIND_DECL: {
                // Nothing
                // TODO(rune): Initialization expression
            } break;

            case DK_STMT_KIND_EXPR: {
                dk_emit_expr(e, stmt->expr);

                if (stmt->expr->type->size > 0) {
                    dk_emit_inst1(e, DK_BC_OPCODE_POP);
                }
            } break;

            case DK_STMT_KIND_RETURN: {
                dk_emit_expr(e, stmt->expr);
                dk_emit_inst1(e, DK_BC_OPCODE_RET);
            } break;

            case DK_STMT_KIND_IF: {
                dk_emit_expr(e, stmt->expr);
                dk_emit_inst1(e, DK_BC_OPCODE_NOT);

                // TODO(rune): This whole business is a hack.
                // TODO(rune): This whole business is a hack.
                // TODO(rune): This whole business is a hack.

                dk_bc_inst_prefix prefix = {
                    .opcode = DK_BC_OPCODE_BR,
                    .operand_size = 3, // NOTE(rune): 64-bits
                };
                dk_emit_u8(e, prefix.u8);
                u64 *else_pos = dk_emit_u64(e, U64_MAX);

                dk_emit_stmt_list(e, stmt->then);

                dk_emit_inst2(e, DK_BC_OPCODE_LDI, 1);
                dk_bc_inst_prefix prefix2 = {
                    .opcode = DK_BC_OPCODE_BR,
                    .operand_size = 3, // NOTE(rune): 64-bits
                };
                dk_emit_u8(e, prefix2.u8);

                u64 *end_pos = dk_emit_u64(e, U64_MAX);

                *else_pos = e->body.size;

                dk_emit_stmt_list(e, stmt->else_);

                *end_pos = e->body.size;
            } break;

            case DK_STMT_KIND_WHILE: {
                i64 start_pos = e->body.size;
                dk_emit_expr(e, stmt->expr);
                dk_emit_inst1(e, DK_BC_OPCODE_NOT);

                // TODO(rune): This whole business is a hack.
                // TODO(rune): This whole business is a hack.
                // TODO(rune): This whole business is a hack.

                dk_bc_inst_prefix prefix = {
                    .opcode = DK_BC_OPCODE_BR,
                    .operand_size = 3, // NOTE(rune): 64-bits
                };

                dk_emit_u8(e, prefix.u8);
                u64 *end_pos = dk_emit_u64(e, U64_MAX);

                dk_emit_stmt_list(e, stmt->then);

                dk_emit_inst2(e, DK_BC_OPCODE_LDI, 1); // TODO(rune): Unconditional jump opcode.
                dk_emit_inst2(e, DK_BC_OPCODE_BR, start_pos);

                *end_pos = e->body.size;
            } break;

            default: {
                assert(false && "Invalid stmt kind.");
            } break;
        }
    }
}

static void dk_emit_tree(dk_emitter *e, dk_tree *tree) {
    for_list (dk_func, func, tree->funcs) {
        if (dk_global_err->err_list.count > 0) break;

        if (func->kind == DK_FUNC_KIND_USER) {
            dk_emit_symbol(e, func->symbol_id, 0);

            // rune: Prelude
            for_list (dk_local, local, func->locals) {
                if (local->flags & DK_LOCAL_FLAG_ARG) {
                    dk_emit_inst2(e, DK_BC_OPCODE_STL, local->off);
                }
            }

            // rune: Function body
            dk_emit_stmt_list(e, func->stmts);

            // rune: Epilogue
            if (func->type->size > 0) {
                dk_emit_inst2(e, DK_BC_OPCODE_LDI, 0);
            }
            dk_emit_inst1(e, DK_BC_OPCODE_RET);
        }
    }

#if DK_DEBUG_PRINT_EMIT
    print(ANSI_FG_BRIGHT_MAGENTA);
    print("==== EMIT ====\n");
    dk_print_program((dk_program) { e->head, e->body });
    print("\n");
#endif
}


static dk_program dk_program_from_tree(dk_tree *tree) {
    dk_emitter e = { 0 };
    dk_emit_tree(&e, tree);

    dk_program program = { 0 };
    program.head = e.head;
    program.body = e.body;
    return program;
}

////////////////////////////////////////////////////////////////
//
//
// Runtime
//
//
////////////////////////////////////////////////////////////////

static str dk_run_program(dk_program program, arena *output_arena) {
    typedef struct dk_call_frame dk_call_frame;
    struct dk_call_frame {
        i64 loc_base;
        i64 loc_size;
        i64 return_pos;
        dk_call_frame *prev;
    };

    i64 ip = 0;

    str_list output_list = { 0 };

    dk_buffer *head = &program.head;
    dk_buffer *body = &program.body;

    dk_bc_symbol *symbols = (dk_bc_symbol *)head->data;
    i64 symbol_count      = head->size / sizeof(dk_bc_symbol);

    dk_buffer data_stack = { 0 };
    dk_buffer call_stack = { 0 };

    // rune: Setup initial call frame.
    i64 hardcoded_local_size = 64; // TODO(rune): Encode number of locals and arguments.
    dk_call_frame *frame = dk_buffer_push_struct(&call_stack, dk_call_frame);
    frame->return_pos = -1;
    frame->loc_base = call_stack.size;
    frame->loc_size = hardcoded_local_size;

    dk_buffer_push(&call_stack, hardcoded_local_size);

    while (1) {
        dk_bc_inst_prefix prefix = *dk_buffer_read_struct(body, &ip, dk_bc_inst_prefix);
        dk_bc_opcode opcode      = prefix.opcode;
        u64 operand              = 0;

        if (dk_bc_opcode_infos[prefix.opcode].operand_kind != DK_BC_OPERAND_KIND_NONE) {
            switch (prefix.operand_size) {
                case 0: operand = (u64)dk_buffer_read_u8(body, &ip); break;
                case 1: operand = (u64)dk_buffer_read_u16(body, &ip); break;
                case 2: operand = (u64)dk_buffer_read_u32(body, &ip); break;
                case 3: operand = (u64)dk_buffer_read_u64(body, &ip); break;
                default: {
                    assert(false); // TODO(rune): Better error runtime error reporting.
                } break;
            }
        }

        assert(opcode < DK_BC_OPCODE_COUNT); // TODO(rune): Better error runtime error reporting.

        switch (prefix.opcode) {
            case DK_BC_OPCODE_NOP: { } break;

            case DK_BC_OPCODE_LDI: {
                dk_buffer_push_u64(&data_stack, operand);
            } break;

            case DK_BC_OPCODE_LDL: {
                u64 loc_val = *dk_buffer_get_u64(&call_stack, frame->loc_base + operand * 8); // TODO(rune): Properly sized locals (no more *8)
                dk_buffer_push_u64(&data_stack, loc_val);
            } break;

            case DK_BC_OPCODE_POP: {
                dk_buffer_pop_u64(&data_stack);
            } break;

            case DK_BC_OPCODE_STL: {
                u64 val = dk_buffer_pop_u64(&data_stack);
                *dk_buffer_get_u64(&call_stack, frame->loc_base + operand * 8) = val; // TODO(rune): Properly sized locals (no more *8)
            } break;

            case DK_BC_OPCODE_DUP: {
                u64 val = dk_buffer_pop_u64(&data_stack);
                dk_buffer_push_u64(&data_stack, val);
                dk_buffer_push_u64(&data_stack, val);
            } break;

#define DK_BC_BINOP_IMPL(calc)                          \
            do {                                        \
                u64 b = dk_buffer_pop_u64(&data_stack); \
                u64 a = dk_buffer_pop_u64(&data_stack); \
                u64 c = calc;                           \
                dk_buffer_push_u64(&data_stack, c);     \
            } while (0)

            case DK_BC_OPCODE_ADD:  DK_BC_BINOP_IMPL(a + b); break;
            case DK_BC_OPCODE_SUB:  DK_BC_BINOP_IMPL(a - b); break;
            case DK_BC_OPCODE_UMUL: DK_BC_BINOP_IMPL(a * b); break;
            case DK_BC_OPCODE_UDIV: DK_BC_BINOP_IMPL(a / b); break;

            case DK_BC_OPCODE_IMUL: DK_BC_BINOP_IMPL(u64(i64(a) * i64(b))); break;
            case DK_BC_OPCODE_IDIV: DK_BC_BINOP_IMPL(u64(i64(a) / i64(b))); break;

            case DK_BC_OPCODE_FADD: DK_BC_BINOP_IMPL(u64_from_f64(f64_from_u64(a) + f64_from_u64(b))); break;
            case DK_BC_OPCODE_FSUB: DK_BC_BINOP_IMPL(u64_from_f64(f64_from_u64(a) - f64_from_u64(b))); break;
            case DK_BC_OPCODE_FMUL: DK_BC_BINOP_IMPL(u64_from_f64(f64_from_u64(a) * f64_from_u64(b))); break;
            case DK_BC_OPCODE_FDIV: DK_BC_BINOP_IMPL(u64_from_f64(f64_from_u64(a) / f64_from_u64(b))); break;

            case DK_BC_OPCODE_AND:  DK_BC_BINOP_IMPL(a && b); break;
            case DK_BC_OPCODE_OR:   DK_BC_BINOP_IMPL(a || b); break;

            case DK_BC_OPCODE_EQ:   DK_BC_BINOP_IMPL(i64(a) == i64(b)); break;
            case DK_BC_OPCODE_LT:   DK_BC_BINOP_IMPL(i64(a) < i64(b));  break;
            case DK_BC_OPCODE_GT:   DK_BC_BINOP_IMPL(i64(a) > i64(b));  break;

            case DK_BC_OPCODE_FEQ: DK_BC_BINOP_IMPL(u64_from_f64(f64_from_u64(a) == f64_from_u64(b))); break;
            case DK_BC_OPCODE_FLT: DK_BC_BINOP_IMPL(u64_from_f64(f64_from_u64(a) < f64_from_u64(b))); break;
            case DK_BC_OPCODE_FGT: DK_BC_BINOP_IMPL(u64_from_f64(f64_from_u64(a) > f64_from_u64(b))); break;

#undef DK_BC_BINOP_IMPL

            case DK_BC_OPCODE_NOT: {
                u64 a = dk_buffer_pop_u64(&data_stack);
                u64 c = !a;
                dk_buffer_push_u64(&data_stack, c);
            } break;

            case DK_BC_OPCODE_CALL: {
                u32 id = (u32)operand;

                // TODO(rune): Better system for built-in procs.
                if (id == 0xdeadbeef) {
                    u64 a = dk_buffer_pop_u64(&data_stack);
                    str_list_push_fmt(&output_list, output_arena, "%\n", a);
                    dk_buffer_push_u64(&data_stack, 0); // TODO(rune): What should print return?
                } else if (id == 0xdeadbeef + 1) {
                    u64 a = dk_buffer_pop_u64(&data_stack);
                    str_list_push_fmt(&output_list, output_arena, "%\n", f64_from_u64(a));
                    dk_buffer_push_u64(&data_stack, 0); // TODO(rune): What should print return?
                } else if (id == 0xdeadbeef + 2) {
                    u64 a = dk_buffer_pop_u64(&data_stack);
                    str_list_push_fmt(&output_list, output_arena, "%\n", a ? "sand" : "falsk");
                    dk_buffer_push_u64(&data_stack, 0); // TODO(rune): What should print return?
                } else {
                    // TODO(rune): Better symbol lookup.
                    dk_bc_symbol *symbol = null;
                    for_n (i64, i, symbol_count) {
                        if (symbols[i].id == id) {
                            symbol = &symbols[i];
                        }
                    }
                    assert(symbol != null); // TODO(rune): Better error runtime error reporting.

                    dk_call_frame *next_frame = dk_buffer_push_struct(&call_stack, dk_call_frame);
                    next_frame->prev  = frame;
                    frame             = next_frame;
                    frame->loc_base   = call_stack.size;
                    frame->loc_size   = hardcoded_local_size;
                    frame->return_pos = ip;

                    ip = symbol->pos;

                    dk_buffer_push(&call_stack, frame->loc_size);
                }

            } break;

            case DK_BC_OPCODE_RET: {
                if (frame->return_pos == -1) {
                    goto exit;
                }

                ip = frame->return_pos;;
                dk_buffer_pop(&call_stack, frame->loc_size);
                dk_buffer_pop_struct(&call_stack, dk_call_frame);
                frame = frame->prev;

                assert(frame != null);
            } break;

            case DK_BC_OPCODE_BR: {
                u64 condition = dk_buffer_pop_u64(&data_stack);
                if (condition) {
                    ip = operand;
                }
            } break;

            case DK_BC_OPCODE_I2F: {
                u64 val  = dk_buffer_pop_u64(&data_stack);
                u64 cast = u64_from_f64(f64(i64(val)));
                dk_buffer_push_u64(&data_stack, cast);
            } break;

            case DK_BC_OPCODE_F2I: {
                u64 val = dk_buffer_pop_u64(&data_stack);
                u64 cast = i64(f64_from_u64(val));
                dk_buffer_push_u64(&data_stack, cast);
            } break;

            default: {
                assert(false && "Invalid instruction.");
            } break;
        }
    }

exit:
    if (data_stack.size != 8) {
        str_list_push(&output_list, output_arena, dk_tprint("Invalid stack size on exit. Was % but expected %.", data_stack.size, 8));
    }

    str output = str_list_concat(&output_list, output_arena);
    return output;
}

////////////////////////////////////////////////////////////////
// rune: High level api

static dk_tree *dk_tree_from_token_list(dk_token_list tokens, arena *arena) {
    dk_parser p = { 0 };
    dk_parser_init(&p, tokens, arena);
    dk_tree *tree = dk_parse_tree(&p);
    return tree;
}

static dk_program dk_program_from_str(str s, dk_err_sink *err, arena *arena) {
    dk_global_err = err; // TODO(rune): Remove global.

    dk_token_list tokens = dk_token_list_from_str(s, arena);
    dk_tree *tree = dk_tree_from_token_list(tokens, arena);
    dk_check_tree(tree, arena);
    dk_program program = dk_program_from_tree(tree);
    return program;
}

////////////////////////////////////////////////////////////////
// rune: Debug print

static void dk_print_level(i64 level) {
    static readonly str colors[] = {
        STR(ANSI_FG_BLUE),
        STR(ANSI_FG_MAGENTA),
        STR(ANSI_FG_RED),
        STR(ANSI_FG_YELLOW),
        STR(ANSI_FG_GREEN),
        STR(ANSI_FG_CYAN),
    };

    print(ANSI_FAINT);
    for_n (i64, i, level) {
        print(colors[i % countof(colors)]);

        if (i + 1 < level) {
            print("│ ");
        } else {
            print("├─");
        }
    }
    print(ANSI_RESET);
}

static void dk_print_literal(dk_literal literal, i64 level) {
    dk_print_level(level);
    switch (literal.kind) {
        case DK_LITERAL_KIND_INT:   println("literal/int %", literal.int_);     break;
        case DK_LITERAL_KIND_FLOAT: println("literal/float %", literal.float_); break;
        case DK_LITERAL_KIND_BOOL:  println("literal/bool %", literal.bool_);   break;
        default:                    assert(false && "Invalid literal kind");    break;
    }
}

static void dk_print_token(dk_token *token, i64 level) {
    dk_print_level(level);
    // TODO(rune): Print filename
    print("%:%\t", token->loc.row + 1, token->loc.col + 1);
    print(dk_str_from_token_kind(token->kind));
    print("\t");
    print(token->text);
    print("\n");
}

static void dk_print_clause_part(dk_clause_part *part, i64 level) {
    dk_print_level(level);
    switch (part->kind) {
        case DK_CLAUSE_PART_KIND_WORD: {
            println("clause-part/word %(literal)", part->word);
        } break;

        case DK_CLAUSE_PART_KIND_LIST: {
            print("clause-part/list");
            if (part->flags & DK_CLAUSE_PART_FLAG_DEN_DET) print(ANSI_FG_GRAY " (den/det)" ANSI_FG_DEFAULT);
            print("\n");
            dk_print_clause_list(part->list, level + 1);
        } break;

        case DK_CLAUSE_PART_KIND_LITERAL: {
            println("clause-part/literal");
            dk_print_literal(part->literal, level + 1);
        } break;

        default: {
            assert(false && "Invalid clause part kind.");
        } break;
    }
}

static void dk_print_clause_part_list(dk_clause_part_list parts, i64 level) {
    for_list (dk_clause_part, part, parts) {
        dk_print_clause_part(part, level);
    }
}

static void dk_print_clause(dk_clause *clause, i64 level) {
    dk_print_level(level);
    println("clause");
    dk_print_clause_part_list(clause->parts, level + 1);
}

static void dk_print_clause_list(dk_clause_list clauses, i64 level) {
    for_list (dk_clause, clause, clauses) {
        dk_print_clause(clause, level);
    }
}

static void dk_print_stmt(dk_stmt *stmt, i64 level) {
    dk_print_level(level);
    switch (stmt->kind) {
        case DK_STMT_KIND_DECL: {
            println("stmt/decl %(literal)    %(literal)", stmt->name, stmt->type_name);
        } break;

        case DK_STMT_KIND_EXPR: {
            println("stmt/expr");
            if (stmt->expr) {
                dk_print_expr(stmt->expr, level + 1);
            } else {
                dk_print_clause_list(stmt->clauses, level + 1);
            }
        } break;

        case DK_STMT_KIND_RETURN: {
            println("stmt/return");
            if (stmt->expr) {
                dk_print_expr(stmt->expr, level + 1);
            } else {
                dk_print_clause_list(stmt->clauses, level + 1);
            }
        } break;

        case DK_STMT_KIND_WHILE:
        case DK_STMT_KIND_IF: {
            bool is_if = stmt->kind == DK_STMT_KIND_IF;
            println(is_if ? "stmt/if" : "stmt/while");
            if (stmt->expr) {
                dk_print_expr(stmt->expr, level + 1);
            } else {
                dk_print_clause_list(stmt->clauses, level + 1);
            }

            dk_print_level(level + 1);
            println("then");
            dk_print_stmt_list(stmt->then, level + 2);

            if (is_if) {
                dk_print_level(level + 1);
                println("then");
                dk_print_stmt_list(stmt->then, level + 2);
            }
        } break;

        default: {
            assert(false && "Invalid stmt kind.");
        } break;
    }
}

static void dk_print_stmt_list(dk_stmt_list stmts, i64 level) {
    for_list (dk_stmt, stmt, stmts) {
        dk_print_stmt(stmt, level);
    }
}

static void dk_print_pattern_part(dk_pattern_part *part, i64 level) {
    dk_print_level(level);
    println("pattern-part %(literal)    %(literal)", part->name, part->type_name);
}

static void dk_print_pattern(dk_pattern pattern, i64 level) {
    dk_print_level(level);
    println("pattern");
    for_list (dk_pattern_part, part, pattern.parts) {
        dk_print_pattern_part(part, level + 1);
    }
}

static void dk_print_func(dk_func *func, i64 level) {
    dk_print_level(level);
    println("func returns %", func->type_name);

    dk_print_pattern(func->pattern, level + 1);
    dk_print_stmt_list(func->stmts, level + 1);
}

static void dk_print_func_list(dk_func_list funcs, i64 level) {
    for_list (dk_func, func, funcs) {
        dk_print_func(func, level);
    }
}

static void dk_print_type(dk_type *type, i64 level) {
    dk_print_level(level);
    println("type % size %", type->name, type->size);
}

static void dk_print_type_list(dk_type_list types, i64 level) {
    for_list (dk_type, type, types) {
        dk_print_type(type, level);
    }
}

static void dk_print_expr(dk_expr *expr, i64 level) {
    dk_print_level(level);
    switch (expr->kind) {
        case DK_EXPR_KIND_FUNC: {
            println("expr/func type %(literal)", expr->type->name);
            dk_print_expr_list(expr->func_args, level + 1);
        } break;

        case DK_EXPR_KIND_LITERAL: {
            println("expr/literal type %(literal)", expr->type->name);
            dk_print_literal(expr->literal, level + 1);
        } break;

        case DK_EXPR_KIND_LOCAL: {
            println("expr/local type %(literal)", expr->type->name);
            dk_print_local(expr->local, level + 1);
        } break;

        case DK_EXPR_KIND_LIST: {
            println("expr/list");
            dk_print_expr_list(expr->list, level + 1);
        } break;

        case DK_EXPR_KIND_ASSIGN: {
            println("expr/assign");
            dk_print_expr_list(expr->func_args, level + 1);
        } break;

        default: {
            assert(false && "Invalid expr kind.");
        } break;
    }
}

static void dk_print_expr_list(dk_expr_list exprs, i64 level) {
    for_list (dk_expr, expr, exprs) {
        dk_print_expr(expr, level);
    }
}

static void dk_print_tree(dk_tree *tree, i64 level) {
    dk_print_level(level);
    println("tree");
    dk_print_type_list(tree->types, level + 1);
    dk_print_func_list(tree->funcs, level + 1);
}

static void dk_print_local(dk_local *local, i64 level) {
    dk_print_level(level);
    println("local %(literal) type %(literal) offset %(literal)", local->name, local->type->name, local->off);
}

static void dk_print_local_list(dk_local_list locals, i64 level) {
    for_list (dk_local, local, locals) {
        dk_print_local(local, level);
    }
}

static void dk_print_program(dk_program program) {
    dk_buffer *head = &program.head;
    dk_buffer *body = &program.body;

    // rune: Head
    {
        i64 read_pos = 0;
        while (read_pos < head->size) {
            dk_bc_symbol *symbol = dk_buffer_read_struct(head, &read_pos, dk_bc_symbol);
            print(ANSI_FG_CYAN "%(hexpad)" ANSI_FG_DEFAULT " %(hexpad)" ANSI_FG_GRAY " %(hexpad)\n", symbol->id, symbol->size, symbol->pos);
        }
    }

    // rune: Body
    {
        i64 read_pos = 0;
        while (read_pos < body->size) {
            print(ANSI_FG_GRAY);
            print("%(hexpad)", read_pos);

            dk_bc_inst_prefix prefix       = *dk_buffer_read_struct(body, &read_pos, dk_bc_inst_prefix);
            dk_bc_opcode_info *opcode_info = &dk_bc_opcode_infos[prefix.opcode];

            print(ANSI_FG_YELLOW);
            print(" %\t", opcode_info->name);

            any operand = { 0 };
            if (opcode_info->operand_kind != DK_BC_OPERAND_KIND_NONE) {
                switch (prefix.operand_size) {
                    case 0: operand = anyof(dk_buffer_read_u8(body, &read_pos)); break;
                    case 1: operand = anyof(dk_buffer_read_u16(body, &read_pos)); break;
                    case 2: operand = anyof(dk_buffer_read_u32(body, &read_pos)); break;
                    case 3: operand = anyof(dk_buffer_read_u64(body, &read_pos)); break;
                }

                if (opcode_info->operand_kind == DK_BC_OPERAND_KIND_IMM) print(ANSI_FG_MAGENTA "%(hexpad)", operand);
                if (opcode_info->operand_kind == DK_BC_OPERAND_KIND_SYM) print(ANSI_FG_CYAN    "%(hexpad)", operand);
                if (opcode_info->operand_kind == DK_BC_OPERAND_KIND_LOC) print(ANSI_FG_GREEN   "%(hexpad)", operand);
                if (opcode_info->operand_kind == DK_BC_OPERAND_KIND_POS) print(ANSI_FG_GRAY    "%(hexpad)", operand);
            }

            print(ANSI_FG_DEFAULT);
            print("\n");
        }
    }

    print(ANSI_RESET);
}
