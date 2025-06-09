#pragma once

enum class PlayerType { Human, AI };

struct GameSettings {
    PlayerType whitePlayer = PlayerType::Human;
    PlayerType blackPlayer = PlayerType::Human;
    int whiteDepth = 3;
    int blackDepth = 3;
};


