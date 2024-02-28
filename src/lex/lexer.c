#include "lexer.h"
#include "position.h"
#include <stdio.h>
#include <string.h>

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
    lexer->row = 0;
    lexer->col = 0;

    lexer->tk = -1;
    lexer->lexeme = NULL;
    lexer->lit_kind = -1;
}

void lexer_free() {
    if (lexer == NULL) return;

    free(lexer->filename);
    free(lexer->buffer);
    free(lexer);
    lexer = NULL;
}

void next() {
    _next_skip_white_space();
    position pos = (position){lexer->filename, lexer->off, lexer->row, lexer->col};

    // reserved words/identifier
    if ((lexer->ch >= 'a' && lexer->ch <= 'z') || (lexer->ch >= 'A' && lexer->ch <= 'Z') || lexer->ch == '_') {
        _scan_word();
    }
}

static void _next_skip_white_space() {
    do {
        _next_ch();
    } while (lexer->ch == ' ');
}

static void _next_ch() {
    if (lexer->off < lexer->buffer_len - 1) {
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
    char *word = (char *)calloc(MAX_WORD_LEN, sizeof(char));
    if (word == NULL) {
        perror("Lexer: fail to calloc memory for word");
        return true;
    }

    word[MAX_WORD_LEN - 1] = '\0';
    char *w = word;
    while ((lexer->ch >= 'a' && lexer->ch <= 'z') ||
           (lexer->ch >= 'A' && lexer->ch <= 'Z' || (lexer->ch >= '0' && lexer->ch <= '9') || lexer->ch == '_')) {
        if (*w == '\0') {
            printf("Lexer: word reach max length of %d", MAX_WORD_LEN);
            free(word);
            return false;
        }
        *w = lexer->ch;
        w++;
        _next_ch();
    }

    lexer->lexeme = word;
    _contract();
    return true;
}

static bool _scan_number() {
    char *number = (char *)calloc(MAX_NUMBER_LEN, sizeof(char));
    if (number == NULL) {
        perror("Lexer: fail to calloc memory for number");
        return false;
    }

    number[MAX_NUMBER_LEN - 1] = '\0';
    char *n = number;
    while (lexer->ch >= '0' && lexer->ch <= '9') {
        if (*n == '\0') {
            printf("Lexer: number reach max length of %d", MAX_NUMBER_LEN);
            free(number);
            return false;
        }
        *n = lexer->ch;
        n++;
        _next_ch();
    }

    lexer->lexeme = number;
    _contract();
    return true;
}