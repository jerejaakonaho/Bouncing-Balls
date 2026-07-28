#include <SFML/Graphics/RenderTexture.hpp>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include "ParticleEngine.hpp"

int main() {
    unsigned int windowHeight{1000};
    unsigned int windowWidth{1000};
    ParticleEngine ParticleEngine;
    sf::Texture circleTexture;

    if (!ParticleEngine.initTexture("assets/circle.png", circleTexture)) {
        std::cerr << "Failed to load texture" << std::endl;
        return -1;
    }

    ParticleEngine.initializeCircles(windowHeight, windowWidth, circleTexture);

    sf::RenderStates renderStates;
    renderStates.texture = &circleTexture;

	sf::RenderWindow window( sf::VideoMode( { windowHeight, windowWidth } ), "Bouncing Balls" );
	window.setFramerateLimit(10000);

	sf::Clock fpsClock;
	int frameCount{};
	sf::Clock dtClock;

	while ( window.isOpen() )
	{
	    float dt = dtClock.restart().asSeconds();

		while ( const std::optional event = window.pollEvent() )
		{
			if ( event->is<sf::Event::Closed>() )
				window.close();
		}

		ParticleEngine.CircleLoop(windowHeight, windowWidth, dt);

		window.clear();
		window.draw(ParticleEngine.vertexBuffer, renderStates);

		frameCount++;
		if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
		    float fps  = frameCount / fpsClock.restart().asSeconds();
			window.setTitle("Bouncing Balls FPS: " + std::to_string(static_cast<int>(fps)));
			frameCount = 0;
		}
		window.display();
	}
}
