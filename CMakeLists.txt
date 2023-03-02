#---------------------------CMake Project Config-------------------------------
cmake_minimum_required(VERSION 3.15)
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
  include(UpdateSubmodules)
endif()

#---------------------------compression-exercise-------------------------------
add_subdirectory(src)
