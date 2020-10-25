cmake_minimum_required(VERSION 3.16)
project(ReHitmanTools)

set(CMAKE_CXX_STANDARD 20)

if(
        NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Modules/fmt/CMakeLists.txt" OR
        NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Modules/nlohmann/CMakeLists.txt" OR
        NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Modules/zlib/CMakeLists.txt"
)
    message(FATAL_ERROR "The submodules were not downloaded!\nPlease, init submodules via git submodule update --init --recursive")
endif()

add_subdirectory(Modules/fmt)
add_subdirectory(Modules/zlib)
add_subdirectory(Modules/nlohmann)

add_subdirectory(Tools/GMSInfo)