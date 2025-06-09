#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Board.hpp"
#include "GameSettings.hpp"


void drawOption(sf::RenderWindow& window, const sf::Font& font, const std::string& label, int x, int y, bool selected) {
    sf::Text text(font);
    text.setString(label);
    text.setCharacterSize(30);
    text.setFillColor(selected ? sf::Color::Red : sf::Color::White);
    text.setPosition(sf::Vector2f(x, y));
    window.draw(text);
}




GameSettings showMenu(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        throw std::runtime_error("Could not load font arial.ttf");
    }

    int selectedOption = 0;
    GameSettings settings;
    bool wasPressed = false;

    std::vector<std::string> options = {
        "Human vs Human",
        "Human (W) vs AI (B)",
        "AI (W) vs Human (B)",
        "AI vs AI",
        "White Depth: 3",
        "Black Depth: 3"
    };

    while (window.isOpen()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            if (!wasPressed) {
                selectedOption = (selectedOption - 1 + options.size()) % options.size();
                wasPressed = true;
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            if (!wasPressed) {
                selectedOption = (selectedOption + 1) % options.size();
                wasPressed = true;
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            if (!wasPressed) {
                if (selectedOption == 4 && settings.whiteDepth > 1) {
                    --settings.whiteDepth;
                    options[4] = "White Depth: " + std::to_string(settings.whiteDepth);
                }
                else if (selectedOption == 5 && settings.blackDepth > 1) {
                    --settings.blackDepth;
                    options[5] = "Black Depth: " + std::to_string(settings.blackDepth);
                }
                wasPressed = true;
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            if (!wasPressed) {
                if (selectedOption == 4) {
                    ++settings.whiteDepth;
                    options[4] = "White Depth: " + std::to_string(settings.whiteDepth);
                }
                else if (selectedOption == 5) {
                    ++settings.blackDepth;
                    options[5] = "Black Depth: " + std::to_string(settings.blackDepth);
                }
                wasPressed = true;
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
            if (!wasPressed) {
                switch (selectedOption) {
                case 0: settings.whitePlayer = PlayerType::Human; settings.blackPlayer = PlayerType::Human; return settings;
                case 1: settings.whitePlayer = PlayerType::Human; settings.blackPlayer = PlayerType::AI; return settings;
                case 2: settings.whitePlayer = PlayerType::AI; settings.blackPlayer = PlayerType::Human; return settings;
                case 3: settings.whitePlayer = PlayerType::AI; settings.blackPlayer = PlayerType::AI; return settings;
                }
                wasPressed = true;
            }
        }
        else {
            wasPressed = false;
        }

        window.clear();
        for (int i = 0; i < options.size(); ++i) {
            drawOption(window, font, options[i], 100, 100 + i * 50, selectedOption == i);
        }
        window.display();
    }

    return settings;
}



int main() {
    sf::RenderWindow window(sf::VideoMode({ 640, 640 }), "Warcaby SFML");
    window.setFramerateLimit(60);

    GameSettings settings = showMenu(window);

    if (!window.isOpen()) return 0;

    Board board;
    
   // board.loadScenario(11);
    board.play(window, settings);


    return 0;
}
