cmake_minimum_required(VERSION 3.21)
project(arx)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS} arx/include)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Now build our tools
set(ARX_SOURCES
    arx/src/main.cpp
    arx/src/lexer.cpp
    arx/src/parser.cpp
    arx/src/error.cpp
    arx/src/codegen.cpp
    arx/src/utils.cpp)

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
target_link_libraries(arx ${llvm_libs})

install(TARGETS arx DESTINATION bin)
