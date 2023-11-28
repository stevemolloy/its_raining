#include <stdio.h>
#include <raylib.h>

#define WIDTH 800
#define HEIGHT 600

int main(void) {
  InitWindow(WIDTH, HEIGHT, "It's raining...");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);
    EndDrawing();
  }

  return 0;
}
