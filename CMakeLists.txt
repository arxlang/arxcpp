cmake_minimum_required(VERSION 3.21)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

set(TARGET_NAME arx)

project(${TARGET_NAME})

include(c++-standards)
cxx_17()

set(PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_PATH}/bin)
message(STATUS "PROJECT_PATH: ${PROJECT_PATH}")
message(STATUS "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

find_package(glog REQUIRED)
message(STATUS "Found glog ${glog_VERSION}")

find_package(CLI11 CONFIG REQUIRED)
message(STATUS "Found CLI ${CLI11_VERSION}")

# Set your project compile flags. E.g. if using the C++ header files you will
# need to enable C++11 support for your compiler.

include_directories(${LLVM_INCLUDE_DIRS} ${PROJECT_PATH}/arx/include)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Now build our tools
set(ARX_LIB_SOURCES
    ${PROJECT_PATH}/arx/src/lexer.cpp ${PROJECT_PATH}/arx/src/parser.cpp
    ${PROJECT_PATH}/arx/src/error.cpp ${PROJECT_PATH}/arx/src/codegen.cpp
    ${PROJECT_PATH}/arx/src/settings.cpp ${PROJECT_PATH}/arx/src/utils.cpp)

set(ARX_SOURCES ${PROJECT_PATH}/arx/src/main.cpp ${ARX_LIB_SOURCES})

add_executable(arx ${ARX_SOURCES})

# Find the libraries that correspond to the LLVM components that we wish to use
llvm_map_components_to_libnames(
  llvm_libs
  support
  core
  irreader
  orcjit
  mcjit
  native
  object
  executionengine
  all)

option(ENABLE_TESTS "Build unit tests" OFF)
if(ENABLE_TESTS)
  set(TARGET_TEST_NAME "ArxTests")
  find_package(GTest REQUIRED)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)

  option(CLANG_TIDY "CLANG_TIDY" ON)
  option(CPPCHECK "CPPCHECK" ON)
  option(CODE_COVERAGE "CODE_COVERAGE" ON)
  # option(IWYU "IWYU" ON)

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  include(code-coverage)
  include(formatting)
  include(linter)
  include(sanitizers)

  clang_tidy(
    --config-file="${PROJECT_PATH}/.clang-tidy"
    ${PROJECT_PATH}/arx/**/*.cpp
    ${PROJECT_PATH}/arx/**/*.h
    -p="${PROJECT_PATH}/build"
    --header-filter="$ENV{CONDA_PREFIX}/include"
    --extra-arg="-I${PROJECT_PATH}/arx/include")

  cppcheck()
  # include_what_you_use()

  target_code_coverage(${TARGET_NAME} AUTO EXTERNAL)
  target_code_coverage(${TARGET_TEST_NAME} AUTO EXTERNAL)

endif()

# Link against LLVM libraries
target_link_libraries(arx CLI11::CLI11 glog::glog ${llvm_libs})

install(TARGETS arx DESTINATION bin)
