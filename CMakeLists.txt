cmake_minimum_required(VERSION 3.10)
project(Compiler VERSION 0.1 DESCRIPTION "Basic Compiler/Toy Language" LANGUAGES C)

add_executable(main src/main.c src/lexer.c src/DynamicArray.c src/Strings.c)

target_include_directories(main
  PUBLIC
    src/
)
