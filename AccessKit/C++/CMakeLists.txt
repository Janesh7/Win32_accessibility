cmake_minimum_required(VERSION 3.20)

project(windows_example)

find_package(ACCESSKIT REQUIRED)
set(CMAKE_CXX_STANDARD 17)

add_executable(main main.cpp)
target_link_libraries(main PUBLIC accesskit)
target_compile_definitions(main PRIVATE -DUNICODE -D_UNICODE)