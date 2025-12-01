cmake_minimum_required(VERSION 3.20)
project(QuadTreeGameMenu CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(quadtree_game_menu
    main.cpp
    QuadTree.h
    Entity.h
)

target_link_libraries(quadtree_game_menu
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)
