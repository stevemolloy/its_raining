cmake_minimum_required(VERSION 3.27.1)

project(itsraining
  VERSION 0.9
  DESCRIPTION "A tool for learning texts"
  LANGUAGES C
)

configure_file("src/config.h.in" "config.h")

add_executable(${PROJECT_NAME} "src/main.c" "src/memo.c")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

# CHECK ALL THE SUBMODULES
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/includes/raylib/CMakeLists.txt")
    message(FATAL_ERROR "The raylib submodule was not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

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

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

