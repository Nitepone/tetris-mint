cmake_minimum_required (VERSION 3.3)

find_package(Threads)
find_package(Curses)

set(ADDITIONAL_LIBRARIES form menu)

add_executable(unit_tests test_tetris_game.c $<TARGET_OBJECTS:ttetrislib>)

target_link_libraries(unit_tests ttetrislib unity ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBRARIES})

add_test(test_basic test_tetris_game)
