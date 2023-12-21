#include "raylib.h"
#define INITIAL_QA_COUNT 64
#include <assert.h>
#include <errno.h>

#include "memo.h"

size_t read_entire_file_to_lines(char *file_path, char **buffer, char ***lines) {
  *buffer = read_entire_file(file_path);
  size_t num_lines = string_to_lines(buffer, lines);
  return num_lines;
}

char *read_entire_file(char *file_path) {
  // Reads an entire file into a char array, and returns a ptr to this. The ptr should be freed by the caller
  FILE *f = fopen(file_path, "r");
  if (f==NULL) {
    fprintf(stderr, "Could not read %s: %s\n", file_path, strerror(errno));
    exit(1);
  }

  fseek(f, 0L, SEEK_END);
  int sz = ftell(f);
  fseek(f, 0L, SEEK_SET);

  char *contents = calloc(2*sz, sizeof(char));
  if (contents==NULL) {
    fprintf(stderr, "Could not allocate memory. Buy more RAM I guess?\n");
    exit(1);
  }
  fread(contents, 1, sz, f);

  fclose(f);
  
  return contents;
}

size_t string_to_lines(char **string, char ***lines) {
  char *cursor = *string;
  size_t num_lines = count_lines(cursor);

  *lines = calloc(num_lines, sizeof(char*));

  size_t line_ctr = 0;
  while (*cursor) {
    (*lines)[line_ctr] = cursor;
    assert(line_ctr < num_lines);
    advance_to_char(&cursor, '\n');
    *cursor = '\0';
    (cursor)++;
    line_ctr++;
  }

  return num_lines;
}

size_t count_lines(char *contents) {
  size_t result = 0;
  while (*contents) {
    if (*contents == '\n') result++;
    contents++;
  }
  return result;
}

void advance_to_char(char **string, char c) {
  while (**string != c) (*string)++;
}

QandA empty_qanda(void) {
  QandA result = {0};
  result.capacity = INITIAL_QA_COUNT;
  result.statements = calloc(INITIAL_QA_COUNT, sizeof(char*));

  if (result.statements==NULL) {
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
    exit(1);
  }

  return result;
}

size_t space_estimate_for_qanda(QandA qanda) {
  size_t result = 0;
  for (size_t i=0; i<qanda.length; i++) {
    result += 2 * strlen(qanda.statements[i]);
  }
  return result;
}

void expand_qanda(QandA *qanda) {
  qanda->capacity *= 2;
  qanda->statements = realloc(qanda->statements, qanda->capacity*sizeof(char*));
  if (qanda->statements==NULL) {
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
    exit(1);
  }
}

void parse_lines_to_qanda(QandA *qanda, char **lines, size_t num_lines) {
  qanda->title = lines[0];
  for (size_t i=1; i<num_lines; i++) {
    if (strlen(lines[i]) == 0) {
      continue;
    }
    append_to_qanda(qanda, lines[i]);
  }
}

void append_to_qanda(QandA *qanda, char *statement) {
  while (qanda->length >= qanda->capacity) expand_qanda(qanda);

  qanda->statements[qanda->length] = statement;
  qanda->length++;
}

void free_qanda(QandA *qanda) {
  for (size_t i=0; i<qanda->length; i++) {
    free(qanda->statements[i]);
  }
  free(qanda->statements);
}

/*
int parse_string_to_qanda(QandA *qanda, char *text) {
  char *cursor = text;
  char *title = text;
  size_t len = 0;

  while (strncmp("\n", cursor, 1) != 0) cursor++;
  len = cursor - title;
  char *t = calloc(len, sizeof(char));
  if (t==NULL) {
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
    exit(1);
  }
  memcpy(t, title, len);

  qanda->title = t;

  while (strncmp("\nQ:", cursor, 3) !=0 ) cursor++;
    // cursor now points at "Q:".  Advance it by two places and then search for "A:"

  while (1) {
    cursor += 3;
    while (*cursor==' ') cursor++;
    char *question = cursor;

    while (strncmp("\nA:", cursor, 3) !=0 ) cursor++;
    // cursor now points at the "A:" following the question

    len = cursor-question;
    char *q = calloc(len, sizeof(char));
    if (q==NULL) {
      fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
      exit(1);
    }
    memcpy(q, question, len);

    cursor += 3;
    while (*cursor==' ') cursor++;
    char *answer = cursor;
    while (*cursor != '\0' && strncmp("\nQ:", cursor, 3) !=0 ) cursor++;
    // cursor now points at the "Q:" following the answer or the null terminator
    
    len = cursor - answer;
    char *a = calloc(len, sizeof(char));
    if (a==NULL) {
      fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
      exit(1);
    }
    while (answer[len-1]=='\n') len--; // Because lines can end with multiple newlines
    memcpy(a, answer, len);

    append_to_qanda(qanda, statement);

    if (*cursor=='\0') break;
  }

  return 1;
}
*/

void get_qanda_string(QandA qanda, char *str, size_t statement_num) {
  strcpy(str, "\0");
  for (size_t i=0; i<statement_num; i++) {
    strcat(str, qanda.statements[i]);
    if (i != statement_num-1) strcat(str, "\n\n");
  }
  strcat(str, "\0");
} 

void adjust_string_for_width(char *orig_str, float usable_width, Font font, float fontsize) {
  Vector2 text_size = MeasureTextEx(font, orig_str, fontsize, 0);
  if (text_size.x <= usable_width) return;

  // Split into words
  size_t num_chars = strlen(orig_str);
  for (size_t i=0; i<num_chars; i++) {
    if (orig_str[i] == ' ') {
      orig_str[i] = '\n';
    }
  }

  for (size_t i=0; i<strlen(orig_str); i++) {
    if (orig_str[i] == '\n') {
      if (num_chars - i > 4 && orig_str[i+1] == '\n' && (orig_str[i+2] == 'Q' || orig_str[i+2] == 'A') && orig_str[i+3] == ':') {
        i += 3;
        continue;
      }
      orig_str[i] = ' ';
      Vector2 size = MeasureTextEx(font, orig_str, fontsize, 0);
      if (size.x > usable_width) {
        orig_str[i] = '\n';
      }
    }
  }
  
  return;
}

