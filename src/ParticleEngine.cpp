#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/System/Angle.hpp>
#include "ParticleEngine.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <filesystem>


struct ParticleEngine::Impl {
    // Performance affecting parameters
    const int particleCount{5000};
    const float radius{5.f};
    const float speed{300.f};
    sf::Color color{sf::Color::White};
    // GPU Vectors
    const int vertexCount{particleCount * 6};
    std::vector<sf::Vertex> vertices;
    sf::VertexBuffer vertexBuffer{sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Stream};
    // Circle texture as a .png
    sf::Texture circleTexture;
    std::filesystem::path PNGPath{"assets/circle.png"};

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

    // INITIALIZATION OF CIRCLES AND TEXTURE
    bool initTexture(const std::filesystem::path &fileName, sf::Texture &circleTexture) {
        return circleTexture.loadFromFile(fileName);
    }
    void initializeCircles(uint windowHeight, uint windowWidth, const sf::Texture& texture) {
        posX.resize(particleCount);
        posY.resize(particleCount);
        dirX.resize(particleCount);
        dirY.resize(particleCount);

        vertices.resize(vertexCount);
        if (!vertexBuffer.create(vertexCount)) {throw std::runtime_error("Failed to create Vertex Buffer");} // Allocate GPU memory


        // Grid variables:
        cellSize = radius * 2.0f;
        columns = static_cast<int>(std::ceil(windowWidth / cellSize));
        rows = static_cast<int>(std::ceil(windowHeight / cellSize));
        gridHeads.resize(columns * rows, -1);
        nextParticle.resize(particleCount, -1);

        // Texture corners
        sf::Vector2f texSize(texture.getSize());
        sf::Vector2f topLeft(0.f, 0.f);
        sf::Vector2f topRight(texSize.x, 0.f);
        sf::Vector2f bottomRight(texSize.x, texSize.y);
        sf::Vector2f bottomLeft(0.f, texSize.y);

        // Used for deterministic randomness. Makes benchmarks consistent across runs.
        std::mt19937 gen(67);

        std::uniform_real_distribution<float> xDist(radius, windowWidth - radius);
        std::uniform_real_distribution<float> yDist(radius, windowHeight - radius);

        std::uniform_real_distribution<float> angleDist(0.f, 360.f);

        for (size_t i{}; i < particleCount; i++) {
            posX[i] = xDist(gen);
            posY[i] = yDist(gen);
            sf::Angle angle = sf::degrees(angleDist(gen));
            dirX[i] = std::cos(angle.asRadians()) * speed;
            dirY[i] = std::sin(angle.asRadians()) * speed;

            float startX = xDist(gen);
            float startY = yDist(gen);

            posX[i] = startX;
            posY[i] = startY;

            // Map texture coordinates:
            size_t v = i * 6;

            // Triangle 1:
            vertices[v + 0].texCoords = topLeft;
            vertices[v + 1].texCoords = topRight;
            vertices[v + 2].texCoords = bottomRight;

            // Triangle 2:
            vertices[v + 3].texCoords = bottomRight;
            vertices[v + 4].texCoords = bottomLeft;
            vertices[v + 5].texCoords = topLeft;

            for (size_t j{0}; j < 6; ++j) {
                vertices[v + j].color = color;
            }
        }
    }

    // SPATIAL GRID IMPLEMENTATION
    inline int getCellIndex(float x, float y) const {
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

    // COLLISION AND MOVEMENT
    void moveCircles(float dt) {
        for (size_t i{}; i < particleCount; i++) {
            posX[i] += dirX[i] * dt;
            posY[i] += dirY[i] * dt;
        }
    }

    inline void handleParticleCollision(int i, int j, float radiusSum, float radiusSumSquared) {
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

    void handleCollisionsWithSpatialGrid() {
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
                        handleParticleCollision(current_i, current_j, radiusSum, radiusSumSquared);
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
                            handleParticleCollision(current_i, neighbor_j, radiusSum, radiusSumSquared);
                            neighbor_j = nextParticle[neighbor_j];
                        }
                    }
                    current_i = nextParticle[current_i];
                }
            }
        }

    }

    void handleWallCollisions(uint windowHeight, uint windowWidth) {
        for (size_t i{}; i < particleCount; i++) {
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

    // GRAPHICS SYNC AND RENDERING
    void syncGraphics() {
        for (size_t i{}; i < particleCount; i++) {
            size_t v = i * 6;
            float left = posX[i] - radius;
            float right = posX[i] + radius;
            float top = posY[i] - radius;
            float bottom = posY[i] + radius;

            vertices[v + 0].position = {left, top};
            vertices[v + 1].position = {right, top};
            vertices[v + 2].position = {right, bottom};
            vertices[v + 3].position = {right, bottom};
            vertices[v + 4].position = {left, bottom};
            vertices[v + 5].position = {left, top};
        }
        (void)vertexBuffer.update(vertices.data()); // (void) to tell the coimpiler we are intentionally ignoring the return bool
    }

    void drawParticles(sf::RenderWindow &window) {
        sf::RenderStates renderStates;
        renderStates.texture = &circleTexture;
        window.draw(vertexBuffer, renderStates);
    }

    void CircleLoop(int windowHeight, int windowWidth, float dt) {
        moveCircles(dt);
        handleWallCollisions(windowHeight, windowWidth);
        buildGrid(particleCount);
        handleCollisionsWithSpatialGrid();
        syncGraphics();
    }

};

ParticleEngine::ParticleEngine(int windowHeight, int windowWidth)
    : pImpl{std::make_unique<Impl>()}
{
    if (!pImpl->initTexture(pImpl->PNGPath, pImpl->circleTexture)) {throw std::runtime_error("Circle image not found");}
    pImpl->initializeCircles(windowHeight, windowWidth, pImpl->circleTexture);
}

void ParticleEngine::CircleLoop(int windowHeight, int windowWidth, float dt) {
    pImpl->CircleLoop(windowHeight, windowWidth, dt);
}

void ParticleEngine::drawParticles(sf::RenderWindow &window) {
    pImpl->drawParticles(window);
}

ParticleEngine::~ParticleEngine() = default;
