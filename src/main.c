#include <assert.h>
#include <gcrypt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <string.h>
#include <sys/types.h>

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

#define SCROLLBARWIDTH (PADDING * 2)

#define NEXT_BTN_TEXT  "Next"
#define BACK_BTN_TEXT  "Back"
#define RESET_BTN_TEXT "Reset"

int main(void) {
  AppState state             = {0};
  state.state                = WAITING_FOR_FILE;
  state.file_path            = NULL;
  state.buffer               = NULL;
  state.lines                = NULL;
  state.reveal_statement_num = 0;
  state.string_to_print      = NULL;
  state.num_lines            = 0;
  state.qanda                = empty_qanda();
  state.scroll_location      = 1.0;
  state.scroll_speed         = 0.0;
  state.prompt_text          = "Enter password to decrypt this file.";

  char dummy_password[MAX_PASSWD_CHARS+1] = "\0";

  PasswordDetails passwd_details = {0};
  passwd_details.passwd[0]   = '\0';
  passwd_details.lettercount = 0;

  InitWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "It's raining...");

  int monitor_number = GetCurrentMonitor();
  int window_width = GetMonitorWidth(monitor_number) / 2;
  int window_height = GetMonitorHeight(monitor_number) * 2 / 3;
  SetWindowSize(window_width, window_height);
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetWindowMinSize(4*PADDING + 3*BTNWIDTH, 4*PADDING + TITLEHEIGHT + 300 + CONTROLSHEIGHT);

  SetTargetFPS(60);

  state.font = LoadFontEx("fonts/Alegreya-VariableFont_wght.ttf", FONTSIZE, NULL, 0);
  Font ui_font = LoadFontEx("fonts/Alegreya-VariableFont_wght.ttf", FONTSIZE, NULL, 0);

  Vector2 next_btn_text_size  = MeasureTextEx(ui_font, NEXT_BTN_TEXT, FONTSIZE, 0);
  Vector2 back_btn_text_size  = MeasureTextEx(ui_font, BACK_BTN_TEXT, FONTSIZE, 0);
  Vector2 reset_btn_text_size = MeasureTextEx(ui_font, RESET_BTN_TEXT, FONTSIZE, 0);

  int usable_width = window_width - 2*PADDING - SCROLLBARWIDTH;;
  int controls_y = window_height - CONTROLSHEIGHT - PADDING;
  Rectangle title_box        = {.x=PADDING, .y=PADDING, .width=usable_width, .height=TITLEHEIGHT};
  Vector2 title_location     = {.x=title_box.x, title_box.y+PADDING/2.0};
  Rectangle controls_box     = {.x=PADDING, .y=controls_y, .width=usable_width, .height=CONTROLSHEIGHT};
  Rectangle text_box         = {.x=PADDING, .y=PADDING + TITLEHEIGHT + PADDING, .width=window_width-2*PADDING, .height=controls_y - 2*PADDING - title_box.height };
  Rectangle scroll_bar_area_rect  = {.x=text_box.x + text_box.width - SCROLLBARWIDTH, .y = text_box.y, .width=SCROLLBARWIDTH, .height=text_box.height};
  Vector2 main_text_location = {.x=text_box.x, text_box.y+PADDING/2.0};

  Rectangle back_btn_rect  = {.x=PADDING, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle next_btn_rect  = {.x=PADDING*2+BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
  Rectangle reset_btn_rect = {.x=window_width-PADDING-BTNWIDTH, .y=window_height-CONTROLSHEIGHT-PADDING+BTNPADDING, .width=BTNWIDTH, .height=BTNHEIGHT};
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

  float scroll_location = 1.0f;
  float scroll_speed = 0.0f;

  SetTextLineSpacing(FONTSIZE);

  Vector2 text_size = MeasureTextEx(state.font, state.string_to_print, FONTSIZE, 0);

  while (!WindowShouldClose()) {
    if (IsFileDropped()) {
      FilePathList files = LoadDroppedFiles();
      assert(files.count > 0 && "Dropping files should never result in zero files on the drop list, right?");

      state.file_path = calloc(strlen(files.paths[0])+1, sizeof(char));
      strcpy(state.file_path, files.paths[0]);
      TraceLog(LOG_INFO, "Received a file to open");
      state.state = GOT_FILE;

      UnloadDroppedFiles(files);
    }

    if (state.state == GOT_FILE) {
      if (is_file_encrypted(state.file_path)) {
        TraceLog(LOG_INFO, "Waiting for the user to enter a password");
        state.state = WAITING_FOR_PASSWD;
      } else {
        if (state.string_to_print != NULL) {
          free(state.string_to_print);
          state.string_to_print = NULL;
        }
        state.buffer = read_entire_file(state.file_path);
        // Get codepoints from text
        int codepointCount = 0;
        int *codepoints = LoadCodepoints(state.buffer, &codepointCount);

        // Removed duplicate codepoints to generate smaller font atlas
        int codepointsNoDupsCount = 0;
        int *codepointsNoDups = CodepointRemoveDuplicates(codepoints, codepointCount, &codepointsNoDupsCount);
        state.font = LoadFontEx("./fonts/Alegreya-VariableFont_wght.ttf", FONTSIZE, codepointsNoDups, codepointsNoDupsCount);
        UnloadCodepoints(codepoints);
        state.num_lines = string_to_lines(&state.buffer, &state.lines);
        parse_lines_to_qanda(&state.qanda, state.lines, state.num_lines);

        state.string_to_print = calloc(space_estimate_for_qanda(state.qanda), sizeof(char));

        state.reveal_statement_num = 0;
        get_qanda_string(state.qanda, state.string_to_print, state.reveal_statement_num);
        adjust_string_for_width(state.string_to_print, usable_width, state.font, FONTSIZE);
        text_size = MeasureTextEx(state.font, state.string_to_print, FONTSIZE, 0);
        state.scroll_location = 1.0;
        TraceLog(LOG_INFO, "Displaying the file");
        state.state = DISPLAYING_FILE;
      }
    }
    Color next_btn_colour = LIGHTGRAY;
    Color back_btn_colour = LIGHTGRAY;
    Color reset_btn_colour = LIGHTGRAY;

    scroll_location += scroll_speed;
    if (scroll_location < 0.0) {
      scroll_location = 0.0;
    } else if (scroll_location > 1.0f) {
      scroll_location = 1.0f;
    }
    scroll_speed *= 0.9;
    if (IsWindowResized()) {
      window_width = GetScreenWidth();
      window_height = GetScreenHeight();

      usable_width = window_width - 2*PADDING - SCROLLBARWIDTH;
      controls_y = window_height - CONTROLSHEIGHT - PADDING;
      title_box.width = usable_width;
      controls_box.width = usable_width;
      controls_box.y = controls_y;
      text_box.width = window_width - 2*PADDING;
      text_box.height = controls_y - 3*PADDING - title_box.height;
      back_btn_rect.y = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
      next_btn_rect.y    = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
      reset_btn_rect.y   = window_height-CONTROLSHEIGHT-PADDING+BTNPADDING;
      reset_btn_rect.x   = window_width-PADDING-BTNWIDTH;
      scroll_bar_area_rect.x = text_box.x + text_box.width - SCROLLBARWIDTH;
      scroll_bar_area_rect.width = SCROLLBARWIDTH;
      scroll_bar_area_rect.height = text_box.height;

      next_btn_text_location.x  = next_btn_rect.x + (BTNWIDTH/2.0)-(next_btn_text_size.x/2.0);
      next_btn_text_location.y  = next_btn_rect.y + (BTNHEIGHT/2.0) - (next_btn_text_size.y/2.0);
      back_btn_text_location.x  = back_btn_rect.x + (BTNWIDTH/2.0)-(back_btn_text_size.x/2.0);
      back_btn_text_location.y  = back_btn_rect.y + (BTNHEIGHT/2.0) - (back_btn_text_size.y/2.0);
      reset_btn_text_location.x = reset_btn_rect.x + (BTNWIDTH/2.0)-(reset_btn_text_size.x/2.0);
      reset_btn_text_location.y = reset_btn_rect.y + (BTNHEIGHT/2.0) - (reset_btn_text_size.y/2.0);

      if (state.state == DISPLAYING_FILE) {
        get_qanda_string(state.qanda, state.string_to_print, state.reveal_statement_num);
        adjust_string_for_width(state.string_to_print, usable_width, state.font, FONTSIZE);
      }
    }

    Vector2 mouse_pos = GetMousePosition();
    if (CheckCollisionPointRec(mouse_pos, next_btn_rect)) {
      next_btn_colour = WHITE;
    }
    if (CheckCollisionPointRec(mouse_pos, back_btn_rect)) {
      back_btn_colour = WHITE;
    }
    if (CheckCollisionPointRec(mouse_pos, reset_btn_rect)) {
      reset_btn_colour = WHITE;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      if (CheckCollisionPointRec(mouse_pos, next_btn_rect)) {
        next_btn_colour = GRAY;
      }
      if (CheckCollisionPointRec(mouse_pos, back_btn_rect)) {
        back_btn_colour = GRAY;
      }
      if (CheckCollisionPointRec(mouse_pos, reset_btn_rect)) {
        reset_btn_colour = GRAY;
      }
    }

    if (state.state == DISPLAYING_FILE) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mouse_pos, next_btn_rect)) {
          if (state.reveal_statement_num < state.num_lines-1) {
            Vector2 last_size = text_size;
            state.reveal_statement_num += 1;
            get_qanda_string(state.qanda, state.string_to_print, state.reveal_statement_num);
            adjust_string_for_width(state.string_to_print, usable_width, state.font, FONTSIZE);
            text_size = MeasureTextEx(state.font, state.string_to_print, FONTSIZE, 0);
            scroll_location = last_size.y / text_size.y;
            scroll_speed = 10 / text_size.y;
          }
        }

        if (CheckCollisionPointRec(mouse_pos, back_btn_rect)) {
          if (state.reveal_statement_num > 0) {
            state.reveal_statement_num -= 1;
            get_qanda_string(state.qanda, state.string_to_print, state.reveal_statement_num);
            adjust_string_for_width(state.string_to_print, usable_width, state.font, FONTSIZE);
            text_size = MeasureTextEx(state.font, state.string_to_print, FONTSIZE, 0);
            scroll_location = 1.0;
          }
        }

        if (CheckCollisionPointRec(mouse_pos, reset_btn_rect)) {
          state.reveal_statement_num = 0;
          get_qanda_string(state.qanda, state.string_to_print, state.reveal_statement_num);
          adjust_string_for_width(state.string_to_print, usable_width, state.font, FONTSIZE);
          text_size = MeasureTextEx(state.font, state.string_to_print, FONTSIZE, 0);
          scroll_location = 1.0;
        }
      }
    }

    Vector2 adjusted_text_location = main_text_location;
    Rectangle scroll_bar_rect = scroll_bar_area_rect;
    scroll_bar_rect.x += 2;
    scroll_bar_rect.width -= 4;
    if (text_size.y > text_box.height) {
      adjusted_text_location.y -= scroll_location * (text_size.y - text_box.height);
      scroll_bar_rect.height *= text_box.height / text_size.y;
      scroll_bar_rect.y = text_box.y + scroll_location * (text_box.height - scroll_bar_rect.height);
    }

    float wheel_move = GetMouseWheelMove();
    if (wheel_move != 0.0f && CheckCollisionPointRec(GetMousePosition(), text_box)) {
      scroll_speed -= wheel_move * 10 / text_size.y;
    }

    BeginDrawing();
      ClearBackground(BACKGROUND_COLOUR);

      if (state.state == WAITING_FOR_PASSWD) {
        Vector2 prompt_size = MeasureTextEx(state.font, state.prompt_text, FONTSIZE, 0);
        Vector2 prompt_loc = {.x = window_width/2.0 - prompt_size.x/2.0, .y = window_height/2.0 - 3.0*prompt_size.y/2.0};
        DrawTextEx(ui_font, state.prompt_text, prompt_loc, FONTSIZE, 0, LIGHTGRAY);

        Rectangle textBox = {.x=prompt_loc.x, .y=prompt_loc.y+10+prompt_size.y, .width=prompt_size.x, .height=prompt_size.y};
        DrawRectangleRec(textBox, LIGHTGRAY);

        int key = GetCharPressed();
        // Check if more characters have been pressed on the same frame
        while (key > 0) {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125) && (passwd_details.lettercount < MAX_PASSWD_CHARS)) {
              dummy_password[passwd_details.lettercount]   = '*';
              dummy_password[passwd_details.lettercount+1] = '\0';
              passwd_details.passwd[passwd_details.lettercount]   = (char)key;
              passwd_details.passwd[passwd_details.lettercount+1] = '\0'; // Add null terminator at the end of the string.
              passwd_details.lettercount++;
            }
            key = GetCharPressed();  // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            passwd_details.lettercount--;
            if (passwd_details.lettercount < 0) passwd_details.lettercount = 0;
            passwd_details.passwd[passwd_details.lettercount] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER)) {
          TraceLog(LOG_INFO, "Checking the password");
          state.state = CHECKING_PASSWD;
        }
        float spacing = 2;
        Vector2 passwd_text_size = MeasureTextEx(state.font, passwd_details.passwd, FONTSIZE, spacing);
        DrawTextEx(ui_font, dummy_password, (Vector2){.x=window_width/2.0 - passwd_text_size.x/2.0, .y=textBox.y+textBox.height/4.0}, FONTSIZE, spacing, BACKGROUND_COLOUR);
      } else if (state.state == CHECKING_PASSWD) {
        free(state.buffer);
        state.buffer = NULL;
        free(state.lines);
        state.lines = NULL;
        if (state.string_to_print != NULL) {
          free(state.string_to_print);
          state.string_to_print = NULL;
        }
        // TODO: This should ask for a password
        state.buffer = decrypt_file(state.file_path, passwd_details.passwd);
        
        state.state = BUILDING_FILE;
        if (is_utf8(state.buffer)) {
          state.state = BUILDING_FILE;
        } else {
          state.state = WAITING_FOR_PASSWD;
        }
      } else if (state.state == BUILDING_FILE) {
        TraceLog(LOG_INFO, "Building the UI contents for the user");
        // Get codepoints from text
        int codepointCount = 0;
        int *codepoints = LoadCodepoints(state.buffer, &codepointCount);
        // Removed duplicate codepoints to generate smaller font atlas
        int codepointsNoDupsCount = 0;
        int *codepointsNoDups = CodepointRemoveDuplicates(codepoints, codepointCount, &codepointsNoDupsCount);
        state.font = LoadFontEx("./fonts/Alegreya-VariableFont_wght.ttf", FONTSIZE, codepointsNoDups, codepointsNoDupsCount);
        UnloadCodepoints(codepoints);
        state.num_lines = string_to_lines(&state.buffer, &state.lines);
        parse_lines_to_qanda(&state.qanda, state.lines, state.num_lines);
        state.string_to_print = calloc(space_estimate_for_qanda(state.qanda), sizeof(char));

        state.reveal_statement_num = 0;
        get_qanda_string(state.qanda, state.string_to_print, state.reveal_statement_num);
        adjust_string_for_width(state.string_to_print, usable_width, state.font, FONTSIZE);
        text_size = MeasureTextEx(state.font, state.string_to_print, FONTSIZE, 0);
        scroll_location = 1.0;
        state.state = DISPLAYING_FILE;

      } else if (state.state == DISPLAYING_FILE) {
        DrawTextEx(state.font, state.qanda.title, title_location, FONTSIZE, 0, LIGHTGRAY);

        BeginScissorMode(text_box.x, text_box.y, text_box.width, text_box.height);
          // char *utf8_repr = LoadUTF8(state.string_to_print, strlen(state.string_to_print));
          DrawTextEx(state.font, state.string_to_print, adjusted_text_location, FONTSIZE, 0, LIGHTGRAY);
          // DrawTextEx(state.font, utf8_repr, adjusted_text_location, FONTSIZE, 0, LIGHTGRAY);
        EndScissorMode();

        DrawRectangleRec(scroll_bar_area_rect, DARKGRAY);
        DrawRectangleRec(scroll_bar_rect, LIGHTGRAY);

        DrawRectangleRounded(next_btn_rect, 0.4, 4, next_btn_colour);
        DrawTextEx(ui_font, NEXT_BTN_TEXT, next_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);

        DrawRectangleRounded(back_btn_rect, 0.4, 4, back_btn_colour);
        DrawTextEx(ui_font, BACK_BTN_TEXT, back_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);

        DrawRectangleRounded(reset_btn_rect, 0.4, 4, reset_btn_colour);
        DrawTextEx(ui_font, RESET_BTN_TEXT, reset_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);
      } else {
        char *prompt_text = "Drag and drop the input file here";
        Vector2 prompt_size = MeasureTextEx(state.font, prompt_text, FONTSIZE, 0);
        Vector2 prompt_loc = {.x = window_width/2.0 - prompt_size.x/2.0, .y = window_height/2.0 - prompt_size.y/2.0};
        DrawTextEx(ui_font, prompt_text, prompt_loc, FONTSIZE, 0, LIGHTGRAY);
      }

    EndDrawing();
  }

  if (state.string_to_print != NULL) free(state.string_to_print);
  free_qanda(&state.qanda);
  if (state.buffer != NULL) free(state.buffer);
  if (state.lines != NULL) free(state.lines);

  return 0;
}
