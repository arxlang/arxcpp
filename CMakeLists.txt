cmake_minimum_required(VERSION 3.21)
project(arx)

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

option(ENABLE_TESTS "Build unit tests" OFF)
if (ENABLE_TESTS)
  find_package(GTest REQUIRED)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)
endif()

# Link against LLVM libraries
target_link_libraries(arx glog::glog ${llvm_libs})

install(TARGETS arx DESTINATION bin)
