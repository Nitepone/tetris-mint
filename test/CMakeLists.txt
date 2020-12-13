cmake_minimum_required (VERSION 2.8)


find_package(Threads)
find_package(Curses)
add_executable(unit_tests test_tetris_game.c $<TARGET_OBJECTS:ttetrislib>)

target_link_libraries(unit_tests ttetrislib unity ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_test(test_basic test_tetris_game)
