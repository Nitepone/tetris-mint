#[=======================================================================[.rst:
FindCurses
----------

Find the curses, pdcurses, or ncurses include file and library.

https://cmake.org/cmake/help/v3.4/manual/cmake-developer.7.html#find-modules

#]=======================================================================]

# guard against creating the curses target multiple times
include_guard(GLOBAL)

set(_PDCURSES_MODULE_NAME curses)
if (PDCURSES_REPO_DIR)
	set(_PDCURSES_REPO_DIR ${PDCURSES_REPO_DIR})
else()
	set(_PDCURSES_REPO_DIR $ENV{HOME}/Documents/PDCurses)
	message(
		WARNING
		"The variable PDCURSES_REPO_DIR was not set via a command line argument."
		" Guessing ${_PDCURSES_REPO_DIR}"
	)
endif()
get_filename_component(_PDCURSES_ARCHIVE_PATH ${_PDCURSES_REPO_DIR}/wincon/pdcurses.a ABSOLUTE)

set(CURSES_FOUND True)
set(CURSES_LIBRARY ${_PDCURSES_MODULE_NAME})
set(CURSES_LIBRARIES ${_PDCURSES_MODULE_NAME})
set(CURSES_INCLUDE_DIRS ${_PDCURSES_REPO_DIR})

# this is a helper function cmake provides to implement expected behavior of a FindXXXX.cmake file
find_package_handle_standard_args(Curses DEFAULT_MSG
        CURSES_LIBRARY CURSES_INCLUDE_DIRS)

include_directories(CURSES_INCLUDE_DIRS)
# setting policy CMP0111 to NEW will mandate that the target IMPORTED_LOCATION is set
cmake_policy(SET CMP0111 NEW)
# STATIC: indicates that this library is composed of archives of object files for use when linking other targets
add_library(${_PDCURSES_MODULE_NAME} STATIC IMPORTED GLOBAL)
set_target_properties(${_PDCURSES_MODULE_NAME} PROPERTIES IMPORTED_LOCATION ${_PDCURSES_ARCHIVE_PATH})

unset(_PDCURSES_MODULE_NAME)
unset(_PDCURSES_REPO_DIR)
