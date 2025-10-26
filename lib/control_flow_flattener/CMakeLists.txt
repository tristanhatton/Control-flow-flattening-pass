set(CFF_TARGET control_flow_flattener)

file(GLOB_RECURSE CFF_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(${CFF_TARGET} SHARED ${CFF_SOURCES})

target_include_directories(${CFF_TARGET}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

llvm_map_components_to_libnames(LLVM_LIBS
  Support
  Core
  Passes
)

target_link_libraries(${CFF_TARGET} PRIVATE ${LLVM_LIBS})