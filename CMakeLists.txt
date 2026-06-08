cmake_minimum_required(VERSION 3.15)
project(WavPlayer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---------- Com Utility (Internal, Header-Only) ----------
# 不创建独立的库目标，直接由 WavPlayer 包含头文件路径即可

# ---------- Wav Player ----------
aux_source_directory(${CMAKE_SOURCE_DIR}/src/WavPlayer/ WAV_PLAYER_SRC_LIST)

add_executable(WavPlayer ${WAV_PLAYER_SRC_LIST})

target_include_directories(WavPlayer
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src                # 使 #include "ComUtility/xxx.hpp" 有效
        ${CMAKE_SOURCE_DIR}/include/ComUtility     # 可选，允许直接 #include "ComMixins.hpp"
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/WavPlayer
)

target_link_libraries(WavPlayer PRIVATE ole32 avrt)

set_target_properties(WavPlayer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(WavPlayer PROPERTIES WIN32_EXECUTABLE ON)