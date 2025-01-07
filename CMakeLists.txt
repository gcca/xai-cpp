cmake_minimum_required(VERSION 3.30)
project(
  xai-cpp
  VERSION 0.1.0
  LANGUAGES CXX)

option(XAI_DISABLE_TESTS "Disable tests" OFF)

find_package(Boost REQUIRED COMPONENTS system thread json)
find_package(OpenSSL REQUIRED)

add_library(xai OBJECT xai.cpp dev.cpp)
target_compile_options(
  xai
  PUBLIC -Wall
         -Wextra
         -Werror
         -Wpedantic
         -Wshadow
         -Weverything
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
         -fsafe-buffer-usage-suggestions
         -Wno-c++98-compat
         -Wno-padded
         -Wno-covered-switch-default
         -Wno-unsafe-buffer-usage)
target_include_directories(xai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(xai PUBLIC Boost::json OpenSSL::SSL OpenSSL::Crypto)

add_library(xAI::xAI INTERFACE IMPORTED GLOBAL)
set_target_properties(
  xAI::xAI
  PROPERTIES INTERFACE_SOURCES $<TARGET_OBJECTS:xai>
             INTERFACE_INCLUDE_DIRECTORIES
             $<TARGET_PROPERTY:xai,INTERFACE_INCLUDE_DIRECTORIES>
             INTERFACE_COMPILE_OPTIONS
             $<TARGET_PROPERTY:xai,INTERFACE_COMPILE_OPTIONS>
             INTERFACE_LINK_LIBRARIES
             $<TARGET_PROPERTY:xai,INTERFACE_LINK_LIBRARIES>)

add_executable(xai-repl xai-repl.cc)
target_link_libraries(xai-repl xAI::xAI)

if(NOT XAI_DISABLE_TESTS)
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

install(TARGETS xai
  EXPORT xai
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

