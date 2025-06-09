#pragma once
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <tuple>
#include <limits> // dla INT_MIN / INT_MAX
#include "GameSettings.hpp" 


constexpr int TILE_SIZE = 80;



class Board {
public:
    

    static const int SIZE = 8;
    Board();
    void draw(sf::RenderWindow& window) const;
    void play(sf::RenderWindow& window, const GameSettings& settings);
    void loadScenario(int id); //do testów tylko
    


private:
    mutable int selectedRow = -1, selectedCol = -1;
    mutable std::vector<std::pair<int, int>> possibleMoves;
    mutable bool inCombo = false;
    mutable int comboRow = -1, comboCol = -1;
    



    enum Piece { EMPTY, WHITE, WHITE_KING, BLACK, BLACK_KING };
    std::vector<Piece> board;
    
    struct MoveBackup {
        int x1, y1, x2, y2;
        Piece movedPiece;
        Piece capturedPiece;
        int capturedX = -1, capturedY = -1;
        bool wasKingBefore = false;

        //wartoœci do combo
        bool inComboBefore;
        int comboRowBefore, comboColBefore;
        int selectedRowBefore, selectedColBefore;
    };



    bool isValidMove(int x1, int y1, int x2, int y2, bool whiteTurn, bool& isCapture) const;
    bool movePiece(int x1, int y1, int x2, int y2, bool whiteTurn);
    std::string pieceToStr(Piece p) const;
    bool isInside(int x, int y) const;
    bool isPlayerPiece(Piece p, bool whiteTurn) const;
    Piece get(int x, int y) const;
    void set(int x, int y, Piece value);
    bool hasCapture(bool whiteTurn) const;
    void updatePossibleMoves(int x, int y, bool whiteTurn) const;
    int evaluate() const;
    std::vector<std::tuple<int, int, int, int>> generateAllMoves(bool whiteTurn) const;
    int minimax(int depth, int alpha, int beta, bool maximizingPlayer, bool whiteTurn);
    std::tuple<int, int, int, int> findBestMove(bool whiteTurn, int depth);
    bool canBeCaptured(int row, int col, bool isWhite) const;
    bool isGameOver(bool whiteTurn) const;
    MoveBackup applyMove(int x1, int y1, int x2, int y2, bool whiteTurn);
    void undoMove(const MoveBackup& backup);


};