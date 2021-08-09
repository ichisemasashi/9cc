#include "9cc.h"

static Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs  = lhs;
  node->rhs  = rhs;
  return node;
}

static Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
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
static bool consume(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }

  token = token->next;
  return true;
}

/*
  if next token is the symbol of expected, token++.
  otherwise report error.
 */
static void expect(char *op) {
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    error_at(token->str, "expected \"%s\"", op);
  }

  token = token->next;
}

/*
  if next token is a number, token++ and return the number.
  otherwise report error.
 */
static int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "Not a number");
  }

  int val = token->val;
  token = token->next;
  return val;
}
/* 
static bool at_eof() {
  return token->kind == TK_EOF;
}
*/

/*
  make new Token and concat cur
 */
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str  = str;
  tok->len  = len;
  cur->next = tok;
  return tok;
}

static bool startswith(char *p, char *q) {
  return (memcmp(p, q, strlen(q)) == 0);
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

    if (startswith(p, "==") ||
	startswith(p, "!=") ||
	startswith(p, "<=") ||
	startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "Cannot Tokenize");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

/*
  BNF
  expr = equality
  equality = relational ("==" rerational | "!=" relational)*
  relational = add ("<" add | "<=" add | ">" add | ">=" add)*
  add = mul ("+" mul | "-" mul)*
  mul  = unary ("*" unary | "/" unary)*
  unary = ("+" | "-")? primary
  primary = num | "(" expr ")"
 */
static Node *primary() {
  if (consume("(")) {
      Node *node = expr();
      expect(")");
      return node;
    } else {
      return new_num(expect_number());
    }
}

static Node *unary() {
  if (consume("+")) {
    return unary();
  } else if (consume("-")) {
    return new_binary(ND_SUB, new_num(0), unary());
  } else {
    return primary();
  }
}

static Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_binary(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_binary(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}
static Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_binary(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_binary(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}


static Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<")) {
      node = new_binary(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_binary(ND_LE, node, add());
    } else if (consume(">")) {
      node = new_binary(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_binary(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

static Node *equality() {
  Node *node = relational();

  for(;;) {
    if (consume("==")) {
      node = new_binary(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_binary(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

Node *expr() {
  return equality();
}
