/*
include the steps you would use to find and resolve L-R or R-R conflicts. Check whether the grammar provided is SLR(1).
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_RULES 10
#define MAX_RHS 4
#define MAX_ITEMS 50
#define MAX_STATES 20
#define MAX_STACK 100
#define MAX_TOKENS 3
#define MAX_NTS 2

typedef struct { char lhs; char rhs[MAX_RHS][10]; int len; } Production;
typedef struct { int rule, dot; } Item;
typedef struct { Item items[MAX_ITEMS]; int count; } State;
typedef struct { char type; int num; } Action;

Production g[MAX_RULES]; int nrules = 4;
const char *terms[MAX_TOKENS] = {"id", "+", "$"};
const char nts[MAX_NTS] = {'E','T'};
State states[MAX_STATES]; int nstates = 0;
Action action[MAX_STATES][MAX_TOKENS];
int go_to[MAX_STATES][MAX_NTS];
int conflicts = 0;

int term_index(const char *s) { for (int i=0;i<MAX_TOKENS;i++) if (!strcmp(s,terms[i])) return i; return -1; }
int nt_index(char c) { for (int i=0;i<MAX_NTS;i++) if (nts[i]==c) return i; return -1; }
int same_item(Item a, Item b) { return a.rule==b.rule && a.dot==b.dot; }
int add_item(State *s, Item it) { for(int i=0;i<s->count;i++) if(same_item(s->items[i],it)) return 0; if(s->count<MAX_ITEMS) s->items[s->count++]=it; return 1; }

void closure(State *s) {
    int changed=1;
    while(changed) {
        changed=0;
        int n=s->count;
        for(int i=0;i<n;i++) {
            Item it=s->items[i]; Production p=g[it.rule];
            if(it.dot<p.len && strlen(p.rhs[it.dot])==1) {
                char c=p.rhs[it.dot][0];
                if(nt_index(c)>=0) for(int r=0;r<nrules;r++) if(g[r].lhs==c) {
                    Item ni={r,0}; if(add_item(s,ni)) changed=1;
                }
            }
        }
    }
}

State goto_state(State s, const char *sym) {
    State out={0};
    for(int i=0;i<s.count;i++) {
        Item it=s.items[i]; Production p=g[it.rule];
        if(it.dot<p.len && !strcmp(p.rhs[it.dot],sym)) { Item ni={it.rule,it.dot+1}; add_item(&out,ni); }
    }
    if(out.count) closure(&out);
    return out;
}
int same_state(State a, State b) {
    if(a.count!=b.count) return 0;
    for(int i=0;i<a.count;i++) { int found=0; for(int j=0;j<b.count;j++) if(same_item(a.items[i],b.items[j])) {found=1;break;} if(!found)return 0; }
    return 1;
}
int find_state(State s) { for(int i=0;i<nstates;i++) if(same_state(states[i],s)) return i; return -1; }

/* Returns FOLLOW(A) membership for this grammar. */
int in_follow(char lhs, const char *terminal) {
    if(lhs=='E' || lhs=='T') return !strcmp(terminal,"+") || !strcmp(terminal,"$");
    return 0;
}
void set_action(int st, int t, char type, int num) {
    Action *a=&action[st][t];
    if(a->type=='\0') { a->type=type; a->num=num; return; }
    if(a->type==type && a->num==num) return;
    conflicts++;
    printf("CONFLICT in state %d on '%s': ",st,terms[t]);
    if(a->type=='s') printf("Shift %d",a->num); else if(a->type=='r') printf("Reduce by rule %d",a->num); else printf("Accept");
    printf(" versus ");
    if(type=='s') printf("Shift %d",num); else if(type=='r') printf("Reduce by rule %d",num); else printf("Accept");
    printf("\n");
}

void build_table(void) {
    State start={0}; Item first={0,0}; add_item(&start,first); closure(&start);
    states[0]=start; nstates=1;
    for(int i=0;i<MAX_STATES;i++) for(int t=0;t<MAX_TOKENS;t++) action[i][t]=(Action){0,0};
    for(int i=0;i<MAX_STATES;i++) for(int n=0;n<MAX_NTS;n++) go_to[i][n]=-1;

    for(int i=0;i<nstates;i++) {
        for(int t=0;t<2;t++) {
            State next=goto_state(states[i],terms[t]);
            if(next.count) {
                int dest=find_state(next);
                if(dest<0) { if(nstates>=MAX_STATES) {fprintf(stderr,"Too many states\n"); exit(1);} dest=nstates; states[nstates++]=next; }
                set_action(i,t,'s',dest);
            }
        }
        for(int n=0;n<MAX_NTS;n++) {
            char sym[2]={nts[n],'\0'}; State next=goto_state(states[i],sym);
            if(next.count) {
                int dest=find_state(next);
                if(dest<0) { if(nstates>=MAX_STATES) {fprintf(stderr,"Too many states\n"); exit(1);} dest=nstates; states[nstates++]=next; }
                go_to[i][n]=dest;
            }
        }
        for(int j=0;j<states[i].count;j++) {
            Item it=states[i].items[j]; Production p=g[it.rule];
            if(it.dot==p.len) {
                if(it.rule==0) set_action(i,term_index("$"),'a',0);
                else for(int t=0;t<MAX_TOKENS;t++) if(in_follow(p.lhs,terms[t])) set_action(i,t,'r',it.rule);
            }
        }
    }
}

void print_grammar(void) {
    printf("--- Augmented Grammar ---\n");
    for(int r=0;r<nrules;r++) {
        if(r==0) printf("0: E' ->"); else printf("%d: %c ->",r,g[r].lhs);
        for(int j=0;j<g[r].len;j++) printf(" %s",g[r].rhs[j]);
        printf("\n");
    }
    printf("\nFOLLOW(E) = { +, $ }\nFOLLOW(T) = { +, $ }\n\n");
}
void print_states(void) {
    printf("--- Canonical LR(0) Item Sets ---\n");
    for(int i=0;i<nstates;i++) {
        printf("I%d:\n",i);
        for(int j=0;j<states[i].count;j++) {
            Item it=states[i].items[j]; Production p=g[it.rule];
            if(it.rule==0) printf("  E' ->"); else printf("  %c ->",p.lhs);
            for(int k=0;k<=p.len;k++) {
                if(k==it.dot) printf(" .");
                if(k<p.len) printf(" %s",p.rhs[k]);
            }
            printf("\n");
        }
    }
    printf("\n");
}
void print_table(void) {
    printf("--- SLR(1) Parsing Table ---\n");
    printf("State |   id   |   +    |   $    ||  E  |  T\n");
    printf("------------------------------------------------\n");
    for(int i=0;i<nstates;i++) {
        printf("%5d |",i);
        for(int t=0;t<MAX_TOKENS;t++) {
            Action a=action[i][t]; char b[16]="-";
            if(a.type=='s') snprintf(b,sizeof b,"s%d",a.num);
            else if(a.type=='r') snprintf(b,sizeof b,"r%d",a.num);
            else if(a.type=='a') snprintf(b,sizeof b,"accept");
            printf(" %6s |",b);
        }
        printf("|");
        for(int n=0;n<MAX_NTS;n++) { if(go_to[i][n]>=0) printf(" %3d |",go_to[i][n]); else printf("   - |"); }
        printf("\n");
    }
    printf("\n");
}
void parse(const char input[][10], int n) {
    int st[MAX_STACK], top=0, ip=0; char syms[MAX_STACK][10];
    st[0]=0; syms[0][0]='\0';
    printf("--- Parsing Trace ---\n");
    printf("%-24s | %-15s | %s\n","Stack","Input","Action");
    printf("-----------------------------------------------------------------------\n");
    while(ip<n) {
        int ti=term_index(input[ip]);
        if(ti<0) { printf("Invalid token: %s\n",input[ip]); return; }
        char stackbuf[256]="", inbuf[128]="";
        for(int i=0;i<=top;i++) { char b[32]; if(i==0) snprintf(b,sizeof b,"%d",st[i]); else snprintf(b,sizeof b," %s %d",syms[i],st[i]); strncat(stackbuf,b,sizeof(stackbuf)-strlen(stackbuf)-1); }
        for(int i=ip;i<n;i++) { if(i>ip) strcat(inbuf," "); strcat(inbuf,input[i]); }
        Action a=action[st[top]][ti];
        if(a.type=='s') {
            printf("%-24s | %-15s | Shift %d\n",stackbuf,inbuf,a.num);
            if(top+1>=MAX_STACK) {puts("Stack overflow");return;}
            top++; st[top]=a.num; strcpy(syms[top],input[ip]); ip++;
        } else if(a.type=='r') {
            Production p=g[a.num]; char desc[80]; snprintf(desc,sizeof desc,"Reduce %c ->",p.lhs);
            for(int k=0;k<p.len;k++) { strcat(desc," "); strcat(desc,p.rhs[k]); }
            printf("%-24s | %-15s | %s\n",stackbuf,inbuf,desc);
            top-=p.len; if(top<0) {puts("Parser stack error");return;}
            int ni=nt_index(p.lhs); int dest=go_to[st[top]][ni];
            if(dest<0) {puts("Missing GOTO entry");return;}
            top++; st[top]=dest; syms[top][0]=p.lhs; syms[top][1]='\0';
        } else if(a.type=='a') { printf("%-24s | %-15s | Accept\n",stackbuf,inbuf); puts("\nInput successfully parsed!"); return; }
        else { printf("%-24s | %-15s | Error (Rejected)\n",stackbuf,inbuf); puts("Input rejected."); return; }
    }
    puts("Input ended without '$'; rejected.");
}
int main(void) {
    /* Rule 0 uses Z to represent the augmented start symbol E'. */
    g[0]=(Production){'Z',{{"E"}},1};
    g[1]=(Production){'E',{{"E"},{"+"},{"T"}},3};
    g[2]=(Production){'E',{{"T"}},1};
    g[3]=(Production){'T',{{"id"}},1};
    print_grammar(); build_table(); print_states(); print_table();
    if(conflicts==0) puts("Conflict check: No shift-reduce or reduce-reduce conflicts. Grammar is SLR(1).\n");
    else printf("Conflict check: %d conflict(s) found; grammar is not SLR(1) with this table.\n\n",conflicts);
    const char input[][10]={"id","+","id","$"};
    parse(input,4);
    return 0;
}

/*
==================== SAMPLE INPUT / OUTPUT ====================
Input: none required from the user (input is hard-coded in the program).

Output:
--- Augmented Grammar ---
0: E' -> E
1: E -> E + T
2: E -> T
3: T -> id

FOLLOW(E) = { +, $ }
FOLLOW(T) = { +, $ }

--- Canonical LR(0) Item Sets ---
I0:
  E' -> . E
  E -> . E + T
  E -> . T
  T -> . id
I1:
  T -> id .
I2:
  E' -> E .
  E -> E . + T
I3:
  E -> T .
I4:
  E -> E + . T
  T -> . id
I5:
  E -> E + T .

--- SLR(1) Parsing Table ---
State |   id   |   +    |   $    ||  E  |  T
------------------------------------------------
    0 |     s1 |      - |      - ||   2 |   3 |
    1 |      - |     r3 |     r3 ||   - |   - |
    2 |      - |     s4 | accept ||   - |   - |
    3 |      - |     r2 |     r2 ||   - |   - |
    4 |     s1 |      - |      - ||   - |   5 |
    5 |      - |     r1 |     r1 ||   - |   - |

Conflict check: No shift-reduce or reduce-reduce conflicts. Grammar is SLR(1).

--- Parsing Trace ---
Stack                    | Input           | Action
-----------------------------------------------------------------------
0                        | id + id $       | Shift 1
0 id 1                   | + id $          | Reduce T -> id
0 T 3                    | + id $          | Reduce E -> T
0 E 2                    | + id $          | Shift 4
0 E 2 + 4                | id $            | Shift 1
0 E 2 + 4 id 1           | $               | Reduce T -> id
0 E 2 + 4 T 5            | $               | Reduce E -> E + T
0 E 2                    | $               | Accept

Input successfully parsed!
===============================================================
*/
