/*
Q> Build a clean, C-based, table-driven Deterministic Finite Automaton (DFA) scanner.

Task Description:
Write a program in C that reads a stream of characters from an input file or string and emits a sequence of tokens. The lexer must implement the following state transitions represented as a 2D matrix (transition table).

Required Token Types:

    TOKEN_INT: Non-empty sequences of digits (e.g., 42, 007).
    TOKEN_FLOAT: Digits containing exactly one decimal point with at least one trailing digit (e.g., 3.14, 0.5).
    TOKEN_IDENT: Identifiers starting with an alphabetic character or underscore, followed by alphanumeric characters or underscores (e.g., var_name, x1).
    TOKEN_ASSIGN: The single symbol =.
    TOKEN_EQUALS: The double symbol ==.
    TOKEN_ERROR: Unrecognized inputs.

Requirements:Construct the explicit state transition table int DFA[NUM_STATES][CHAR_CLASSES] manually in C.Implement a input mapping function char_to_class(char c) to map ASCII bytes to character classes (e.g., DIGIT, ALPHA, DOT, EQUALS, OTHER).Maintain a loop that feeds characters into the DFA until a dead state or delimiter is hit, then emits the matching token.
(Do not use standard C regex libraries (<regex.h>). )
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    CC_DIGIT,
    CC_ALPHA,
    CC_DOT,
    CC_EQUALS,
    CC_OTHER,
    NUM_CLASSES
} CharClass;

typedef enum {
    STATE_START,
    STATE_INT,
    STATE_DOT,
    STATE_FLOAT,
    STATE_IDENT,
    STATE_ASSIGN,
    STATE_EQUALS,
    STATE_ERROR,
    NUM_STATES
} State;

typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_IDENT,
    TOKEN_ASSIGN,
    TOKEN_EQUALS,
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

static const int DFA[NUM_STATES][NUM_CLASSES] = {
    // CC_DIGIT      CC_ALPHA     CC_DOT       CC_EQUALS    CC_OTHER
    { STATE_INT,    STATE_IDENT, STATE_ERROR, STATE_ASSIGN, STATE_ERROR }, // STATE_START
    { STATE_INT,    STATE_ERROR, STATE_DOT,   STATE_ERROR,  STATE_ERROR }, // STATE_INT
    { STATE_FLOAT,  STATE_ERROR, STATE_ERROR, STATE_ERROR,  STATE_ERROR }, // STATE_DOT
    { STATE_FLOAT,  STATE_ERROR, STATE_ERROR, STATE_ERROR,  STATE_ERROR }, // STATE_FLOAT
    { STATE_IDENT,  STATE_IDENT, STATE_ERROR, STATE_ERROR,  STATE_ERROR }, // STATE_IDENT
    { STATE_ERROR,  STATE_ERROR, STATE_ERROR, STATE_EQUALS, STATE_ERROR }, // STATE_ASSIGN
    { STATE_ERROR,  STATE_ERROR, STATE_ERROR, STATE_ERROR,  STATE_ERROR }, // STATE_EQUALS
    { STATE_ERROR,  STATE_ERROR, STATE_ERROR, STATE_ERROR,  STATE_ERROR }  // STATE_ERROR
};

CharClass char_to_class(char c) {
    if (isdigit((unsigned char)c)) return CC_DIGIT;
    if (isalpha((unsigned char)c) || c == '_') return CC_ALPHA;
    if (c == '.') return CC_DOT;
    if (c == '=') return CC_EQUALS;
    return CC_OTHER;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_INT:    return "TOKEN_INT";
        case TOKEN_FLOAT:  return "TOKEN_FLOAT";
        case TOKEN_IDENT:  return "TOKEN_IDENT";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_EQUALS: return "TOKEN_EQUALS";
        case TOKEN_ERROR:  return "TOKEN_ERROR";
        default:           return "TOKEN_EOF";
    }
}

void get_next_token(FILE *fp) {
    int c = fgetc(fp);

    while (c != EOF && isspace(c)) {
        c = fgetc(fp);
    }

    if (c == EOF) return;

    char buffer[256];
    int buf_idx = 0;

    State current_state = STATE_START;
    State last_accepting_state = STATE_ERROR;
    int last_accepting_pos = -1;

    while (c != EOF) {
        CharClass cls = char_to_class((char)c);
        State next_state = DFA[current_state][cls];

        if (next_state == STATE_ERROR) {
            ungetc(c, fp);
            break;
        }

        current_state = next_state;
        buffer[buf_idx++] = (char)c;

        if (current_state == STATE_INT || current_state == STATE_FLOAT ||
            current_state == STATE_IDENT || current_state == STATE_ASSIGN ||
            current_state == STATE_EQUALS) {
            last_accepting_state = current_state;
            last_accepting_pos = buf_idx;
        }

        c = fgetc(fp);
    }

    if (last_accepting_state != STATE_ERROR) {
        while (buf_idx > last_accepting_pos) {
            buf_idx--;
            ungetc(buffer[buf_idx], fp);
        }
        buffer[buf_idx] = '\0';

        TokenType type;
        switch (last_accepting_state) {
            case STATE_INT:    type = TOKEN_INT; break;
            case STATE_FLOAT:  type = TOKEN_FLOAT; break;
            case STATE_IDENT:  type = TOKEN_IDENT; break;
            case STATE_ASSIGN: type = TOKEN_ASSIGN; break;
            case STATE_EQUALS: type = TOKEN_EQUALS; break;
            default:           type = TOKEN_ERROR; break;
        }
        printf("EMIT: %-15s | Lexeme: \"%s\"\n", token_type_to_string(type), buffer);
    } else {
        if (buf_idx == 0 && c != EOF) {
            buffer[0] = (char)fgetc(fp);
            buffer[1] = '\0';
        } else {
            buffer[buf_idx] = '\0';
        }
        printf("EMIT: %-15s | Lexeme: \"%s\"\n", token_type_to_string(TOKEN_ERROR), buffer);
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc > 1) {
        fp = fopen(argv[1], "r");
        if (!fp) {
            perror("File opening failed");
            return EXIT_FAILURE;
        }
    } else {
        fp = tmpfile();
        fputs("var_name = 42 x1 == 3.14 007 0.5 ==", fp);
        rewind(fp);
    }

    printf("--- DFA Lexical Analyzer Output ---\n");
    while (!feof(fp)) {
        get_next_token(fp);
        int ch = fgetc(fp);
        if (ch == EOF) break;
        ungetc(ch, fp);
    }

    fclose(fp);
    return EXIT_SUCCESS;
}

/*
==================== SAMPLE INPUT / OUTPUT ====================
Input: none required from the user. Run as ./assign5 (uses the built-in test string
"var_name = 42 x1 == 3.14 007 0.5 ==").  Optionally: ./assign5 <input_file>

Output:
--- DFA Lexical Analyzer Output ---
EMIT: TOKEN_IDENT     | Lexeme: "var_name"
EMIT: TOKEN_ASSIGN    | Lexeme: "="
EMIT: TOKEN_INT       | Lexeme: "42"
EMIT: TOKEN_IDENT     | Lexeme: "x1"
EMIT: TOKEN_EQUALS    | Lexeme: "=="
EMIT: TOKEN_FLOAT     | Lexeme: "3.14"
EMIT: TOKEN_INT       | Lexeme: "007"
EMIT: TOKEN_FLOAT     | Lexeme: "0.5"
EMIT: TOKEN_EQUALS    | Lexeme: "=="
===============================================================
*/
