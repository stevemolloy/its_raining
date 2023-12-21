#ifndef _MEMO_H
#define _MEMO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <raylib.h>

typedef struct {
  char *title;
  size_t length;
  size_t capacity;
  char **statements;
} QandA;

QandA empty_qanda(void);

void expand_qanda(QandA *qanda);

void append_to_qanda(QandA *qanda, char *statement);

void free_qanda(QandA *qanda);

int parse_string_to_qanda(QandA *qanda, char *text);

size_t space_estimate_for_qanda(QandA qanda);

void get_qanda_string(QandA qanda, char *str, size_t statement_num);

void adjust_string_for_width(char *orig_str, float usable_width, Font font, float fontsize);

size_t string_to_lines(char **string, char ***lines);

char *read_entire_file(char *file_path);

size_t read_entire_file_to_lines(char *file_path, char **buffer, char ***lines);

size_t count_lines(char *contents);

void advance_to_char(char **string, char c);

void parse_lines_to_qanda(QandA *qanda, char **lines, size_t num_lines);

#endif // !_MEMO_H

