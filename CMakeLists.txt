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

# Options
SET(JSON_Install OFF CACHE BOOL "Do not generate install target in nlohmann")
SET(JSON_BuildTests OFF CACHE BOOL "Do not build tests of nlohmann")
SET(FMT_INSTALL OFF CACHE BOOL "Do not generate install targets of fmt")
SET(FMT_TEST OFF CACHE BOOL "Do not generate install targets of fmt")
SET(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Modules/zlib")
SET(SPDLOG_FMT_EXTERNAL_HO ON CACHE BOOL "Do not use own fmt")
SET(BUILD_SHARED_LIBS OFF)

# Modules
add_subdirectory(Modules/fmt)
add_subdirectory(Modules/zlib)
add_subdirectory(Modules/CLI11)
add_subdirectory(Modules/gtest)
add_subdirectory(Modules/spdlog)
add_subdirectory(Modules/minizip)
add_subdirectory(Modules/nlohmann)
add_subdirectory(Modules/BMLOC)

# Our projects
add_subdirectory(Tools/GMSInfo)
add_subdirectory(Tools/LOCC)