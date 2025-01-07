cmake_minimum_required(VERSION 3.30)
project(
  xai-cpp
  VERSION 0.1.0
  LANGUAGES CXX)

option(XAI_ENABLE_TESTS "Enable tests" OFF)

find_package(Boost REQUIRED COMPONENTS system thread json)
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xia OBJECT xai.cpp dev.cpp)
target_compile_options(
  xia
  PUBLIC -Wall
         -Wextra
         -Werror
         -Wpedantic
         -Wshadow
         -Wconversion
         -Wsign-conversion
         -Wnon-virtual-dtor
         -Wold-style-cast
         -Wfloat-equal
         -Wformat=2
         -Wnull-dereference
         -Wundef
         -Wuninitialized
         -Wcast-align
         -Wformat-security
         -Wstrict-overflow
         -Wswitch-enum
         -Wunused-variable
         -Wunused-parameter
         -Wpointer-arith
         -Wcast-align
         -Wno-variadic-macros
         -fexceptions
         -Wno-c++98-compat
         -Wno-padded
         -Wno-covered-switch-default
         -Wno-unsafe-buffer-usage)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(xia PUBLIC -Weverything -fsafe-buffer-usage-suggestions)
endif()

target_include_directories(xia PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(xia PUBLIC Boost::json OpenSSL::SSL OpenSSL::Crypto)

add_library(xAI::xAI INTERFACE IMPORTED GLOBAL)
set_target_properties(
  xAI::xAI
  PROPERTIES INTERFACE_SOURCES $<TARGET_OBJECTS:xia>
             INTERFACE_INCLUDE_DIRECTORIES
             $<TARGET_PROPERTY:xia,INTERFACE_INCLUDE_DIRECTORIES>
             INTERFACE_COMPILE_OPTIONS
             $<TARGET_PROPERTY:xia,INTERFACE_COMPILE_OPTIONS>
             INTERFACE_LINK_LIBRARIES
             $<TARGET_PROPERTY:xia,INTERFACE_LINK_LIBRARIES>)

add_executable(xai-repl xai-repl.cc)
target_link_libraries(xai-repl xAI::xAI)

if(XAI_ENABLE_TESTS)
  find_package(GTest REQUIRED)
  include(GoogleTest)
  enable_testing()

  add_executable(xai-test xai-test.cc test.cpp)
  target_link_libraries(xai-test xAI::xAI GTest::GTest GTest::Main)
  target_compile_options(
    xai-test PUBLIC -Wno-c++98-compat-pedantic -Wno-global-constructors
                    -Wno-weak-vtables)

  gtest_discover_tests(xai-test)
endif()

install(TARGETS xia
  EXPORT xia
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

add_library(xai STATIC $<TARGET_OBJECTS:xia>)
target_include_directories(xai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(xai PUBLIC Boost::json OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(xai PROPERTIES OUTPUT_NAME "xai")

install(TARGETS xai
  EXPORT xai
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(FILES xai.hpp DESTINATION include)
