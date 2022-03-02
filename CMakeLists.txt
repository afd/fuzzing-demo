cmake_minimum_required(VERSION 3.13)

project(fuzzing-demo
    VERSION 1.0
    DESCRIPTION "libFuzzer demo"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  set(USE_SANITIZER "" CACHE STRING
          "Use the clang sanitizer [asan|ubsan]")
  if("${USE_SANITIZER}" STREQUAL "asan")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope)
    add_link_options(-fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope)
  elseif("${USE_SANITIZER}" STREQUAL "ubsan")
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
  elseif(NOT "${USE_SANITIZER}" STREQUAL "")
    message(FATAL_ERROR "Unknown argument to USE_SANITIZER: ${USE_SANITIZER} - options are [asan|ubsan]")
  endif()
else()
  message(FATAL_ERROR "The clang compiler is required")
endif()

add_library(sat_solver
  src/solver.h
  src/solver.cc
  )
target_include_directories(sat_solver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ally-sat
        src/main.cc
        )
target_link_libraries(ally-sat PRIVATE sat_solver)

add_executable(fuzz-target
        src/fuzz.cc
        )
target_link_libraries(fuzz-target PRIVATE sat_solver)
target_compile_options(fuzz-target PRIVATE "-fsanitize=address,undefined,fuzzer")
target_link_options(fuzz-target PRIVATE "-fsanitize=address,undefined,fuzzer")
