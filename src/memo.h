#ifndef _MEMO_H
#define _MEMO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
  char *title;
  size_t length;
  size_t capacity;
  char **questions;
  char **answers;
  size_t output_length;
} QandA;

QandA empty_qanda(void);

void expand_qanda(QandA *qanda);

void append_to_qanda(QandA *qanda, char *question, char *answer);

void free_qanda(QandA *qanda);

int parse_string_to_qanda(QandA *qanda, char *text);

void get_qanda_string(QandA qanda, char *str, size_t q_num, bool inc_answer);

#endif // !_MEMO_H

