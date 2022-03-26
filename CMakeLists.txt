cmake_minimum_required(VERSION 3.21)

set(TARGET_NAME arx)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_PATH}/bin)
message(STATUS "PROJECT_PATH: ${PROJECT_PATH}")
message(STATUS "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

find_package (glog REQUIRED)

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS} ${PROJECT_PATH}/arx/include)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Now build our tools
set(ARX_LIB_SOURCES
  ${PROJECT_PATH}/arx/src/lexer.cpp
  ${PROJECT_PATH}/arx/src/parser.cpp
  ${PROJECT_PATH}/arx/src/error.cpp
  ${PROJECT_PATH}/arx/src/codegen.cpp
  ${PROJECT_PATH}/arx/src/settings.cpp
  ${PROJECT_PATH}/arx/src/utils.cpp)

set(ARX_SOURCES
  ${PROJECT_PATH}/arx/src/main.cpp
  ${ARX_LIB_SOURCES})

add_executable(arx ${ARX_SOURCES})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(
    llvm_libs support core irreader orcjit mcjit native object executionengine
)

# clang-tidy
find_program (CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(
      CMAKE_CXX_CLANG_TIDY
      "${CLANG_TIDY_EXE};-format-style=file;-header-filter='${CMAKE_SOURCE_DIR}/*'"
      CACHE STRING
      ""
      FORCE)
else()
    message(STATUS "clang-tidy NOT found!")
    set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # clear it
endif()

option(ENABLE_TESTS "Build unit tests" OFF)
if (ENABLE_TESTS)
  find_package(GTest REQUIRED)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=leak")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=memory")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")

  FIND_PROGRAM(LCOV_PATH lcov)

  # Where TARGET_NAME is the executable target's name
  add_custom_target(ccov-${TARGET_NAME}
      COMMAND LLVM_PROFILE_FILE=${TARGET_NAME}.profraw $<TARGET_FILE:${TARGET_NAME}>
      COMMAND llvm-profdata merge -sparse ${TARGET_NAME}.profraw -o ${TARGET_NAME}.profdata
      COMMAND llvm-cov report $<TARGET_FILE:${TARGET_NAME}> -instr-profile=${TARGET_NAME}.profdata
      DEPENDS ${TARGET_NAME}
  )
endif()

# Link against LLVM libraries
target_link_libraries(arx glog::glog ${llvm_libs})

install(TARGETS arx DESTINATION bin)
