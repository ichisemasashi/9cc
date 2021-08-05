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
    error("NOT '%c'", op);
  }

  token = token->next;
}

/*
  if next token is a number, token++ and return the number.
  otherwise report error.
 */
int expect_number() {
  if (token->kind != TK_NUM) {
    error("Not a number");
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
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // skip white and spaces
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("Cannot Tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Wrong number of args\n");
    return 1;
  }

  // Tokenize
  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  
  printf("  mov rax, %d\n", expect_number());

  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
