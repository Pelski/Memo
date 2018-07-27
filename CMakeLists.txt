cmake_minimum_required(VERSION 3.10)
project(memo)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp colors.h)
add_executable(memo ${SOURCE_FILES})

include_directories(/usr/local/opt/sqlite/include)
link_directories(/usr/local/opt/sqlite/lib)
target_link_libraries(memo sqlite3)
