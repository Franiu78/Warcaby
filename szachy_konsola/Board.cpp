#include "Board.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include <optional>
#include <SFML/Graphics.hpp>


Board::Board() {
    board.resize(SIZE * SIZE, EMPTY);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < SIZE; ++j)
            if ((i + j) % 2 == 1)
                set(i, j, BLACK);

    for (int i = 5; i < SIZE; ++i)
        for (int j = 0; j < SIZE; ++j)
            if ((i + j) % 2 == 1)
                set(i, j, WHITE);
}


void Board::draw(sf::RenderWindow& window) const {
    constexpr int TILE_SIZE = 80;
    sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    sf::CircleShape piece(TILE_SIZE / 2 - 10);

    sf::Color light(232, 235, 239);
    sf::Color dark(125, 135, 150);

    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            tile.setPosition(sf::Vector2f(col * TILE_SIZE, row * TILE_SIZE));
            tile.setFillColor((row + col) % 2 == 0 ? light : dark);
            window.draw(tile);

            Piece p = get(row, col);
            if (p == EMPTY) continue;

            piece.setPosition(sf::Vector2f(col * TILE_SIZE + 10, row * TILE_SIZE + 10));
            piece.setFillColor((p == WHITE || p == WHITE_KING) ? sf::Color::White : sf::Color::Black);
            window.draw(piece);

            // rysowanie "korony"
            if (p == WHITE_KING || p == BLACK_KING) {
                sf::CircleShape crown(TILE_SIZE / 2 - 25);
                crown.setPosition(sf::Vector2f(col * TILE_SIZE + 25, row * TILE_SIZE + 25));
                crown.setFillColor(sf::Color(255, 215, 0)); // z³oty kolor
                window.draw(crown);
            }
        }
    }

    // Rysuj zaznaczone pole (jeœli jakieœ jest)
    if (selectedRow != -1 && selectedCol != -1) {
        sf::RectangleShape highlight(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        highlight.setPosition(sf::Vector2f(selectedCol * TILE_SIZE, selectedRow * TILE_SIZE));
        highlight.setFillColor(sf::Color(255, 255, 0, 100));  // pó³przezroczysty ¿ó³ty
        window.draw(highlight);
    }
    // zaznacz mo¿liwe ruchy
    for (const auto& move : possibleMoves) {
        sf::RectangleShape hint(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        hint.setPosition(sf::Vector2f(move.second * TILE_SIZE, move.first * TILE_SIZE));
        hint.setFillColor(sf::Color(0, 255, 0, 100)); // zielony pó³przezroczysty
        window.draw(hint);
    }

}


std::string Board::pieceToStr(Piece p) const {
    switch (p) {
    case WHITE: return "o";
    case WHITE_KING: return "O";
    case BLACK: return "x";
    case BLACK_KING: return "X";
    case EMPTY: default: return ".";
    }
}

bool Board::isInside(int x, int y) const {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

bool Board::isPlayerPiece(Piece p, bool whiteTurn) const {
    if (whiteTurn) return p == WHITE || p == WHITE_KING;
    return p == BLACK || p == BLACK_KING;
}

Board::Piece Board::get(int x, int y) const {
    return board[x * SIZE + y];
}

void Board::set(int x, int y, Piece value) {
    board[x * SIZE + y] = value;
}

bool Board::isValidMove(int x1, int y1, int x2, int y2, bool whiteTurn, bool& isCapture) const {
    if (!isInside(x1, y1) || !isInside(x2, y2)) return false;
    Piece p = get(x1, y1);
    Piece target = get(x2, y2);
    if (target != EMPTY) return false;

    int dx = x2 - x1;
    int dy = y2 - y1;

    // Zwyk³y ruch o jedno pole
    if (std::abs(dx) == 1 && std::abs(dy) == 1) {
        if (inCombo) return false; // nie mo¿na wykonaæ zwyk³ego ruchu w trakcie kombinacji
        if (hasCapture(whiteTurn)) return false; // nie mo¿na wykonaæ zwyk³ego ruchu je¿eli jest bicie 
        if ((p == WHITE && dx == -1) || (p == BLACK && dx == 1)) return true;
        if (p == WHITE_KING || p == BLACK_KING) return true;
    }

    // Bicie
    if (std::abs(dx) == 2 && std::abs(dy) == 2) {
        int mx = x1 + dx / 2;
        int my = y1 + dy / 2;
        Piece mid = get(mx, my);
        if (mid != EMPTY && !isPlayerPiece(mid, whiteTurn)) {
            if ((p == WHITE && dx == -2) || (p == BLACK && dx == 2)) {
                isCapture = true;
                return true;
            }
            if (p == WHITE_KING || p == BLACK_KING) {
                isCapture = true;
                return true;
            }
        }
    }

    return false;
}

bool Board::movePiece(int x1, int y1, int x2, int y2, bool whiteTurn) {
    bool isCapture = false;
    if (!isValidMove(x1, y1, x2, y2, whiteTurn, isCapture)) return false;

    set(x2, y2, get(x1, y1));
    set(x1, y1, EMPTY);

    // Promocja na damkê
    Piece moved = get(x2, y2);
    if (moved == WHITE && x2 == 0) {
        moved = WHITE_KING;
        set(x2, y2, moved);
    }
    else if (moved == BLACK && x2 == SIZE - 1) {
        moved = BLACK_KING;
        set(x2, y2, moved);
    }

    if (isCapture) {
        int mx = (x1 + x2) / 2;
        int my = (y1 + y2) / 2;
        set(mx, my, EMPTY);

        bool canContinue = false;
        int dxs[] = { -2, -2, 2, 2 };
        int dys[] = { -2, 2, -2, 2 };
        Piece current = get(x2, y2);
        bool isKing = (current == WHITE_KING || current == BLACK_KING);

        for (int i = 0; i < 4; ++i) {
            int nx = x2 + dxs[i];
            int ny = y2 + dys[i];
            int mx = x2 + dxs[i] / 2;
            int my = y2 + dys[i] / 2;

            if (!isInside(nx, ny)) continue;
            if (get(nx, ny) != EMPTY) continue;

            Piece mid = get(mx, my);
            if (mid == EMPTY || isPlayerPiece(mid, whiteTurn)) continue;

            // pionki mog¹ biæ tylko do przodu
            if (!isKing) {
                if (whiteTurn && dxs[i] != -2) continue;
                if (!whiteTurn && dxs[i] != 2) continue;
            }

            canContinue = true;
            break;
        }


        if (canContinue) {
            inCombo = true;
            comboRow = selectedRow = x2;
            comboCol = selectedCol = y2;
        }
        else {
            inCombo = false;
            comboRow = comboCol = -1;
            selectedRow = selectedCol = -1;
        }

        return true;
    }

    // Koniec tury
    inCombo = false;
    comboRow = comboCol = -1;
    selectedRow = selectedCol = -1;
    return true;
}


bool Board::hasCapture(bool whiteTurn) const {
    const int dxs[] = { -2, -2, 2, 2 };
    const int dys[] = { -2, 2, -2, 2 };

    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            Piece p = get(x, y);
            if (!isPlayerPiece(p, whiteTurn)) continue;

            bool isKing = (p == WHITE_KING || p == BLACK_KING);

            for (int d = 0; d < 4; ++d) {
                int x2 = x + dxs[d];
                int y2 = y + dys[d];
                int mx = x + dxs[d] / 2;
                int my = y + dys[d] / 2;

                if (!isInside(x2, y2) || !isInside(mx, my)) continue;
                if (get(x2, y2) != EMPTY) continue;

                Piece mid = get(mx, my);
                if (mid == EMPTY || isPlayerPiece(mid, whiteTurn)) continue;

                // Pionki mog¹ biæ tylko w przód
                if (!isKing) {
                    if (whiteTurn && dxs[d] != -2) continue;
                    if (!whiteTurn && dxs[d] != 2) continue;
                }

                return true;
            }
        }
    }

    return false;
}



void Board::updatePossibleMoves(int row, int col, bool whiteTurn) const {
    possibleMoves.clear();

    Piece p = get(row, col);
    if (p == EMPTY) return;

    bool isCapturePossible = hasCapture(whiteTurn);

    const int dxs[] = { -2, -2, 2, 2, -1, -1, 1, 1 };
    const int dys[] = { -2, 2, -2, 2, -1, 1, -1, 1 };

    for (int i = 0; i < 8; ++i) {
        int x2 = row + dxs[i];
        int y2 = col + dys[i];
        bool isCapture = false;

        if (isValidMove(row, col, x2, y2, whiteTurn, isCapture)) {
            // Jeœli jakiekolwiek bicie istnieje, dopuszczaj TYLKO ruchy bêd¹ce biciem
            if ((inCombo || isCapturePossible) && isCapture)
                possibleMoves.push_back({ x2, y2 });
            else if (!isCapturePossible && !inCombo)
                possibleMoves.push_back({ x2, y2 });



        }
    }
}

void Board::loadScenario(int id) {
    std::fill(board.begin(), board.end(), EMPTY);

    // Domyœlnie usuñ zaznaczenie
    selectedRow = selectedCol = -1;
    possibleMoves.clear();

    switch (id) {
    case 1: // Bicie obowi¹zkowe — inny pionek nie mo¿e siê ruszyæ
        set(5, 2, WHITE);
        set(6, 6, WHITE);
        set(4, 3, BLACK);
        break;

    case 2: // Bicie wielokrotne
        set(5, 2, WHITE);
        set(4, 3, BLACK);
        set(4, 5, BLACK);
        set(2, 3, BLACK);
        break;

    case 3: // Próba ruchu pionka w ty³ (niedozwolona)
        set(4, 4, WHITE);
        break;

    case 4: // Bicie do ty³u dozwolone
        set(4, 4, WHITE);
        set(3, 3, BLACK);
        break;

    case 5: // Promocja na damkê
        set(1, 2, WHITE);
        break;

    case 6: // Ruch damki w dowolnym kierunku
        set(4, 4, WHITE_KING);
        break;

    case 7: // Bicie damk¹ w ty³ i przód
        set(4, 4, WHITE_KING);
        set(3, 3, BLACK);
        set(5, 5, BLACK);
        break;

    case 8: // Tylko jeden pionek mo¿e biæ — inne nie mog¹ siê ruszyæ
        set(5, 0, WHITE);
        set(5, 6, WHITE);
        set(4, 1, BLACK);
        break;
    case 9: // Tylko jeden pionek mo¿e kontynuowaæ bicie
        set(4, 1, WHITE);
        set(5, 6, WHITE);
        set(3, 2, BLACK);
        set(1, 4, BLACK);
        set(4, 5, BLACK);
        break;
    case 10: // B³¹d: wybrany pionek mo¿e ruszyæ siê bez bicia
        set(5, 2, WHITE);   // Bije w prawo
        set(4, 3, BLACK);
        set(5, 6, WHITE);   // Ten pionek nie mo¿e siê ruszyæ
        break;

    case 11: // AI mo¿e wygraæ ruchem bicia
        set(2, 3, BLACK); // AI (Black) pionek
        set(3, 4, WHITE); // Gracz (White) pionek
        break;



    default:
        break;
    }
}

void Board::play(sf::RenderWindow& window, const GameSettings& settings) {
    constexpr int TILE_SIZE = 80;
    bool whiteTurn = true;
    bool wasMouseDown = false;
    sf::Clock aiClock;
    const sf::Time aiDelay = sf::milliseconds(500);  // opóŸnienie AI

    while (window.isOpen()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            window.close();

        // Czy ruch wykonuje AI
        bool currentAI = (whiteTurn && settings.whitePlayer == PlayerType::AI) ||
            (!whiteTurn && settings.blackPlayer == PlayerType::AI);

        if (generateAllMoves(whiteTurn).empty()) {
            std::cout << (whiteTurn ? "White" : "Black") << " has no moves. Game over.\n";
            break; // zakoñcz grê
        }

        // G³êbokoœæ MinMax dla obecnego gracza
        int depth = whiteTurn ? settings.whiteDepth : settings.blackDepth;

        // Ruch AI
        if (currentAI && aiClock.getElapsedTime() > aiDelay) {
            if (!inCombo) {
                auto [x1, y1, x2, y2] = findBestMove(whiteTurn, whiteTurn ? settings.whiteDepth : settings.blackDepth);
                if (x1 != -1 && movePiece(x1, y1, x2, y2, whiteTurn)) {
                    if (!inCombo) whiteTurn = !whiteTurn;
                    aiClock.restart();
                }
            }
            else {
                // Kontynuacja kombinacji
                auto moves = generateAllMoves(whiteTurn);
                for (auto [x1, y1, x2, y2] : moves) {
                    if (x1 == comboRow && y1 == comboCol) {
                        if (movePiece(x1, y1, x2, y2, whiteTurn)) {
                            if (!inCombo) whiteTurn = !whiteTurn;
                            aiClock.restart();
                            break;
                        }
                    }
                }
            }
        }

      


        // Obs³uga myszy — tylko jeœli gracz to cz³owiek
        if (!currentAI && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (!wasMouseDown) {
                wasMouseDown = true;
                sf::Vector2i pos = sf::Mouse::getPosition(window);
                int row = pos.y / TILE_SIZE;
                int col = pos.x / TILE_SIZE;

                if (selectedRow == -1) {
                    Piece p = get(row, col);
                    if (isPlayerPiece(p, whiteTurn)) {
                        bool isCaptureAvailable = hasCapture(whiteTurn);
                        bool canCapture = false;

                        if (inCombo) {
                            if (row == comboRow && col == comboCol) {
                                selectedRow = row;
                                selectedCol = col;
                                updatePossibleMoves(row, col, whiteTurn);
                            }
                        }
                        else {
                            for (int dx : {-2, 2}) {
                                for (int dy : {-2, 2}) {
                                    int x2 = row + dx;
                                    int y2 = col + dy;
                                    bool isCap = false;
                                    if (isValidMove(row, col, x2, y2, whiteTurn, isCap) && isCap)
                                        canCapture = true;
                                }
                            }

                            if (!isCaptureAvailable || canCapture) {
                                selectedRow = row;
                                selectedCol = col;
                                updatePossibleMoves(row, col, whiteTurn);
                            }
                        }
                    }
                }
                else {
                    if (movePiece(selectedRow, selectedCol, row, col, whiteTurn)) {
                        if (inCombo) {
                            updatePossibleMoves(comboRow, comboCol, whiteTurn);
                        }
                        else {
                            whiteTurn = !whiteTurn;
                            possibleMoves.clear();
                        }
                    }

                    if (!inCombo) {
                        selectedRow = selectedCol = -1;
                        possibleMoves.clear();
                    }
                }
            }
        }
        else {
            wasMouseDown = false;
        }

        // Rysowanie
        window.clear();
        draw(window);
        window.display();
    }
}

bool Board::canBeCaptured(int row, int col, bool isWhite) const {
    const int dirs[4][2] = { {-2,-2}, {-2,2}, {2,-2}, {2,2} };

    for (auto [dx, dy] : dirs) {
        int x2 = row + dx;
        int y2 = col + dy;
        int mx = row + dx / 2;
        int my = col + dy / 2;

        if (!isInside(x2, y2) || !isInside(mx, my)) continue;

        Piece middle = get(mx, my);
        Piece destination = get(x2, y2);

        if ((isWhite && isPlayerPiece(middle, false)) || (!isWhite && isPlayerPiece(middle, true))) {
            if (destination == EMPTY)
                return true;
        }
    }
    return false;
}

int Board::evaluate() const {
    const int PAWN_VALUE = 100;
    const int KING_VALUE = 200;
    const int THREAT_PENALTY = 200;

    int score = 0;

    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            Piece p = get(i, j);
            if (p == EMPTY) continue;

            bool isWhite = (p == WHITE || p == WHITE_KING);
            int value = 0;

            if (p == WHITE || p == BLACK)
                value += PAWN_VALUE;
            else
                value += KING_VALUE;

            if (canBeCaptured(i, j, isWhite))
                value -= THREAT_PENALTY;

            score += isWhite ? value : -value;
        }
    }

    return score;
}


std::vector<std::tuple<int, int, int, int>> Board::generateAllMoves(bool whiteTurn) const {
    if (inCombo) {
        std::vector<std::tuple<int, int, int, int>> comboMoves;

        for (int dx = -2; dx <= 2; ++dx) {
            for (int dy = -2; dy <= 2; ++dy) {
                if (dx == 0 || dy == 0) continue;

                int x2 = comboRow + dx;
                int y2 = comboCol + dy;
                bool isCap = false;
                if (isValidMove(comboRow, comboCol, x2, y2, whiteTurn, isCap) && isCap) {
                    comboMoves.emplace_back(comboRow, comboCol, x2, y2);
                }
            }
        }

        return comboMoves;
    }

    std::vector<std::tuple<int, int, int, int>> moves;
    bool mustCapture = hasCapture(whiteTurn);  // SPRAWDZENIE OBOWI¥ZKOWEGO BICIA

    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            Piece p = get(x, y);
            if (!isPlayerPiece(p, whiteTurn)) continue;

            for (int dx = -2; dx <= 2; ++dx) {
                for (int dy = -2; dy <= 2; ++dy) {
                    if (dx == 0 || dy == 0) continue;
                    int x2 = x + dx;
                    int y2 = y + dy;
                    bool isCap = false;
                    if (isValidMove(x, y, x2, y2, whiteTurn, isCap)) {
                        if (!mustCapture || isCap) {
                            moves.emplace_back(x, y, x2, y2);
                        }
                    }
                }
            }
        }
    }

    return moves;
}




int Board::minimax(int depth, int alpha, int beta, bool maximizingPlayer, bool whiteTurn) {
    if (isGameOver(whiteTurn))
        return whiteTurn ? INT_MIN + 1 : INT_MAX - 1;

    if (depth == 0)
        return evaluate();

    auto moves = generateAllMoves(whiteTurn);
    if (moves.empty())
        return maximizingPlayer ? -10000 : 10000;

    int bestEval = maximizingPlayer ? INT_MIN : INT_MAX;

    for (auto [x1, y1, x2, y2] : moves) {
        MoveBackup backup = applyMove(x1, y1, x2, y2, whiteTurn);

        // Czy po tym ruchu AI ma kontynuowaæ combo?
        bool continueCombo = inCombo && comboRow == x2 && comboCol == y2;

        int eval;
        if (continueCombo) {
            // Ten sam gracz gra dalej — g³êbokoœæ nie spada
            eval = minimax(depth, alpha, beta, maximizingPlayer, whiteTurn);
        }
        else {
            // Nastêpny gracz — prze³¹cz kolor i zmniejsz g³êbokoœæ
            eval = minimax(depth - 1, alpha, beta, !maximizingPlayer, !whiteTurn);
        }

        undoMove(backup);

        if (maximizingPlayer) {
            bestEval = std::max(bestEval, eval);
            alpha = std::max(alpha, eval);
        }
        else {
            bestEval = std::min(bestEval, eval);
            beta = std::min(beta, eval);
        }

        if (beta <= alpha)
            break;
    }

    return bestEval;
}




std::tuple<int, int, int, int> Board::findBestMove(bool whiteTurn, int depth) {
    auto moves = generateAllMoves(whiteTurn);
    if (moves.empty()) {
        return { -1, -1, -1, -1 }; // brak ruchów = koniec gry
    }


    int bestScore = whiteTurn ? INT_MIN : INT_MAX;
    std::tuple<int, int, int, int> bestMove = { -1, -1, -1, -1 };

    
    for (const auto& [x1, y1, x2, y2] : moves) {
        MoveBackup backup = applyMove(x1, y1, x2, y2, whiteTurn);
        int score = minimax(depth - 1, INT_MIN, INT_MAX, !whiteTurn, !whiteTurn);;
        undoMove(backup);
        

        if ((whiteTurn && score > bestScore) || (!whiteTurn && score < bestScore)) {
            bestScore = score;
            bestMove = { x1, y1, x2, y2 };
        }
    }

    return bestMove;
}

bool Board::isGameOver(bool whiteTurn) const {
    return generateAllMoves(whiteTurn).empty();
}

Board::MoveBackup Board::applyMove(int x1, int y1, int x2, int y2, bool whiteTurn) {
    MoveBackup backup;
    backup.x1 = x1;
    backup.y1 = y1;
    backup.x2 = x2;
    backup.y2 = y2;
    backup.movedPiece = get(x1, y1);
    backup.wasKingBefore = (backup.movedPiece == WHITE_KING || backup.movedPiece == BLACK_KING);

    // Zapamiêtaj stan combo
    backup.inComboBefore = inCombo;
    backup.comboRowBefore = comboRow;
    backup.comboColBefore = comboCol;
    backup.selectedRowBefore = selectedRow;
    backup.selectedColBefore = selectedCol;

    // Wykonaj ruch
    set(x2, y2, backup.movedPiece);
    set(x1, y1, EMPTY);

    // Bicie
    if (std::abs(x2 - x1) == 2) {
        int mx = (x1 + x2) / 2;
        int my = (y1 + y2) / 2;
        backup.capturedX = mx;
        backup.capturedY = my;
        backup.capturedPiece = get(mx, my);
        set(mx, my, EMPTY);
    }
    else {
        backup.capturedPiece = EMPTY;
    }

    // Promocja
    if (backup.movedPiece == WHITE && x2 == 0) set(x2, y2, WHITE_KING);
    if (backup.movedPiece == BLACK && x2 == SIZE - 1) set(x2, y2, BLACK_KING);

    // Sprawdzenie kontynuacji combosa
    bool canContinue = false;
    Piece current = get(x2, y2);
    bool isKing = (current == WHITE_KING || current == BLACK_KING);
    const int dxs[] = { -2, -2, 2, 2 };
    const int dys[] = { -2, 2, -2, 2 };

    for (int i = 0; i < 4; ++i) {
        int nx = x2 + dxs[i];
        int ny = y2 + dys[i];
        int mx = x2 + dxs[i] / 2;
        int my = y2 + dys[i] / 2;

        if (!isInside(nx, ny) || !isInside(mx, my)) continue;
        if (get(nx, ny) != EMPTY) continue;
        Piece mid = get(mx, my);
        if (mid == EMPTY || isPlayerPiece(mid, whiteTurn)) continue;

        if (!isKing) {
            if (whiteTurn && dxs[i] != -2) continue;
            if (!whiteTurn && dxs[i] != 2) continue;
        }

        canContinue = true;
        break;
    }

    if (canContinue) {
        inCombo = true;
        comboRow = x2;
        comboCol = y2;
        selectedRow = x2;
        selectedCol = y2;
    }
    else {
        inCombo = false;
        comboRow = comboCol = -1;
        selectedRow = selectedCol = -1;
    }

    return backup;
}


void Board::undoMove(const MoveBackup& backup) {
    set(backup.x1, backup.y1, backup.movedPiece);
    set(backup.x2, backup.y2, EMPTY);

    if (backup.capturedPiece != EMPTY) {
        set(backup.capturedX, backup.capturedY, backup.capturedPiece);
    }

    // Przywróæ stan combo
    inCombo = backup.inComboBefore;
    comboRow = backup.comboRowBefore;
    comboCol = backup.comboColBefore;
    selectedRow = backup.selectedRowBefore;
    selectedCol = backup.selectedColBefore;
}




