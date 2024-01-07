cmake_minimum_required(VERSION 3.28.1)

project(itsraining
  VERSION 0.9
  DESCRIPTION "A tool for learning texts"
  LANGUAGES C
)

add_executable(itsraining "src/main.c" "src/memo.c" "src/memo.h")

