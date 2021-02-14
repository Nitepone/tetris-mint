file(STRINGS "version.txt" ver)
cmake_policy(SET CMP0048 NEW)
message("Project Version: ${ver}")
project(tetris-mint VERSION ${ver} LANGUAGES C)
cmake_minimum_required (VERSION 3.0)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

#set(CMAKE_VERBOSE_MAKEFILE ON)


set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "admin@night.horse")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Online Tetris for the CLI")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "ncurses")

message("Enabling building of base binaries")
set(BUILD_TESTS 1)
add_subdirectory(src)
message("Enabling building of release packages")
include(CPack)

if(BUILD_TESTS)
  message("Enabling building of testing binaries")
  include(CTest)
  add_subdirectory(unity)
    include_directories(src)
    add_subdirectory(test)
endif()
