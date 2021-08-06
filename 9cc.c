#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Types of Token
typedef enum {
	      TK_RESERVED, // symbols
	      TK_NUM,      // numbers
	      TK_EOF,      // end of input
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;  // Type of Token
  Token *next;     // next Token
  int val;         // if kind is TK_NUM, the number
  char *str;       // token strings
};
// types of AST node
typedef enum {
	      ND_ADD, // +
	      ND_SUB, // -
	      ND_MUL, // *
	      ND_DIV, // /
	      ND_NUM, // integers
} NodeKind;

typedef struct Node Node;

// AST node
struct Node {
  NodeKind kind; // type of node
  Node *lhs;     // left hand side
  Node *rhs;     // right hand side
  int val;       // only kind=ND_NUM
};

extern Node *expr();


Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs  = lhs;
  node->rhs  = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val  = val;
  return node;
}
// Current token
Token *token;

/*
   function that report error
   take the same args of 'printf'
*/
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// input program strings
char *user_input;

/*
  report the spot of error
 */
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos x ' '
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}


/*
   if next token is the symbol of expected, token++ and return true.
   otherwise return false.
 */
bool consume(char op) {
  if (token->kind != TK_RESERVED ||
      token->str[0] != op) {
    return false;
  }

  token = token->next;
  return true;
}

/*
  if next token is the symbol of expected, token++.
  otherwise report error.
 */
void expect(char op) {
  if (token->kind != TK_RESERVED ||
      token->str[0] != op)  {
    error_at(token->str, "NOT '%c'", op);
  }

  token = token->next;
}

/*
  if next token is a number, token++ and return the number.
  otherwise report error.
 */
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "Not a number");
  }

  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}


/*
  make new Token and concat cur
 */
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str  = str;
  cur->next = tok;
  return tok;
}

/*
  tokenize input strings p and return it.
 */
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // skip white and spaces
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "Cannot Tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

/*
  BNF
  expr = mul ("+" mul | "-" mul)*
  mul  = primary ("*" primary | "/" primary)*
  primary = num | "(" expr ")"
 */

Node *primary() {
  if (consume('(')) {
      Node *node = expr();
      expect(')');
      return node;
    } else {
      return new_node_num(expect_number());
    }
}


Node *mul() {
  Node *node = primary();

  for (;;) {
    if (consume('*')) {
      node = new_node(ND_MUL, node, primary());
    } else if (consume('/')) {
      node = new_node(ND_DIV, node, primary());
    } else {
      return node;
    }
  }
}

Node *expr(){
  Node *node = mul();

  for (;;) {
    if (consume('+')) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume('-')) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Wrong number of args\n");
    return 1;
  }

  user_input = argv[1];
  
  // Tokenize
  token = tokenize();
  Node *node = expr();
  
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // code generate down to AST
  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
