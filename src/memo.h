#ifndef _MEMO_H
#define _MEMO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <raylib.h>

#define GCRY_CIPHER GCRY_CIPHER_AES256
#define GCRY_MODE GCRY_CIPHER_MODE_CBC

#define KEY_LENGTH 32 // 256 bits
#define IV_LENGTH 16  // 128 bits

#define MAX_PASSWD_CHARS 12

typedef enum {
  WAITING_FOR_FILE,
  GOT_FILE,
  WAITING_FOR_PASSWD,
  CHECKING_PASSWD,
  BUILDING_FILE,
  DISPLAYING_FILE,
} StateName;

typedef struct {
  char *title;
  size_t length;
  size_t capacity;
  char **statements;
} QandA;

typedef struct {
  StateName state;
  char *file_path;
  char *buffer;
  char **lines;
  size_t reveal_statement_num;
  char* string_to_print;
  size_t num_lines;
  QandA qanda;
  float scroll_location;
  float scroll_speed;
  char *prompt_text;
  Font font;
} AppState;

typedef struct {
  char passwd[MAX_PASSWD_CHARS + 1];
  int lettercount;
} PasswordDetails;

void handle_error(const char *msg);

bool is_file_encrypted(char *filepath);

char *decrypt_file(const char *input_filename, const char *password);

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

