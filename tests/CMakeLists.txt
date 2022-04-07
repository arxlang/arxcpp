cmake_minimum_required(VERSION 3.21)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

# sanitizer tests
set(SAN_FLAGS "-fsanitize=address -O1 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${SAN_FLAGS} \
-fprofile-instr-generate \
-fcoverage-mapping")
set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} ${SAN_FLAGS} \
-fprofile-instr-generate \
-fcoverage-mapping")

add_executable(
  ${TARGET_TEST_NAME}
  __main__.cpp
  test_lexer.cpp
  test_parser.cpp
  test_codegen.cpp
  test_utils.cpp
  test_error.cpp
  ${ARX_LIB_SOURCES})

# link arx code
target_link_libraries(${TARGET_TEST_NAME} glog::glog gtest ${ZLIB_LIBRARIES}
                      ${CMAKE_DL_LIBS} ${llvm_libs})

set(TEST_ARGS "--gtest_output=xml:../")

add_test(${TARGET_TEST_NAME} ${TARGET_TEST_NAME} ${TEST_ARGS})

# SANIT TESTS
set(TEST_PROGRAMS ${TARGET_TEST_NAME})
set(SANITY_TESTS ${TEST_PROGRAMS})
set_tests_properties(${SANITY_TESTS} PROPERTIES LABELS "sanity")
