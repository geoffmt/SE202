#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

__attribute__((noreturn))
static void error(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

void __print_err(const char *s) {
  fprintf(stderr, "%s\n", s);
}

void __print(const char *s) {
  fprintf(stdout, "%s\n", s);
}

void __print_int(const int32_t i) {
  printf("%d", i);
}

void __flush(void) {
  fflush(stdout);
}

const char *__getchar(void) {
  char * s = (char *) malloc(sizeof(char));
  *s = getchar();
  if(*s == EOF){
    return "";
  }
  return s;  
}

int32_t __ord(const char *s){
  unsigned char c = *s;
  if(c == '\0'){
    return -1;
  }
  return (int32_t) c;
}

const char *__chr(int32_t i) {
  char * s = (char *) malloc(sizeof(char));
  if (i == 0) {
    return "";
  }
  if (i<0 || i>255){
    error("char out of range [0;255]");
  }
  *s = (char) i%128;
  return s;
}

int32_t __size(const char *s) {
  int32_t i = 0;
  while (s[i] != '\0') {
    i++;
    if (i == 0) {
      error("Maximal size reached.");
    }
  }
  return i;
}

const char *__substring(const char *s, int32_t first, int32_t length) {
  error("UNIMPLEMENTED __substring");
}

const char *__concat(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __concat");
}

int32_t __strcmp(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __strcmp");
}

int32_t __streq(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __streq");
}

int32_t __not(int32_t i) {
  error("UNIMPLEMENTED __not");
}

void __exit(int32_t c) {
  error("UNIMPLEMENTED __exit");
}
