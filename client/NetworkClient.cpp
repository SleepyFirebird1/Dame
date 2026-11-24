#include "NetworkClient.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

NetworkClient::NetworkClient() : clientSocket(-1), isConnected(false), myColor(0) {}

NetworkClient::~NetworkClient() {
    isConnected = false;
    if (clientSocket != -1) close(clientSocket);
    // Zamknięcie wątku
    if (receiveThread.joinable()) receiveThread.join();
}

bool NetworkClient::connectToServer(const std::string& ip, int port) {
    // Utworzenie gniazda
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) 
    {
        std::cerr << "Błąd: Nie mozna utworzyć gniazda klienckiego.\n";
        return false;
    }

    // Konfiguracja adresu serwera
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    // Konwersja adresu ip z tekstu
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) 
    {
        std::cerr << "Błąd: Nieprawidłowy adres IP.\n";
        return false;
    }

    std::cout << "Łączenie z serwerem " << ip << ":" << port << "...\n";
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) 
    {
        std::cerr << "Blad: Nie udalo sie polaczyc z serwerem!\n";
        return false;
    }
    else
    {
        isConnected = true;
        std::cout << "Polaczono pomyslnie! Oczekiwanie na przeciwnika...\n";
    }

    // Uruchomienie wątku nasluchującego serwera
    receiveThread = std::thread(&NetworkClient::receiveLoop, this);
    
    return true;
}

void NetworkClient::receiveLoop() {
    while (isConnected) 
    {
        MessageType msgType;
        int bytes = recv(clientSocket, &msgType, sizeof(msgType), 0);
        
        if (bytes <= 0) 
        {
            setStatus("Rozlaczono z serwerem", true);
            isConnected = false;
            break;
        }

        // Obsługa odebranych wiadomości
        if (msgType == MessageType::MATCH_FOUND) 
        {
            MatchFoundPacket packet;
            recv(clientSocket, &packet, sizeof(packet), 0);
            myColor = packet.playerColor;
            
            // Wysyłamy informację do GUI
            setStatus("Przeciwnik dolaczyl. Gra rozpoczeta!", false);
        }
        else if (msgType == MessageType::INVALID_MOVE) 
        {
            // Błąd ustawiamy na true, by tekst w GUI zaświecił się na czerwono
            setStatus("Nieprawidlowy ruch! Sprobuj ponownie.", true);
        }
        else if (msgType == MessageType::GAME_OVER) 
        {
            setStatus("Koniec gry! Przeciwnik uciekl lub przegral.", true);
            isConnected = false;
        }
        else if (msgType == MessageType::BOARD_UPDATE)
        {
            BoardUpdatePacket packet;
            bytes = recv(clientSocket, &packet, sizeof(packet), 0);
            
            if (bytes <= 0) 
            {
                setStatus("Rozlaczono podczas pobierania planszy", true);
                isConnected = false;
                break;
            }

            // Nadpisanie lokalnej planszy danymi z serwera
            for (int y = 0; y < 8; ++y) 
            {
                for (int x = 0; x < 8; ++x) 
                {
                    // Rzutowanie odebranego bajtu z powrotem na Piece
                    Piece p = static_cast<Piece>(packet.grid[y][x]);
                    board.setPiece(x, y, p);
                }
            }
            currentTurn = packet.currentTurn;
            
            // Po udanym pobraniu planszy czyścimy ewentualne komunikaty o błędach
            setStatus("Zaktualizowano plansze.", false);
        }
    }
}

bool NetworkClient::sendMove(int fromX, int fromY, int toX, int toY) {
    // Wysłanie nagłówka z typem wiadomości
    MessageType type = MessageType::MOVE;
    if (send(clientSocket, &type, sizeof(type), 0) <= 0) {
        return false;
    }

    // Przygotowanie pakietu do wysłania
    MovePacket packet;
    packet.fromX = fromX;
    packet.fromY = fromY;
    packet.toX = toX;
    packet.toY = toY;

    if (send(clientSocket, &packet, sizeof(packet), 0) <= 0) {
        return false;
    }

    return true;
}
