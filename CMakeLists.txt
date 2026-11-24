cmake_minimum_required(VERSION 3.15)
project(WarcabySieciowe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(server_app
    server/main.cpp
    server/Server.cpp
    server/GameSession.cpp
    common/Board.cpp
)

add_executable(client_app
    client/main.cpp
    client/NetworkClient.cpp
    common/Board.cpp
)

target_link_libraries(client_app PRIVATE SFML::Graphics SFML::Window SFML::System)