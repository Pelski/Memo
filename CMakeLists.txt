cmake_minimum_required(VERSION 3.10)
project(memo)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.67.0 COMPONENTS system filesystem REQUIRED)

if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS})
endif()

set(SOURCE_FILES main.cpp colors.h)
add_executable(memo ${SOURCE_FILES})

include_directories(/usr/local/opt/sqlite/include)
link_directories(/usr/local/opt/sqlite/lib)
target_link_libraries(memo sqlite3)

if(Boost_FOUND)
    target_link_libraries(memo ${Boost_LIBRARIES})
endif()
