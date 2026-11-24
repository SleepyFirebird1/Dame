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
    Piece grid[8][8];

  public:
    Board();

    Piece getPiece(int x, int y) const;
    void setPiece(int x, int y, Piece piece);

    bool hasForcedCapture(Piece playerColor) const;
    bool isMoveValid(int fromX, int fromY, int toX, int toY,
                     Piece playerColor) const;
    void makeMove(int fromX, int fromY, int toX, int toY);

    Piece checkWinCondition() const;
};