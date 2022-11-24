cmake_minimum_required(VERSION 3.16)
project(loadmap C CXX)

file(GLOB LOADMAP_SRC_FILES
    "src/*.cpp"
    "src/*.h"
)

add_library(loadmap_x64 SHARED ${LOADMAP_SRC_FILES})
add_library(loadmap_x86 SHARED ${LOADMAP_SRC_FILES})

target_link_libraries(loadmap_x64 PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64_win_vc_64/ida.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64_win_vc_64/network.lib"
)

target_link_libraries(loadmap_x86 PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64_win_vc_32/ida.lib" 
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64_win_vc_32/network.lib"
)

target_include_directories(loadmap_x64 PUBLIC "include")
target_include_directories(loadmap_x86 PUBLIC "include")