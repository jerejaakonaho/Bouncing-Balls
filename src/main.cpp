#include <SFML/Graphics.hpp>
#include "ParticleEngine.hpp"

int main() {
    uint windowHeight{1000};
    uint windowWidth{1000};
    ParticleEngine ParticleEngine;
    ParticleEngine.initializeCircles(windowHeight, windowWidth);

	sf::RenderWindow window( sf::VideoMode( { windowHeight, windowWidth } ), "Bouncing Balls" );
	window.setFramerateLimit(1000);

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
		for (const auto& circle : ParticleEngine.circles) {
		    window.draw(circle);
		}

		frameCount++;
		if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
		    float fps  = frameCount / fpsClock.restart().asSeconds();
			window.setTitle("Bouncing Balls FPS: " + std::to_string(static_cast<int>(fps)));
			frameCount = 0;
		}
		window.display();
	}
}
