#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Angle.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>

class CircleEngine {
public:
    int circleAmount{1000};
    float radius{5.f};
    float speed{100.f};
    sf::Color color{sf::Color::White};
    std::vector<sf::CircleShape> circles;
    std::vector<sf::Vector2i> coordinates;
    std::vector<sf::Vector2f> directions;

    void moveCircles(float dt) {
        for (size_t i{}; i < circles.size(); i++) {
            circles[i].move(directions[i] * dt);
        }
    }

    void checkCollisions() {
        for (size_t i{}; i < circles.size(); i++) {
            for (size_t j{i + 1}; j < circles.size(); j++) {
                sf::Vector2f pos1 = circles[i].getPosition();
                sf::Vector2f pos2 = circles[j].getPosition();

                // Distance squared, calculating distance is slower due to square root operation
                float dx = pos2.x - pos1.x;
                float dy = pos2.y - pos1.y;

                float dSquared = (dx * dx) + (dy * dy);

                // Now compare distance squared to sum of the radiuses squared (of the circles)
                float radiusSum = radius + radius;
                float radiusSumSquared = radiusSum * radiusSum;

                if (dSquared < radiusSumSquared) {
                    // Collided, change direction

                    // Real distance and normal vectors
                    float distance = std::sqrt(dSquared);
                    float nx = dx / distance;
                    float ny = dy / distance;

                    // Circles might overlap so shift them to prevent sticking
                    float overlap = radiusSum - distance;
                    circles[i].move({-nx * (overlap * 0.5f), -ny * (overlap * 0.5f)});
                    circles[j].move({nx * (overlap * 0.5f), ny * (overlap * 0.5f)});

                    // Dot products
                    float dot_i = (directions[i].x * nx) + (directions[i].y * ny);
                    float dot_j = (directions[j].x * nx) + (directions[j].y * ny);

                    // Mirror the direction: 2 * dot product * normal vector
                    directions[i].x -= 2.f * dot_i * nx;
                    directions[i].y -= 2.f * dot_i * ny;

                    directions[j].x -= 2.f * dot_j * nx;
                    directions[j].y -= 2.f * dot_j * ny;
                }
            }
        }
    }

    void checkWallCollisions(uint windowHeight, uint windowWidth) {
        for (size_t i{}; i < circles.size(); i++) {
             sf::Vector2f pos = circles[i].getPosition();
             bool positionChanged = false;


             // X axis, if out of bounds return to bounds and reverse direction.
             if (pos.x - radius < 0.f) {
                 pos.x = radius;
                 directions[i].x *= -1.f;
                 positionChanged = true;
             } else if (pos.x + radius > windowWidth) {
                 pos.x = windowWidth - radius;
                 directions[i].x *= -1.f;
                 positionChanged = true;
             }

             // Y axis:
             if (pos.y - radius < 0.f) {
                 pos.y = radius;
                 directions[i].y *= -1.f;
                 positionChanged = true;
             } else if (pos.y + radius > windowHeight) {
                 pos.y = windowHeight - radius;
                 directions[i].y *= -1.f;
                 positionChanged = true;
             }

             // If ball hit wall update new position
             if (positionChanged) circles[i].setPosition(pos);
        }
    }

    void initializeCircles(uint windowHeight, uint windowWidth) {
        circles.resize(circleAmount);
        coordinates.resize(circleAmount);
        directions.resize(circleAmount);


        // Used for deterministic randomness. Makes benchmarks consistent across runs.
        std::mt19937 gen(67);

        std::uniform_real_distribution<float> xDist(radius, windowWidth - radius);
        std::uniform_real_distribution<float> yDist(radius, windowHeight - radius);

        std::uniform_real_distribution<float> angleDist(0.f, 360.f);

        for (size_t i{}; i < circles.size(); i++) {
            circles[i].setRadius(radius);
            circles[i].setFillColor(color);

            float startX = xDist(gen);
            float startY = yDist(gen);
            circles[i].setPosition({startX, startY});
            circles[i].setOrigin({radius, radius});

            sf::Angle angle = sf::degrees(angleDist(gen));

            directions[i].x = std::cos(angle.asRadians()) * speed;
            directions[i].y = std::sin(angle.asRadians()) * speed;
        }
    }

    void CircleLoop(uint windowHeight, uint windowWidth, float dt) {
        moveCircles(dt);
        checkWallCollisions(windowHeight, windowWidth);
        checkCollisions();
    }
};
