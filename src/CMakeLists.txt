cmake_minimum_required (VERSION 3.0)

project(tetris-mint)

add_definitions('-g')
add_definitions('-Wall')
add_definitions('-std=gnu99')
add_definitions('-fcommon')

find_package(Threads)
find_package(Curses)

list(APPEND tetrismint_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tetris_game.c
    ${CMAKE_CURRENT_LIST_DIR}/client_conn.c
    ${CMAKE_CURRENT_LIST_DIR}/controller.c
    ${CMAKE_CURRENT_LIST_DIR}/generic.c
    ${CMAKE_CURRENT_LIST_DIR}/list.c
    ${CMAKE_CURRENT_LIST_DIR}/message.c
    ${CMAKE_CURRENT_LIST_DIR}/offline.c
    ${CMAKE_CURRENT_LIST_DIR}/player.c
    ${CMAKE_CURRENT_LIST_DIR}/render.c
    ${CMAKE_CURRENT_LIST_DIR}/widgets.c
    ${CMAKE_CURRENT_LIST_DIR}/curses_text_entry.c
)

add_library(tetrismintlib OBJECT ${tetrismint_SOURCES} log.h log.c party.c party.h event.c event.h curses_combobox.c curses_combobox.h)

add_executable(solo_main solo_main.c tetris_game.c)
add_executable(tetris-mint client.c $<TARGET_OBJECTS:tetrismintlib>)
add_executable(tetris-mint-server server.c $<TARGET_OBJECTS:tetrismintlib>)
add_executable(test_render test_render.c $<TARGET_OBJECTS:tetrismintlib>)
add_executable(test_client_conn test_client_conn.c $<TARGET_OBJECTS:tetrismintlib>)
add_executable(test_player test_player.c $<TARGET_OBJECTS:tetrismintlib>)

target_link_libraries(solo_main ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(tetris-mint ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(tetris-mint-server ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test_render ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test_client_conn ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test_player ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# install for cpack packaging
install(
    TARGETS tetris-mint
    RUNTIME DESTINATION bin
)
install(
    TARGETS tetris-mint-server
    RUNTIME DESTINATION bin
)