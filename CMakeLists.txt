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

list(APPEND sources main.cc config.cc parser.cc optimizer.cc utils.cc reference.cc action.cc stringnode.cc group.cc alternation.cc sequence.cc character_class.cc directive.cc expand.cc grammar.cc rule.cc term.cc node.cc checker.cc packcc_wrapper.c)
#~ list(APPEND sources ast.cc ast_format.cc ast_opt.cc c_api.cc char_class.cc config.cc io.cc peg.c parser.cc)
add_library(common INTERFACE)
target_include_directories(common BEFORE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_features(common INTERFACE cxx_std_17)

add_executable(pegof ${sources})
target_link_libraries(pegof common)

add_executable(pegof_test ${sources})
target_compile_options(pegof_test PRIVATE --coverage -g -O0)
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
