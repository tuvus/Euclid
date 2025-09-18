cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME game)
project(${PROJECT_NAME} VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_TOOLCHAIN_FILE, "${CMAKE_SOURCE_DIR}/vcpkg.cmake")

set(Sources
        src/card_game.cpp
        src/main.cpp
        src/lobby_scene.cpp
        src/game_scene.cpp
        src/unit.cpp
        src/tower.cpp
        src/deck.cpp
        src/card.cpp
)

set(Headers
        include/card_game.h
        include/lobby_scene.h
        include/menu_scene.h
        include/game_scene.h
        include/unit.h
        include/path.h
        include/card_player.h
        include/tower.h
        include/card.h
        include/deck.h
        include/unit_ui.h
        include/tower_ui.h
        include/deck_ui.h
        include/unit_card.h
        include/card_ui.h
        include/tower_card.h
)

add_executable(game ${Sources} ${Headers})

#set(DONT_RUN_game_ENGINE_TESTS TRUE)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(${CMAKE_SOURCE_DIR}/../engine ${CMAKE_CURRENT_BINARY_DIR}/euclid)
target_link_libraries(${PROJECT_NAME} PUBLIC euclid)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Copy the resources directory to the build
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})