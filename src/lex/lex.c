#include "lex.h"
#include "global.h"
#include <string.h>

char                *filename;             // current parsing file name
static char         *buffer;               // buffer of read file
static unsigned long buffer_len;           // length of buffer
char                 ch;                   // current parsing char, inits & ends with EOF
int                  off;                  // current offset of buffer, starts from 0
int                  row;                  // current row of file, starts from 1
int                  col;                  // current col of file, starts from 0
enum Token           tk;                   // current parsed token
enum LitKind         lk;                   // lit kind, available only when tk is _lit
char                 lexeme[MAX_LINE_LEN]; // lexeme string, available only when tk is _lit or _ident
char                *lex_bad_msg;          // error message

bool lex_init(char *filepath, bool _debug) {
    if (filepath == NULL) perror("lexer: can not find source file");

    FILE *f = fopen(filepath, "r");
    if (f == NULL) perror("Lexer: can not find source file");

    char *full_name = strrchr(filepath, '/') + 1;
    if (full_name == NULL) {
        printf("Lexer: file suffix is illegal\n");
        fclose(f);
    }
    char *suffix = strrchr(full_name, '.') + 1;
    if (suffix != NULL && strcmp(suffix, "sl")) {
        printf("Lexer: file suffix is illegal\n");
        fclose(f);
    }
    int filename_len = strlen(full_name) - strlen(suffix) - 1;
    filename = calloc(filename_len + 1, sizeof(char));
    strncpy(filename, full_name, filename_len);

    if (_debug) printf("Lexer: find file: %s\n", filename);

    // remember to free buffer if failed

    size_t buffer_size = 0;
    // no more than 1023 chars/line
    char   line[MAX_LINE_LEN] = {0};
    while (fgets(line, sizeof(line), f) != NULL) {
        size_t line_len = strlen(line);
        // +1 for null terminator
        char  *new_buffer = realloc(buffer, buffer_size + line_len + 1);
        if (new_buffer == NULL) {
            free(buffer);
            perror("Lexer: fail to realloc memory");
            fclose(f);
            return false;
        }
        buffer = new_buffer;
        // +1 for null terminator
        memcpy(buffer + buffer_size, line, line_len + 1);
        buffer_size += line_len;
    }
    fclose(f);

    if (_debug) printf("Lexer: file content:\n%s\n", buffer);
    buffer_len = buffer_size;

    debug = _debug;
    ch = EOF;
    off = -1;
    row = 1;
    col = 0;

    tk = -1;
    lexeme[0] = '\0';
    lk = -1;

    return true;
}

enum Token lex_next() {
    _next_skip_white_space();

    // reserved words/identifier
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_') {
        if (!_scan_word()) {
            lex_bad_msg = "reach max length of word";
            return tk = _illegal;
        }

        enum Token _tk = lookup_reserved_tk(lexeme);
        if (_tk == _not_exist) return tk = _ident;
        else return tk = _tk;
    }

    // lit
    // int / float
    if (ch >= '0' && ch <= '9') {
        if (!_scan_number(false)) {
            lex_bad_msg = "reach max length of number";
            return tk = _illegal;
        }

        bool is_float = false;
        _next_ch();
        if (ch == '.') {
            is_float = true;
            _next_ch();
            if (!_scan_number(true)) {
                lex_bad_msg = "reach max length of number";
                return tk = _illegal;
            }
        }

        if (!is_float) _contract();
        lk = is_float ? float_lk : int_lk;
        return tk = _lit;
    }
    // string
    if (ch == '"') {
        int   offset = 1;
        char *s = lexeme;
        while (off + offset < buffer_len && buffer[off + offset] != '"') {
            if (s - lexeme == MAX_LINE_LEN - 1) {
                off = offset - 1;
                lex_bad_msg = "reach max length of line";
                return tk = _illegal;
            }
            // string can only be declared in one line
            if (buffer[off + offset] == '\n') {
                off = offset - 1;
                lex_bad_msg = "illegal line end in string literal";
                return tk = _illegal;
            }

            *s = buffer[off + offset];
            s++;
            offset++;
        }

        // update lexer's offset to offset of matched right '"'
        off += offset;

        // if no right '"' matched, make it illegal token
        if (off >= buffer_len) {
            lex_bad_msg = "illegal line end in string literal";
            return tk = _illegal;
        }

        lk = string_lk;
        return tk = _lit;
    }
    // char
    if (ch == '\'') {
        _next_ch();
        if (ch == '\'') {
            lex_bad_msg = "empty char literal";
            return tk = _illegal;
        }

        lexeme[0] = ch;
        lexeme[1] = '\0';
        _next_ch();
        if (ch != '\'') {
            _contract();
            lex_bad_msg = "unclosed char literal";
            return tk = _illegal;
        }

        lk = char_lk;
        return tk = _lit;
    }

    // symbols
    // non-prefix char symbols
    if (ch == '@') return tk = _at;
    if (ch == '~') return tk = _not;
    if (ch == '*') return tk = _mul;
    if (ch == '%') return tk = _rem;
    if (ch == '^') return tk = _xor;
    if (ch == '(') return tk = _lparen;
    if (ch == ')') return tk = _rparen;
    if (ch == '[') return tk = _lbracket;
    if (ch == ']') return tk = _rbracket;
    if (ch == '{') return tk = _lbrace;
    if (ch == '}') return tk = _rbrace;
    if (ch == ',') return tk = _comma;
    if (ch == '.') return tk = _period;
    if (ch == ';') return tk = _semi;
    if (ch == ':') return tk = _colon;
    if (ch == '?') return tk = _ques;
    if (ch == EOF) return tk = _eof;
    if (ch == '\n') {
        _newline();
        return lex_next();
    }

    // prefixed char symbols
    char pch = ch;
    _next_ch();

    // newline: \r\n
    if (pch == '\r') {
        if (ch == '\n') {
            _newline();
            return lex_next();
        }

        lex_bad_msg = "illegal token";
        return tk = _illegal;
    }
    // LT, LE, SHL: <, <=, <<
    if (pch == '<') {
        if (ch == '=') return tk = _le;
        if (ch == '<') return tk = _shl;

        _contract();
        return tk = _lt;
    }
    // GT, GE, SHR: >, >=, >>
    if (pch == '>') {
        if (ch == '=') return tk = _ge;
        if (ch == '>') return tk = _shr;

        _contract();
        return tk = _gt;
    }
    // ASSIGN, EQ: =, ==
    if (pch == '=') {
        if (ch == '=') return tk = _eq;

        _contract();
        return tk = _assign;
    }
    // LNOT, NE: !, !=
    if (pch == '!') {
        if (ch == '=') return tk = _ne;

        _contract();
        return tk = _lnot;
    }
    // ADD, INC: +, ++
    if (pch == '+') {
        if (ch == '+') return tk = _inc;

        _contract();
        return tk = _add;
    }
    // SUB, DEC, RARROW, NEGATIVE_NUMBER: -, --, ->
    if (pch == '-') {
        if (ch == '-') return tk = _dec;
        if (ch == '>') return tk = _rarrow;
        if (ch >= '0' && ch <= '9') {
            if (!_scan_number(false)) {
                lex_bad_msg = "reach max length of number";
                return tk = _illegal;
            }

            bool is_float = false;
            _next_ch();
            if (ch == '.') {
                is_float = true;
                _next_ch();
                if (!_scan_number(true)) {
                    lex_bad_msg = "reach max length of number";
                    return tk = _illegal;
                }
            }

            if (!is_float) _contract();
            lk = is_float ? float_lk : int_lk;
            return tk = _lit;
        }

        _contract();
        return tk = _sub;
    }
    // QUO, COMMENT: /, //
    if (pch == '/') {
        if (ch == '/') {
            int   offset = 1;
            char *s = lexeme;
            while (buffer[off + offset] != '\n') {
                if (s - lexeme == MAX_LINE_LEN - 1) {
                    off = offset - 1;
                    lex_bad_msg = "reach max length of line";
                    return tk = _illegal;
                }

                *s = buffer[off + offset];
                s++;
                offset++;
            }
            *s = '\0';
            // do not eat '\n'
            off += offset - 1;
            return tk = _comment;
        }

        _contract();
        return tk = _quo;
    }
    // AND, LAND: &, &&
    if (pch == '&') {
        if (ch == '&') return tk = _land;

        _contract();
        return tk = _and;
    }
    // OR, LOR: |, ||
    if (pch == '|') {
        if (ch == '|') return tk = _lor;

        _contract();
        return tk = _or;
    }

    // pch is illegal
    _contract();
    lexeme[0] = pch;
    lexeme[1] = '\0';
    lex_bad_msg = "illegal token";
    return tk = _illegal;
}

static void _next_skip_white_space() {
    do _next_ch();
    while (ch == ' ');
}

static void _next_ch() {
    // avoid -1 and unsigned lang compare
    if (off == -1 || off < buffer_len - 1) {
        ch = buffer[++off];
        col++;
    } else {
        off = buffer_len;
        ch = EOF;
    }
}

static void _contract() {
    if (off > 0) {
        off--;
        col--;
    }
}

static void _newline() {
    row++;
    col = 0;
}

static bool _scan_word() {
    char *w = lexeme;
    while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z' || (ch >= '0' && ch <= '9') || ch == '_')) {
        if (w - lexeme == MAX_WORD_LEN - 1) return false;
        *w = ch;
        w++;
        _next_ch();
    }

    *w = '\0';
    _contract();
    return true;
}

static bool _scan_number(bool float_part) {
    char *n;
    if (float_part) {
        n = lexeme + strlen(lexeme);
        *n = '.';
        n++;
    } else n = lexeme;

    while (ch >= '0' && ch <= '9') {
        if (n - lexeme == MAX_NUMBER_LEN - 1) return false;

        *n = ch;
        n++;
        _next_ch();
    }

    *n = '\0';
    _contract();
    return true;
}