cmake_minimum_required(VERSION 3.2)

project(pegof)

set(PACKCC packcc CACHE FILEPATH "Specify file path of packcc command.")

add_custom_command(
    OUTPUT parser.c parser.h
    COMMAND ${PACKCC} ARGS -o parser ${CMAKE_CURRENT_SOURCE_DIR}/parser.peg
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.peg
    VERBATIM
)

add_executable(pegof main.cc ast.cc c_api.cc parser.c source.cc)

target_include_directories(pegof BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
