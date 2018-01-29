cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(acirc VERSION 0.1.0 LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

find_package(FLEX)
FLEX_TARGET(Scanner src/scan.l ${CMAKE_CURRENT_SOURCE_DIR}/src/scan.c)

find_package(BISON)
BISON_TARGET(Parser src/parse.y ${CMAKE_CURRENT_SOURCE_DIR}/src/parse.c)

set(acirc_SOURCES
  src/acirc.c
  src/degree.c
  src/depth.c
  src/ngates.c
  src/nmuls.c
  src/eval.c
  src/storage.c
  ${FLEX_Scanner_OUTPUTS}
  ${BISON_Parser_OUTPUTS}
  )
set(acirc_HEADERS
  src/acirc.h
  )

add_library(acirc SHARED ${acirc_SOURCES})
target_link_libraries(acirc PUBLIC threadpool gmp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-function -std=gnu11 -march=native")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pg -ggdb -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

install(TARGETS acirc LIBRARY DESTINATION lib)
install(FILES ${acirc_HEADERS} DESTINATION include)

# Test files

enable_testing()
macro(add_test_ _name)
  add_executable("${_name}" "t/${_name}.c")
  target_include_directories("${_name}" PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries("${_name}" PUBLIC acirc)
  add_test(NAME "${_name}" COMMAND "${_name}")
endmacro()

add_test_(test_circ)
