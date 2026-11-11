cmake_minimum_required(VERSION 3.15)
project(WarcabySieciowe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app)
# --- 1. Znalezienie SFML w systemie ---
# Szukamy komponentów: graphics (rysowanie), window (okna i zdarzenia), system (wątki, zegary wewnątrz SFML)
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# ==========================================
# CZĘŚĆ SERWEROWA (bez zmian, nie ruszamy)
# ==========================================
add_executable(server_app 
    server/main.cpp 
    server/Server.cpp 
    server/GameSession.cpp 
    common/Board.cpp
)

# ==========================================
# CZĘŚĆ KLIENCKA (Aktualizacja)
# ==========================================
add_executable(client_app 
    client/main.cpp 
    client/NetworkClient.cpp 
    common/Board.cpp
)

# --- 2. Linkowanie SFML do Klienta ---
target_link_libraries(client_app PRIVATE SFML::Graphics SFML::Window SFML::System)