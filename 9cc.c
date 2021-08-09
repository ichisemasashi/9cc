#include "9cc.h"


int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: Wrong number of args", argv[0]);
  }

  user_input = argv[1];
  
  // Tokenize
  tokenize();
  program();

  // assenbry header
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // prologue: get 26*8 memory for variables
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // code generate down to AST
  gen();

  // epiloge: return the last val on RAX.
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
