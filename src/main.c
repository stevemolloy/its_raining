#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define WIDTH 800
#define HEIGHT 600
#define FONTSIZE 32
#define BACKGROUND_COLOUR CLITERAL(Color){ 0, 128, 255, 255 }
#define PADDING 10
#define TITLEHEIGHT 60
#define CONTROLSHEIGHT 60

#define INITIAL_QA_COUNT 40

typedef struct {
  char *title;
  size_t length;
  size_t capacity;
  char **questions;
  char **answers;
  size_t output_length;
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
    memcpy(q, question, len);

    cursor += 3;
    while (*cursor==' ') cursor++;
    char *answer = cursor;
    while (*cursor != '\0' && strncmp("\nQ:", cursor, 3) !=0 ) cursor++;
    // cursor now points at the "Q:" following the answer or the null terminator
    
    len = cursor - answer;
    char *a = calloc(len, sizeof(char));
    while (answer[len]=='\n') len--; // Because lines can end with multiple newlines
    while (answer[len-1]=='\n') len--; // Because lines can end with multiple newlines
    memcpy(a, answer, len);

    append_to_qanda(qanda, q, a);

    if (*cursor=='\0') break;
  }

  return 1;
}

void get_qanda_string(QandA qanda, char *str, size_t num_qs, bool inc_last_answer) {
  strcpy(str, "\0");
  for (size_t i=0; i<num_qs; i++) {
    strcat(str, "Q: ");
    strcat(str, *(qanda.questions + i));
    strcat(str, "\n");
    if (i==num_qs-1 && !inc_last_answer) continue;
    strcat(str, "A: ");
    strcat(str, *(qanda.answers + i));
    strcat(str, "\n");
  }
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

  char* string_to_print = calloc(qanda.output_length, sizeof(char));

  InitWindow(WIDTH, HEIGHT, "It's raining...");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  Font font = LoadFontEx("fonts/NotoSans-Regular.ttf", FONTSIZE, NULL, 0);
  GuiSetFont(font);

  Rectangle text_box_bounds = {.x=PADDING, .y=PADDING+TITLEHEIGHT, .width=WIDTH-2*PADDING, .height=HEIGHT-2*PADDING-CONTROLSHEIGHT-PADDING-TITLEHEIGHT };
  // Rectangle control_bounds  = {.x=PADDING, .y=HEIGHT-CONTROLSHEIGHT-PADDING, .width=WIDTH-2*PADDING, .height=CONTROLSHEIGHT };

  GuiSetStyle(DEFAULT, TEXT_SIZE, FONTSIZE);
  GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, 24);
  GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xEEEEEEFF);
  GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD);   // WARNING: If wrap mode enabled, text editing is not supported
  GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);
  GuiSetStyle(BUTTON, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
  GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0x000000FF);

  size_t reveal_q_num = 0;
  while (!WindowShouldClose()) {
    get_qanda_string(qanda, string_to_print, reveal_q_num, true);
    Rectangle advance_btn_rect = {.x=PADDING, .y=HEIGHT-CONTROLSHEIGHT-PADDING, .width=150, .height=CONTROLSHEIGHT};
    Rectangle decr_btn_rect    = {.x=PADDING*2+150, .y=HEIGHT-CONTROLSHEIGHT-PADDING, .width=150, .height=CONTROLSHEIGHT};
    Rectangle reset_btn_rect   = {.x=HEIGHT-CONTROLSHEIGHT, .y=HEIGHT-CONTROLSHEIGHT-PADDING, .width=150, .height=CONTROLSHEIGHT};
    Rectangle title_box_rect   = {.x=PADDING, .y=PADDING, .width=WIDTH-2*PADDING, .height=TITLEHEIGHT};

    char *title = qanda.title;

    BeginDrawing();
      ClearBackground(BACKGROUND_COLOUR);
      GuiTextBox(title_box_rect, title, 1000, false);
      GuiTextBox(text_box_bounds, string_to_print, 100000, false);
      if (GuiButton(advance_btn_rect, "Next")) {
        reveal_q_num++;
        if (reveal_q_num>qanda.length) reveal_q_num = qanda.length;
      }
      if (GuiButton(decr_btn_rect, "Back")) {
        if (reveal_q_num>0) reveal_q_num--;
      }
      if (GuiButton(reset_btn_rect, "Reset")) {
        reveal_q_num = 0;
      }
    EndDrawing();
  }

  free(string_to_print);
  free_qanda(&qanda);
  UnloadFileText(file_contents);

  return 0;
}
