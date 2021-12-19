cmake_minimum_required(VERSION 3.21)

project(myarrow)

find_package(Arrow REQUIRED)

add_executable(myarrow src/myarrow.cpp)
target_link_libraries(myarrow PRIVATE arrow_shared)
