#pragma once
#include "../common/Board.hpp"
#include "../common/Protocol.hpp"

class GameSession
{
  private:
    int playerWhiteSocket;
    int playerBlackSocket;

    Board board;
    Piece currentTurn;
    bool isActive;

    void broadcastBoard();
    void notifyGameOver(const char *reason);
    void handleDisconnect();

    template <typename T>
    bool sendPacket(int socket, MessageType type, const T &payload);
    bool sendHeader(int socket, MessageType type);

  public:
    GameSession(int player1Socket, int player2Socket);
    ~GameSession();

    void run();
};