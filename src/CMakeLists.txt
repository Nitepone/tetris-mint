cmake_minimum_required (VERSION 2.8)

# projectname is the same as the main-executable
project(ttetris)

add_definitions('-g')
add_definitions('-Wall')
add_definitions('-std=gnu99')

find_package(Threads)
find_package(Curses)

set(ADDITIONAL_LIBRARIES form menu)

list(APPEND ttetris_SOURCES
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
)

add_library(ttetrislib OBJECT ${ttetris_SOURCES} log.h log.c)

add_executable(solo_main solo_main.c tetris_game.c)
add_executable(client client.c $<TARGET_OBJECTS:ttetrislib>)
add_executable(server server.c $<TARGET_OBJECTS:ttetrislib>)
add_executable(test_render test_render.c $<TARGET_OBJECTS:ttetrislib>)
add_executable(test_client_conn test_client_conn.c $<TARGET_OBJECTS:ttetrislib>)
add_executable(test_player test_player.c $<TARGET_OBJECTS:ttetrislib>)

target_link_libraries(solo_main ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBRARIES})
target_link_libraries(client ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBRARIES})
target_link_libraries(server ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBRARIES})
target_link_libraries(test_render ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBRARIES})
target_link_libraries(test_client_conn ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBRARIES})
target_link_libraries(test_player ${CURSES_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ADDITIONAL_LIBRARIES})
