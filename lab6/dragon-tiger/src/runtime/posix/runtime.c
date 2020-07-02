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
  char * substr = (char *) malloc(sizeof(char) * (length + 1));
  int32_t size = __size(s); // get length the string containing substring
  uint32_t idx = 0;

  if (first < 0 || length < 0 || (first + length) > size){
    error("arguments incorrect or out of bounds.");
  }

  while (idx < length) {
    substr[idx] = s[first + idx];
    idx++;
  }

  return substr;
}

const char *__concat(const char *s1, const char *s2) {
  int32_t size_1 = __size(s1);
  int32_t size_2 = __size(s2);
  int32_t size_total = size_1 + size_2;
  int32_t idx = 0;
  char * s3 = (char *) malloc(sizeof(char)*(size_total + 1));

  while (idx < size_1) {
    s3[idx] = s1[idx];
    idx++;
  }

  while (idx < size_total) {
    s3[idx] = s2[idx - size_1];
    idx++;
  }

  return s3;
}

int32_t __strcmp(const char *s1, const char *s2) {
  int cmp = strcmp(s1, s2);

  if (cmp < 0) {
    return -1;
  } else if (cmp > 0) {
    return 1;
  } else {
    return 0;
  }
}

int32_t __streq(const char *s1, const char *s2) {
  int32_t res = strcmp(s1, s2);
  return (res == 0);
}

int32_t __not(int32_t i) {
  return !i;
}

void __exit(int32_t c) {
  exit(c);
}
