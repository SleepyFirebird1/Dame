#include "Server.hpp"
#include "GameSession.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

Server::Server(int port) : port(port), isRunning(false), serverSocket(-1) {}

Server::~Server()
{
    stop();
}

void Server::start()
{
    // Utworzenie gniazda ipv4 w protokole tcp
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Błąd: Nie udało się utworzyć gniazda\n";
        return;
    }
    // Dodanie opcji SO_REUSEADDR eliminuje problem zajtego portu po zamkniciu serwera
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Konfiguracja adresu i portu
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Bindowanie portu
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Błąd: Nie udało się zbindować gniazda\n";
        return;
    }

    // Rozpoczęcie nasłuchiwania
    if (listen(serverSocket, 10) < 0)
    {
        std::cerr << "Błąd: Nie udało się rozpocząć nasłuchiwania\n";
        return;
    }

    isRunning = true;
    std::cout << "Serwer uruchomiony. Nasłuchiwanie na porcie: " << port << "\n";

    // Petla akceptująca graczy
    acceptClients();

}

void Server::stop()
{
    isRunning = false;
    if (serverSocket != -1)
    {
        close(serverSocket);
        serverSocket = -1;
    }
}

void Server::acceptClients()
{
    sockaddr_in clientAddr;
    socklen_t clientSize = sizeof(clientAddr);

    while (isRunning)
    {
        // Akceptacja klienta czkekającego w serverSocket
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientSize);

        if (clientSocket < 0)
        {
            if (isRunning) std::cerr << "Błąd: Klient nie został zaakceptowany\n";
            continue;
        }
        else
        {
            std::cout << "Nowy gracz połączony Socket ID: " << clientSocket << "\n";
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            waitingPlayers.push_back(clientSocket);
        }

        matchPlayers();
    }
}

void Server::matchPlayers()
{
    std::lock_guard<std::mutex> lock(queueMutex);

    while (waitingPlayers.size() >= 2)
    {
        int player1 = waitingPlayers[0];
        int player2 = waitingPlayers[1];

        // Usunięcie dwóch pierwszych graczy z kolejki
        waitingPlayers.erase(waitingPlayers.begin(), waitingPlayers.begin() + 2);

        std::cout << "Znaleziono pare P1: " << player1 << " P2: " << player2 << "\n";

        // Uruchomienie sesje dla graczy w nowym wątku
        startGameSession(player1, player2);
    }
}

void Server::startGameSession(int player1Socket, int player2Socket)
{
    std::thread sessionThread([player1Socket, player2Socket]()
    {
       std::cout << "Rozpoczynam nową grę dla gniazd " << player1Socket << " i " << player2Socket << "\n";
       
       GameSession session(player1Socket, player2Socket);

       session.run();

       std::cout << " Wątek gry dla gniazd " << player1Socket << " i " << player2Socket << " zakonczońy.\n";

    });

    sessionThread.detach();
}