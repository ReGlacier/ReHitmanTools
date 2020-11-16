cmake_minimum_required(VERSION 3.16)
project(HBM_GMSTool)

set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 4) # TODO: Think about how to do it better
    message(FATAL_ERROR "Supported only x86 arch!")
endif()

file(GLOB_RECURSE GMSTOOL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(HBM_GMSTool ${GMSTOOL_SOURCES})

target_link_libraries(HBM_GMSTool PRIVATE nlohmann_json zlibstatic minizip CLI11::CLI11 BMFormats::Localization)
target_link_libraries(HBM_GMSTool PUBLIC spdlog)

target_compile_definitions(HBM_GMSTool PRIVATE -D_CRT_SECURE_NO_WARNINGS=1)
target_include_directories(HBM_GMSTool PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Modules/zlib # hotfix for zlib
        ${CMAKE_CURRENT_BINARY_DIR}
        $<TARGET_FILE_DIR:zlibstatic>/.. # hotfix for zlib (final path contains type of build)
        $<TARGET_FILE_DIR:zlibstatic>)

add_custom_command(
        TARGET HBM_GMSTool POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/data/typeids.json $<TARGET_FILE_DIR:HBM_GMSTool>)

# Type definitions generator tool
add_custom_target(
        GenerateGlacierTypeDefs
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/utils/decompose.py ${CMAKE_CURRENT_SOURCE_DIR}/data/typeids.json ${CMAKE_CURRENT_BINARY_DIR}/GlacierTypeDefs.h
        COMMENT "Generate CPP definitions by data/typeids.json"
)
add_dependencies(HBM_GMSTool GenerateGlacierTypeDefs)