cmake_minimum_required(VERSION 3.2)

project(pegof)

set(PACKCC packcc CACHE FILEPATH "Specify file path of packcc command.")

add_custom_command(
    OUTPUT parser.c parser.h
    COMMAND ${PACKCC} ARGS -o parser ${CMAKE_CURRENT_SOURCE_DIR}/parser.peg
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.peg
    VERBATIM
)

add_executable(pegof main.c format.c optimize.c parser.c system.c utility.c)

target_compile_features(pegof PRIVATE c_std_99)
target_include_directories(pegof BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
