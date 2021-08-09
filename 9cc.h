#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Types of Token
typedef enum {
	      TK_RESERVED, // symbols
	      TK_IDENT,    // ID
	      TK_NUM,      // numbers
	      TK_EOF,      // end of input
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;  // Type of Token
  Token *next;     // next Token
  int val;         // if kind is TK_NUM, the number
  char *str;       // token strings
  int len;
};
// types of AST node
typedef enum {
	      ND_ADD, // +
	      ND_SUB, // -
	      ND_MUL, // *
	      ND_DIV, // /
	      ND_EQ,  // ==
	      ND_NE,  // !=
	      ND_LT,  // <
	      ND_LE,  // <=
	      ND_ASSIGN, // =
	      ND_LVAR,  // local variable
	      ND_NUM, // integers
} NodeKind;

typedef struct Node Node;

// AST node
struct Node {
  NodeKind kind; // type of node
  Node *lhs;     // left hand side
  Node *rhs;     // right hand side
  int val;       // only kind=ND_NUM
  int offset;    // only kind=ND_LVAR
};

extern Token *token;
extern char *user_input;
extern Node *code[];

extern void program();
extern Node *expr();
extern void error(char *fmt, ...);
extern void error_at(char *loc, char *fmt, ...);
extern void tokenize();
extern Node *expr();
extern void gen();

