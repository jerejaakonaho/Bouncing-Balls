#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Angle.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>

class CircleEngine {
public:
    int circleAmount{5000};
    float radius{5.f};
    float speed{300.f};
    sf::Color color{sf::Color::White};
    std::vector<sf::CircleShape> circles;
    std::vector<float> posX;
    std::vector<float> posY;
    std::vector<float> dirX;
    std::vector<float> dirY;

    void moveCircles(float dt) {
        for (size_t i{}; i < circleAmount; i++) {
            posX[i] += dirX[i] * dt;
            posY[i] += dirY[i] * dt;
        }
    }

    void checkCollisions() {
        // Used in the loop, moved here to prevent calculating every loop.
        float radiusSum = radius + radius;
        float radiusSumSquared = radiusSum * radiusSum;

        for (size_t i{}; i < circleAmount; i++) {
            for (size_t j{i + 1}; j < circleAmount; j++) {
                float pos1_x = posX[i];
                float pos1_y = posY[i];

                float pos2_x = posX[j];
                float pos2_y = posY[j];


                // Distance squared, calculating distance is slower due to square root operation
                float dx = pos2_x - pos1_x;
                float dy = pos2_y - pos1_y;

                float dSquared = (dx * dx) + (dy * dy);

                if (dSquared < radiusSumSquared) {
                    // Collided, change direction

                    // Real distance and normal vectors
                    float distance = std::sqrt(dSquared);
                    float nx = dx / distance;
                    float ny = dy / distance;

                    // Circles might overlap so shift them to prevent sticking
                    float overlap = radiusSum - distance;
                    posX[i] += -nx * (overlap * 0.5f);
                    posY[i] += -ny * (overlap * 0.5f);
                    posX[j] += nx * (overlap * 0.5f);
                    posY[j] += ny * (overlap * 0.5f);

                    // Dot products
                    float dot_i = (dirX[i] * nx) + (dirY[i] * ny);
                    float dot_j = (dirX[j] * nx) + (dirY[j] * ny);

                    // Mirror the direction: 2 * dot product * normal vector
                    dirX[i] -= 2.f * dot_i * nx;
                    dirY[i] -= 2.f * dot_i * ny;

                    dirX[j] -= 2.f * dot_j * nx;
                    dirY[j] -= 2.f * dot_j * ny;
                }
            }
        }
    }

    void checkWallCollisions(uint windowHeight, uint windowWidth) {
        for (size_t i{}; i < circleAmount; i++) {
             // X axis, if out of bounds return to bounds and reverse direction.
             if (posX[i] - radius < 0.f) {
                 posX[i] = radius;
                 dirX[i] *= -1.f;
             } else if (posX[i] + radius > windowWidth) {
                 posX[i] = windowWidth - radius;
                 dirX[i] *= -1.f;
             }

             // Y axis:
             if (posY[i] - radius < 0.f) {
                 posY[i] = radius;
                 dirY[i] *= -1.f;
             } else if (posY[i] + radius > windowHeight) {
                 posY[i] = windowHeight - radius;
                 dirY[i] *= -1.f;
             }
        }
    }

    void initializeCircles(uint windowHeight, uint windowWidth) {
        circles.resize(circleAmount);
        posX.resize(circleAmount);
        posY.resize(circleAmount);
        dirX.resize(circleAmount);
        dirY.resize(circleAmount);

        // Used for deterministic randomness. Makes benchmarks consistent across runs.
        std::mt19937 gen(67);s

        std::uniform_real_distribution<float> xDist(radius, windowWidth - radius);
        std::uniform_real_distribution<float> yDist(radius, windowHeight - radius);

        std::uniform_real_distribution<float> angleDist(0.f, 360.f);

        for (size_t i{}; i < circles.size(); i++) {
            circles[i].setRadius(radius);
            circles[i].setFillColor(color);
            circles[i].setOrigin({radius, radius});

            float startX = xDist(gen);
            float startY = yDist(gen);

            posX[i] = startX;
            posY[i] = startY;

            sf::Angle angle = sf::degrees(angleDist(gen));

            dirX[i] = std::cos(angle.asRadians()) * speed;
            dirY[i] = std::sin(angle.asRadians()) * speed;
        }
    }

    void syncGraphics() {
        for (size_t i{}; i < circleAmount; i++) {
            circles[i].setPosition({posX[i], posY[i]});
        }
    }

    void CircleLoop(uint windowHeight, uint windowWidth, float dt) {
        moveCircles(dt);
        checkWallCollisions(windowHeight, windowWidth);
        checkCollisions();
        syncGraphics();
    }
};
