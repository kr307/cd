/*
Imagine you are building a compiler for a simple programming language with the following token class specifications:

    Identifiers consist of letters (a-z, A-Z) and digits (0-9) and are case-sensitive.
    Keywords: if, else, while, int.
    Operators: +, -, *, /.
    Parentheses: (, ).

Your task is to write a tokenizer that follows the maximal munch principle. 

Write a shell script or a program in C that takes an input program as a string and outputs the sequence of tokens. For simplicity, you can assume that there are spaces between tokens.

If the Input is: 
if (x = 1) {
    y = y + 1;
} else {
    y = y - 1;
}

The output should be: 
if, (, x, =, 1, ), {, y, =, y, +, 1, ;, }, else, {, y, =, y, -, 1, ;, }
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main() {
    const char *input = "if (x = 1) {\n    y = y + 1;\n} else {\n    y = y - 1;\n}";
    char token[256];
    int first = 1;
    int i = 0;

    while (input[i] != '\0') {
        // Skip whitespace
        if (isspace(input[i])) {
            i++;
            continue;
        }

        int t_len = 0;

        // Identifiers, Keywords, or Numbers (alphanumeric block)
        if (isalnum(input[i])) {
            while (isalnum(input[i])) {
                token[t_len++] = input[i++];
            }
        } 
        // Single-character tokens (Operators, Delimiters, Braces)
        else {
            token[t_len++] = input[i++];
        }

        token[t_len] = '\0';

        // Print formatted tokens with comma separation
        if (!first) {
            printf(", ");
        }
        printf("%s", token);
        first = 0;
    }

    printf("\n");
    return 0;
}

/*
==================== SAMPLE INPUT / OUTPUT ====================
Input: none required from the user (input is hard-coded in the program).

Output:
if, (, x, =, 1, ), {, y, =, y, +, 1, ;, }, else, {, y, =, y, -, 1, ;, }
===============================================================
*/
