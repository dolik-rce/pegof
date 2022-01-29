cmake_minimum_required(VERSION 3.2)

project(pegof)

set(PACKCC packcc CACHE FILEPATH "Specify file path of packcc command.")

add_custom_command(
    OUTPUT peg.c peg.h
    COMMAND ${PACKCC} ARGS -l -o peg ${CMAKE_CURRENT_SOURCE_DIR}/parser.peg
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.peg
    VERBATIM
)

add_executable(pegof main.cc ast.cc c_api.cc char_class.cc config.cc io.cc peg.c parser.cc)

add_custom_command(
    TARGET pegof POST_BUILD
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update_readme.sh
    VERBATIM
)

add_custom_target(test tests/run.sh DEPENDS pegof WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(pegof BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
