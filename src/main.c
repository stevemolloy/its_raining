#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <string.h>

#include "memo.h"

#define BACKGROUND_COLOUR CLITERAL(Color){ 0x20, 0x20, 0x20, 0xFF }

#define INITIAL_WIDTH 900
#define INITIAL_HEIGHT 600

#define FONTSIZE 30
#define PADDING 10
#define TITLEHEIGHT (FONTSIZE+10)
#define CONTROLSHEIGHT (FONTSIZE+10)

#define BTNPADDING 2
#define BTNWIDTH 125
#define BTNHEIGHT (CONTROLSHEIGHT-2*BTNPADDING)

int main(void) {
  const char* catechism_file = "./fake_catechism.txt";
  char *file_contents = LoadFileText(catechism_file);

  QandA qanda = empty_qanda();
  parse_string_to_qanda(&qanda, file_contents);

  char* string_to_print = calloc(qanda.output_length, sizeof(char));
  if (string_to_print==NULL) {
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
    exit(1);
  }

  InitWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "It's raining...");

  int monitor_number = GetCurrentMonitor();
  int window_width = GetMonitorWidth(monitor_number) / 2;
  int window_height = GetMonitorHeight(monitor_number) * 2 / 3;
  SetWindowSize(window_width, window_height);
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetWindowMinSize(4*PADDING + 3*BTNWIDTH, 4*PADDING + TITLEHEIGHT + 300 + CONTROLSHEIGHT);

  SetTargetFPS(60);

  Font font = LoadFontEx("fonts/Alegreya-VariableFont_wght.ttf", FONTSIZE, NULL, 0);

  int usable_width = window_width - 2*PADDING;
  int controls_y = window_height - CONTROLSHEIGHT - PADDING;
  Rectangle title_box        = {.x=PADDING, .y=PADDING, .width=usable_width, .height=TITLEHEIGHT};
  Rectangle controls_box     = {.x=PADDING, .y=controls_y, .width=usable_width, .height=CONTROLSHEIGHT};
  Rectangle text_box         = {.x=PADDING, .y=PADDING + TITLEHEIGHT + PADDING, .width=window_width-2*PADDING, .height=controls_y - 2*PADDING - title_box.height };
  Rectangle advance_btn_rect = {.x=PADDING, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle decr_btn_rect    = {.x=PADDING*2+BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle reset_btn_rect   = {.x=window_width-PADDING-BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};

  size_t reveal_q_num = 0;
  bool include_answer = false;
  get_qanda_string(qanda, string_to_print, reveal_q_num, include_answer);

  while (!WindowShouldClose()) {
    if (IsWindowResized()) {
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
    }

    BeginDrawing();
      ClearBackground(BACKGROUND_COLOUR);

      DrawRectangleRec(title_box, RED);
      DrawRectangleRec(text_box, BLUE);
      DrawRectangleRec(controls_box, RED);
      DrawRectangleRec(advance_btn_rect, GREEN);
      DrawRectangleRec(decr_btn_rect, GREEN);
      DrawRectangleRec(reset_btn_rect, GREEN);
    EndDrawing();
  }

  free(string_to_print);
  free_qanda(&qanda);
  UnloadFileText(file_contents);

  return 0;
}
