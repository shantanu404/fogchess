cmake_minimum_required(VERSION 3.10)
project(fogchess)

set(CMAKE_CXX_STANDARD 17)

# Source files
set(SOURCES
    src/gamestate.cpp
    src/serializer.cpp
    src/utils.cpp
    src/fogchess.cpp
)

# Header files
include_directories(src)

add_executable(fogchess ${SOURCES})
