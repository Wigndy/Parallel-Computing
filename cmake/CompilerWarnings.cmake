# cmake/CompilerWarnings.cmake

function(set_project_warnings project_name)
  set(CLANG_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wcast-align
    -Wunused
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
  )

  set(GCC_WARNINGS
    ${CLANG_WARNINGS}
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
  )

  if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_compile_options(${project_name} INTERFACE ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${project_name} INTERFACE ${GCC_WARNINGS})
  endif()
endfunction()
