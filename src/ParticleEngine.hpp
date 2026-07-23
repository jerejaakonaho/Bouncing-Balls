#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Angle.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>

class ParticleEngine {
public:
    // Performance affecting parameters
    int circleAmount{5000};
    float radius{5.f};
    float speed{300.f};

    // Contain the
    sf::Color color{sf::Color::White};
    std::vector<sf::CircleShape> circles;

    std::vector<float> posX;
    std::vector<float> posY;
    std::vector<float> dirX;
    std::vector<float> dirY;

    // Index based link list for spatial grid
    std::vector<int> gridHeads{};
    std::vector<int> nextParticle{};
    int columns{};
    int rows{};
    float cellSize{};

    __attribute__((always_inline)) inline int getCellIndex(float x, float y) const {
        int column = static_cast<int>(x / cellSize);
        int row = static_cast<int>(y / cellSize);

        // Particles CAN go over the boundaries, so return them to the screen if they are.
        column = std::max(0, std::min(column, columns - 1));
        row = std::max(0, std::min(row, rows - 1));

        // Return the index
        return (row * columns) + column;
    }

    void buildGrid(int circleAmount) {
        // -1 means there is no particle
        std::fill(gridHeads.begin(), gridHeads.end(), -1);

        for (size_t i{}; i < circleAmount; ++i) {
            int cellIndex = getCellIndex(posX[i], posY[i]);
            // Point this particles next to whatever was at the head
            nextParticle[i] = gridHeads[cellIndex];
            gridHeads[cellIndex] = i;
        }
    }

    void moveCircles(float dt) {
        for (size_t i{}; i < circleAmount; i++) {
            posX[i] += dirX[i] * dt;
            posY[i] += dirY[i] * dt;
        }
    }

    // OLD O(N²) loop
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

    __attribute__((always_inline)) inline void handleCollision(int i, int j, float radiusSum, float radiusSumSquared) {
        float dx = posX[j] - posX[i];
        float dy = posY[j] - posY[i];
        float dSquared = (dx * dx) + (dy * dy);

        if (dSquared < radiusSumSquared && dSquared > 0.00001f) {
            float distance = std::sqrt(dSquared);
            float nx = dx / distance;
            float ny = dy / distance;
            float overlap = radiusSum - distance;

            // Shift particles to prevent sticking
            posX[i] -= nx * (overlap * 0.5f);
            posY[i] -= ny * (overlap * 0.5f);
            posX[j] += nx * (overlap * 0.5f);
            posY[j] += ny * (overlap * 0.5f);

            // Calculate bounce vectors
            float dot_i = (dirX[i] * nx) + (dirY[i] * ny);
            float dot_j = (dirX[j] * nx) + (dirY[j] * ny);

            dirX[i] -= 2.f * dot_i * nx;
            dirY[i] -= 2.f * dot_i * ny;
            dirX[j] -= 2.f * dot_j * nx;
            dirY[j] -= 2.f * dot_j * ny;
        }
    }

    void checkCollisionsWithSpatialGrid() {
        float radiusSum = radius + radius;
        float radiusSumSquared = radiusSum * radiusSum;

        // Go through the grid
        for (size_t y{}; y < rows; ++y) {
            for (size_t x{}; x < columns; ++x) {
                int cell = y * columns + x;
                int current_i = gridHeads[cell];

                while (current_i != -1) {
                    // Check collisions in the same cell
                    int current_j = nextParticle[current_i];
                    while (current_j != -1) {
                        handleCollision(current_i, current_j, radiusSum, radiusSumSquared);
                        current_j = nextParticle[current_j];
                    }
                    // Determine the amount of neighbours, all cells look only for half of neighbours so there is no needless overlapping (east, sw, s and se directions)
                    int neighbours[4];
                    int neighbourCount{};

                    // East
                    if (x + 1 < columns) { neighbours[neighbourCount++] = cell + 1; }
                    // South West
                    if (y + 1 < rows && x - 1 >= 0) { neighbours[neighbourCount++] = cell + columns - 1; }
                    // South
                    if (y + 1 < rows) { neighbours[neighbourCount++] = cell + columns; }
                    // South East
                    if (y + 1 < rows && x + 1 < columns) { neighbours[neighbourCount++] = cell + columns + 1; }

                    for (size_t n{0}; n < neighbourCount; ++n) {
                        int neighborCell = neighbours[n];
                        int neighbor_j = gridHeads[neighborCell];

                        while (neighbor_j != -1) {
                            handleCollision(current_i, neighbor_j, radiusSum, radiusSumSquared);
                            neighbor_j = nextParticle[neighbor_j];
                        }
                    }
                    current_i = nextParticle[current_i];
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

        // Grid variables:
        cellSize = radius * 2.0f;
        columns = static_cast<int>(std::ceil(windowWidth / cellSize));
        rows = static_cast<int>(std::ceil(windowHeight / cellSize));

        gridHeads.resize(columns * rows, -1);
        nextParticle.resize(circleAmount, -1);

        // Used for deterministic randomness. Makes benchmarks consistent across runs.
        std::mt19937 gen(67);

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
        buildGrid(circleAmount);
        checkCollisionsWithSpatialGrid();
        syncGraphics();
    }
};
