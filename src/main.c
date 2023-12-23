#include <assert.h>
#include <gcrypt.h>
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

#define GCRY_CIPHER GCRY_CIPHER_AES256
#define GCRY_MODE GCRY_CIPHER_MODE_CBC

#define KEY_LENGTH 32 // 256 bits
#define IV_LENGTH 16  // 128 bits

void handle_error(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

bool is_file_encrypted(char *filepath) {
  unsigned char *contents = (unsigned char *)read_entire_file(filepath);
  while (*contents) {
    if (*(contents)++ > 127) return true;
  }
  return false;
}

char *decrypt_file(const char *input_filename, const char *password) {
    gcry_cipher_hd_t handle;
    gcry_error_t err;

    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        handle_error("Failed to open input file");
    }

    // Read the IV from the beginning of the encrypted file
    unsigned char iv[IV_LENGTH];
    fread(iv, 1, IV_LENGTH, input_file);

    // Derive the key from the password using a key derivation function (KDF)
    unsigned char key[KEY_LENGTH];
    err = gcry_kdf_derive(password, strlen(password), GCRY_KDF_PBKDF2, GCRY_MD_SHA256, "salt", 4, 4096, KEY_LENGTH, key);
    if (err) {
        handle_error("Key derivation failed");
    }

    // Initialize the cipher handle
    err = gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);
    if (err) {
        handle_error("Cipher initialization failed");
    }

    // Set the key for the cipher handle
    err = gcry_cipher_setkey(handle, key, KEY_LENGTH);
    if (err) {
        gcry_cipher_close(handle);
        handle_error("Failed to set cipher key");
    }

    // Set the IV for the cipher handle
    err = gcry_cipher_setiv(handle, iv, IV_LENGTH);
    if (err) {
        gcry_cipher_close(handle);
        handle_error("Failed to set cipher IV");
    }

    // Decrypt the file content block by block
    size_t block_size = gcry_cipher_get_algo_blklen(GCRY_CIPHER);
    unsigned char *buffer = malloc(block_size);
    size_t decrypted_capacity = block_size; // Initial capacity
    size_t decrypted_size = 0;              // Actual size

    char *decrypted_data = malloc(decrypted_capacity + 1); // +1 for null terminator

    while (1) {
        size_t bytes_read = fread(buffer, 1, block_size, input_file);
        if (bytes_read == 0) {
            break; // End of file
        }

        // Decrypt the block
        err = gcry_cipher_decrypt(handle, buffer, block_size, NULL, 0);
        if (err) {
            free(buffer);
            free(decrypted_data);
            gcry_cipher_close(handle);
            fclose(input_file);
            handle_error("Decryption failed");
        }

        // Resize the buffer if needed
        while (decrypted_size + bytes_read > decrypted_capacity) {
            decrypted_capacity *= 2;
            decrypted_data = realloc(decrypted_data, decrypted_capacity + 1); // +1 for null terminator
        }

        // Append the decrypted block to the result
        memcpy(decrypted_data + decrypted_size, buffer, bytes_read);
        decrypted_size += bytes_read;
    }

    free(buffer);
    gcry_cipher_close(handle);
    fclose(input_file);

    // Resize the final result to the actual size
    decrypted_data = realloc(decrypted_data, decrypted_size + 1); // +1 for null terminator

    // Null-terminate the string
    decrypted_data[decrypted_size] = '\0';

    while (decrypted_data[strlen(decrypted_data)-1] == '\r') {
      decrypted_data[strlen(decrypted_data)-1] = '\0';
    }

    return decrypted_data;
}

// TODO: The routine should not need the file to be given as input. It should be optional
int main(int argc, char **argv) {
  if (argc > 2) {
    handle_error("Too many command line inputs");
  }

  char *file_path = NULL;
  if (argc == 2) {
    file_path = argv[1];
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

  QandA qanda = empty_qanda();
  size_t reveal_statement_num = 0;
  char* string_to_print = NULL;
  size_t num_lines = 0;
  char *buffer = NULL;
  (void)buffer;
  char **lines = NULL;
  (void)lines;

  if (file_path != NULL) {
    if (is_file_encrypted(file_path)) {
      TraceLog(LOG_INFO, "Decrypting %s", file_path);
      buffer = decrypt_file(file_path, "12345");
    } else {
      TraceLog(LOG_INFO, "Loading %s", file_path);
      buffer = read_entire_file(file_path);
    }
    num_lines = string_to_lines(&buffer, &lines);

    parse_lines_to_qanda(&qanda, lines, num_lines);

    string_to_print = calloc(space_estimate_for_qanda(qanda), sizeof(char));
    if (string_to_print==NULL) {
      handle_error("Unable to allocate memory. Stopping execution");
    }

    get_qanda_string(qanda, string_to_print, reveal_statement_num);
    adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
  }

  Vector2 text_size = MeasureTextEx(font, string_to_print, FONTSIZE, 0);

  while (!WindowShouldClose()) {
    if (IsFileDropped()) {
      FilePathList files = LoadDroppedFiles();
      assert(files.count > 0 && "Dropping files should never result in zero files on the drop list, right?");

      file_path = files.paths[0];

      if (is_file_encrypted(file_path)) {
        free(buffer);
        buffer = NULL;
        free(lines);
        lines = NULL;
        if (string_to_print != NULL) {
          free(string_to_print);
          string_to_print = NULL;
        }

        // TODO: This should ask for a password
        buffer = decrypt_file(file_path, "12345");
        num_lines = string_to_lines(&buffer, &lines);
        parse_lines_to_qanda(&qanda, lines, num_lines);

        string_to_print = calloc(space_estimate_for_qanda(qanda), sizeof(char));

        reveal_statement_num = 0;
        get_qanda_string(qanda, string_to_print, reveal_statement_num);
        adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
        text_size = MeasureTextEx(font, string_to_print, FONTSIZE, 0);
        scroll_location = 1.0;
      } else {
        if (string_to_print != NULL) {
          free(string_to_print);
          string_to_print = NULL;
        }
        buffer = read_entire_file(file_path);
        num_lines = string_to_lines(&buffer, &lines);
        parse_lines_to_qanda(&qanda, lines, num_lines);

        string_to_print = calloc(space_estimate_for_qanda(qanda), sizeof(char));

        reveal_statement_num = 0;
        get_qanda_string(qanda, string_to_print, reveal_statement_num);
        adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
        text_size = MeasureTextEx(font, string_to_print, FONTSIZE, 0);
        scroll_location = 1.0;
      }
      UnloadDroppedFiles(files);
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

      if (file_path != NULL) {
        get_qanda_string(qanda, string_to_print, reveal_statement_num);
        adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
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

    if (num_lines > 0) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mouse_pos, next_btn_rect)) {
          if (reveal_statement_num < num_lines-1) {
            Vector2 last_size = text_size;
            reveal_statement_num += 1;
            get_qanda_string(qanda, string_to_print, reveal_statement_num);
            adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
            text_size = MeasureTextEx(font, string_to_print, FONTSIZE, 0);
            scroll_location = last_size.y / text_size.y;
            scroll_speed = 10 / text_size.y;
          }
        }

        if (CheckCollisionPointRec(mouse_pos, back_btn_rect)) {
          if (reveal_statement_num > 0) {
            reveal_statement_num -= 1;
            get_qanda_string(qanda, string_to_print, reveal_statement_num);
            adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
            text_size = MeasureTextEx(font, string_to_print, FONTSIZE, 0);
            scroll_location = 1.0;
          }
        }

        if (CheckCollisionPointRec(mouse_pos, reset_btn_rect)) {
          reveal_statement_num = 0;
          get_qanda_string(qanda, string_to_print, reveal_statement_num);
          adjust_string_for_width(string_to_print, usable_width, font, FONTSIZE);
          text_size = MeasureTextEx(font, string_to_print, FONTSIZE, 0);
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

      if (file_path != NULL) {
        DrawTextEx(font, qanda.title, title_location, FONTSIZE, 0, LIGHTGRAY);

        BeginScissorMode(text_box.x, text_box.y, text_box.width, text_box.height);
          DrawTextEx(font, string_to_print, adjusted_text_location, FONTSIZE, 0, LIGHTGRAY);
        EndScissorMode();

        DrawRectangleRec(scroll_bar_area_rect, DARKGRAY);
        DrawRectangleRec(scroll_bar_rect, LIGHTGRAY);

        DrawRectangleRounded(next_btn_rect, 0.4, 4, next_btn_colour);
        DrawTextEx(font, NEXT_BTN_TEXT, next_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);

        DrawRectangleRounded(back_btn_rect, 0.4, 4, back_btn_colour);
        DrawTextEx(font, BACK_BTN_TEXT, back_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);

        DrawRectangleRounded(reset_btn_rect, 0.4, 4, reset_btn_colour);
        DrawTextEx(font, RESET_BTN_TEXT, reset_btn_text_location, FONTSIZE, 0, BACKGROUND_COLOUR);
      } else {
        char *prompt_text = "Drag and drop the input file here";
        Vector2 prompt_size = MeasureTextEx(font, prompt_text, FONTSIZE, 0);
        Vector2 prompt_loc = {.x = window_width/2.0 - prompt_size.x/2.0, .y = window_height/2.0 - prompt_size.y/2.0};
        DrawTextEx(font, prompt_text, prompt_loc, FONTSIZE, 0, LIGHTGRAY);
      }

    EndDrawing();
  }

  if (string_to_print != NULL) free(string_to_print);
  free_qanda(&qanda);
  if (buffer != NULL) free(buffer);
  if (lines != NULL) free(lines);

  return 0;
}
