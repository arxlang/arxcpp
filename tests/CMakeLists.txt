cmake_minimum_required(VERSION 3.21)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(TEST_BASE_PATH "./tmp")
add_definitions("-DBASE_PATH=\"${TEST_BASE_PATH}\"")

# sanitizer tests
set(SAN_FLAGS "-fsanitize=address -O1 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SAN_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SAN_FLAGS}")

add_executable(LexerTest LexerTest.cpp)
add_executable(ParserTest ParserTest.cpp)
add_executable(CodeGenTest CodeGenTest.cpp)
add_executable(UtilsTest UtilsTest.cpp)
add_executable(ErrorTest ErrorTest.cpp)

# link arx code
set(EXECUTE_TEST_LIBS gtest ${ZLIB_LIBRARIES} ${CMAKE_DL_LIBS})

target_link_libraries(LexerTest ${EXECUTE_TEST_LIBS})
target_link_libraries(ParserTest ${EXECUTE_TEST_LIBS})
target_link_libraries(CodeGenTest ${EXECUTE_TEST_LIBS})
target_link_libraries(UtilsTest ${EXECUTE_TEST_LIBS})
target_link_libraries(ErrorTest ${EXECUTE_TEST_LIBS})

set(TEST_ARGS "--gtest_output=xml:../")

add_test(LexerTest LexerTest ${TEST_ARGS})
add_test(ParserTest ParserTest ${TEST_ARGS})
add_test(CodeGenTest CodeGenTest ${TEST_ARGS})
add_test(UtilsTest UtilsTest ${TEST_ARGS})
add_test(ErrorTest ErrorTest ${TEST_ARGS})

# SANIT TESTS
set(TEST_PROGRAMS
  LexerTest
  ParserTest
  CodeGenTest
  UtilsTest
  ErrorTest)
set(SANITY_TESTS ${TEST_PROGRAMS})
set_tests_properties(${SANITY_TESTS} PROPERTIES LABELS "sanity")
