#pragma once
#include <string>
#include <thread>
#include <atomic>
#include "../common/Protocol.hpp"
#include "../common/Board.hpp"

class NetworkClient {
private:
    int clientSocket;
    std::atomic<bool> isConnected;
    int myColor; // 1 - Biale, 3 - Czarne
    Board board;
    int currentTurn = 1;

    // Wyświetlanie wiadomości w kliencie
    std::string currentStatusMsg = "Ladowanie gry...";
    bool isStatusError = false;
    mutable std::mutex statusMutex;

    // Wątek do odbierania wiadomosci w tle
    std::thread receiveThread;
    void receiveLoop();

    
    public:
    NetworkClient();
    ~NetworkClient();
    
    bool connectToServer(const std::string& ip, int port);
    bool sendMove(int fromX, int fromY, int toX, int toY);
    
    // Gettery
    int getMyColor() const { return myColor; }
    const Board& getBoard() const { return board; }
    int getCurrentTurn() const {return currentTurn; }

    // Wyświetlanie wiadomości w kliencie
    void setStatus(const std::string& msg, bool isError = false) {
        std::lock_guard<std::mutex> lock(statusMutex);
        currentStatusMsg = msg;
        isStatusError = isError;
    }

    std::pair<std::string, bool> getStatus() const {
        std::lock_guard<std::mutex> lock(statusMutex);
        return {currentStatusMsg, isStatusError};
    }
};