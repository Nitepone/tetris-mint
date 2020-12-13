project(terminally-tetris C)
cmake_minimum_required (VERSION 2.8)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

#set(CMAKE_VERBOSE_MAKEFILE ON)

SET(TARGET_GROUP "test")
  add_subdirectory(src)

if(TARGET_GROUP STREQUAL release)
# If we had any files that ONLY were compiled into our release, for example startup code,
  # it's really handy to put it here.
elseif(TARGET_GROUP STREQUAL test)
# Our tests need to let CTest know we're running tests
  include(CTest)

add_subdirectory(unity)
#  add_library(unity_cmake STATIC unity/src/unity.c)
  include_directories(src)
#  include_directories(unity_cmake PUBLIC unity/src)
  add_subdirectory(test)
else()
  message(FATAL_ERROR "I don't know the TARGET_GROUP you gave me!")
endif()
