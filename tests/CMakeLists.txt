cmake_minimum_required(VERSION 3.21)

add_executable(LexerTest LexerTest.cpp)

set(EXECUTE_TEST_LIBS gtest ${CMAKE_DL_LIBS} ${ZLIB_LIBRARIES})

target_link_libraries(LexerTest ${EXECUTE_TEST_LIBS})

set(TEST_ARGS "--gtest_output=xml:../")

add_test(LexerTest LexerTest ${TEST_ARGS})

# SANIT TESTS
set(TEST_PROGRAMS
  LexerTest)

set(SANITY_TESTS ${TEST_PROGRAMS})

set_tests_properties(${SANITY_TESTS} PROPERTIES LABELS "sanity")
