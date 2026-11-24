#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <netinet/in.h>

class Server
{
    private:
        int serverSocket;
        int port;
        bool isRunning;

        // Kolejka oczekujących graczy
        std::vector<int> waitingPlayers;
        std::mutex queueMutex;

        void acceptClients();
        void matchPlayers();
        void startGameSession(int player1Socket, int player2Socket);

    public:
        Server(int port);
        ~Server();

        void start();
        void stop();
    
};