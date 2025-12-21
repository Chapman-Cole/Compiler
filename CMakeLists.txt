cmake_minimum_required(VERSION 3.10)
project(Compiler VERSION 0.1 DESCRIPTION "Basic Compiler/Toy Language" LANGUAGES C)

add_executable(main src/main.c src/lexer.c src/DynamicArray.c src/Strings.c)

target_include_directories(main
  PUBLIC
    src/
)

# --------------------------------------------------------------------------

add_executable(visualizer src/visualizer.c src/DynamicArray.c src/Strings.c)

if(WIN32)
    target_include_directories(visualizer PRIVATE src/ C:/raylib/raylib/src/)
    #For some reason you need \\\\ to get a single slash
    target_link_libraries(visualizer PRIVATE "C:\\\\raylib\\\\raylib\\\\src\\\\libraylib.a" gdi32 winmm)
else()
    target_include_directories(visualizer PRIVATE src/ /home/Cole/Programs/raylib/src/)
    target_link_libraries(visualizer PRIVATE /home/Cole/Programs/raylib/build/libraylib.a m)
endif()

# ---------------------------------------------------------------------------

add_executable(webtarget src/visualizer.c src/DynamicArray.c src/Strings.c)

set_target_properties(webtarget PROPERTIES
  SUFFIX ".html"
  OUTPUT_NAME "visualizer"
  LINK_FLAGS "-s WASM=1 -s USE_GLFW=3 -s ASYNCIFY"
)

target_include_directories(webtarget PRIVATE src/ /home/Cole/Programs/raylib/src/)

target_link_libraries(webtarget PRIVATE /home/Cole/Programs/raylib/src/libraylib.web.a m)
