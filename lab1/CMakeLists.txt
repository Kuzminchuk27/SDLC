cmake_minimum_required(VERSION 3.16)
project(PressureCalculator LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PressureCalculator WIN32
    src/main.cpp
    src/model/pressuremodel.cpp
    src/view/mainwindow.cpp
    src/view/inputdialog.cpp
    src/controller/pressurecontroller.cpp
)

target_include_directories(PressureCalculator PRIVATE src)
target_compile_definitions(PressureCalculator PRIVATE UNICODE _UNICODE)
target_link_libraries(PressureCalculator PRIVATE comctl32)

if(MSVC)
    target_compile_options(PressureCalculator PRIVATE /utf-8 /W4)
    target_sources(PressureCalculator PRIVATE app.manifest)
else()
    target_sources(PressureCalculator PRIVATE app_mingw.rc)
    target_link_options(PressureCalculator PRIVATE -municode -static)
endif()
