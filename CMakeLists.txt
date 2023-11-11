cmake_minimum_required(VERSION 3.11)

project(pegof)

set(PACKCC packcc CACHE FILEPATH "Specify file path of packcc command.")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_custom_command(
    OUTPUT peg.c peg.h
    COMMAND ${PACKCC} ARGS -l -o peg ${CMAKE_CURRENT_SOURCE_DIR}/parser.peg
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.peg
    VERBATIM
)

list(APPEND sources ast/action.cc ast/alternation.cc ast/capture.cc ast/code.cc ast/directive.cc ast/expand.cc ast/grammar.cc ast/group.cc ast/character_class.cc ast/node.cc ast/reference.cc ast/rule.cc ast/sequence.cc ast/stringnode.cc ast/term.cc config.cc checker.cc log.cc main.cc optimizer.cc packcc_wrapper.c parser.cc utils.cc)
add_library(common INTERFACE)
target_include_directories(common BEFORE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_features(common INTERFACE cxx_std_17)

add_executable(pegof ${sources})
target_link_libraries(pegof common)

add_executable(pegof_test ${sources})
target_compile_options(pegof_test PRIVATE --coverage -g -O0 -fsanitize=address -fsanitize=undefined)
target_link_options(pegof_test PRIVATE -fsanitize=address -fsanitize=undefined)
target_link_libraries(pegof_test common gcov)

add_custom_command(
    TARGET pegof POST_BUILD
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update_readme.sh
    VERBATIM
)

add_custom_target(
    test
    ${CMAKE_COMMAND} -E env PACKCC=${PACKCC} BUILDDIR=${CMAKE_CURRENT_BINARY_DIR} tests/run.sh
    DEPENDS pegof_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    uncrustify
    ./uncrustify.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
