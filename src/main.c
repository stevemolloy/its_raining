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
  char* catechism_file = "./fake_catechism.txt";
  char *buffer;
  char **lines;
  size_t num_lines = read_entire_file_to_lines(catechism_file, &buffer, &lines);

  QandA qanda = empty_qanda();
  parse_lines_to_qanda(&qanda, lines, num_lines);

  char* string_to_print = calloc(space_estimate_for_qanda(qanda), sizeof(char));
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
  Vector2 title_location     = {.x=title_box.x, title_box.y+PADDING/2.0};
  Rectangle controls_box     = {.x=PADDING, .y=controls_y, .width=usable_width, .height=CONTROLSHEIGHT};
  Rectangle text_box         = {.x=PADDING, .y=PADDING + TITLEHEIGHT + PADDING, .width=window_width-2*PADDING, .height=controls_y - 2*PADDING - title_box.height };
  Vector2 main_text_location = {.x=text_box.x, text_box.y+PADDING/2.0};
  Rectangle advance_btn_rect = {.x=PADDING, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle decr_btn_rect    = {.x=PADDING*2+BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle reset_btn_rect   = {.x=window_width-PADDING-BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};

  SetTextLineSpacing(FONTSIZE);

  size_t reveal_statement_num = 0;
  get_qanda_string(qanda, string_to_print, reveal_statement_num);

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

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 click_pos = GetMousePosition();

      if (CheckCollisionPointRec(click_pos, advance_btn_rect)) {
        if (reveal_statement_num < num_lines-1) {
          reveal_statement_num += 1;
          get_qanda_string(qanda, string_to_print, reveal_statement_num);
        }
      }

      if (CheckCollisionPointRec(click_pos, decr_btn_rect)) {
        if (reveal_statement_num > 0) {
          reveal_statement_num -= 1;
          get_qanda_string(qanda, string_to_print, reveal_statement_num);
        }
      }

      if (CheckCollisionPointRec(click_pos, reset_btn_rect)) {
        reveal_statement_num = 0;
        get_qanda_string(qanda, string_to_print, reveal_statement_num);
      }
    }

    BeginDrawing();
      ClearBackground(BACKGROUND_COLOUR);

      DrawTextEx(font, qanda.title, title_location, FONTSIZE, 0, LIGHTGRAY);
      DrawTextEx(font, string_to_print, main_text_location, FONTSIZE, 0, LIGHTGRAY);
      DrawRectangleRounded(advance_btn_rect, 0.4, 4, LIGHTGRAY);
      DrawRectangleRounded(decr_btn_rect, 0.4, 4, LIGHTGRAY);
      DrawRectangleRounded(reset_btn_rect, 0.4, 4, LIGHTGRAY);

    EndDrawing();
  }

  free(string_to_print);
  // free_qanda(&qanda);
  free(buffer);
  free(lines);

  return 0;
}
