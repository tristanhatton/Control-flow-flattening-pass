cmake_minimum_required(VERSION 3.20)

project(llvm_ir_passes LANGUAGES CXX)

find_package(LLVM REQUIRED CONFIG)

include(AddLLVM)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

if (NOT LLVM_ENABLE_RTTI)
  if (MSVC)
    add_compile_options(/GR-)
  else()
    add_compile_options(-fno-rtti)
  endif()
endif()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(lib)
add_subdirectory(test)


