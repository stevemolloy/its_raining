#define INITIAL_QA_COUNT 40

#include "memo.h"

QandA empty_qanda(void) {
  QandA result = {0};
  result.capacity = INITIAL_QA_COUNT;
  result.questions = calloc(INITIAL_QA_COUNT, sizeof(char*));
  result.answers   = calloc(INITIAL_QA_COUNT, sizeof(char*));

  if (result.questions==NULL || result.answers==NULL) {
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
    exit(1);
  }

  return result;
}

void expand_qanda(QandA *qanda) {
  qanda->capacity *= 2;
  qanda->questions = realloc(qanda->questions, qanda->capacity*sizeof(char*));
  qanda->answers   = realloc(qanda->answers, qanda->capacity*sizeof(char*));
  if (qanda->questions==NULL || qanda->answers==NULL) {
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
    exit(1);
  }
}

void append_to_qanda(QandA *qanda, char *question, char *answer) {
  while (qanda->length >= qanda->capacity) expand_qanda(qanda);

  qanda->questions[qanda->length] = question;
  qanda->answers[qanda->length]   = answer;
  qanda->output_length += 3 + strlen(question) + 1 + 3 + strlen(answer) + 3; // "Q: " + '\n' + "A: " + '\n' + '\n' + '\0'
  qanda->length++;
}

void free_qanda(QandA *qanda) {
  for (size_t i=0; i<qanda->length; i++) {
    free(qanda->questions[i]);
    free(qanda->answers[i]);
  }
  free(qanda->questions);
  free(qanda->answers);
  free(qanda->title);
}

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

    append_to_qanda(qanda, q, a);

    if (*cursor=='\0') break;
  }

  return 1;
}

void get_qanda_string(QandA qanda, char *str, size_t q_num, bool inc_answer) {
  strcpy(str, "\0");
  strcat(str, "Q: ");
  strcat(str, qanda.questions[q_num]);
  if (!inc_answer) return;
  strcat(str, "\n");
  strcat(str, "A: ");
  strcat(str, qanda.answers[q_num]);
} 

