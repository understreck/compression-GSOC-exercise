file(
  GLOB SRC
  "./*.cpp"
)

if(BUILD_SHARED_LIBS)
  add_library(compression-exercise SHARED)
  add_library(compression-exercise::shared ALIAS compression-exercise)
else()
  add_library(compression-exercise STATIC)
  add_library(compression-exercise::static ALIAS compression-exercise)
endif()

target_sources(
  compression-exercise
  PRIVATE ${SRC}
)

target_include_directories(
  compression-exercise
  PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(compression-exercise)

set_target_properties(
  compression-exercise
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED TRUE
)

if(CLANG_TIDY)
  set_target_properties(
    compression-exercise PROPERTIES
    CXX_CLANG_TIDY "clang-tidy;-header-filter=./include"
  )
endif()

if(MSVC)
  target_compile_options(
    compression-exercise
    PRIVATE /W4
  )
else()
  target_compile_options(
    compression-exercise
    PRIVATE -Wall
    PRIVATE -Werror
    PRIVATE -Wextra
  )
endif()
