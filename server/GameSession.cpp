#include "GameSession.hpp"
#include <iostream>
#include <unistd.h>

GameSession::GameSession(int player1Socket, int player2Socket) :
    playerWhiteSocket(player1Socket), playerBlackSocket(player2Socket), currentTurn(Piece::WHITE_PAWN), isActive(true) {}
    
GameSession::~GameSession()
{
    close(playerWhiteSocket);
    close(playerBlackSocket);
}

void GameSession::run()
{
    std::cout << "Gra wystartowała\n";

    // Wysłanie informacji do graczy kim grają
    MatchFoundPacket player1Start{ 1 }; // 1 = Białe
    MatchFoundPacket player2Start{ 3 }; // 3 = Czarne
    sendPacket(playerWhiteSocket, MessageType::MATCH_FOUND, player1Start);
    sendPacket(playerBlackSocket, MessageType::MATCH_FOUND, player2Start);

    // Pętla gry
    while (isActive)
    {
        int activeSocket;
        if (currentTurn == Piece::WHITE_PAWN)
        {
            activeSocket = playerWhiteSocket;
        }
        else
        {
            activeSocket = playerBlackSocket;
        }

        // Odebranie nagłówka/typu wiadomości
        MessageType msgType;
        int bytesRead = recv(activeSocket, &msgType, sizeof(msgType), 0);

        if (bytesRead <= 0)
        {
            handleDisconnect();
            break;
        }

        if (msgType == MessageType::MOVE)
        {
            MovePacket moveReq;
            bytesRead = recv(activeSocket, &moveReq, sizeof(moveReq), 0);

            if (bytesRead <= 0 )
            {
                handleDisconnect();
                break;
            }

            if (board.isMoveValid(moveReq.fromX, moveReq.fromY, moveReq.toX, moveReq.toY, currentTurn))
            {
                // Wykonanie ruchu na planszy servera
                board.makeMove(moveReq.fromX, moveReq.fromY, moveReq.toX, moveReq.toY);
                // Zmiana tury
                if (currentTurn == Piece::WHITE_PAWN)
                {
                    currentTurn = Piece::BLACK_PAWN;
                } 
                else
                {
                    currentTurn = Piece::WHITE_PAWN;
                }
                
                broadcastBoard();

                // Weryfikacja, czy ktoś nie zwycięzył
                Piece winner = board.checkWinCondition();
                if (winner == Piece::WHITE_PAWN) {
                    notifyGameOver("Wygrał gracz BIAŁY!");
                } 
                else if (winner == Piece::BLACK_PAWN) {
                    notifyGameOver("Wygrał gracz CZARNY!");
                }
            }
            else
            {
                sendHeader(activeSocket, MessageType::INVALID_MOVE);
            }
        }
    }
}

void GameSession::broadcastBoard() {
    BoardUpdatePacket packet;
    
    // Kopiowanie serwerowej planszy do pakietu sieciowego
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            // board.getPiece() zwraca typ enum Piece, wiec potrzebne jest rzutowanie na 1-bajtowy uint8_t
            packet.grid[y][x] = static_cast<uint8_t>(board.getPiece(x, y));
        }
    }
    packet.currentTurn = static_cast<uint8_t>(currentTurn);

    // Wysłanie wiadomości do obydwu graczy
    sendPacket(playerWhiteSocket, MessageType::BOARD_UPDATE, packet);
    sendPacket(playerBlackSocket, MessageType::BOARD_UPDATE, packet);
}

void GameSession::notifyGameOver(const char* reason) {
    std::cout << "Koniec gry! Powód: " << reason << "\n";
    
    isActive = false;

    // 3. Informujemy obu graczy, że gra dobiegła końca
    // Używamy naszej funkcji sendHeader, bo póki co wysyłamy sam sygnał GAME_OVER bez dodatkowych danych
    sendHeader(playerWhiteSocket, MessageType::GAME_OVER);
    sendHeader(playerBlackSocket, MessageType::GAME_OVER);
}

void GameSession::handleDisconnect()
{
    notifyGameOver("Jeden z graczy zerwal polaczenie (Walkower).");
}

bool GameSession::sendHeader(int socket, MessageType type)
{
    if (send(socket, &type, sizeof(type), 0) > 0) return true;
    else return false;
}

template<typename T>
bool GameSession::sendPacket(int socket, MessageType type, const T& payload)
{
    if (!sendHeader(socket, type)) return false;
    if (send(socket, &payload, sizeof(payload), 0) > 0) return true;
    else return false;
}