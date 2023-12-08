#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <string.h>

#include "memo.h"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define WIDTH 500
#define HEIGHT 600

#define BACKGROUND_COLOUR CLITERAL(Color){ 0x20, 0x20, 0x20, 0xFF }

#define FONTSIZE 30
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
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
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
  GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, 30);
  GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD); 
  GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);
  GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
  GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0x000000FF);
  GuiSetStyle(BUTTON, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
  GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

  size_t reveal_q_num = 0;
  bool include_answer = false;
  get_qanda_string(qanda, string_to_print, reveal_q_num, include_answer);
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
      include_answer = !include_answer;
      if (include_answer) {
        if (reveal_q_num<qanda.length-1) reveal_q_num++;
      } 
      get_qanda_string(qanda, string_to_print, reveal_q_num, include_answer);
    }

    if (GuiButton(decr_btn_rect, "Back")) {
      include_answer = !include_answer;
      if (reveal_q_num>0) reveal_q_num--;

      get_qanda_string(qanda, string_to_print, reveal_q_num, include_answer);
    }

    if (GuiButton(reset_btn_rect, "Reset")) {
      reveal_q_num = 1;
      include_answer = false;
      get_qanda_string(qanda, string_to_print, reveal_q_num, include_answer);
    }

    BeginDrawing();
      ClearBackground(BACKGROUND_COLOUR);

      GuiTextBox(title_box, qanda.title, strlen(qanda.title), false);

      GuiTextBox(text_box, string_to_print, strlen(string_to_print), false);

    EndDrawing();
  }

  free(string_to_print);
  free_qanda(&qanda);
  UnloadFileText(file_contents);

  return 0;
}
