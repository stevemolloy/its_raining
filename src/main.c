#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <string.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

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
  qanda->questions = realloc(qanda->questions, qanda->capacity*sizeof(char*));
  qanda->answers   = realloc(qanda->answers, qanda->capacity*sizeof(char*));
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
  if (t==NULL) {
    fprintf(stderr, "Unable to allocate memory. Buy more RAM I guess?\n");
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
      fprintf(stderr, "Unable to allocate memory. Buy more RAM I guess?\n");
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
      fprintf(stderr, "Unable to allocate memory. Buy more RAM I guess?\n");
      exit(1);
    }
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
    strcat(str, "\n\n");
  }
} 

#define WIDTH 800
#define HEIGHT 600
#define FONTSIZE 24
#define BACKGROUND_COLOUR CLITERAL(Color){ 0x10, 0x10, 0x20, 0xFF }
#define PADDING 10
#define TITLEHEIGHT (FONTSIZE+10)
#define CONTROLSHEIGHT (FONTSIZE+10)
#define BTNPADDING 2
#define BTNWIDTH 125
#define BTNHEIGHT (CONTROLSHEIGHT-2*BTNPADDING)

int main(void) {
  const char* catechism_file = "./catholic_catechism.txt";
  char *file_contents = LoadFileText(catechism_file);
  if (strlen(file_contents)<7) {
    fprintf(stderr, "File %s does not use the correct format\n", catechism_file);
    exit(1);
  }

  QandA qanda = empty_qanda();
  parse_string_to_qanda(&qanda, file_contents);

  char* string_to_print = calloc(qanda.output_length, sizeof(char));
  if (string_to_print==NULL) {
    fprintf(stderr, "Unable to allocate memory. Buy more RAM I guess?\n");
    exit(1);
  }

  int window_width = WIDTH;
  int window_height = HEIGHT;
  InitWindow(window_width, window_height, "It's raining...");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  Font font = LoadFontEx("fonts/Alegreya-VariableFont_wght.ttf", FONTSIZE, NULL, 0);
  GuiSetFont(font);

  int usable_width = window_width - 2*PADDING;
  int controls_y = window_height - CONTROLSHEIGHT - PADDING;
  Rectangle title_box        = {.x=PADDING, .y=PADDING, .width=usable_width, .height=TITLEHEIGHT};
  Rectangle controls_box     = {.x=PADDING, .y=controls_y, .width=usable_width, .height=CONTROLSHEIGHT};
  Rectangle text_box         = {.x=PADDING, .y=PADDING + TITLEHEIGHT + PADDING, .width=window_width-2*PADDING, .height=controls_y - 2*PADDING - title_box.height };
  Rectangle advance_btn_rect = {.x=PADDING, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle decr_btn_rect    = {.x=PADDING*2+BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle reset_btn_rect   = {.x=window_width-PADDING-BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};

  int min_allowed_window_width  = 4*PADDING + 3*BTNWIDTH;
  int min_allowed_window_height = 4*PADDING + TITLEHEIGHT + 300 + CONTROLSHEIGHT;
  SetWindowMinSize(min_allowed_window_width, min_allowed_window_height);

  GuiSetStyle(DEFAULT, TEXT_SIZE, FONTSIZE);
  GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xEEEEEEFF);
  GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0x000000FF);
  GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, 24);
  GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD); 
  // GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);
  // GuiSetStyle(BUTTON, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);

  size_t reveal_q_num = 0;
  while (!WindowShouldClose()) {
    window_width = GetScreenWidth();
    window_height = GetScreenHeight();

    usable_width = window_width - 2*PADDING;
    controls_y = window_height - CONTROLSHEIGHT - PADDING;
    title_box.width = usable_width;
    controls_box.width = usable_width;
    controls_box.y = controls_y;
    text_box.width = window_width - 2*PADDING;
    text_box.height = controls_y - 3*PADDING - title_box.height;
    advance_btn_rect.y = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
    decr_btn_rect.y    = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
    reset_btn_rect.y   = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
    reset_btn_rect.x   = window_width-PADDING-BTNWIDTH;

    if (GuiButton(advance_btn_rect, "Next")) {
      reveal_q_num++;
      if (reveal_q_num>qanda.length) reveal_q_num = qanda.length;
      get_qanda_string(qanda, string_to_print, reveal_q_num, true);

      size_t line_count = 0;
      size_t char_count = 0;
      for (size_t i=0; i<strlen(string_to_print); i++) {
        if (string_to_print[i] == '\n') {
          line_count += 1 + char_count / (window_width / 10);
          char_count = 0;
        }
        char_count++;
      }
      TraceLog(LOG_INFO, "line_count = %zu", line_count);
    }
    if (GuiButton(decr_btn_rect, "Back")) {
      if (reveal_q_num>0) reveal_q_num--;
    }
    if (GuiButton(reset_btn_rect, "Reset")) {
      reveal_q_num = 0;
    }

    BeginDrawing();
      ClearBackground(BACKGROUND_COLOUR);

      GuiTextBox(title_box, qanda.title, TEXT_SIZE, false);

      // DrawRectangleRec(GetTextBounds(TEXTBOX, text_box), RED);
      GuiTextBox(text_box, string_to_print, 100000, false);

    EndDrawing();
  }

  free(string_to_print);
  free_qanda(&qanda);
  UnloadFileText(file_contents);

  return 0;
}
