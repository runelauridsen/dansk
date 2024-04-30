////////////////////////////////////////////////////////////////
// rune: Char flags

// NOTE(rune): Table generated with:
#if 0
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define countof(a)          (sizeof(a) / sizeof(*(a)))

// NOTE(rune): We want underscore to be a seperate flag, so we can have a combined CHAR_FLAG_WORD similar to regex's \w.
int isunderscore(int c) {
    return c == '_';
}

int main(void) {
    struct {
        int (*func)(int);
        char *name;
    } classes[] = {
        { iscntrl,      "CHAR_FLAG_CONTROL"     },
        { isprint,      "CHAR_FLAG_PRINTABLE"   },
        { isspace,      "CHAR_FLAG_WHITESPACE"  },
        { isupper,      "CHAR_FLAG_UPPER"       },
        { islower,      "CHAR_FLAG_LOWER"       },
        { isalpha,      "CHAR_FLAG_LETTER"      },
        { isdigit,      "CHAR_FLAG_DIGIT"       },
        { isxdigit,     "CHAR_FLAG_HEXDIGIT"    },
        { ispunct,      "CHAR_FLAG_PUNCT"       },
        { isunderscore, "CHAR_FLAG_UNDERSCORE"  },
    };

    char *char_names[] = {
        "NULL (Null character)",
        "SOH (Start of Header)",
        "STX (Start of Text)",
        "ETX (End of Text)",
        "EOT (End of Transmission)",
        "ENQ (Enquiry)",
        "ACK (Acknowledgement)",
        "BEL (Bell)",
        "BS (Backspace)",
        "HT (Horizontal Tab)",
        "LF (Line feed)",
        "VT (Vertical Tab)",
        "FF (Form feed)",
        "CR (Carriage return)",
        "SO (Shift Out)",
        "SI (Shift In)",
        "DLE (Data link escape)",
        "DC1 (Device control 1)",
        "DC2 (Device control 2)",
        "DC3 (Device control 3)",
        "DC4 (Device control 4)",
        "NAK (Negative acknowledgement)",
        "SYN (Synchronous idle)",
        "ETB (End of transmission block)",
        "CAN (Cancel)",
        "EM (End of medium)",
        "SUB (Substitute)",
        "ESC (Escape)",
        "FS (File separator)",
        "GS (Group separator)",
        "RS (Record separator)",
        "US (Unit separator)",
        "(space)",
        "! (exclamation mark)",
        "\" (Quotation mark)",
        "# (Number sign)",
        "$ (Dollar sign)",
        "% (Percent sign)",
        "& (Ampersand)",
        "' (Apostrophe)",
        "( (round brackets or parentheses)",
        ") (round brackets or parentheses)",
        "* (Asterisk)",
        "+ (Plus sign)",
        ", (Comma)",
        "- (Hyphen)",
        ". (Full stop , dot)",
        "/ (Slash)",
        "0 (number zero)",
        "1 (number one)",
        "2 (number two)",
        "3 (number three)",
        "4 (number four)",
        "5 (number five)",
        "6 (number six)",
        "7 (number seven)",
        "8 (number eight)",
        "9 (number nine)",
        ": (Colon)",
        "; (Semicolon)",
        "< (Less-than sign)",
        "= (Equals sign)",
        "> (Greater-than sign; Inequality)",
        "? (Question mark)",
        "@ (At sign)",
        "A (Capital A)",
        "B (Capital B)",
        "C (Capital C)",
        "D (Capital D)",
        "E (Capital E)",
        "F (Capital F)",
        "G (Capital G)",
        "H (Capital H)",
        "I (Capital I)",
        "J (Capital J)",
        "K (Capital K)",
        "L (Capital L)",
        "M (Capital M)",
        "N (Capital N)",
        "O (Capital O)",
        "P (Capital P)",
        "Q (Capital Q)",
        "R (Capital R)",
        "S (Capital S)",
        "T (Capital T)",
        "U (Capital U)",
        "V (Capital V)",
        "W (Capital W)",
        "X (Capital X)",
        "Y (Capital Y)",
        "Z (Capital Z)",
        "[ (square brackets or box brackets)",
        "\\ (Backslash)",
        "] (square brackets or box brackets)",
        "^ (Caret or circumflex accent)",
        "_ (underscore , understrike , underbar or low line)",
        "` (Grave accent)",
        "a (Lowercase  a)",
        "b (Lowercase  b)",
        "c (Lowercase  c)",
        "d (Lowercase  d)",
        "e (Lowercase  e)",
        "f (Lowercase  f)",
        "g (Lowercase  g)",
        "h (Lowercase  h)",
        "i (Lowercase  i)",
        "j (Lowercase  j)",
        "k (Lowercase  k)",
        "l (Lowercase  l)",
        "m (Lowercase  m)",
        "n (Lowercase  n)",
        "o (Lowercase  o)",
        "p (Lowercase  p)",
        "q (Lowercase  q)",
        "r (Lowercase  r)",
        "s (Lowercase  s)",
        "t (Lowercase  t)",
        "u (Lowercase  u)",
        "v (Lowercase  v)",
        "w (Lowercase  w)",
        "x (Lowercase  x)",
        "y (Lowercase  y)",
        "z (Lowercase  z)",
        "{ (curly brackets or braces)",
        "| (vertical-bar, vbar, vertical line or vertical slash)",
        "} (curly brackets or braces)",
        "~ (Tilde; swung dash)",
        "DEL (Delete)",
    };

    for (int i = 0; i < countof(char_names); i++) {
        int num_classes = 0;

        printf("    [%i]", i);

        for (int j = 0; j < countof(classes); j++) {
            // NOTE(rune): See comment on isunderscore.
            if (i == '_' && classes[j].func == ispunct) continue;

            if (classes[j].func(i)) {
                if (num_classes == 0) {
                    printf(" = (u8)%s", classes[j].name);
                } else {
                    printf("|%s", classes[j].name);
                }

                num_classes++;
            }

        }

        printf(", // %s\n", char_names[i]);
    }

    for (int i = 128; i < 256; i++) {
        printf("    [%i] = CHAR_FLAG_NON_ASCII,\n", i);
    }
}
#endif

// NOTE(rune): Even though we only have char_flags in the ascii-range, the table is 256 items long,
// so we can always index with an u8 wihtout to a branch to check for <128.
static readonly char_flags char_flags_table[256] = {
    [0] = CHAR_FLAG_CONTROL, // NULL (Null character)
    [1] = CHAR_FLAG_CONTROL, // SOH (Start of Header)
    [2] = CHAR_FLAG_CONTROL, // STX (Start of Text)
    [3] = CHAR_FLAG_CONTROL, // ETX (End of Text)
    [4] = CHAR_FLAG_CONTROL, // EOT (End of Transmission)
    [5] = CHAR_FLAG_CONTROL, // ENQ (Enquiry)
    [6] = CHAR_FLAG_CONTROL, // ACK (Acknowledgement)
    [7] = CHAR_FLAG_CONTROL, // BEL (Bell)
    [8] = CHAR_FLAG_CONTROL, // BS (Backspace)
    [9] = CHAR_FLAG_CONTROL|CHAR_FLAG_WHITESPACE, // HT (Horizontal Tab)
    [10] = CHAR_FLAG_CONTROL|CHAR_FLAG_WHITESPACE, // LF (Line feed)
    [11] = CHAR_FLAG_CONTROL|CHAR_FLAG_WHITESPACE, // VT (Vertical Tab)
    [12] = CHAR_FLAG_CONTROL|CHAR_FLAG_WHITESPACE, // FF (Form feed)
    [13] = CHAR_FLAG_CONTROL|CHAR_FLAG_WHITESPACE, // CR (Carriage return)
    [14] = CHAR_FLAG_CONTROL, // SO (Shift Out)
    [15] = CHAR_FLAG_CONTROL, // SI (Shift In)
    [16] = CHAR_FLAG_CONTROL, // DLE (Data link escape)
    [17] = CHAR_FLAG_CONTROL, // DC1 (Device control 1)
    [18] = CHAR_FLAG_CONTROL, // DC2 (Device control 2)
    [19] = CHAR_FLAG_CONTROL, // DC3 (Device control 3)
    [20] = CHAR_FLAG_CONTROL, // DC4 (Device control 4)
    [21] = CHAR_FLAG_CONTROL, // NAK (Negative acknowledgement)
    [22] = CHAR_FLAG_CONTROL, // SYN (Synchronous idle)
    [23] = CHAR_FLAG_CONTROL, // ETB (End of transmission block)
    [24] = CHAR_FLAG_CONTROL, // CAN (Cancel)
    [25] = CHAR_FLAG_CONTROL, // EM (End of medium)
    [26] = CHAR_FLAG_CONTROL, // SUB (Substitute)
    [27] = CHAR_FLAG_CONTROL, // ESC (Escape)
    [28] = CHAR_FLAG_CONTROL, // FS (File separator)
    [29] = CHAR_FLAG_CONTROL, // GS (Group separator)
    [30] = CHAR_FLAG_CONTROL, // RS (Record separator)
    [31] = CHAR_FLAG_CONTROL, // US (Unit separator)
    [32] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_WHITESPACE, // (space)
    [33] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ! (exclamation mark)
    [34] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // " (Quotation mark)
    [35] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // # (Number sign)
    [36] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // $ (Dollar sign)
    [37] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // % (Percent sign)
    [38] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // & (Ampersand)
    [39] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ' (Apostrophe)
    [40] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ( (round brackets or parentheses)
    [41] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ) (round brackets or parentheses)
    [42] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // * (Asterisk)
    [43] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // + (Plus sign)
    [44] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // , (Comma)
    [45] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // - (Hyphen)
    [46] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // . (Full stop , dot)
    [47] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // / (Slash)
    [48] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 0 (number zero)
    [49] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 1 (number one)
    [50] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 2 (number two)
    [51] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 3 (number three)
    [52] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 4 (number four)
    [53] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 5 (number five)
    [54] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 6 (number six)
    [55] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 7 (number seven)
    [56] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 8 (number eight)
    [57] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_DIGIT|CHAR_FLAG_HEXDIGIT, // 9 (number nine)
    [58] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // : (Colon)
    [59] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ; (Semicolon)
    [60] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // < (Less-than sign)
    [61] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // = (Equals sign)
    [62] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // > (Greater-than sign; Inequality)
    [63] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ? (Question mark)
    [64] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // @ (At sign)
    [65] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // A (Capital A)
    [66] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // B (Capital B)
    [67] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // C (Capital C)
    [68] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // D (Capital D)
    [69] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // E (Capital E)
    [70] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // F (Capital F)
    [71] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // G (Capital G)
    [72] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // H (Capital H)
    [73] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // I (Capital I)
    [74] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // J (Capital J)
    [75] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // K (Capital K)
    [76] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // L (Capital L)
    [77] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // M (Capital M)
    [78] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // N (Capital N)
    [79] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // O (Capital O)
    [80] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // P (Capital P)
    [81] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // Q (Capital Q)
    [82] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // R (Capital R)
    [83] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // S (Capital S)
    [84] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // T (Capital T)
    [85] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // U (Capital U)
    [86] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // V (Capital V)
    [87] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // W (Capital W)
    [88] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // X (Capital X)
    [89] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // Y (Capital Y)
    [90] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UPPER|CHAR_FLAG_LETTER, // Z (Capital Z)
    [91] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // [ (square brackets or box brackets)
    [92] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // \ (Backslash)
    [93] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ] (square brackets or box brackets)
    [94] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ^ (Caret or circumflex accent)
    [95] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_UNDERSCORE, // _ (underscore , understrike , underbar or low line)
    [96] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ` (Grave accent)
    [97] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // a (Lowercase  a)
    [98] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // b (Lowercase  b)
    [99] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // c (Lowercase  c)
    [100] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // d (Lowercase  d)
    [101] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // e (Lowercase  e)
    [102] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER|CHAR_FLAG_HEXDIGIT, // f (Lowercase  f)
    [103] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // g (Lowercase  g)
    [104] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // h (Lowercase  h)
    [105] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // i (Lowercase  i)
    [106] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // j (Lowercase  j)
    [107] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // k (Lowercase  k)
    [108] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // l (Lowercase  l)
    [109] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // m (Lowercase  m)
    [110] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // n (Lowercase  n)
    [111] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // o (Lowercase  o)
    [112] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // p (Lowercase  p)
    [113] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // q (Lowercase  q)
    [114] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // r (Lowercase  r)
    [115] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // s (Lowercase  s)
    [116] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // t (Lowercase  t)
    [117] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // u (Lowercase  u)
    [118] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // v (Lowercase  v)
    [119] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // w (Lowercase  w)
    [120] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // x (Lowercase  x)
    [121] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // y (Lowercase  y)
    [122] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_LOWER|CHAR_FLAG_LETTER, // z (Lowercase  z)
    [123] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // { (curly brackets or braces)
    [124] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // | (vertical-bar, vbar, vertical line or vertical slash)
    [125] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // } (curly brackets or braces)
    [126] = CHAR_FLAG_PRINTABLE|CHAR_FLAG_PUNCT, // ~ (Tilde; swung dash)
    [127] = CHAR_FLAG_CONTROL, // DEL (Delete)
    [128] = CHAR_FLAG_NON_ASCII,
    [129] = CHAR_FLAG_NON_ASCII,
    [130] = CHAR_FLAG_NON_ASCII,
    [131] = CHAR_FLAG_NON_ASCII,
    [132] = CHAR_FLAG_NON_ASCII,
    [133] = CHAR_FLAG_NON_ASCII,
    [134] = CHAR_FLAG_NON_ASCII,
    [135] = CHAR_FLAG_NON_ASCII,
    [136] = CHAR_FLAG_NON_ASCII,
    [137] = CHAR_FLAG_NON_ASCII,
    [138] = CHAR_FLAG_NON_ASCII,
    [139] = CHAR_FLAG_NON_ASCII,
    [140] = CHAR_FLAG_NON_ASCII,
    [141] = CHAR_FLAG_NON_ASCII,
    [142] = CHAR_FLAG_NON_ASCII,
    [143] = CHAR_FLAG_NON_ASCII,
    [144] = CHAR_FLAG_NON_ASCII,
    [145] = CHAR_FLAG_NON_ASCII,
    [146] = CHAR_FLAG_NON_ASCII,
    [147] = CHAR_FLAG_NON_ASCII,
    [148] = CHAR_FLAG_NON_ASCII,
    [149] = CHAR_FLAG_NON_ASCII,
    [150] = CHAR_FLAG_NON_ASCII,
    [151] = CHAR_FLAG_NON_ASCII,
    [152] = CHAR_FLAG_NON_ASCII,
    [153] = CHAR_FLAG_NON_ASCII,
    [154] = CHAR_FLAG_NON_ASCII,
    [155] = CHAR_FLAG_NON_ASCII,
    [156] = CHAR_FLAG_NON_ASCII,
    [157] = CHAR_FLAG_NON_ASCII,
    [158] = CHAR_FLAG_NON_ASCII,
    [159] = CHAR_FLAG_NON_ASCII,
    [160] = CHAR_FLAG_NON_ASCII,
    [161] = CHAR_FLAG_NON_ASCII,
    [162] = CHAR_FLAG_NON_ASCII,
    [163] = CHAR_FLAG_NON_ASCII,
    [164] = CHAR_FLAG_NON_ASCII,
    [165] = CHAR_FLAG_NON_ASCII,
    [166] = CHAR_FLAG_NON_ASCII,
    [167] = CHAR_FLAG_NON_ASCII,
    [168] = CHAR_FLAG_NON_ASCII,
    [169] = CHAR_FLAG_NON_ASCII,
    [170] = CHAR_FLAG_NON_ASCII,
    [171] = CHAR_FLAG_NON_ASCII,
    [172] = CHAR_FLAG_NON_ASCII,
    [173] = CHAR_FLAG_NON_ASCII,
    [174] = CHAR_FLAG_NON_ASCII,
    [175] = CHAR_FLAG_NON_ASCII,
    [176] = CHAR_FLAG_NON_ASCII,
    [177] = CHAR_FLAG_NON_ASCII,
    [178] = CHAR_FLAG_NON_ASCII,
    [179] = CHAR_FLAG_NON_ASCII,
    [180] = CHAR_FLAG_NON_ASCII,
    [181] = CHAR_FLAG_NON_ASCII,
    [182] = CHAR_FLAG_NON_ASCII,
    [183] = CHAR_FLAG_NON_ASCII,
    [184] = CHAR_FLAG_NON_ASCII,
    [185] = CHAR_FLAG_NON_ASCII,
    [186] = CHAR_FLAG_NON_ASCII,
    [187] = CHAR_FLAG_NON_ASCII,
    [188] = CHAR_FLAG_NON_ASCII,
    [189] = CHAR_FLAG_NON_ASCII,
    [190] = CHAR_FLAG_NON_ASCII,
    [191] = CHAR_FLAG_NON_ASCII,
    [192] = CHAR_FLAG_NON_ASCII,
    [193] = CHAR_FLAG_NON_ASCII,
    [194] = CHAR_FLAG_NON_ASCII,
    [195] = CHAR_FLAG_NON_ASCII,
    [196] = CHAR_FLAG_NON_ASCII,
    [197] = CHAR_FLAG_NON_ASCII,
    [198] = CHAR_FLAG_NON_ASCII,
    [199] = CHAR_FLAG_NON_ASCII,
    [200] = CHAR_FLAG_NON_ASCII,
    [201] = CHAR_FLAG_NON_ASCII,
    [202] = CHAR_FLAG_NON_ASCII,
    [203] = CHAR_FLAG_NON_ASCII,
    [204] = CHAR_FLAG_NON_ASCII,
    [205] = CHAR_FLAG_NON_ASCII,
    [206] = CHAR_FLAG_NON_ASCII,
    [207] = CHAR_FLAG_NON_ASCII,
    [208] = CHAR_FLAG_NON_ASCII,
    [209] = CHAR_FLAG_NON_ASCII,
    [210] = CHAR_FLAG_NON_ASCII,
    [211] = CHAR_FLAG_NON_ASCII,
    [212] = CHAR_FLAG_NON_ASCII,
    [213] = CHAR_FLAG_NON_ASCII,
    [214] = CHAR_FLAG_NON_ASCII,
    [215] = CHAR_FLAG_NON_ASCII,
    [216] = CHAR_FLAG_NON_ASCII,
    [217] = CHAR_FLAG_NON_ASCII,
    [218] = CHAR_FLAG_NON_ASCII,
    [219] = CHAR_FLAG_NON_ASCII,
    [220] = CHAR_FLAG_NON_ASCII,
    [221] = CHAR_FLAG_NON_ASCII,
    [222] = CHAR_FLAG_NON_ASCII,
    [223] = CHAR_FLAG_NON_ASCII,
    [224] = CHAR_FLAG_NON_ASCII,
    [225] = CHAR_FLAG_NON_ASCII,
    [226] = CHAR_FLAG_NON_ASCII,
    [227] = CHAR_FLAG_NON_ASCII,
    [228] = CHAR_FLAG_NON_ASCII,
    [229] = CHAR_FLAG_NON_ASCII,
    [230] = CHAR_FLAG_NON_ASCII,
    [231] = CHAR_FLAG_NON_ASCII,
    [232] = CHAR_FLAG_NON_ASCII,
    [233] = CHAR_FLAG_NON_ASCII,
    [234] = CHAR_FLAG_NON_ASCII,
    [235] = CHAR_FLAG_NON_ASCII,
    [236] = CHAR_FLAG_NON_ASCII,
    [237] = CHAR_FLAG_NON_ASCII,
    [238] = CHAR_FLAG_NON_ASCII,
    [239] = CHAR_FLAG_NON_ASCII,
    [240] = CHAR_FLAG_NON_ASCII,
    [241] = CHAR_FLAG_NON_ASCII,
    [242] = CHAR_FLAG_NON_ASCII,
    [243] = CHAR_FLAG_NON_ASCII,
    [244] = CHAR_FLAG_NON_ASCII,
    [245] = CHAR_FLAG_NON_ASCII,
    [246] = CHAR_FLAG_NON_ASCII,
    [247] = CHAR_FLAG_NON_ASCII,
    [248] = CHAR_FLAG_NON_ASCII,
    [249] = CHAR_FLAG_NON_ASCII,
    [250] = CHAR_FLAG_NON_ASCII,
    [251] = CHAR_FLAG_NON_ASCII,
    [252] = CHAR_FLAG_NON_ASCII,
    [253] = CHAR_FLAG_NON_ASCII,
    [254] = CHAR_FLAG_NON_ASCII,
    [255] = CHAR_FLAG_NON_ASCII,
};

static char_flags u32_get_char_flags(u32 c) {
    char_flags ret = 0;
    if (c < countof(char_flags_table)) {
        ret = char_flags_table[c];
    } else {
        ret = CHAR_FLAG_NON_ASCII;
    }
    return ret;
}

static char_flags u8_get_char_flags(u8 c) {
    static_assert(countof(char_flags_table) == 256, "char_flags_table size should be 256");
    return char_flags_table[c];
}

////////////////////////////////////////////////////////////////
// rune: Ascii

static bool u8_is_digit(u8 c) { return u8_get_char_flags(c) & CHAR_FLAG_DIGIT; }
static bool u8_is_upper(u8 c) { return u8_get_char_flags(c) & CHAR_FLAG_UPPER; }
static bool u8_is_lower(u8 c) { return u8_get_char_flags(c) & CHAR_FLAG_LOWER; }
static bool u8_is_letter(u8 c) { return u8_get_char_flags(c) & CHAR_FLAG_LETTER; }
static bool u8_is_punct(u8 c) { return u8_get_char_flags(c) & CHAR_FLAG_PUNCT; }
static bool u8_is_word(u8 c) { return u8_get_char_flags(c) & CHAR_FLAG_WORD; }
static bool u8_is_printable(u8 c) { return u8_get_char_flags(c) & CHAR_FLAG_PRINTABLE; }
static bool u8_is_whitespace(u8 c) { return u8_get_char_flags(c) & CHAR_FLAG_WHITESPACE; }
static bool u8_is_nonascii(u8 c) { return c >= 128; }

static bool u32_is_letter(u32 c) { return c <= 127 && u8_is_letter(u8(c)); }
static bool u32_is_digit(u32 c) { return c <= 127 && u8_is_digit(u8(c)); }
static bool u32_is_upper(u32 c) { return c <= 127 && u8_is_upper(u8(c)); }
static bool u32_is_lower(u32 c) { return c <= 127 && u8_is_lower(u8(c)); }
static bool u32_is_punct(u32 c) { return c <= 127 && u8_is_punct(u8(c)); }
static bool u32_is_word(u32 c) { return c <= 127 && u8_is_word(u8(c)); }
static bool u32_is_printable(u32 c) { return c <= 127 && u8_is_printable(u8(c)); }
static bool u32_is_whitespace(u32 c) { return c <= 127 && u8_is_whitespace(u8(c)); }
static bool u32_is_nonascii(u32 c) { return c >= 128; }

static u8 u8_to_lower(u8 c) { return u8_is_upper(c) ? c + 32 : c; }
static u8 u8_to_upper(u8 c) { return u8_is_lower(c) ? c - 32 : c; }

static u32 u32_to_lower(u32 c) { return u32_is_upper(c) ? c + 32 : c; }
static u32 u32_to_upper(u32 c) { return u32_is_lower(c) ? c - 32 : c; }

////////////////////////////////////////////////////////////////
// rune: Zero terminated string

static i64 cstr_len(char *s) {
    char *c = s;

    while (*c) {
        c++;
    }

    i64 ret = c - s;
    return ret;
}

static bool cstr_eq(char *a, char *b) {
    bool ret = false;

    if (a && b) {
        while ((*a) && (*a == *b)) {
            a++;
            b++;
        }

        ret = (*a == *b);
    }

    return ret;
}

////////////////////////////////////////////////////////////////
// rune: Counted string

static str str_from_cstr(char *cstr) {
    str ret = str_make((u8 *)cstr, strlen(cstr));
    return ret;
}

static bool str_eq(str a, str b) {
    bool ret = false;

    if (a.count == b.count) {
        if (a.count) {
            ret = memcmp(a.v, b.v, a.count) == 0;
        } else {
            ret = true;
        }
    }

    return ret;
}

static bool str_eq_nocase(str a, str b) {
    bool ret = false;

    if (a.count == b.count) {
        if (a.count) {
            ret = _memicmp(a.v, b.v, a.count) == 0;
        } else {
            ret = true;
        }
    }

    return ret;
}

static i64 str_idx_of_u8(str s, u8 c) {
    i64 ret = -1;

    u8 *found = memchr(s.v, c, s.count);
    if (found) {
        ret = found - s.v;
    }

    return ret;
}

static i64 str_idx_of_str(str a, str b) {
    i64 ret = -1;

    if (b.count == 0 && a.count > 0) {
        ret = 0;
    } else if (a.count >= b.count) {
        u8 first = b.v[0];
        u8 last  = b.v[b.count - 1];

        for (i64 i = 0; i <= a.count - b.count; i++) {
            if ((a.v[i] == first) && (a.v[i + b.count - 1] == last)) {
                if (memcmp(a.v + i, b.v, b.count) == 0) {
                    ret = i;
                    break;
                }
            }
        }
    }

    return ret;
}

static bool u8_eq_nocase(u8 a, u8 b) {
    return u8_to_upper(a) == u8_to_upper(b);
}

static i64 str_idx_of_str_nocase(str a, str b) {
    i64 ret = -1;

    if (b.count == 0 && a.count > 0) {
        ret = 0;
    } else if (a.count >= b.count) {
        u8 first = b.v[0];
        u8 last  = b.v[b.count - 1];

        for (i64 i = 0; i <= a.count - b.count; i++) {
            if (u8_eq_nocase(a.v[i], first) && u8_eq_nocase(a.v[i + b.count - 1], last)) {
                if (_memicmp(a.v + i, b.v, b.count) == 0) {
                    ret = i;
                    break;
                }
            }
        }
    }

    return ret;
}

static i64 str_idx_of_last_u8(str s, u8 c) {
    i64 ret = -1;

    for (i64 i = 0; i < s.count; i++) {
        i64 i_rev = s.count - i - 1;
        if (s.v[i_rev] == c) {
            ret = i_rev;
            break;
        }
    }

    return ret;
}

static str substr_idx(str s, i64 idx) {
    str ret = str("");

    if (s.count > idx) {
        ret.v     = s.v + idx;
        ret.count = s.count - idx;
    }

    return ret;
}

static str substr_len(str s, i64 idx, i64 len) {
    str ret = str("");

    if (s.count >= idx) {
        ret.v     = s.v + idx;
        ret.count = min(len, s.len - idx);
    }

    return ret;
}

static str substr_range(str s, i64_range r) {
    str ret = str("");

    if (r.min >= 0 && r.max <= s.count) {
        ret.v     = s.v + r.min;
        ret.count = r.max - r.min;
    }

    return ret;
}

static str str_left(str s, i64 len) {
    str ret = substr_len(s, 0, len);
    return ret;
}

static str str_right(str s, i64 len) {
    i64 clamped = min(s.count, len);
    str ret = substr_idx(s, s.count - clamped);
    return ret;
}

static str str_chop_left(str *s, i64 idx) {
    str ret = str("");

    ret = substr_len(*s, 0, idx);
    *s = substr_idx(*s, idx);

    return ret;
}

static str str_chop_right(str *s, i64 idx) {
    str ret = str("");

    if (idx < s->count) {
        ret = substr_idx(*s, idx);
        *s = substr_len(*s, 0, idx);
    }

    return ret;
}

static str str_chop_by_delim(str *s, str delim) {
    str ret = str("");

    i64 idx = str_idx_of_str(*s, delim);
    if (idx != -1) {
        ret = substr_len(*s, 0, idx);
        *s  = substr_idx(*s, idx + delim.count);
    } else {
        ret = *s;
        *s = str("");
    }

    return ret;
}

static str str_chop_by_whitespace(str *s) {
    i64 i = 0;
    for (; i < s->len; i++) {
        if (u8_is_whitespace(s->v[i])) {
            break;
        }
    }

    str ret = str_chop_left(s, i);
    *s = str_trim_left(*s);
    return ret;
}

static str str_trim(str s) {
    s = str_trim_left(s);
    s = str_trim_right(s);

    return s;
}

static str str_trim_left(str s) {
    while (s.count && u8_is_whitespace(s.v[0])) {
        s.v     += 1;
        s.count -= 1;
    }

    return s;
}

static str str_trim_right(str s) {
    while (s.count && u8_is_whitespace(s.v[s.count - 1])) {
        s.count--;
    }

    return s;
}

static bool str_starts_with_str(str s, str prefix) {
    i64 idx = str_idx_of_str(s, prefix);
    bool ret = idx == 0;
    return ret;
}

static bool str_ends_with_str(str s, str suffix) {
    // TODO(rune): @Speed Optimize -> no need to search through the whole string.

    i64 idx = str_idx_of_str(s, suffix);
    bool ret = idx == s.count - suffix.count;
    return ret;
}

static bool str_starts_with_u8(str s, u8 c) {
    bool ret = false;
    if (s.count > 0) ret = (s.v[0] == c);
    return ret;
}

static bool str_ends_with_u8(str s, u8 c) {
    bool ret = false;
    if (s.count > 0) ret = (s.v[s.count - 1] == c);
    return ret;
}

static str_x3 str_split_x3(str s, i64 a, i64 b) {
    assert(a <= b);
    str_x3 ret = { 0 };
    ret.v[0] = substr_len(s, 0, a);
    ret.v[1] = substr_len(s, a, b - a);
    ret.v[2] = substr_idx(s, b);
    return ret;
}

////////////////////////////////////////////////////////////////
// rune: String list

static str str_list_push(str_list *list, arena *arena, str s) {
    str_node *node = arena_push_struct(arena, str_node);
    node->v = s;
    slist_push(list, node);
    list->count     += 1;
    list->total_len += s.len;
    return s;
}

static str str_list_push_front(str_list *list, arena *arena, str s) {
    str_node *node = arena_push_struct(arena, str_node);
    node->v = s;
    slist_push_front(list, node);
    list->count     += 1;
    list->total_len += s.len;
    return s;
}

static str str_list_push_fmt_args(str_list *list, arena *arena, args args) {
    str s = arena_print_args(arena, args);
    str_list_push(list, arena, s);
    return s;
}

static void str_list_join(str_list *list, str_list append) {
    if (list->last) {
        list->last->next = append.first;
    } else {
        list->first = append.first;
    }

    list->last       = append.last;
    list->count     += append.count;
    list->total_len += append.total_len;
}

static str str_list_concat(str_list *list, arena *arena) {
    str s = str_list_concat_sep(list, arena, str(""));
    return s;
}

static str str_list_concat_sep(str_list *list, arena *arena, str sep) {
    str ret = { 0 };
    if (list->count > 0) {
        i64 len = list->total_len + sep.len * (list->count - 1);
        u8 *dst = arena_push_size(arena, len + 1, 0);
        i64 off = 0;
        for_list (str_node, node, *list) {
            str s = node->v;
            memcpy(dst + off, s.v, s.len);
            off += s.len;
            if (node != list->last) {
                memcpy(dst + off, sep.v, sep.len);
                off += sep.len;
            }
        }

        assert(off == len);
        ret = str_make(dst, len);
    }

    return ret;
}

static str_list str_split_by_whitespace(str s, arena *arena) {
    str_list ret = { 0 };
    while (s.len) {
        str part = str_chop_by_whitespace(&s);
        if (part.len) {
            str_list_push(&ret, arena, part);
        }
    }
    return ret;
}

static str_list str_split_by_delim(str s, str delim, arena *arena) {
    str_list ret = { 0 };
    while (s.len) {
        str part = str_chop_by_delim(&s, delim);
        if (part.len) {
            str_list_push(&ret, arena, part);
        }
    }
    return ret;
}

////////////////////////////////////////////////////////////////
// rune: String array

static str_array str_array_reserve(i64 count, arena *arena) {
    str_array array = { 0 };
    if (count > 0) {
        array.v     = arena_push_array(arena, str, count);
        array.count = count;
    }
    return array;
}

static str_array str_array_from_list(str_list list, arena *arena) {
    str_array array = str_array_reserve(list.count, arena);
    str_node *node = list.first;
    for_n (i64, i, list.count) {
        array.v[i] = node->v;
        node = node->next;
    }
    return array;
}

////////////////////////////////////////////////////////////////
// rune: Comparison

static i32 str_cmp_case(str a, str b) {
    i32 ret = strncmp((char *)a.v, (char *)b.v, min(a.len, b.len));
    if (ret == 0 && a.len < b.len) ret = -1;
    if (ret == 0 && a.len > b.len) ret =  1;
    return ret;
}

static i32 str_cmp_nocase(str a, str b) {
    i32 ret = _strnicmp((char *)a.v, (char *)b.v, min(a.len, b.len));
    if (ret == 0 && a.len < b.len) ret = -1;
    if (ret == 0 && a.len > b.len) ret =  1;
    return ret;
}

////////////////////////////////////////////////////////////////
// rune: Sorting

static i32 str_cmp_case_(const void *a, const void *b) { return str_cmp_case(*(str *)a, *(str *)b); }
static i32 str_cmp_nocase_(const void *a, const void *b) { return str_cmp_nocase(*(str *)a, *(str *)b); }

static void str_sort_case(str_array array) {
    qsort(array.v, array.count, sizeof(str), str_cmp_case_);
}

static void str_sort_nocase(str_array array) {
    qsort(array.v, array.count, sizeof(str), str_cmp_nocase_);
}

////////////////////////////////////////////////////////////////
// rune: Wide strings

static bool wstr_eq(wstr a, wstr b) {
    bool ret = (a.len == b.len) && (memcmp(a.v, b.v, a.len * sizeof(u16)) == 0);
    return ret;
}

////////////////////////////////////////////////////////////////
// rune: Fuzzy match

static void fuzzy_match_list_insert_after(fuzzy_match_list *list, fuzzy_match_node *after, i64_range range, arena *arena) {
    fuzzy_match_node *node = arena_push_struct(arena, fuzzy_match_node);
    node->range = range;

    if (after) {
        node->next = after->next;
        after->next = node;
    } else {
        node->next = list->first;
        list->first = node;
    }

    list->count++;
}

static fuzzy_match_list fuzzy_match_list_from_str(str haystack, str_list needles, arena *arena) {
    YO_PROFILE_BEGIN(fuzzy_match_list_from_str);
    fuzzy_match_list matches = { 0 };
    for_list (str_node, n, needles) {
        i64 at = 0;
        while (at < haystack.len) {
            str remaining = substr_idx(haystack, at);
            i64 idx = str_idx_of_str_nocase(remaining, n->v);
            if (idx != -1) {
                i64_range found_range = i64_range(at + idx, at + idx + n->v.len);
                fuzzy_match_node *insert_after = null;

                bool overlap = false;
                for_list (fuzzy_match_node, check, matches) {
                    if (ranges_overlap(check->range, found_range)) {
                        overlap = true;
                        break;
                    }

                    if (check->range.min < found_range.min) {
                        insert_after = check;
                    }
                }

                if (overlap) {
                    at = found_range.max;
                } else {
                    fuzzy_match_list_insert_after(&matches, insert_after, found_range, arena);
                    break;
                }
            } else {
                break;
            }
        }
    }
    YO_PROFILE_END(fuzzy_match_list_from_str);
    fuzzy_match_list *_ret = &matches;
    return *_ret;
}
