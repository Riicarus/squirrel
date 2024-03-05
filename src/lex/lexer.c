#include "lexer.h"
#include <string.h>

struct _lexer *lexer = &(struct _lexer){};

void lexer_init(char *filepath, bool debug) {
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
    lexer->filename = (char *)calloc(filename_len, sizeof(char));
    strncpy(lexer->filename, full_name, filename_len);

    if (debug) printf("Lexer: find file: %s\n", lexer->filename);

    // remember to free buffer if failed
    char  *buffer = NULL;
    size_t buffer_size = 0;
    // no more than 1023 chars/line
    char   line[MAX_LINE_LEN] = {0};
    while (fgets(line, sizeof(line), f) != NULL) {
        size_t line_len = strlen(line);
        // +1 for null terminator
        char  *new_buffer = (char *)realloc(buffer, buffer_size + line_len + 1);
        if (new_buffer == NULL) {
            free(buffer);
            perror("Lexer: fail to realloc memory");
            fclose(f);
        }
        buffer = new_buffer;
        // +1 for null terminator
        memcpy(buffer + buffer_size, line, line_len + 1);
        buffer_size += line_len;
    }
    fclose(f);

    if (debug) printf("Lexer: file content:\n%s\n", buffer);
    lexer->buffer = buffer;
    lexer->buffer_len = buffer_size;

    lexer->debug = debug;
    lexer->ch = EOF;
    lexer->off = -1;
    lexer->row = 1;
    lexer->col = 0;

    lexer->tk = -1;
    lexer->lexeme[0] = '\0';
    lexer->lit_kind = -1;
}

void lexer_free() {
    if (lexer == NULL) return;

    free(lexer->filename);
    free(lexer->buffer);
    free(lexer);
    lexer = NULL;
}

token lexer_next() {
    _next_skip_white_space();

    // reserved words/identifier
    if ((lexer->ch >= 'a' && lexer->ch <= 'z') || (lexer->ch >= 'A' && lexer->ch <= 'Z') || lexer->ch == '_') {
        if (!_scan_word()) {
            lexer->bad_msg = "reach max length of word";
            return lexer->tk = _illegal;
        }

        token tk = lookup_reserved_tk(lexer->lexeme);
        if (tk == _not_exist) return lexer->tk = _ident;
        else return lexer->tk = tk;
    }

    // lit
    // int / float
    if (lexer->ch >= '0' && lexer->ch <= '9') {
        if (!_scan_number(false)) {
            lexer->bad_msg = "reach max length of number";
            return lexer->tk = _illegal;
        }

        bool is_float = false;
        _next_ch();
        if (lexer->ch == '.') {
            is_float = true;
            _next_ch();
            if (!_scan_number(true)) {
                lexer->bad_msg = "reach max length of number";
                return lexer->tk = _illegal;
            }
        }

        if (!is_float) _contract();
        lexer->lit_kind = is_float ? float_lit : int_lit;
        return lexer->tk = _lit;
    }
    // string
    if (lexer->ch == '"') {
        int   off = 1;
        char *s = lexer->lexeme;
        while (lexer->off + off < lexer->buffer_len && lexer->buffer[lexer->off + off] != '"') {
            if (s - lexer->lexeme == MAX_LINE_LEN - 1) {
                lexer->off = off - 1;
                lexer->bad_msg = "reach max length of line";
                return lexer->tk = _illegal;
            }
            // string can only be declared in one line
            if (lexer->buffer[lexer->off + off] == '\n') {
                lexer->off = off - 1;
                lexer->bad_msg = "illegal line end in string literal";
                return lexer->tk = _illegal;
            }

            *s = lexer->buffer[lexer->off + off];
            s++;
            off++;
        }

        // update lexer's offset to offset of matched right '"'
        lexer->off += off;

        // if no right '"' matched, make it illegal token
        if (lexer->off >= lexer->buffer_len) {
            lexer->bad_msg = "illegal line end in string literal";
            return lexer->tk = _illegal;
        }

        lexer->lit_kind = string_lit;
        return lexer->tk = _lit;
    }
    // char
    if (lexer->ch == '\'') {
        _next_ch();
        if (lexer->ch == '\'') {
            lexer->bad_msg = "empty char literal";
            return lexer->tk = _illegal;
        }

        lexer->lexeme[0] = lexer->ch;
        lexer->lexeme[1] = '\0';
        _next_ch();
        if (lexer->ch != '\'') {
            _contract();
            lexer->bad_msg = "unclosed char literal";
            return lexer->tk = _illegal;
        }

        lexer->lit_kind = char_lit;
        return lexer->tk = _lit;
    }

    // symbols
    // non-prefix char symbols
    if (lexer->ch == '~') return lexer->tk = _not;
    if (lexer->ch == '*') return lexer->tk = _mul;
    if (lexer->ch == '%') return lexer->tk = _rem;
    if (lexer->ch == '^') return lexer->tk = _xor;
    if (lexer->ch == '(') return lexer->tk = _lparen;
    if (lexer->ch == ')') return lexer->tk = _rparen;
    if (lexer->ch == '[') return lexer->tk = _lbracket;
    if (lexer->ch == ']') return lexer->tk = _rbracket;
    if (lexer->ch == '{') return lexer->tk = _lbrace;
    if (lexer->ch == '}') return lexer->tk = _rbrace;
    if (lexer->ch == ',') return lexer->tk = _comma;
    if (lexer->ch == '.') return lexer->tk = _period;
    if (lexer->ch == ';') return lexer->tk = _semi;
    if (lexer->ch == ':') return lexer->tk = _colon;
    if (lexer->ch == '?') return lexer->tk = _ques;
    if (lexer->ch == EOF) return lexer->tk = _eof;
    if (lexer->ch == '\n') {
        _newline();
        return lexer_next();
    }

    // prefixed char symbols
    char pch = lexer->ch;
    _next_ch();

    // newline: \r\n
    if (pch == '\r') {
        if (lexer->ch == '\n') {
            _newline();
            return lexer_next();
        }

        lexer->bad_msg = "illegal token";
        return lexer->tk = _illegal;
    }
    // LT, LE, SHL: <, <=, <<
    if (pch == '<') {
        if (lexer->ch == '=') return lexer->tk = _le;
        if (lexer->ch == '<') return lexer->tk = _shl;

        _contract();
        return lexer->tk = _lt;
    }
    // GT, GE, SHR: >, >=, >>
    if (pch == '>') {
        if (lexer->ch == '=') return lexer->tk = _ge;
        if (lexer->ch == '>') return lexer->tk = _shr;

        _contract();
        return lexer->tk = _gt;
    }
    // ASSIGN, EQ: =, ==
    if (pch == '=') {
        if (lexer->ch == '=') return lexer->tk = _eq;

        _contract();
        return lexer->tk = _assign;
    }
    // LNOT, NE: !, !=
    if (pch == '!') {
        if (lexer->ch == '=') return lexer->tk = _ne;

        _contract();
        return lexer->tk = _lnot;
    }
    // ADD, INC: +, ++
    if (pch == '+') {
        if (lexer->ch == '+') return lexer->tk = _inc;

        _contract();
        return lexer->tk = _add;
    }
    // SUB, DEC, RARROW: -, --, ->
    if (pch == '-') {
        if (lexer->ch == '-') return lexer->tk = _dec;
        if (lexer->ch == '>') return lexer->tk = _rarrow;

        _contract();
        return lexer->tk = _sub;
    }
    // QUO, COMMENT: /, //
    if (pch == '/') {
        if (lexer->ch == '/') {
            int   off = 1;
            char *s = lexer->lexeme;
            while (lexer->buffer[lexer->off + off] != '\n') {
                if (s - lexer->lexeme == MAX_LINE_LEN - 1) {
                    lexer->off = off - 1;
                    lexer->bad_msg = "reach max length of line";
                    return lexer->tk = _illegal;
                }

                *s = lexer->buffer[lexer->off + off];
                s++;
                off++;
            }
            *s = '\0';
            // do not eat '\n'
            lexer->off += off - 1;
            return lexer->tk = _comment;
        }

        _contract();
        return lexer->tk = _quo;
    }
    // AND, LAND: &, &&
    if (pch == '&') {
        if (lexer->ch == '&') return lexer->tk = _land;

        _contract();
        return lexer->tk = _and;
    }
    // OR, LOR: |, ||
    if (pch == '|') {
        if (lexer->ch == '|') return lexer->tk = _lor;

        _contract();
        return lexer->tk = _or;
    }

    // pch is illegal
    _contract();
    lexer->lexeme[0] = pch;
    lexer->lexeme[1] = '\0';
    lexer->bad_msg = "illegal token";
    return lexer->tk = _illegal;
}

static void _next_skip_white_space() {
    do {
        _next_ch();
    } while (lexer->ch == ' ');
}

static void _next_ch() {
    // avoid -1 and unsigned lang compare
    if (lexer->off == -1 || lexer->off < lexer->buffer_len - 1) {
        lexer->ch = lexer->buffer[++lexer->off];
        lexer->col++;
    } else {
        lexer->off = lexer->buffer_len;
        lexer->ch = EOF;
    }
}

static void _contract() {
    if (lexer->off > 0) {
        lexer->off--;
        lexer->col--;
    }
}

static void _newline() {
    lexer->row++;
    lexer->col = 0;
}

static bool _scan_word() {
    char *w = lexer->lexeme;
    while ((lexer->ch >= 'a' && lexer->ch <= 'z') ||
           (lexer->ch >= 'A' && lexer->ch <= 'Z' || (lexer->ch >= '0' && lexer->ch <= '9') || lexer->ch == '_')) {
        if (w - lexer->lexeme == MAX_WORD_LEN - 1) return false;
        *w = lexer->ch;
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
        n = lexer->lexeme + strlen(lexer->lexeme);
        *n = '.';
        n++;
    } else n = lexer->lexeme;

    while (lexer->ch >= '0' && lexer->ch <= '9') {
        if (n - lexer->lexeme == MAX_NUMBER_LEN - 1) return false;

        *n = lexer->ch;
        n++;
        _next_ch();
    }

    *n = '\0';
    _contract();
    return true;
}