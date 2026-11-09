/*Write programs to build LR(1) parsing tables for the grammar given in the last assignment*/

#include <stdio.h>
#include <string.h>
#define MAX 50
/*
Grammar:
0. E' -> E
1. E -> E + T
2. E -> T
3. T -> id
*/
char lhs[] = {'Z', 'E', 'E', 'T'};
char *rhs[] = {"E", "E+T", "T", "id"};
int nprod = 4;
char terminals[] = {'i', '+', '$'};
char nonterminals[] = {'E', 'T'};
typedef struct {
int prod;
int dot;
char lookahead;
} Item;
typedef struct {
Item item[MAX];
int count;
} State;
State states[MAX];
int nstates = 0;
int action[MAX][3];
int go_to[MAX][2];
int termIndex(char c) {
if (c == 'i') return 0;
if (c == '+') return 1;
if (c == '$') return 2;
return -1;

}
int nontermIndex(char c) {
if (c == 'E') return 0;
if (c == 'T') return 1;
return -1;
}
int rhsLength(int p) {
return strlen(rhs[p]);
}
char symbolAt(Item x) {
if (x.dot >= rhsLength(x.prod))
return '\0';
return rhs[x.prod][x.dot];
}
int sameItem(Item a, Item b) {
return a.prod == b.prod &&
a.dot == b.dot &&
a.lookahead == b.lookahead;
}
int addItem(State *s, Item x) {
for (int i = 0; i < s->count; i++)
if (sameItem(s->item[i], x))
return 0;
s->item[s->count++] = x;
return 1;
}
void closure(State *s) {
int changed;
do {
changed = 0;
for (int i = 0; i < s->count; i++) {
Item x = s->item[i];
char c = symbolAt(x);
if (c == 'E' || c == 'T') {
for (int p = 1; p < nprod; p++) {
if (lhs[p] == c) {
Item y;
y.prod = p;
y.dot = 0;

y.lookahead = x.lookahead;
if (addItem(s, y))
changed = 1;
}
}
}
}
} while (changed);
}
int sameState(State *a, State *b) {
if (a->count != b->count)
return 0;
for (int i = 0; i < a->count; i++) {
int found = 0;
for (int j = 0; j < b->count; j++) {
if (sameItem(a->item[i], b->item[j])) {
found = 1;
break;
}
}
if (!found)
return 0;
}
return 1;
}
int findState(State *s) {
for (int i = 0; i < nstates; i++)
if (sameState(&states[i], s))
return i;
return -1;
}
int goToState(int state, char symbol) {
State next = {0};
for (int i = 0; i < states[state].count; i++) {
Item x = states[state].item[i];
if (symbolAt(x) == symbol) {
x.dot++;
addItem(&next, x);

}
}
if (next.count == 0)
return -1;
closure(&next);
int existing = findState(&next);
if (existing != -1)
return existing;
states[nstates] = next;
return nstates++;
}
void printItem(Item x) {
printf("%c -> ", lhs[x.prod]);
int len = rhsLength(x.prod);
for (int i = 0; i <= len; i++) {
if (i == x.dot)
printf(".");
if (i < len)
printf("%c", rhs[x.prod][i]);
}
printf(", %c", x.lookahead);
}
int main(void) {
memset(action, 0, sizeof(action));
memset(go_to, -1, sizeof(go_to));
states[0].count = 0;
Item start = {0, 0, '$'};
addItem(&states[0], start);
closure(&states[0]);
nstates = 1;
for (int i = 0; i < nstates; i++) {
char symbols[] = {'E', 'T', '+', 'i'};
for (int j = 0; j < 4; j++)

goToState(i, symbols[j]);
}
printf("\nCANONICAL LR(1) ITEM SETS\n");
for (int i = 0; i < nstates; i++) {
printf("\nI%d:\n", i);
for (int j = 0; j < states[i].count; j++) {
printItem(states[i].item[j]);
printf("\n");
}
}
/*
Rebuild transitions to populate ACTION and GOTO.
Action values:
positive = shift to state + 1
negative = reduce by production number
999 = accept
*/
memset(action, 0, sizeof(action));
memset(go_to, -1, sizeof(go_to));
for (int i = 0; i < nstates; i++) {
char symbols[] = {'E', 'T', '+', 'i'};
for (int j = 0; j < 4; j++) {
int dest = goToState(i, symbols[j]);
if (dest == -1)
continue;
int t = termIndex(symbols[j]);
int nt = nontermIndex(symbols[j]);
if (t != -1)
action[i][t] = dest + 1;
if (nt != -1)
go_to[i][nt] = dest;
}
for (int j = 0; j < states[i].count; j++) {
Item x = states[i].item[j];
if (x.dot == rhsLength(x.prod)) {
int t = termIndex(x.lookahead);

if (x.prod == 0 && x.lookahead == '$')
action[i][t] = 999;
else if (t != -1)
action[i][t] = -x.prod;
}
}
}
printf("\nLR(1) PARSING TABLE\n");
printf("\nState\t id\t +\t $\t E\t T\n");
for (int i = 0; i < nstates; i++) {
printf("%d\t", i);
for (int j = 0; j < 3; j++) {
if (action[i][j] == 999)
printf("acc\t");
else if (action[i][j] > 0)
printf("s%d\t", action[i][j] - 1);
else if (action[i][j] < 0)
printf("r%d\t", -action[i][j]);
else
printf("-\t");
}
for (int j = 0; j < 2; j++) {
if (go_to[i][j] == -1)
printf("-\t");
else
printf("%d\t", go_to[i][j]);
}
printf("\n");
}
printf("\nProduction Rules:\n");
printf("0. E' -> E\n");
printf("1. E -> E+T\n");
printf("2. E -> T\n");
printf("3. T -> id\n");
return 0;
}

/*
==================== SAMPLE INPUT / OUTPUT ====================
Input: none required from the user (input is hard-coded in the program).

Output:

CANONICAL LR(1) ITEM SETS

I0:
Z -> .E, $
E -> .E+T, $
E -> .T, $
T -> .id, $

I1:
Z -> E., $
E -> E.+T, $

I2:
E -> T., $

I3:
T -> i.d, $

I4:
E -> E+.T, $
T -> .id, $

I5:
E -> E+T., $

LR(1) PARSING TABLE

State	 id	 +	 $	 E	 T
0	s3	-	-	1	2	
1	-	s4	acc	-	-	
2	-	-	r2	-	-	
3	-	-	-	-	-	
4	s3	-	-	-	5	
5	-	-	r1	-	-	

Production Rules:
0. E' -> E
1. E -> E+T
2. E -> T
3. T -> id
===============================================================
*/
