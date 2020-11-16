cmake_minimum_required(VERSION 3.16)
project(BMLOC)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE BMLOC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_library(BMLOC STATIC ${BMLOC_SOURCES})
add_library(BMFormats::Localization ALIAS BMLOC)

target_compile_definitions(BMLOC PRIVATE -D_CRT_SECURE_NO_WARNINGS=1)
target_include_directories(BMLOC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(BMLOC PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/internal/include)
target_link_libraries(BMLOC PUBLIC nlohmann_json)

# Tests
enable_testing()

file(GLOB_RECURSE BLOC_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

add_executable(BMLOC_Tests ${BLOC_TEST_SOURCES})
target_link_libraries(BMLOC_Tests BMFormats::Localization gtest gmock gtest_main)

add_test(BMLOC_AllTests BMLOC_Tests)