cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME euclid)
project(${PROJECT_NAME} VERSION 0.1.0 DESCRIPTION "Game engine for C++ games" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

set(Sources
        src/application.cpp
        src/networking/network.cpp
        src/networking/rpc_manager.cpp
        src/ui/context.cpp
        src/ui/element.cpp
        src/ui/container.cpp
        src/ui/vbox.cpp
        src/ui/hbox.cpp
        src/ui/text.cpp
        src/ui/button.cpp
        src/game_manager.cpp
        src/game_object.cpp
)

set(Headers
        ${PROJECT_SOURCE_DIR}/include/engine/application.h
        ${PROJECT_SOURCE_DIR}/include/engine/application_factory.h
        ${PROJECT_SOURCE_DIR}/include/engine/networking/network.h
        ${PROJECT_SOURCE_DIR}/include/engine/networking/rpc_manager.h
        ${PROJECT_SOURCE_DIR}/include/engine/ui/eui.h
        ${PROJECT_SOURCE_DIR}/include/engine/networking/rpc_manager.h
        ${PROJECT_SOURCE_DIR}/include/engine/scene.h
        ${PROJECT_SOURCE_DIR}/include/engine/game_manager.h
        ${PROJECT_SOURCE_DIR}/include/engine/game_object.h
        ${PROJECT_SOURCE_DIR}/include/engine/emath.h
)

add_library(${PROJECT_NAME} STATIC ${Sources} ${Headers})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include/engine ${PROJECT_SOURCE_DIR})

find_package(glfw3 CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(rpclib CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib glfw GameNetworkingSockets::static GameNetworkingSockets::GameNetworkingSockets rpclib::rpc)
find_package(GameNetworkingSockets CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GameNetworkingSockets::static GameNetworkingSockets::GameNetworkingSockets)

# if (NOT DONT_RUN_GAME_ENGINE_TESTS)
#     add_subdirectory(tests)
# endif ()

# Copy compile_commands.json after build config
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
)
