/*
Q2> Write routines: compute_first (grammar), compute_follow (grammar, first_sets) and generate_ll-1_table(grammar, first_sets, follow_sets) that builds up a lookup table indexed by [Non-Terminal][Terminal].

Integrate your routines in a single program and validate your output for the following grammar:
E -> E + T | T 
T -> T * F | F 
F -> id | ( E )
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NT 20
#define MAX_PROD 50
#define MAX_LEN 50
#define MAX_TERMINALS 30

typedef struct {
    char lhs;
    char rhs[MAX_LEN];
} Production;

typedef struct {
    char nonterminals[MAX_NT];
    int nt_count;

    char terminals[MAX_TERMINALS];
    int t_count;

    Production productions[MAX_PROD];
    int p_count;
} Grammar;

int is_nonterminal(Grammar *g, char c) {
    for (int i = 0; i < g->nt_count; i++)
        if (g->nonterminals[i] == c)
            return 1;

    return 0;
}

int terminal_index(Grammar *g, char c) {
    for (int i = 0; i < g->t_count; i++)
        if (g->terminals[i] == c)
            return i;

    return -1;
}

int nt_index(Grammar *g, char c) {
    for (int i = 0; i < g->nt_count; i++)
        if (g->nonterminals[i] == c)
            return i;

    return -1;
}

void add_terminal(Grammar *g, char c) {
    if (terminal_index(g, c) == -1)
        g->terminals[g->t_count++] = c;
}

void compute_first(Grammar *g, int first[MAX_NT][MAX_TERMINALS],
                   int epsilon[MAX_NT]) {

    int changed = 1;

    while (changed) {
        changed = 0;

        for (int p = 0; p < g->p_count; p++) {
            int A = nt_index(g, g->productions[p].lhs);
            char *rhs = g->productions[p].rhs;

            int all_epsilon = 1;

            if (strcmp(rhs, "#") == 0) {
                if (!epsilon[A]) {
                    epsilon[A] = 1;
                    changed = 1;
                }
                continue;
            }

            for (int i = 0; rhs[i]; i++) {
                char X = rhs[i];

                if (!is_nonterminal(g, X)) {
                    int t = terminal_index(g, X);

                    if (!first[A][t]) {
                        first[A][t] = 1;
                        changed = 1;
                    }

                    all_epsilon = 0;
                    break;
                }

                int B = nt_index(g, X);

                for (int t = 0; t < g->t_count; t++) {
                    if (first[B][t] && !first[A][t]) {
                        first[A][t] = 1;
                        changed = 1;
                    }
                }

                if (!epsilon[B]) {
                    all_epsilon = 0;
                    break;
                }
            }

            if (all_epsilon && !epsilon[A]) {
                epsilon[A] = 1;
                changed = 1;
            }
        }
    }
}

void first_of_string(Grammar *g, char *str,
                     int first[MAX_NT][MAX_TERMINALS],
                     int epsilon[MAX_NT],
                     int result[MAX_TERMINALS],
                     int *has_epsilon) {

    *has_epsilon = 1;

    for (int i = 0; str[i]; i++) {
        char X = str[i];

        if (!is_nonterminal(g, X)) {
            int t = terminal_index(g, X);
            result[t] = 1;
            *has_epsilon = 0;
            return;
        }

        int B = nt_index(g, X);

        for (int t = 0; t < g->t_count; t++)
            if (first[B][t])
                result[t] = 1;

        if (!epsilon[B]) {
            *has_epsilon = 0;
            return;
        }
    }
}

void compute_follow(Grammar *g,
                    int first[MAX_NT][MAX_TERMINALS],
                    int epsilon[MAX_NT],
                    int follow[MAX_NT][MAX_TERMINALS]) {

    int dollar = terminal_index(g, '$');
    int start = nt_index(g, g->productions[0].lhs);

    follow[start][dollar] = 1;

    int changed = 1;

    while (changed) {
        changed = 0;

        for (int p = 0; p < g->p_count; p++) {
            int A = nt_index(g, g->productions[p].lhs);
            char *rhs = g->productions[p].rhs;

            for (int i = 0; rhs[i]; i++) {
                if (!is_nonterminal(g, rhs[i]))
                    continue;

                int B = nt_index(g, rhs[i]);

                char suffix[MAX_LEN];
                strcpy(suffix, rhs + i + 1);

                int temp[MAX_TERMINALS] = {0};
                int has_epsilon;

                first_of_string(g, suffix, first, epsilon,
                                temp, &has_epsilon);

                for (int t = 0; t < g->t_count; t++) {
                    if (temp[t] && !follow[B][t]) {
                        follow[B][t] = 1;
                        changed = 1;
                    }
                }

                if (has_epsilon || suffix[0] == '\0') {
                    for (int t = 0; t < g->t_count; t++) {
                        if (follow[A][t] && !follow[B][t]) {
                            follow[B][t] = 1;
                            changed = 1;
                        }
                    }
                }
            }
        }
    }
}

void generate_ll1_table(
    Grammar *g,
    int first[MAX_NT][MAX_TERMINALS],
    int epsilon[MAX_NT],
    int follow[MAX_NT][MAX_TERMINALS],
    int table[MAX_NT][MAX_TERMINALS]) {

    for (int i = 0; i < MAX_NT; i++)
        for (int j = 0; j < MAX_TERMINALS; j++)
            table[i][j] = -1;

    for (int p = 0; p < g->p_count; p++) {
        int A = nt_index(g, g->productions[p].lhs);

        int result[MAX_TERMINALS] = {0};
        int has_epsilon;

        first_of_string(
            g,
            g->productions[p].rhs,
            first,
            epsilon,
            result,
            &has_epsilon
        );

        for (int t = 0; t < g->t_count; t++) {
            if (result[t])
                table[A][t] = p;
        }

        if (has_epsilon) {
            for (int t = 0; t < g->t_count; t++) {
                if (follow[A][t])
                    table[A][t] = p;
            }
        }
    }
}

void print_first_follow(
    Grammar *g,
    int first[MAX_NT][MAX_TERMINALS],
    int epsilon[MAX_NT],
    int follow[MAX_NT][MAX_TERMINALS]) {

    printf("\nFIRST sets:\n");

    for (int i = 0; i < g->nt_count; i++) {
        printf("FIRST(%c) = { ", g->nonterminals[i]);

        for (int t = 0; t < g->t_count; t++)
            if (first[i][t])
                printf("%c ", g->terminals[t]);

        if (epsilon[i])
            printf("epsilon ");

        printf("}\n");
    }

    printf("\nFOLLOW sets:\n");

    for (int i = 0; i < g->nt_count; i++) {
        printf("FOLLOW(%c) = { ", g->nonterminals[i]);

        for (int t = 0; t < g->t_count; t++)
            if (follow[i][t])
                printf("%c ", g->terminals[t]);

        printf("}\n");
    }
}

void print_table(Grammar *g, int table[MAX_NT][MAX_TERMINALS]) {

    printf("\nLL(1) Parsing Table:\n\n");

    printf("%-10s", "");

    for (int t = 0; t < g->t_count; t++)
        printf("%-15c", g->terminals[t]);

    printf("\n");

    for (int i = 0; i < g->nt_count; i++) {

        printf("%-10c", g->nonterminals[i]);

        for (int t = 0; t < g->t_count; t++) {

            int p = table[i][t];

            if (p == -1)
                printf("%-15s", "ERROR");
            else
                printf("%-15s", g->productions[p].rhs);
        }

        printf("\n");
    }
}

int main() {

    Grammar g = {0};

    g.nonterminals[0] = 'E';
    g.nonterminals[1] = 'R';
    g.nonterminals[2] = 'T';
    g.nonterminals[3] = 'Y';
    g.nonterminals[4] = 'F';
    g.nt_count = 5;

    add_terminal(&g, '+');
    add_terminal(&g, '*');
    add_terminal(&g, '(');
    add_terminal(&g, ')');
    add_terminal(&g, 'i');
    add_terminal(&g, '$');

    g.productions[g.p_count++] = (Production){'E', "TR"};
    g.productions[g.p_count++] = (Production){'R', "+TR"};
    g.productions[g.p_count++] = (Production){'R', "#"};
    g.productions[g.p_count++] = (Production){'T', "FY"};
    g.productions[g.p_count++] = (Production){'Y', "*FY"};
    g.productions[g.p_count++] = (Production){'Y', "#"};
    g.productions[g.p_count++] = (Production){'F', "i"};
    g.productions[g.p_count++] = (Production){'F', "(E)"};

    int first[MAX_NT][MAX_TERMINALS] = {0};
    int follow[MAX_NT][MAX_TERMINALS] = {0};
    int epsilon[MAX_NT] = {0};
    int table[MAX_NT][MAX_TERMINALS];

    compute_first(&g, first, epsilon);

    compute_follow(&g, first, epsilon, follow);

    generate_ll1_table(
        &g,
        first,
        epsilon,
        follow,
        table
    );

    print_first_follow(&g, first, epsilon, follow);

    print_table(&g, table);

    return 0;
}

/*
==================== SAMPLE INPUT / OUTPUT ====================
Input: none required from the user (input is hard-coded in the program).

Output:

FIRST sets:
FIRST(E) = { ( i }
FIRST(R) = { + epsilon }
FIRST(T) = { ( i }
FIRST(Y) = { * epsilon }
FIRST(F) = { ( i }

FOLLOW sets:
FOLLOW(E) = { ) $ }
FOLLOW(R) = { ) $ }
FOLLOW(T) = { + ) $ }
FOLLOW(Y) = { + ) $ }
FOLLOW(F) = { + * ) $ }

LL(1) Parsing Table:

          +              *              (              )              i              $              
E         ERROR          ERROR          TR             ERROR          TR             ERROR          
R         +TR            ERROR          ERROR          ERROR          ERROR          ERROR          
T         ERROR          ERROR          FY             ERROR          FY             ERROR          
Y         ERROR          *FY            ERROR          ERROR          ERROR          ERROR          
F         ERROR          ERROR          (E)            ERROR          i              ERROR          
===============================================================
*/
