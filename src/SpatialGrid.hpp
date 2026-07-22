#include <vector>
#include <cmath>
#include "Circle.hpp"

class SpatialGrid {
    int windowHeight{1000};
    int windowWidth{1000};
    float cellSize{15.f};
    int columns{};
    int rows{};

    std::vector<std::vector<int>> grid;

    void initializeGrid(uint windowWidth, uint windowHeight) {
        columns = static_cast<int>(std::ceil(windowWidth / cellSize));
        rows = static_cast<int>(std::ceil(windowHeight / cellSize));
        grid.resize(columns * rows);
    }

    void mapCirclesToGrid(const std::vector<float>& posX, const std::vector<float>& posY) {
        // Clear the old frames data:
        for (auto& cell : grid) {
            cell.clear();
        }

        for (size_t i{}; i < posX.size(); i++) {
            int rightColumn = posX[i] / cellSize;
            int rightRow = posY[i] / cellSize;

            if (rightColumn >= 0 && rightColumn < columns && rightRow >= 0 && rightRow < rows) {
                int rightCell = rightRow * columns + rightColumn;
                grid[rightCell].push_back(i);
            }
        }
    }





}
