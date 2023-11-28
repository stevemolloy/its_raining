#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

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

int main(void) {
  const char* catechism_file = "pretend_catechism.txt";
  char *file_contents = LoadFileText(catechism_file);

  InitWindow(WIDTH, HEIGHT, "It's raining...");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  while (!WindowShouldClose()) {

    BeginDrawing();
    ClearBackground(WHITE);
    EndDrawing();
  }

  UnloadFileText(file_contents);

  return 0;
}
