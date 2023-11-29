#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <string.h>

#define WIDTH 800
#define HEIGHT 600

#define INITIAL_QA_COUNT 40

typedef struct {
  size_t length;
  size_t capacity;
  char **questions;
  char **answers;
} QandA;

QandA empty_qanda(void) {
  QandA result = {0};
  result.capacity = INITIAL_QA_COUNT;
  result.questions = calloc(INITIAL_QA_COUNT, sizeof(char*));
  result.answers   = calloc(INITIAL_QA_COUNT, sizeof(char*));

  if (result.questions==NULL || result.answers==NULL) {
    fprintf(stderr, "Unable to allocate memory. Buy more RAM I guess?\n");
    exit(1);
  }

  return result;
}

void expand_qanda(QandA *qanda) {
  qanda->capacity *= 2;
  qanda->questions = realloc(qanda->questions, qanda->capacity);
  qanda->answers   = realloc(qanda->answers, qanda->capacity);
  if (qanda->questions==NULL || qanda->answers==NULL) {
    fprintf(stderr, "Unable to allocate memory. Buy more RAM I guess?\n");
    exit(1);
  }
}

void append_to_qanda(QandA *qanda, char *question, char *answer) {
  while (qanda->length >= qanda->capacity) expand_qanda(qanda);

  qanda->questions[qanda->length] = question;
  qanda->answers[qanda->length]   = answer;
  qanda->length++;
}

void free_qanda(QandA *qanda) {
  for (size_t i=0; i<qanda->length; i++) {
    free(qanda->questions[i]);
    free(qanda->answers[i]);
  }
  free(qanda->questions);
  free(qanda->answers);
}

int parse_string_to_qanda(QandA *qanda, char *text) {
  char *cursor = text;
  while (strncmp("Q:", cursor, 2) !=0 ) cursor++;
    // cursor now points at "Q:".  Advance it by two places and then search for "A:"

    while (1) {
    cursor += 2;
    while (*cursor==' ') cursor++;
    char *question = cursor;

    while (strncmp("A:", cursor, 2) !=0 ) cursor++;
    // cursor now points at the "A:" following the question

    size_t q_len = cursor-question;
    char *q = calloc(q_len, sizeof(char));
    memcpy(q, question, q_len);
    q[q_len-1] = '\0';
    printf("Question: %s\n", q);

    cursor += 2;
    while (*cursor==' ') cursor++;
    char *answer = cursor;
    while (*cursor != '\0' && strncmp("Q:", cursor, 2) !=0 ) cursor++;
    // cursor now points at the "Q:" following the answer or the null terminator
    
    size_t a_len = cursor - answer;
    char *a = calloc(a_len, sizeof(char));
    memcpy(a, answer, a_len);
    a[a_len-1] = '\0';
    printf("Answer: %s\n", a);

    append_to_qanda(qanda, q, a);

    if (*cursor=='\0') break;
  }

  return 1;
}

int main(void) {
  const char* catechism_file = "pretend_catechism.txt";
  char *file_contents = LoadFileText(catechism_file);
  if (strlen(file_contents)<7) {
    fprintf(stderr, "File %s does not use the correct format\n", catechism_file);
    exit(1);
  }

  QandA qanda = empty_qanda();
  parse_string_to_qanda(&qanda, file_contents);

  // InitWindow(WIDTH, HEIGHT, "It's raining...");
  // SetWindowState(FLAG_WINDOW_RESIZABLE);
  // SetTargetFPS(60);
  //
  // while (!WindowShouldClose()) {
  //
  //   BeginDrawing();
  //   ClearBackground(WHITE);
  //   EndDrawing();
  // }

  UnloadFileText(file_contents);

  return 0;
}
