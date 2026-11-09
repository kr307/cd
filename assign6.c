/*
Q1>Write an function transform_grammar (garmmar) to:
a) Eliminate Immediate Left Recursion for all non-terminals.
b) Apply Left Factoring to resolve common prefixes in production rules.
{The function must return a completely transformed grammar object with dynamically generated new non-terminal names, ensuring no left recursion remains and all shared prefixes are separated.}
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RHS 100
#define MAX_LEN 100
#define MAX_NT 100

typedef struct {
    char lhs[20];
    int count;
    char rhs[MAX_RHS][MAX_LEN];
} Production;

typedef struct {
    Production p[MAX_NT];
    int count;
} Grammar;

void add_production(Grammar *g, const char *lhs, const char *rhs) {
    for (int i = 0; i < g->count; i++) {
        if (strcmp(g->p[i].lhs, lhs) == 0) {
            strcpy(g->p[i].rhs[g->p[i].count++], rhs);
            return;
        }
    }

    strcpy(g->p[g->count].lhs, lhs);
    g->p[g->count].count = 1;
    strcpy(g->p[g->count].rhs[0], rhs);
    g->count++;
}

int nonterminal_exists(Grammar *g, const char *name) {
    for (int i = 0; i < g->count; i++) {
        if (strcmp(g->p[i].lhs, name) == 0)
            return 1;
    }
    return 0;
}

void generate_new_nt(Grammar *g, char *base, char *newNT) {
    int k = 1;

    do {
        snprintf(newNT, 20, "%s_%d", base, k++);
    } while (nonterminal_exists(g, newNT));
}

int common_prefix_len(const char *a, const char *b) {
    int i = 0;

    while (a[i] && b[i] && a[i] == b[i])
        i++;

    return i;
}

void eliminate_left_recursion(Grammar *g) {

    int original_count = g->count;

    for (int i = 0; i < original_count; i++) {

        char lhs[20];
        strcpy(lhs, g->p[i].lhs);

        char alpha[MAX_RHS][MAX_LEN];
        char beta[MAX_RHS][MAX_LEN];

        int alpha_count = 0;
        int beta_count = 0;

        int lhs_len = strlen(lhs);

        for (int j = 0; j < g->p[i].count; j++) {

            if (strncmp(g->p[i].rhs[j], lhs, lhs_len) == 0) {

                const char *rest = g->p[i].rhs[j] + lhs_len;

                if (*rest != '\0') {
                    strcpy(alpha[alpha_count++], rest);
                } else {
                    strcpy(beta[beta_count++], "epsilon");
                }

            } else {
                strcpy(beta[beta_count++], g->p[i].rhs[j]);
            }
        }

        if (alpha_count == 0)
            continue;

        char newNT[20];
        generate_new_nt(g, lhs, newNT);

        g->p[i].count = 0;

        for (int j = 0; j < beta_count; j++) {

            char temp[MAX_LEN];

            if (strcmp(beta[j], "epsilon") == 0) {
                snprintf(temp, MAX_LEN, "%s", newNT);
            } else {
                snprintf(temp, MAX_LEN, "%s %s",
                         beta[j], newNT);
            }

            strcpy(g->p[i].rhs[g->p[i].count++], temp);
        }

        strcpy(g->p[g->count].lhs, newNT);
        g->p[g->count].count = 0;

        for (int j = 0; j < alpha_count; j++) {

            char temp[MAX_LEN];

            snprintf(temp, MAX_LEN, "%s %s",
                     alpha[j], newNT);

            strcpy(g->p[g->count].rhs[
                       g->p[g->count].count++],
                   temp);
        }

        strcpy(g->p[g->count].rhs[
                   g->p[g->count].count++],
               "epsilon");

        g->count++;
    }
}

void left_factor(Grammar *g) {

    int changed = 1;

    while (changed) {

        changed = 0;

        for (int i = 0; i < g->count; i++) {

            int best = 0;
            int best_index = -1;

            for (int j = 0; j < g->p[i].count; j++) {

                for (int k = j + 1;
                     k < g->p[i].count;
                     k++) {

                    int len = common_prefix_len(
                        g->p[i].rhs[j],
                        g->p[i].rhs[k]);

                    if (len > best) {
                        best = len;
                        best_index = j;
                    }
                }
            }

            if (best == 0)
                continue;

            char prefix[MAX_LEN];

            strncpy(prefix,
                    g->p[i].rhs[best_index],
                    best);

            prefix[best] = '\0';

            char newNT[20];

            generate_new_nt(
                g,
                g->p[i].lhs,
                newNT);

            Production newProduction;

            strcpy(newProduction.lhs, newNT);
            newProduction.count = 0;

            int old_count = g->p[i].count;

            char old_rhs[MAX_RHS][MAX_LEN];

            for (int j = 0; j < old_count; j++) {
                strcpy(old_rhs[j], g->p[i].rhs[j]);
            }

            g->p[i].count = 0;

            for (int j = 0; j < old_count; j++) {

                if (strncmp(old_rhs[j], prefix, best) == 0) {

                    char suffix[MAX_LEN];

                    strcpy(suffix,
                           old_rhs[j] + best);

                    if (strlen(suffix) == 0)
                        strcpy(suffix, "epsilon");

                    strcpy(
                        newProduction.rhs[
                            newProduction.count++],
                        suffix);

                } else {

                    strcpy(
                        g->p[i].rhs[
                            g->p[i].count++],
                        old_rhs[j]);
                }
            }

            char factored[MAX_LEN];

            snprintf(
                factored,
                MAX_LEN,
                "%s %s",
                prefix,
                newNT);

            strcpy(
                g->p[i].rhs[
                    g->p[i].count++],
                factored);

            g->p[g->count++] = newProduction;

            changed = 1;
            break;
        }
    }
}

Grammar transform_grammar(Grammar g) {

    eliminate_left_recursion(&g);

    left_factor(&g);

    return g;
}

void print_grammar(Grammar *g) {

    for (int i = 0; i < g->count; i++) {

        printf("%s -> ", g->p[i].lhs);

        for (int j = 0; j < g->p[i].count; j++) {

            printf("%s", g->p[i].rhs[j]);

            if (j != g->p[i].count - 1)
                printf(" | ");
        }

        printf("\n");
    }
}

int main() {

    Grammar g = {0};

    int n;

    printf("Enter number of non-terminals: ");
    scanf("%d", &n);
    getchar();   /* consume leftover newline after scanf (once) */

    printf("Enter productions in the form:\n");
    printf("E -> E+T | T\n\n");

    for (int i = 0; i < n; i++) {

        char line[MAX_LEN];

        fgets(line, MAX_LEN, stdin);

        line[strcspn(line, "\n")] = '\0';

        char lhs[20];

        sscanf(line, "%s", lhs);

        char *arrow = strstr(line, "->");

        if (arrow == NULL)
            continue;

        arrow += 2;

        char *token = strtok(arrow, "|");

        while (token != NULL) {

            while (*token == ' ')
                token++;

            char rhs[MAX_LEN];

            strcpy(rhs, token);

            size_t len = strlen(rhs);

            while (len > 0 &&
                   rhs[len - 1] == ' ') {

                rhs[len - 1] = '\0';
                len--;
            }

            add_production(&g, lhs, rhs);

            token = strtok(NULL, "|");
        }
    }

    Grammar transformed =
        transform_grammar(g);

    printf("\nTransformed Grammar:\n");

    print_grammar(&transformed);

    return 0;
}

/*
==================== SAMPLE INPUT / OUTPUT ====================
User input (typed at the prompts):
  3
  E -> E+T | T
  T -> T*F | F
  F -> id | (E)

Output:
Enter number of non-terminals: 3
Enter productions in the form:
E -> E+T | T

E -> E+T | T
T -> T*F | F
F -> id | (E)

Transformed Grammar:
E -> T E_1
T -> F T_1
F -> id | (E)
E_1 -> +T E_1 | epsilon
T_1 -> *F T_1 | epsilon
===============================================================
*/
