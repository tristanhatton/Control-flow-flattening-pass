
set(TEST_IR_DIR ${CMAKE_CURRENT_BINARY_DIR}/ir)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_FILE ${SOURCE_DIR}/source.cpp)
set(OUTPUT_IR_FILE ${TEST_IR_DIR}/source.ll)


file(MAKE_DIRECTORY ${TEST_IR_DIR})


find_program(CLANG_EXECUTABLE clang++ HINTS ${LLVM_TOOLS_BINARY_DIR})

if(NOT CLANG_EXECUTABLE)
  message (FATAL_ERROR "clang++ not found!")
endif()


set(CLANG_IR_FLAGS -emit-llvm -S -O0 -Xclang -disable-llvm-passes)

add_custom_command(
  OUTPUT ${OUTPUT_IR_FILE}
  COMMAND ${CLANG_EXECUTABLE} ${CLANG_IR_FLAGS}
          ${SOURCE_FILE}
          -o ${OUTPUT_IR_FILE}
  DEPENDS ${SOURCE_FILE}
  COMMENT "Generating LLVM IR from source.cpp"
  VERBATIM
)

add_custom_target(generate_test_ir ALL
  DEPENDS ${OUTPUT_IR_FILE}
)

