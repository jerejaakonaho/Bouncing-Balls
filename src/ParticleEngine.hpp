#pragma once
#include <memory> // for pImpl;

namespace sf {
    // Forward declaration for drawParticles parameter
    class RenderWindow;
}

class ParticleEngine {

public:
    // Constructors, default constructor is deleted, destructor is defaulted.
    ParticleEngine(int windowHeight, int windowWidth);
    ~ParticleEngine();

    // Handles moving, collisions and syncing graphics
    void CircleLoop(int windowHeight, int windowWidth, float dt);

    void drawParticles(sf::RenderWindow &window);
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
