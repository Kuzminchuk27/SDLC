cmake_minimum_required(VERSION 3.16)
project(PressureCalculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PressureCalculator WIN32
    src/main.cpp
)

target_include_directories(PressureCalculator PRIVATE src)
target_compile_definitions(PressureCalculator PRIVATE UNICODE _UNICODE)

if(MSVC)
    target_compile_options(PressureCalculator PRIVATE /utf-8 /W4)
else()
    target_link_options(PressureCalculator PRIVATE -municode)
endif()
