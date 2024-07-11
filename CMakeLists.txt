cmake_minimum_required(VERSION 3.11)

project(pegof)

set(PACKCC packcc CACHE FILEPATH "Specify file path of packcc command.")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(check_linker_options option)
    message(CHECK_START "Testing ${option} support")
    try_compile(
        OPTION_WORKS
        SOURCE_FROM_CONTENT check.c "int main(int argc, char *argv[]) { (void)argc; (void)argv; return 0; }"
        LINK_OPTIONS ${option}
    )
    if(OPTION_WORKS)
        message(CHECK_PASS "works")
        list(APPEND OPTIONAL_DEBUG_OPTIONS ${option})
        SET(OPTIONAL_DEBUG_OPTIONS ${OPTIONAL_DEBUG_OPTIONS} PARENT_SCOPE)
    else()
        message(CHECK_FAIL "not supported")
    endif()
endfunction()

check_linker_options(-fsanitize=address)
check_linker_options(-fsanitize=undefined)
check_linker_options(-fno-omit-frame-pointer)

add_custom_command(
    OUTPUT version.cc
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update_versions.sh ${CMAKE_CURRENT_BINARY_DIR}/version.cc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-sign-compare -Werror)

list(APPEND sources src/ast/action.cc src/ast/alternation.cc src/ast/capture.cc src/ast/code.cc src/ast/directive.cc src/ast/expand.cc src/ast/grammar.cc src/ast/group.cc src/ast/character_class.cc src/ast/node.cc src/ast/reference.cc src/ast/rule.cc src/ast/sequence.cc src/ast/string.cc src/ast/term.cc src/config.cc src/checker.cc src/log.cc src/main.cc src/optimizer.cc src/packcc_wrapper.c src/parser.cc src/utils.cc ${CMAKE_CURRENT_BINARY_DIR}/version.cc)

add_library(common INTERFACE)
target_include_directories(common BEFORE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/packcc/src ${CMAKE_CURRENT_BINARY_DIR})
target_compile_features(common INTERFACE cxx_std_17)

add_executable(pegof ${sources})
target_link_libraries(pegof common)

add_executable(pegof_test ${sources})
set_target_properties(pegof_test PROPERTIES EXCLUDE_FROM_ALL 1)
target_compile_options(pegof_test PRIVATE --coverage -g -O0 ${OPTIONAL_DEBUG_OPTIONS})
target_link_options(pegof_test PRIVATE --coverage ${OPTIONAL_DEBUG_OPTIONS})
target_link_libraries(pegof_test common)

add_custom_command(
    TARGET pegof POST_BUILD
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update_readme.sh
    VERBATIM
)

add_custom_target(
    test
    ${CMAKE_COMMAND} -E env PACKCC=${PACKCC} BUILDDIR=${CMAKE_CURRENT_BINARY_DIR} tests/run.sh -T
    DEPENDS pegof pegof_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    test_all
    ${CMAKE_COMMAND} -E env PACKCC=${PACKCC} BUILDDIR=${CMAKE_CURRENT_BINARY_DIR} INCLUDE_SLOW_TESTS=1 tests/run.sh -T
    DEPENDS pegof pegof_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    benchmark
    benchmark/run.sh
    DEPENDS pegof
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    uncrustify
    ./uncrustify.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

include(GNUInstallDirs)
install(TARGETS pegof RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
