cmake_minimum_required(VERSION 3.28.1)

project(itsraining
  VERSION 0.9
  DESCRIPTION "A tool for learning texts"
  LANGUAGES C
)

add_executable(itsraining "src/main.c" "src/memo.c" "src/memo.h")

add_subdirectory("includes/raylib")

target_include_directories( ${PROJECT_NAME}
  PUBLIC includes/raylib/src
)

target_link_directories(${PROJECT_NAME}
  PRIVATE includes/raylib/src
)

target_link_libraries(${PROJECT_NAME}
  raylib
  gcrypt
)

