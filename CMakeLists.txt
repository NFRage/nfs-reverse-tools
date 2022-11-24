cmake_minimum_required(VERSION 3.16)
project(mapsourcegen C CXX)

file(GLOB MAPREADER_SRC_FILE
    "src/MAPReader.h"
    "src/MAPReader.cpp"
)

file(GLOB MAPSOURCEGEN_SRC_FILES
    "src/MapSourceGenerator.cpp"
    "src/stdafx.h"
    "src/stdafx.cpp"
)

add_library(mapreader STATIC ${MAPREADER_SRC_FILE})
target_include_directories(mapreader PUBLIC "include")

add_library(mapsourcegen_x64 SHARED ${MAPSOURCEGEN_SRC_FILES})
add_library(mapsourcegen_x86 SHARED ${MAPSOURCEGEN_SRC_FILES})

target_link_libraries(mapsourcegen_x64 PUBLIC 
    "mapreader"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64_win_vc_64/ida.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64_win_vc_64/network.lib"
)

target_link_libraries(mapsourcegen_x86 PUBLIC 
    "mapreader"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64_win_vc_32/ida.lib" 
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64_win_vc_32/network.lib"
)

add_executable(files_gen "src/parser/FilesGenerator.cpp")
target_link_libraries(files_gen PUBLIC "mapreader")