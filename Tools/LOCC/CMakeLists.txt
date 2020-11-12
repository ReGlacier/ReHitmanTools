cmake_minimum_required(VERSION 3.16)
project(LOCC)

set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 4) # TODO: Think about how to do it better
    message(FATAL_ERROR "Supported only x86 arch!")
endif()

file(GLOB_RECURSE LOCC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(LOCC ${LOCC_SOURCES})

target_link_libraries(LOCC PRIVATE nlohmann_json zlibstatic argh minizip BMFormats::Localization)
target_link_libraries(LOCC PUBLIC spdlog)

target_compile_definitions(LOCC PRIVATE -D_CRT_SECURE_NO_WARNINGS=1)
target_include_directories(LOCC PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Modules/zlib # hotfix for zlib
        ${CMAKE_CURRENT_BINARY_DIR}
        $<TARGET_FILE_DIR:zlibstatic>/.. # hotfix for zlib (final path contains type of build)
        $<TARGET_FILE_DIR:zlibstatic>)