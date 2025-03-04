# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Project name and language
project(Squid)

# Add an executable target
add_executable(squid
  app/squid/main.cc
  app/squid/squid.cc
  app/squid/benchmark.cc
)

# Define a variable for the include directory
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/app/squid/include)

# Use the variable
target_include_directories(squid PRIVATE ${INCLUDE_DIR})
