#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "CppGame");

    // Set a frame rate limit
    window.setFramerateLimit(60);

    // Game loop
    while (window.isOpen())
    {
        // Process events
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Clear the window
        window.clear(sf::Color::Black);

        // ── Draw your game content here ──

        // Update the window
        window.display();
    }

    return 0;
}
