/*
Design and implement a C program that simulates an LR(0) Shift-Reduce Parser for a context-free grammar. 
Construct the LR(0) item sets, generate the parsing action/goto table, and trace the step-by-step stack execution of a given input string.

Consider the following augmented context-free grammar 
E' --> E 
E --> E + T 
E --> T 
T --> id 

1. Write a C program to construct the collection of LR(0) items and generate the corresponding LR(0) parsing table containing ACTION (Shift, Reduce, Accept) and GOTO entries.
2. Parse an input string (tokenized as id + id $) using an explicit stack array.
3. Print a step-by-step trace showing current stack states/symbols, remaining input, and the action taken at each step.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RULES 10
#define MAX_SYMBOLS 10
#define MAX_ITEMS 50
#define MAX_STATES 20
#define MAX_STACK 100

// Grammar structures
typedef struct {
char lhs;
char rhs[MAX_SYMBOLS][10];
int rhs_len;
} Production;

typedef struct {
int rule_idx;
int dot_pos;
} Item;

typedef struct {
Item items[MAX_ITEMS];
int count;
} State;

// Global Grammar Definition
Production grammar[MAX_RULES];
int num_rules = 0;

// Terminal and Non-Terminal definitions
char terminals[][10] = {"id", "+", "$"};
int num_terminals = 3;
char non_terminals[] = {'E', 'T'};
int num_non_terminals = 2;

// Parsing Table
// Action: 's' = shift, 'r' = reduce, 'a' = accept, '\0' = error
typedef struct {
char type;
int num;
} ActionEntry;

ActionEntry action_table[MAX_STATES][10];
int goto_table[MAX_STATES][10];

State states[MAX_STATES];
int num_states = 0;

// Utility functions
int is_terminal(const char* sym) {
for (int i = 0; i < num_terminals; i++) {
if (strcmp(terminals[i], sym) == 0) return i;
}
return -1;
}

int is_non_terminal(char sym) {
for (int i = 0; i < num_non_terminals; i++) {
if (non_terminals[i] == sym) return i;
}
return -1;
}

int same_item(Item a, Item b) {
return (a.rule_idx == b.rule_idx && a.dot_pos == b.dot_pos);
}

int add_item(State *s, Item item) {
for (int i = 0; i < s->count; i++) {
if (same_item(s->items[i], item)) return 0;
}
s->items[s->count++] = item;
return 1;
}

// Compute Closure of an Item Set
void closure(State *s) {
int changed = 1;
while (changed) {
changed = 0;
int current_count = s->count;
for (int i = 0; i < current_count; i++) {
Item item = s->items[i];
Production prod = grammar[item.rule_idx];

if (item.dot_pos < prod.rhs_len) {
char *next_sym = prod.rhs[item.dot_pos];
if (strlen(next_sym) == 1 && is_non_terminal(next_sym[0]) != -1) {
char nt = next_sym[0];
for (int j = 0; j < num_rules; j++) {
if (grammar[j].lhs == nt) {
Item new_item = {j, 0};
if (add_item(s, new_item)) {
changed = 1;
}
}
}
}
}
}
}
}

// Compute GOTO for a state and symbol
State goto_state(State s, const char *sym) {
State res;
res.count = 0;
for (int i = 0; i < s.count; i++) {
Item item = s.items[i];
Production prod = grammar[item.rule_idx];
if (item.dot_pos < prod.rhs_len) {
if (strcmp(prod.rhs[item.dot_pos], sym) == 0) {
Item new_item = {item.rule_idx, item.dot_pos + 1};
add_item(&res, new_item);
}
}
}
if (res.count > 0) {
closure(&res);
}
return res;
}

int same_state(State a, State b) {
if (a.count != b.count) return 0;
for (int i = 0; i < a.count; i++) {
int found = 0;
for (int j = 0; j < b.count; j++) {
if (same_item(a.items[i], b.items[j])) {
found = 1;
break;
}
}
if (!found) return 0;
}
return 1;
}

// Construct Canonical Collection of LR(0) Items
void build_canonical_collection() {
// Initial State: Closure of E' -> . E
State start;
start.count = 0;
Item start_item = {0, 0};
add_item(&start, start_item);
closure(&start);

states[0] = start;
num_states = 1;

for (int i = 0; i < num_states; i++) {
// Check transitions for Terminals
for (int t = 0; t < num_terminals; t++) {
if (strcmp(terminals[t], "$") == 0) continue;
State next = goto_state(states[i], terminals[t]);
if (next.count > 0) {
int existing = -1;
for (int k = 0; k < num_states; k++) {
if (same_state(states[k], next)) {
existing = k;
break;
}
}
if (existing == -1) {
existing = num_states;
states[num_states++] = next;
}
action_table[i][t].type = 's';
action_table[i][t].num = existing;
}
}

// Check transitions for Non-Terminals
for (int nt = 0; nt < num_non_terminals; nt++) {
char sym_str[2] = {non_terminals[nt], '\0'};
State next = goto_state(states[i], sym_str);
if (next.count > 0) {
int existing = -1;
for (int k = 0; k < num_states; k++) {
if (same_state(states[k], next)) {
existing = k;
break;
}
}
if (existing == -1) {
existing = num_states;
states[num_states++] = next;
}
goto_table[i][nt] = existing;
} else {
goto_table[i][nt] = -1;
}
}

// Check Reductions and Accept
for (int j = 0; j < states[i].count; j++) {
Item item = states[i].items[j];
Production prod = grammar[item.rule_idx];
if (item.dot_pos == prod.rhs_len) {
if (item.rule_idx == 0) { // E' -> E .
int dollar_idx = is_terminal("$");
action_table[i][dollar_idx].type = 'a';
action_table[i][dollar_idx].num = 0;
} else { // Reduce rule
for (int t = 0; t < num_terminals; t++) {
action_table[i][t].type = 'r';
action_table[i][t].num = item.rule_idx;
}
}
}
}
}
}

void print_grammar() {
printf("--- Augmented Grammar ---\n");
for (int i = 0; i < num_rules; i++) {
printf("%d: %c -> ", i, grammar[i].lhs);
for (int j = 0; j < grammar[i].rhs_len; j++) {
printf("%s ", grammar[i].rhs[j]);
}
printf("\n");
}
printf("\n");
}

void print_parsing_table() {
printf("--- LR(0) Parsing Table ---\n");
printf("%-7s | %-18s | %-12s\n", "State", "ACTION", "GOTO");
printf("%-7s | %-5s %-5s %-5s | %-5s %-5s\n", "", "id", "+", "$", "E", "T");
printf("---------------------------------------------------\n");

for (int i = 0; i < num_states; i++) {
printf("%-7d | ", i);
for (int t = 0; t < num_terminals; t++) {
ActionEntry act = action_table[i][t];
if (act.type == 's') printf("S%-4d ", act.num);
else if (act.type == 'r') printf("R%-4d ", act.num);
else if (act.type == 'a') printf("ACC ");
else printf("%-5s ", "-");
}
printf("| ");
for (int nt = 0; nt < num_non_terminals; nt++) {
if (goto_table[i][nt] != -1) printf("%-5d ", goto_table[i][nt]);
else printf("%-5s ", "-");
}
printf("\n");
}
printf("\n");
}

// Stack execution simulator
void parse_input(char input[][10], int input_len) {
int state_stack[MAX_STACK];
char symbol_stack[MAX_STACK][10];
int top = 0;

state_stack[top] = 0;
strcpy(symbol_stack[top], "");

int ip = 0; // Input pointer

printf("--- Execution Trace ---\n");
printf("%-25s | %-15s | %-20s\n", "Stack (States/Symbols)", "Input Buffer", "Action");
printf("-------------------------------------------------------------------\n");

while (1) {
int current_state = state_stack[top];
char *current_token = input[ip];
int term_idx = is_terminal(current_token);

if (term_idx == -1) {
printf("Error: Invalid token '%s'\n", current_token);
return;
}

// Format Stack String
char stack_str[100] = "";
for (int i = 0; i <= top; i++) {
char buf[15];
if (i == 0) sprintf(buf, "%d", state_stack[i]);
else sprintf(buf, "%s %d", symbol_stack[i], state_stack[i]);
strcat(stack_str, buf);
if (i < top) strcat(stack_str, " ");
}

// Format Input Buffer
char input_str[100] = "";
for (int i = ip; i < input_len; i++) {
strcat(input_str, input[i]);
if (i < input_len - 1) strcat(input_str, " ");
}

ActionEntry act = action_table[current_state][term_idx];

if (act.type == 's') {
printf("%-25s | %-15s | Shift %d\n", stack_str, input_str, act.num);
top++;
state_stack[top] = act.num;
strcpy(symbol_stack[top], current_token);
ip++;
}
else if (act.type == 'r') {
Production prod = grammar[act.num];
char action_desc[50];
sprintf(action_desc, "Reduce %c -> ", prod.lhs);
for (int k = 0; k < prod.rhs_len; k++) strcat(action_desc, prod.rhs[k]);

printf("%-25s | %-15s | %s\n", stack_str, input_str, action_desc);

// Pop |RHS| items
top -= prod.rhs_len;

// Perform GOTO
int goto_state_val = goto_table[state_stack[top]][is_non_terminal(prod.lhs)];
top++;
state_stack[top] = goto_state_val;
symbol_stack[top][0] = prod.lhs;
symbol_stack[top][1] = '\0';
}
else if (act.type == 'a') {
printf("%-25s | %-15s | Accept\n", stack_str, input_str);
printf("\nInput successfully parsed!\n");
return;
}
else {
printf("%-25s | %-15s | Error (Rejected)\n", stack_str, input_str);
return;
}
}
}

int main() {
// Initialize Action and Goto tables
for (int i = 0; i < MAX_STATES; i++) {
for (int j = 0; j < 10; j++) {
action_table[i][j].type = '\0';
goto_table[i][j] = -1;
}
}

// Rule 0: E' -> E
grammar[0].lhs = 'Z'; // 'Z' represents E'
strcpy(grammar[0].rhs[0], "E");
grammar[0].rhs_len = 1;

// Rule 1: E -> E + T
grammar[1].lhs = 'E';
strcpy(grammar[1].rhs[0], "E");
strcpy(grammar[1].rhs[1], "+");
strcpy(grammar[1].rhs[2], "T");
grammar[1].rhs_len = 3;

// Rule 2: E -> T
grammar[2].lhs = 'E';
strcpy(grammar[2].rhs[0], "T");
grammar[2].rhs_len = 1;

// Rule 3: T -> id
grammar[3].lhs = 'T';
strcpy(grammar[3].rhs[0], "id");
grammar[3].rhs_len = 1;

num_rules = 4;

print_grammar();
build_canonical_collection();
print_parsing_table();

// Input string tokens: id + id $
char input_tokens[][10] = {"id", "+", "id", "$"};
int input_len = 4;

parse_input(input_tokens, input_len);

return 0;
}

/*
==================== SAMPLE INPUT / OUTPUT ====================
Input: none required from the user (input is hard-coded in the program).

Output:
--- Augmented Grammar ---
0: Z -> E 
1: E -> E + T 
2: E -> T 
3: T -> id 

--- LR(0) Parsing Table ---
State   | ACTION             | GOTO        
        | id    +     $     | E     T    
---------------------------------------------------
0       | S1    -     -     | 2     3     
1       | R3    R3    R3    | -     -     
2       | -     S4    ACC | -     -     
3       | R2    R2    R2    | -     -     
4       | S1    -     -     | -     5     
5       | R1    R1    R1    | -     -     

--- Execution Trace ---
Stack (States/Symbols)    | Input Buffer    | Action              
-------------------------------------------------------------------
0                         | id + id $       | Shift 1
0 id 1                    | + id $          | Reduce T -> id
0 T 3                     | + id $          | Reduce E -> T
0 E 2                     | + id $          | Shift 4
0 E 2 + 4                 | id $            | Shift 1
0 E 2 + 4 id 1            | $               | Reduce T -> id
0 E 2 + 4 T 5             | $               | Reduce E -> E+T
0 E 2                     | $               | Accept

Input successfully parsed!
===============================================================
*/
