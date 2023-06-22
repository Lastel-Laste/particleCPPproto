class Grid {
public:
    int cellSize;
    int numCells;
    int gridWidth;
    int gridHeight;
    std::vector<std::unordered_set<Particle*>> cells;

    Grid(int width, int height, int size)
    {
        cellSize = size;
        gridWidth = width / cellSize;
        gridHeight = height / cellSize;
        numCells = gridWidth * gridHeight;
        cells.resize(numCells);
    }

    void clear()
    {
        for (auto& cell : cells) {
            cell.clear();
        }
    }

    void updateParticleCell(Particle* particle)
    {
        int cellX = static_cast<int>(particle->position.x / cellSize);
        int cellY = static_cast<int>(particle->position.y / cellSize);
        int cellIndex = cellY * gridWidth + cellX;
        particle->cellIndex = cellIndex;
        cells[cellIndex].insert(particle);
    }

    void handleCollision(Particle& A, Particle& B, float dx, float dy, float d, float overlap)
    {
        // 충돌 처리 로직
        // ...
    }

    void updateCollision()
    {
        for (auto& cell : cells) {
            for (auto itA = cell.begin(); itA != cell.end(); ++itA) {
                auto particleA = *itA;

                for (int neighborY = -1; neighborY <= 1; ++neighborY) {
                    for (int neighborX = -1; neighborX <= 1; ++neighborX) {
                        int cellX = (particleA->cellIndex % gridWidth) + neighborX;
                        int cellY = (particleA->cellIndex / gridWidth) + neighborY;

                        if (cellX >= 0 && cellX < gridWidth && cellY >= 0 && cellY < gridHeight) {
                            int neighborCellIndex = cellY * gridWidth + cellX;
                            auto& neighborCell = cells[neighborCellIndex];

                            for (auto itB = neighborCell.begin(); itB != neighborCell.end(); ++itB) {
                                auto particleB = *itB;
                                if (particleA != particleB) {
                                    float dx = particleA->position.x - particleB->position.x;
                                    float dy = particleA->position.y - particleB->position.y;
                                    float distance = std::sqrt(dx * dx + dy * dy);
                                    float minDistance = particleA->radius + particleB->radius;

                                    if (distance < minDistance) {
                                        handleCollision(*particleA, *particleB, dx, dy, distance, minDistance - distance);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};