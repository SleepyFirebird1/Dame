#pragma once

enum class Piece 
{
    EMPTY = 0,
    WHITE_PAWN = 1,
    WHITE_QUEEN = 2,
    BLACK_PAWN = 3,
    BLACK_QUEEN = 4
};

class Board
{
    private:
        // Plansza
        Piece grid[8][8];
    public:
        // Konstruktor
        Board();

        Piece getPiece(int x, int y) const;
        void setPiece(int x, int y, Piece piece);

        // Metody logiczne
        bool hasForcedCapture(Piece playerColor) const;
        bool isMoveValid(int fromX, int fromY, int toX, int toY, Piece playerColor) const;

        // Metoda poruszająca, która będzie wykrywać bicie automatycznie
        void makeMove(int fromX, int fromY, int toX, int toY);

        Piece checkWinCondition() const;


};