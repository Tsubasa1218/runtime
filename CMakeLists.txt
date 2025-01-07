cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Form)
add_executable(Form main.cpp)

add_subdirectory(Reactive)
target_link_libraries(Form PUBLIC Reactive)
target_include_directories(Form PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/Reactive")


