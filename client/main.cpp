#include "NetworkClient.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>

int main()
{
    NetworkClient client;

    // Połączenie z serwerem
    bool connected = client.connectToServer("127.0.0.1", 8080);
    if (!connected)
    {
        std::cerr << "Blad: Nie udalo sie polaczyc z serwerem.\n";
    }

    // Wczytanie czcionki systemowej
    sf::Font font;
    bool hasFont =
        font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
    if (!hasFont)
    {
        hasFont = font.openFromFile("/System/Library/Fonts/Helvetica.ttc");
    }

    // Okno oczekiwania na sesję
    sf::RenderWindow waitWindow(sf::VideoMode({360, 140}), "Warcaby Sieciowe");
    waitWindow.setFramerateLimit(60);

    bool sessionStarted = false;

    while (waitWindow.isOpen())
    {
        while (const std::optional<sf::Event> event = waitWindow.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                waitWindow.close();
            }
        }

        // Otwarcie właściwej gry po otrzymaniu koloru od serwera
        if (client.getMyColor() != 0)
        {
            sessionStarted = true;
            waitWindow.close();
            break;
        }

        waitWindow.clear(sf::Color(40, 40, 40));

        if (hasFont)
        {
            sf::Text waitText(font);
            waitText.setCharacterSize(18);

            if (connected)
            {
                waitText.setString("Oczekiwanie na sesje...");
                waitText.setFillColor(sf::Color(255, 215, 0));
            }
            else
            {
                waitText.setString("Blad polaczenia z serwerem");
                waitText.setFillColor(sf::Color(255, 80, 80));
            }

            sf::FloatRect bounds = waitText.getLocalBounds();
            waitText.setOrigin(bounds.getCenter());
            waitText.setPosition(sf::Vector2f{180.f, 70.f});

            waitWindow.draw(waitText);
        }

        waitWindow.display();
    }

    if (!sessionStarted)
    {
        return 0;
    }

    // Konfiguracja okna gry
    constexpr int tileSize = 60;
    constexpr float tileSizeF = static_cast<float>(tileSize);
    sf::RenderWindow window(sf::VideoMode({480, 540}), "Warcaby Sieciowe");
    window.setFramerateLimit(60);

    // Współrzędne aktualnie zaznaczonego pola
    int selectedX = -1;
    int selectedY = -1;

    while (window.isOpen())
    {
        // Obsługa zdarzeń okna i myszy
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (const auto *mousePress =
                    event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mousePress->button == sf::Mouse::Button::Left)
                {
                    // Przeliczenie pozycji kursora na indeksy planszy
                    int gridX = mousePress->position.x / tileSize;
                    int gridY = mousePress->position.y / tileSize;

                    if (gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8)
                    {
                        if (selectedX == -1)
                        {
                            // Zaznaczenie pionka
                            selectedX = gridX;
                            selectedY = gridY;
                        }
                        else
                        {
                            // Odznaczenie przy ponownym kliknięciu w to samo
                            // pole
                            if (selectedX == gridX && selectedY == gridY)
                            {
                                selectedX = -1;
                                selectedY = -1;
                            }
                            else
                            {
                                // Wysłanie ruchu do serwera
                                client.sendMove(selectedX, selectedY, gridX,
                                                gridY);
                                selectedX = -1;
                                selectedY = -1;
                            }
                        }
                    }
                }
            }
        }

        window.clear(sf::Color(40, 40, 40));

        // Rysowanie planszy i pionków
        const Board &board = client.getBoard();

        for (int y = 0; y < 8; ++y)
        {
            for (int x = 0; x < 8; ++x)
            {
                sf::RectangleShape square(sf::Vector2f{tileSizeF, tileSizeF});
                square.setPosition(sf::Vector2f{x * tileSizeF, y * tileSizeF});

                if ((x + y) % 2 == 0)
                {
                    square.setFillColor(sf::Color(240, 217, 181));
                }
                else
                {
                    square.setFillColor(sf::Color(181, 136, 99));
                }

                // Podświetlenie zaznaczonego pola
                if (x == selectedX && y == selectedY)
                {
                    square.setFillColor(sf::Color(130, 151, 105));
                }

                window.draw(square);

                Piece p = board.getPiece(x, y);
                if (p != Piece::EMPTY)
                {
                    sf::CircleShape piece(24.f);
                    piece.setPosition(
                        sf::Vector2f{x * tileSizeF + 6.f, y * tileSizeF + 6.f});

                    if (p == Piece::WHITE_PAWN || p == Piece::WHITE_QUEEN)
                    {
                        piece.setFillColor(sf::Color(245, 245, 245));
                    }
                    else
                    {
                        piece.setFillColor(sf::Color(40, 40, 40));
                    }

                    // Obramowanie dla damki
                    if (p == Piece::WHITE_QUEEN || p == Piece::BLACK_QUEEN)
                    {
                        piece.setOutlineThickness(3.f);
                        piece.setOutlineColor(sf::Color(255, 215, 0));
                    }

                    window.draw(piece);
                }
            }
        }

        // Wyświetlanie paska statusu na dole okna
        if (hasFont)
        {
            int myColor = client.getMyColor();

            // Informacja o przypisanym kolorze
            sf::Text infoText(font);
            infoText.setCharacterSize(14);
            infoText.setPosition(sf::Vector2f{12.f, 483.f});

            if (myColor == 1)
            {
                infoText.setString("Grasz jako: BIALE");
                infoText.setFillColor(sf::Color::White);
            }
            else if (myColor == 3)
            {
                infoText.setString("Grasz jako: CZARNE");
                infoText.setFillColor(sf::Color(180, 180, 180));
            }
            else
            {
                infoText.setString("Oczekiwanie na przeciwnika...");
                infoText.setFillColor(sf::Color(255, 215, 0));
            }
            window.draw(infoText);

            // Informacja o aktualnej turze
            sf::Text turnText(font);
            turnText.setCharacterSize(14);
            turnText.setPosition(sf::Vector2f{12.f, 501.f});

            int currentTurn = client.getCurrentTurn();

            if (currentTurn == myColor && myColor != 0)
            {
                turnText.setString("Obecny ruch: TWOJ");
                turnText.setFillColor(sf::Color(100, 255, 100));
            }
            else
            {
                if (currentTurn == 1)
                {
                    turnText.setString("Obecny ruch: BIALE");
                }
                else if (currentTurn == 3)
                {
                    turnText.setString("Obecny ruch: CZARNE");
                }
                turnText.setFillColor(sf::Color(255, 165, 0));
            }

            if (myColor != 0)
            {
                window.draw(turnText);
            }

            // Komunikaty od serwera
            auto [statusMsg, isError] = client.getStatus();
            sf::Text statusText(font);
            statusText.setCharacterSize(14);
            statusText.setPosition(sf::Vector2f{12.f, 519.f});
            statusText.setString("Status: " + statusMsg);

            if (isError)
            {
                statusText.setFillColor(sf::Color(255, 80, 80));
            }
            else
            {
                statusText.setFillColor(sf::Color(180, 180, 180));
            }
            window.draw(statusText);
        }

        window.display();
    }

    return 0;
}