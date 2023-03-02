#---------------------------CMake Project Config-------------------------------
cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0091 NEW)

project(Compression-Exercise LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    message("Setting build configuration to Release.")
    message("To change, configure with -DCMAKE_BUILD_TYPE=Release/Debug/etc...")

    set(
        CMAKE_BUILD_TYPE Release CACHE
        STRING "Sets the configuration to build (Release, Debug, etc...)"
    )
endif()

option(CLANG_TIDY OFF)
option(BUILD_TESTING OFF)
option(BUILD_SHARED_LIBS OFF)
option(WIN32CONSOLE ON)

#---------------------------Compilation database generation--------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(TOUCH ${CMAKE_BINARY_DIR}/compile_commands.json)
file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)

configure_file(
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    COPYONLY
)

#---------------------------git submodule setup--------------------------------
option(UPDATE_SUBMODULES "Update git submodules before configuration" ON)

if(UPDATE_SUBMODULES)
    find_package(Git REQUIRED)

    message(STATUS "Submodule update")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT)

    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR 
            "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, "
            "please checkout submodules")
    endif()
endif()

#---------------------------zlib-----------------------------------------------
find_package(ZLIB 1.2 QUIET)

if(NOT ZLIB_FOUND)
    add_subdirectory(${PROJECT_SOURCE_DIR}/extern/zlib)
    if(NOT SHARED)
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
    else()
        add_library(ZLIB::ZLIB ALIAS zlib)
    endif()
endif()

#---------------------------compression-exercise-------------------------------
add_subdirectory(src)
