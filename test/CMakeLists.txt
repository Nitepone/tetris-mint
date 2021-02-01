cmake_minimum_required (VERSION 3.3)

# Previously, we used add_definitions() to set the compile flags, but that is not always supported.
set(CMAKE_C_STANDARD 99)

if (WIN32)
    # add our custom cmake modules (for curses)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
    set(ADDITIONAL_LIBS wsock32 ws2_32)
endif()

find_package(Threads)
find_package(Curses)

include_directories(${CURSES_INCLUDE_DIRS})

add_executable(unit_tests test_tetris_game.c $<TARGET_OBJECTS:ttetrislib>)

target_link_libraries(unit_tests ttetrislib unity ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBS})

add_test(test_basic test_tetris_game)
