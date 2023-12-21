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

#define NEXT_BTN_TEXT  "Next"
#define BACK_BTN_TEXT  "Back"
#define RESET_BTN_TEXT "Reset"

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
  Vector2 next_btn_text_size  = MeasureTextEx(font, NEXT_BTN_TEXT, FONTSIZE, 0);
  Vector2 back_btn_text_size  = MeasureTextEx(font, BACK_BTN_TEXT, FONTSIZE, 0);
  Vector2 reset_btn_text_size = MeasureTextEx(font, RESET_BTN_TEXT, FONTSIZE, 0);

  int usable_width = window_width - 2*PADDING;
  int controls_y = window_height - CONTROLSHEIGHT - PADDING;
  Rectangle title_box        = {.x=PADDING, .y=PADDING, .width=usable_width, .height=TITLEHEIGHT};
  Vector2 title_location     = {.x=title_box.x, title_box.y+PADDING/2.0};
  Rectangle controls_box     = {.x=PADDING, .y=controls_y, .width=usable_width, .height=CONTROLSHEIGHT};
  Rectangle text_box         = {.x=PADDING, .y=PADDING + TITLEHEIGHT + PADDING, .width=window_width-2*PADDING, .height=controls_y - 2*PADDING - title_box.height };
  Vector2 main_text_location = {.x=text_box.x, text_box.y+PADDING/2.0};
  Rectangle next_btn_rect = {.x=PADDING, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle back_btn_rect    = {.x=PADDING*2+BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle reset_btn_rect   = {.x=window_width-PADDING-BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Vector2 next_btn_text_location = {
    .x = next_btn_rect.x + (BTNWIDTH/2.0)-(next_btn_text_size.x/2.0),
    .y = next_btn_rect.y + (BTNHEIGHT/2.0) - (next_btn_text_size.y/2.0)
  };
  Vector2 back_btn_text_location = {
    .x = back_btn_rect.x + (BTNWIDTH/2.0)-(back_btn_text_size.x/2.0),
    .y = back_btn_rect.y + (BTNHEIGHT/2.0) - (back_btn_text_size.y/2.0)
  };
  Vector2 reset_btn_text_location = {
    .x = reset_btn_rect.x + (BTNWIDTH/2.0)-(reset_btn_text_size.x/2.0),
    .y = reset_btn_rect.y + (BTNHEIGHT/2.0) - (reset_btn_text_size.y/2.0)
  };

  SetTextLineSpacing(FONTSIZE);

  size_t reveal_statement_num = 0;
  get_qanda_string(qanda, string_to_print, reveal_statement_num);
  adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);

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
      next_btn_rect.y = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
      back_btn_rect.y    = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
      reset_btn_rect.y   = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
      reset_btn_rect.x   = window_width-PADDING-BTNWIDTH;

      next_btn_text_location.x  = next_btn_rect.x + (BTNWIDTH/2.0)-(next_btn_text_size.x/2.0);
      next_btn_text_location.y  = next_btn_rect.y + (BTNHEIGHT/2.0) - (next_btn_text_size.y/2.0);
      back_btn_text_location.x  = back_btn_rect.x + (BTNWIDTH/2.0)-(back_btn_text_size.x/2.0);
      back_btn_text_location.y  = back_btn_rect.y + (BTNHEIGHT/2.0) - (back_btn_text_size.y/2.0);
      reset_btn_text_location.x = reset_btn_rect.x + (BTNWIDTH/2.0)-(reset_btn_text_size.x/2.0);
      reset_btn_text_location.y = reset_btn_rect.y + (BTNHEIGHT/2.0) - (reset_btn_text_size.y/2.0);

      get_qanda_string(qanda, string_to_print, reveal_statement_num);
      adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 click_pos = GetMousePosition();

      if (CheckCollisionPointRec(click_pos, next_btn_rect)) {
        if (reveal_statement_num < num_lines-1) {
          reveal_statement_num += 1;
          get_qanda_string(qanda, string_to_print, reveal_statement_num);
          adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
        }
      }

      if (CheckCollisionPointRec(click_pos, back_btn_rect)) {
        if (reveal_statement_num > 0) {
          reveal_statement_num -= 1;
          get_qanda_string(qanda, string_to_print, reveal_statement_num);
          adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
        }
      }

      if (CheckCollisionPointRec(click_pos, reset_btn_rect)) {
        reveal_statement_num = 0;
        get_qanda_string(qanda, string_to_print, reveal_statement_num);
        adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
      }
    }

    Vector2 text_size = MeasureTextEx(font, string_to_print, FONTSIZE, 0);

    BeginDrawing();
      ClearBackground(BACKGROUND_COLOUR);

      DrawTextEx(font, qanda.title, title_location, FONTSIZE, 0, LIGHTGRAY);

      Vector2 adjusted_text_location = main_text_location;
      if (text_size.y > text_box.height) {
        adjusted_text_location.y -= PADDING + text_size.y - text_box.height;
      }
      BeginScissorMode(text_box.x, text_box.y, text_box.width, text_box.height);
        DrawTextEx(font, string_to_print, adjusted_text_location, FONTSIZE, 0, LIGHTGRAY);
      EndScissorMode();

      DrawRectangleRounded(next_btn_rect, 0.4, 4, LIGHTGRAY);
      DrawTextEx(font, NEXT_BTN_TEXT, next_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);

      DrawRectangleRounded(back_btn_rect, 0.4, 4, LIGHTGRAY);
      DrawTextEx(font, BACK_BTN_TEXT, back_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);

      DrawRectangleRounded(reset_btn_rect, 0.4, 4, LIGHTGRAY);
      DrawTextEx(font, RESET_BTN_TEXT, reset_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);

    EndDrawing();
  }

  free(string_to_print);
  // free_qanda(&qanda);
  free(buffer);
  free(lines);

  return 0;
}
