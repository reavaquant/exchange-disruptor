function(target_set_warnings target)
  if(NOT TARGET "${target}")
    message(FATAL_ERROR "target_set_warnings: '${target}' is not a valid target")
  endif()

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
    )
    if(EXCHANGE_DISRUPTOR_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  elseif (MSVC)
    target_compile_options(${target} PRIVATE
      /W4
      /permissive-
    )
    if(EXCHANGE_DISRUPTOR_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  endif()
endfunction()
