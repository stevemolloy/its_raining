cmake_minimum_required(VERSION 3.28.1)

project(itsraining
  VERSION 0.9
  DESCRIPTION "A tool for learning texts"
  LANGUAGES C
)

configure_file("src/config.h.in" "config.h")

add_executable(${PROJECT_NAME} "src/main.c" "src/memo.c")

add_subdirectory("includes/raylib")

target_include_directories( ${PROJECT_NAME}
  PUBLIC ${PROJECT_BINARY_DIR}
  PUBLIC includes/raylib/src
)

target_link_directories(${PROJECT_NAME}
  PRIVATE includes/raylib/src
)

target_link_libraries(${PROJECT_NAME}
  raylib
  gcrypt
)

