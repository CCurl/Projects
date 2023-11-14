/* file: "tc.c" */
/* originally from http://www.iro.umontreal.ca/~felipe/IFT2030-Automne2002/Complements/tinyc.c */
/* Copyright (C) 2001 by Marc Feeley, All Rights Reserved. */
/* enhanced by Chris Curl */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * This is a compiler for the Tiny-C language.  Tiny-C is a
 * considerably stripped down version of C and it is meant as a
 * pedagogical tool for learning about compilers.  The integer global
 * variables "a" to "z" are predefined and initialized to zero, and it
 * is not possible to declare new variables.  The compiler reads the
 * program from standard input and prints out the value of the
 * variables that are not zero.  The grammar of Tiny-C in EBNF is:
 *
 *  <program> ::= <statement>
 *  <statement> ::= "if" <paren_expr> <statement> |
 *                  "if" <paren_expr> <statement> "else" <statement> |
 *                  "while" <paren_expr> <statement> |
 *                  "do" <statement> "while" <paren_expr> ";" |
 *                  "{" { <statement> } "}" |
 *                  "def" <id> "{" <statement> "}" |
 *                  "val" <id> ";" |
 *                  "void" <id> "(" ")" "{" <statement> "}" |
 *                  <expr> ";" |
 *                  ";"
 *  <paren_expr> ::= "(" <expr> ")"
 *  <expr> ::= <test> | <id> "=" <expr>
 *  <test> ::= <math> | <math> "<" <math> | <math> ">" <math>
 *  <math> ::= <term> | <math> <math_op> <term>
 *  <math_op> ::= "+" | "-" | "*" | "/"
 *  <term> ::= <id> | <unt> | <paren_expr> | <func>
 *  <id> ::= "a" | "b" | "c" | "d" | ... | "z"
 *  <int> ::= <an_unsigned_decimal_integer>
 *  <func> ::= <id> "(" ")" ";"
 *
 * Here are a few invocations of the compiler:
 *
 * % echo "a=b=c=2<3;" | ./a.out
 * a = 1
 * b = 1
 * c = 1
 * % echo "{ i=1; while (i<100) i=i+i; }" | ./a.out
 * i = 128
 * % echo "{ i=125; j=100; while (i-j) if (i<j) j=j-i; else i=i-j; }" | ./a.out
 * i = 25
 * j = 25
 * % echo "{ i=1; do i=i+10; while (i<50); }" | ./a.out
 * i = 51
 * % echo "{ i=1; while ((i=i+10)<50) ; }" | ./a.out
 * i = 51
 * % echo "{ i=7; if (i<5) x=1; if (i<10) y=2; }" | ./a.out
 * i = 7
 * y = 2
 *
 * The compiler does a minimal amount of error checking to help
 * highlight the structure of the compiler.
 */

/*---------------------------------------------------------------------------*/
/* Lexer. */

#define BTWI(n,l,h) ((l<=n)&&(n<=h))
enum { DO_SYM, ELSE_SYM, IF_SYM, WHILE_SYM, LBRA, RBRA, LPAR, RPAR,
       PLUS, MINUS, STAR, SLASH, LESS, GRT, SEMI, EQUAL, INT, ID, EOI };

char *words[] = { "do", "else", "if", "while", NULL };

int ch = ' ';
int sym;
int int_val;
char id_name[100];
FILE *input_fp = NULL;

void message(char *msg) { fprintf(stderr, "%s\n", msg); }
void error(char *err) { message(err); exit(1); }
void syntax_error() { error("-syntax error-"); }

void next_ch() {
    if (input_fp) { ch = fgetc(input_fp); }
    else { ch = getchar(); }
    // printf("%c", ch);
}

void next_sym() {
  again:
    switch (ch) {
      case ' ': case 9: case 10: case 13: next_ch(); goto again;
      case EOF: sym = EOI; break;
      case '{': next_ch(); sym = LBRA;  break;
      case '}': next_ch(); sym = RBRA;  break;
      case '(': next_ch(); sym = LPAR;  break;
      case ')': next_ch(); sym = RPAR;  break;
      case '+': next_ch(); sym = PLUS;  break;
      case '-': next_ch(); sym = MINUS; break;
      case '*': next_ch(); sym = STAR;  break;
      case '/': next_ch(); sym = SLASH; break;
      case '<': next_ch(); sym = LESS;  break;
      case '>': next_ch(); sym = GRT;   break;
      case ';': next_ch(); sym = SEMI;  break;
      case '=': next_ch(); sym = EQUAL; break;
      default:
        if (BTWI(ch,'0','9')) {
            int_val = 0; /* missing overflow check */
            while (BTWI(ch,'0','9')) {
                int_val = int_val*10 + (ch - '0'); next_ch(); }
            sym = INT;
        } else if (BTWI(ch,'a','z')) {
            int i = 0; /* missing overflow check */
            while (BTWI(ch,'a','z') || ch == '_') { id_name[i++]=ch; next_ch(); }
            id_name[i] = '\0';
            sym = 0;
            while (words[sym] != NULL && strcmp(words[sym], id_name) != 0) { sym++; }
            if (words[sym] == NULL) {
              if (id_name[1] == '\0') { sym = ID; } // a-z only for now
              else { syntax_error(); }
            }
        } else { syntax_error(); }
    }
    // printf("-[sym:%d]-",sym);
}

/*---------------------------------------------------------------------------*/
/* Parser. */

enum { VAR, CST, ADD, SUB, MUL, DIV, LT, GT, SET,
       IF1, IF2, WHILE, DO, EMPTY, SEQ, EXPR, PROG };

struct node_s { int kind; struct node_s *o1, *o2, *o3; int val; };
typedef struct node_s node;
int num_nodes = 0;
node nodes[1000];

node *new_node(int k) {
    node *x = &nodes[num_nodes++];
    x->kind = k;
    return x;
}

node *gen(int k, node *o1, node *o2) {
    node *x=new_node(k);
    x->o1=o1; x->o2=o2;
    return x;
}

void expect_sym(int exp) {
    if (sym != exp) { syntax_error(); }
    next_sym();
}

node *paren_expr(); /* forward declaration */

/* <term> ::= <id> | <int> | <paren_expr> */
node *term() {
  node *x;
  if (sym == ID) { x=new_node(VAR); x->val=id_name[0]-'a'; next_sym(); }
  else if (sym == INT) { x=new_node(CST); x->val=int_val; next_sym(); }
  else x = paren_expr();
  return x;
}

int mathop() {
    if (sym==PLUS) { return ADD; }
    else if (sym==MINUS) { return SUB; }
    else if (sym==STAR)  { return MUL; }
    else if (sym==SLASH) { return DIV; }
    return 0;
}

/* <math> ::= <term> | <math> <math_op> <term> */
/* <math_op> ::= "+" | "-" | "*" | "/" */
node *sum() {
  node *x = term();
  while (mathop()) {
    x=gen(mathop(), x, 0);
    next_sym();
    x->o2=term();
  }
  return x;
}

/* <test> ::= <math> | <math> "<" <math> | <math> ">" <math> */
node *test() {
  node *x = sum();
  if (sym == LESS) { next_sym(); return gen(LT, x, sum()); }
  if (sym == GRT)  { next_sym(); return gen(GT, x, sum()); }
  return x;
}

/* <expr> ::= <test> | <id> "=" <expr> */
node *expr() {
  node *x;
  if (sym != ID) return test();
  x = test();
  if ((x->kind==VAR) && (sym==EQUAL)) { next_sym(); return gen(SET, x, expr()); }
  return x;
}

/* <paren_expr> ::= "(" <expr> ")" */
node *paren_expr() {
  node *x;
  expect_sym(LPAR);
  x = expr();
  expect_sym(RPAR);
  return x;
}

node *statement() {
  node *x;
  if (sym == IF_SYM) { /* "if" <paren_expr> <statement> */
      x = new_node(IF1);
      next_sym();
      x->o1 = paren_expr();
      x->o2 = statement();
      if (sym == ELSE_SYM) { /* ... "else" <statement> */
          x->kind = IF2;
          next_sym();
          x->o3 = statement();
      }
  } else if (sym == WHILE_SYM) { /* "while" <paren_expr> <statement> */
     x=new_node(WHILE);
     next_sym();
     x->o1=paren_expr();
     x->o2=statement();
  } else if (sym == DO_SYM) { /* "do" <statement> "while" <paren_expr> ";" */
      x = new_node(DO);
      next_sym();
      x->o1 = statement();
      expect_sym(WHILE_SYM);
      x->o2 = paren_expr();
      expect_sym(SEMI);
  } else if (sym == SEMI) { /* ";" */
      x = new_node(EMPTY);
      next_sym();
  } else if (sym == LBRA) { /* "{" { <statement> } "}" */
      x = new_node(EMPTY);
      next_sym();
      while (sym != RBRA) {
        x =gen(SEQ, x, 0);
        x->o2=statement();
      }
      next_sym();
  } else { /* <expr> ";" */
      x = gen(EXPR, expr(), NULL);
      expect_sym(SEMI);
  }
  return x;
}

/* <program> ::= <statement> */
node *program() {
  next_sym();
  node *x = gen(PROG, statement(), NULL);
  if (sym != EOI) syntax_error();
  return x;
}

/*---------------------------------------------------------------------------*/
/* Code generator. */

enum { IFETCH, ISTORE, IPUSH, IDROP, IADD, ISUB, IMUL, IDIV, ILT, IGT, JZ, JNZ, JMP, HALT };

typedef char code;
code object[1000], *here = object;

void g(code c) { *here++ = c; } /* missing overflow check */
code *hole() { return here++; }
void fix(code *src, code *dst) { *src = dst-src; } /* missing overflow check */

void c(node *x) {
  code *p1, *p2;
  switch (x->kind)
    { case VAR  : g(IFETCH); g(x->val); break;
      case CST  : g(IPUSH); g(x->val); break;
      case ADD  : c(x->o1); c(x->o2); g(IADD); break;
      case MUL  : c(x->o1); c(x->o2); g(IMUL); break;
      case SUB  : c(x->o1); c(x->o2); g(ISUB); break;
      case DIV  : c(x->o1); c(x->o2); g(IDIV); break;
      case LT   : c(x->o1); c(x->o2); g(ILT); break;
      case GT   : c(x->o1); c(x->o2); g(IGT); break;
      case SET  : c(x->o2); g(ISTORE); g(x->o1->val); break;
      case IF1  : c(x->o1); g(JZ); p1=hole(); c(x->o2); fix(p1,here); break;
      case IF2  : c(x->o1); g(JZ); p1=hole(); c(x->o2); g(JMP); p2=hole();
                  fix(p1,here); c(x->o3); fix(p2,here); break;
      case WHILE: p1=here; c(x->o1); g(JZ); p2=hole(); c(x->o2);
                  g(JMP); fix(hole(),p1); fix(p2,here); break;
      case DO   : p1=here; c(x->o1); c(x->o2); g(JNZ); fix(hole(),p1); break;
      case EMPTY: break;
      case SEQ  : c(x->o1); c(x->o2); break;
      case EXPR : c(x->o1); g(IDROP); break;
      case PROG : c(x->o1); g(HALT);  break;
    }
}

/*---------------------------------------------------------------------------*/
/* Virtual machine. */

int globals[26], sp;

#define ACASE    goto again; case
#define TOS      st[sp]
#define NOS      st[sp-1]

void run(code *pc) {
  long st[1000];
  again:
  switch (*pc++) {
      case IFETCH : st[++sp] = globals[*pc++];
      ACASE ISTORE: globals[*pc++] = st[sp];
      ACASE IPUSH : st[++sp] = *pc++;
      ACASE IDROP : --sp;
      ACASE IADD  : NOS += TOS; --sp;
      ACASE ISUB  : NOS -= TOS; --sp;
      ACASE IMUL  : NOS *= TOS; --sp;
      ACASE IDIV  : NOS /= TOS; --sp;
      ACASE ILT   : NOS =  NOS<TOS; --sp;
      ACASE IGT   : NOS =  NOS>TOS; --sp;
      ACASE JMP   : pc += *pc;
      ACASE JZ    : if (st[sp--] == 0) pc += *pc; else pc++;
      ACASE JNZ   : if (st[sp--] != 0) pc += *pc; else pc++;
      ACASE HALT  : return;
  }
}

/*---------------------------------------------------------------------------*/
/* Main program. */

int main(int argc, char *argv[]) {
  if (argc>1) { input_fp = fopen(argv[1], "rt"); }
  c(program());

  printf("(nodes: %d, ", num_nodes);
  printf("code: %ld bytes)\n", (int)(here-&object[0]));

  sp=0;
  for (int i=0; i<26; i++) { globals[i] = 0; }
  run(object);
  for (int i=0; i<26; i++) {
    if (globals[i] != 0) printf("%c = %d\n", 'a'+i, globals[i]);
  }
  if (sp) { error("-stack not empty-"); }
  return 0;
}
