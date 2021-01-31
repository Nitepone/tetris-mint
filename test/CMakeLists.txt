cmake_minimum_required (VERSION 3.0)

find_package(Threads)
find_package(Curses)

add_executable(unit_tests test_tetris_game.c $<TARGET_OBJECTS:tetrismintlib>)

target_link_libraries(unit_tests tetrismintlib unity ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_test(test_basic test_tetris_game)