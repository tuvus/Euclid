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
        src/test_layout_scene.cpp
)

set(Headers
        include/card_game.h
        include/lobby_scene.h
        include/menu_scene.h
        include/game_scene.h
        include/test_layout_scene.h
        include/unit.h
        include/path.h
        ../engine/include/engine/game_ui_manager.h
        include/card_player.h
        include/tower.h
)

add_executable(game ${Sources} ${Headers})

#set(DONT_RUN_game_ENGINE_TESTS TRUE)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(${CMAKE_SOURCE_DIR}/../engine ${CMAKE_CURRENT_BINARY_DIR}/euclid)
target_link_libraries(${PROJECT_NAME} PUBLIC euclid)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Copy the resources directory to the build
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})