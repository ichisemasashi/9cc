#include "9cc.h"


int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: Wrong number of args", argv[0]);
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
