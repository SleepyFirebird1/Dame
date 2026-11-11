#include "Board.hpp"
#include <iostream>

Board::Board()
{
    // Iteracja przez rzędy
    for (int y = 0; y < 8; ++y)
    {
        // Iteracja przez kolumny
        for (int x = 0; x < 8; ++x)
        {
            grid[y][x] = Piece::EMPTY;

            if ((x + y) % 2 != 0)
            {
                if (y <= 2)
                {
                    grid[y][x] = Piece::BLACK_PAWN;
                }
                if (y >= 5)
                {
                    grid[y][x] = Piece::WHITE_PAWN;
                }
            }
        } 
    }
};
Piece Board::getPiece(int x, int y) const 
{
    // Zabezpieczenie przed odwołanie poza plansze
    if (x >= 0 && x < 8 && y >= 0 && y < 8) 
    {
        return grid[y][x];
    }
    return Piece::EMPTY;
}

void Board::setPiece(int x, int y, Piece piece) 
{
    if (x >= 0 && x < 8 && y >= 0 && y < 8) 
    {
        grid[y][x] = piece;
    }
}

bool Board::hasForcedCapture(Piece playerColor) const 
{
    bool isWhitePlayer = (playerColor == Piece::WHITE_PAWN || playerColor == Piece::WHITE_QUEEN);

    for (int y = 0; y < 8; ++y) 
    {
        for (int x = 0; x < 8; ++x) 
        {
            Piece p = grid[y][x];
            
            // Sprawdzenie czy pion naley do wlaściwego gracza
            bool isWhitePiece = (p == Piece::WHITE_PAWN || p == Piece::WHITE_QUEEN);
            bool isBlackPiece = (p == Piece::BLACK_PAWN || p == Piece::BLACK_QUEEN);
            if (isWhitePlayer && !isWhitePiece) continue;
            if (!isWhitePlayer && !isBlackPiece) continue;

            // Sprawdzenie wszystkich kierunków bicia
            int jumpDx[] = {-2, 2, -2, 2};
            int jumpDy[] = {-2, -2, 2, 2};

            for (int i = 0; i < 4; ++i) 
            {
                int toX = x + jumpDx[i];
                int toY = y + jumpDy[i];
                int midX = x + jumpDx[i] / 2;
                int midY = y + jumpDy[i] / 2;

                // Sprawdzenie czy bicie jest w obrębie planszy
                if (toX < 0 || toX > 7 || toY < 0 || toY > 7) continue;

                // Sprawdzenie czy pionek skacze w dobrą stronę
                if (p == Piece::WHITE_PAWN && jumpDy[i] > 0) continue;
                if (p == Piece::BLACK_PAWN && jumpDy[i] < 0) continue;

                // Sprawdzenie czy docelowe pole jest puste
                if (grid[toY][toX] == Piece::EMPTY) 
                {
                    Piece midPiece = grid[midY][midX];
                    bool isMidWhite = (midPiece == Piece::WHITE_PAWN || midPiece == Piece::WHITE_QUEEN);
                    bool isMidBlack = (midPiece == Piece::BLACK_PAWN || midPiece == Piece::BLACK_QUEEN);

                    // Sprawdzenie czy po środku stoi przeciwnik
                    if (isWhitePlayer && isMidBlack) return true;
                    if (!isWhitePlayer && isMidWhite) return true;
                }
            }
        }
    }
    return false;
}

bool Board::isMoveValid(int fromX, int fromY, int toX, int toY, Piece playerColor) const {
    // Sprawdzenie czy ruch jest w obrębie planszy
    if (fromX < 0 || fromX > 7 || fromY < 0 || fromY > 7 || 
        toX < 0 || toX > 7 || toY < 0 || toY > 7) return false;

    Piece movingPiece = grid[fromY][fromX];
    bool isWhitePlayer = (playerColor == Piece::WHITE_PAWN || playerColor == Piece::WHITE_QUEEN);
    bool isWhitePiece = (movingPiece == Piece::WHITE_PAWN || movingPiece == Piece::WHITE_QUEEN);
    bool isBlackPiece = (movingPiece == Piece::BLACK_PAWN || movingPiece == Piece::BLACK_QUEEN);

    // Sprawdzenie czy gracz porusza własnym pionkiem
    if (isWhitePlayer && !isWhitePiece) return false;
    if (!isWhitePlayer && !isBlackPiece) return false;

    // Sprawdzenie czy docelowe pole jest puste
    if (grid[toY][toX] != Piece::EMPTY) return false;

    // Sprawdzenie czy ruch jest po skosie
    int dx = toX - fromX;
    int dy = toY - fromY;
    if (std::abs(dx) != std::abs(dy)) return false;

    int distance = std::abs(dx);
    if (distance != 1 && distance != 2) return false;

    // Zwykłe pionki nie mogą się cofać
    if (movingPiece == Piece::WHITE_PAWN && dy > 0) return false; // Biale ida w gore (Y maleje)
    if (movingPiece == Piece::BLACK_PAWN && dy < 0) return false; // Czarne ida w dol (Y rosnie)

    // Logika zwyklego ruchu i bicia
    if (distance == 1) 
    {
        // Weryfikacja czy nie ma wymuszonego bicia
        if (hasForcedCapture(playerColor)) 
        {
            return false;
        }
        return true;
    } 
    else if (distance == 2) 
    {
        // Weryfikacja co stoi po środku
        int midX = (fromX + toX) / 2;
        int midY = (fromY + toY) / 2;
        Piece midPiece = grid[midY][midX];

        if (midPiece == Piece::EMPTY) return false;

        bool isMidWhite = (midPiece == Piece::WHITE_PAWN || midPiece == Piece::WHITE_QUEEN);
        bool isMidBlack = (midPiece == Piece::BLACK_PAWN || midPiece == Piece::BLACK_QUEEN);

        // Sprawdzenie przed biciem własnego pionka
        if (isWhitePlayer && isMidWhite) return false;
        if (!isWhitePlayer && isMidBlack) return false;

        return true;
    }

    return false;
}

void Board::makeMove(int fromX, int fromY, int toX, int toY) {
    Piece movingPiece = grid[fromY][fromX];
    
    // Przeniesienie pionka na nowe pole
    grid[toY][toX] = movingPiece;
    grid[fromY][fromX] = Piece::EMPTY;

    // Sprawdzenie czy było bicie
    if (std::abs(toX - fromX) == 2) 
    {
        int midX = (fromX + toX) / 2;
        int midY = (fromY + toY) / 2;
        grid[midY][midX] = Piece::EMPTY;
    }

    // Promocja na damke po dojściu do końca planszy
    if (movingPiece == Piece::WHITE_PAWN && toY == 0) 
    {
        grid[toY][toX] = Piece::WHITE_QUEEN;
    } 
    else if (movingPiece == Piece::BLACK_PAWN && toY == 7) 
    {
        grid[toY][toX] = Piece::BLACK_QUEEN;
    }
}

Piece Board::checkWinCondition() const 
{
    bool whiteHasPieces = false;
    bool blackHasPieces = false;

    for (int y = 0; y < 8; ++y) 
    {
        for (int x = 0; x < 8; ++x) 
        {
            Piece p = grid[y][x];
            if (p == Piece::WHITE_PAWN || p == Piece::WHITE_QUEEN) 
            {
                whiteHasPieces = true;
            }
            else if (p == Piece::BLACK_PAWN || p == Piece::BLACK_QUEEN) 
            {
                blackHasPieces = true;
            }
        }
    }

    // Jeśli czarni nie mają już czym grać, wygrywają biali
    if (!blackHasPieces) return Piece::WHITE_PAWN;
    
    // Jeśli biali nie mają już czym grać, wygrywają czarni
    if (!whiteHasPieces) return Piece::BLACK_PAWN;

    // Obie strony mają pionki, gramy dalej
    return Piece::EMPTY;
}