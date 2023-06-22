#include <iostream>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <unordered_map>

class Particle {
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float radius;
    float mass;
    int cellIndex;

    Particle(float x, float y, float r)
    {
        position.x = x;
        position.y = y;
        velocity.x = (std::rand() % 100 - 50) * 0.01f;
        velocity.y = (std::rand() % 100 - 50) * 0.01f;
        acceleration = sf::Vector2f(0.0f, 0.01f);
        radius = r;
        mass = radius * radius * 0.01;
        cellIndex = -1;
    }

    void update()
    {
        velocity.x += acceleration.x;
        velocity.y += acceleration.y;
        position.x += velocity.x;
        position.y += velocity.y;
    }
};

bool operator!=(Particle const &a, Particle const &b) {
    return a.position.x != b.position.x or a.position.y != b.position.y;
}

int canvas_width = 900;
int canvas_height = 900;
float particle_radius = 10;
int particle_num = 1000;
std::vector<Particle> particles;
int sub_steps = 8;
int FPS = 480;

// 그리드 구조체
struct Grid {
    int gridSizeX;
    int gridSizeY;
    int cellSize;
    std::unordered_map<int, std::vector<Particle*>> cells;

    Grid(int sizeX, int sizeY, int size) : gridSizeX(sizeX), gridSizeY(sizeY), cellSize(size) {}

    void clear()
    {
        cells.clear();
    }

    void addToCell(int cellIndex, Particle* particle)
    {
        cells[cellIndex].push_back(particle);
        particle->cellIndex = cellIndex;
    }

    int calculateCellIndex(int gridX, int gridY)
    {
        return gridX * gridSizeY + gridY;
    }

    void updateParticles()
    {
        for (auto& particle : particles) {
            // 현재 파티클이 속한 그리드 셀 좌표
            int gridX = particle.position.x / cellSize;
            int gridY = particle.position.y / cellSize;
            int cellIndex = calculateCellIndex(gridX, gridY);

            if (particle.cellIndex != cellIndex) {
                if (particle.cellIndex != -1) {
                    // 이전 셀에서 파티클 제거
                    cells[particle.cellIndex].erase(
                        std::remove(cells[particle.cellIndex].begin(), cells[particle.cellIndex].end(), &particle),
                        cells[particle.cellIndex].end()
                    );
                }
                // 현재 셀에 파티클 추가
                addToCell(cellIndex, &particle);
                particle.cellIndex = cellIndex;
            }
        }
    }
};

// 파티클 초기화 함수
void init(Grid& grid)
{
    for (int i{ particle_num }; i--;) {
        float x = std::rand() % canvas_width + 1;  // 화면 가로 범위 내의 임의의 x 좌표
        float y = std::rand() % canvas_height + 1;  // 화면 세로 범위 내의 임의의 y 좌표
        particles.push_back(Particle(x, y, particle_radius));
    }

    grid.updateParticles();
}

// 충돌 처리 함수
void handleCollision(Particle &A, Particle &B, 
    float dx, float dy, float d, float overlap) {
    sf::Vector2f normal = sf::Vector2f(dx / d, dy / d);
    sf::Vector2f tangent = sf::Vector2f(-normal.y, normal.x);
    
	// 충돌 벡터의 정사영 계산
    float vA1 = A.velocity.x * normal.x + A.velocity.y * normal.y;
    float vB1 = B.velocity.x * normal.x + B.velocity.y * normal.y;

    // 충돌 벡터의 직교 영 계산
    float vA2 = A.velocity.x * tangent.x + A.velocity.y * tangent.y;
    float vB2 = B.velocity.x * tangent.x + B.velocity.y * tangent.y;

    // 충돌 후의 정사영 속도 계산
    float restitution = 0.9;
    float vA1Final = (vA1 * (A.mass - B.mass) + (1 + restitution) * B.mass * vB1) / (A.mass + B.mass);
    float vB1Final = (vB1 * (B.mass - A.mass) + (1 + restitution) * A.mass * vA1) / (A.mass + B.mass);

    // 충돌 후의 속도 벡터 계산
    sf::Vector2f vA1FinalVec = vA1Final * normal;
    sf::Vector2f vA2FinalVec = vA2 * tangent;
    sf::Vector2f vB1FinalVec = vB1Final * normal;
    sf::Vector2f vB2FinalVec = vB2 * tangent;

    // 충돌 후의 속도 계산
    A.velocity = vA1FinalVec + vA2FinalVec;
    B.velocity = vB1FinalVec + vB2FinalVec;

    // 충돌 후의 위치 조정
    float displacementScale = overlap / (A.mass + B.mass);
    sf::Vector2f displacement = displacementScale * normal;

    A.position += displacement * A.mass;
    B.position -= displacement * B.mass;
}

void updateCollision(Grid& grid) 
{
    float Damping = 0.5;
    
    for (auto& particle : particles)
    {
        float LOver = -particle.position.x;
        float ROver = particle.position.x + 2*particle.radius - canvas_width;
        float DOver = -particle.position.y;
        float UOver = particle.position.y + 2*particle.radius - canvas_height;
        if (LOver > 0) { particle.position.x += LOver; particle.velocity.x *= -1 * Damping; }
        else if (ROver > 0) { particle.position.x -= ROver; particle.velocity.x *= -1 * Damping; }
        if (DOver > 0) { particle.position.y += DOver; particle.velocity.y *= -1 * Damping; }
        else if (UOver > 0) { particle.position.y -= UOver; particle.velocity.y *= -1 * Damping; }
    }

    grid.clear();

    for (auto& particle : particles)
    {
        particle.update();

        // 현재 파티클이 속한 그리드 셀 좌표
        int gridX = particle.position.x / grid.cellSize;
        int gridY = particle.position.y / grid.cellSize;
        int cellIndex = grid.calculateCellIndex(gridX, gridY);

        // 현재 셀에 파티클 추가
        grid.addToCell(cellIndex, &particle);
    }

    for (auto& particle : particles)
    {
        // 현재 파티클이 속한 그리드 셀과 인접한 8개의 그리드 셀에 있는 파티클과 충돌 처리
        int gridX = particle.position.x / grid.cellSize;
        int gridY = particle.position.y / grid.cellSize;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                int neighborGridX = gridX + dx;
                int neighborGridY = gridY + dy;
                int neighborCellIndex = grid.calculateCellIndex(neighborGridX, neighborGridY);

                if (grid.cells.count(neighborCellIndex) > 0) {
                    for (auto neighborParticle : grid.cells[neighborCellIndex]) {
                        if (particle != *neighborParticle) {
                            float dx = particle.position.x - neighborParticle->position.x;
                            float dy = particle.position.y - neighborParticle->position.y;
                            float distance = sqrt(dx * dx + dy * dy);
                            float overlap = particle.radius + neighborParticle->radius - distance;
                            if (overlap > 0) {
                                handleCollision(particle, *neighborParticle, dx, dy, distance, overlap);
                            }
                        }
                    }
                }
            }
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(canvas_width, canvas_height), "My window");
    window.setFramerateLimit(FPS);  // 프레임 속도를 60프레임으로 제한

    Grid grid(canvas_width / particle_radius, canvas_height / particle_radius, particle_radius * 2);
    init(grid);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "폰트를 로드할 수 없습니다." << std::endl;
        return -1;
    }

    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition(10, canvas_height - 30);  // 좌측 하단에 위치

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        for (int i{sub_steps}; i--;)
        {
            updateCollision(grid);
        }

        // Set the background color to rgba(67, 73, 255, 0.583)
        window.clear(sf::Color(193, 195, 255));

        // 파티클 그리기
        for (const auto& particle : particles)
        {
            sf::CircleShape shape(particle.radius);
            shape.setPosition(particle.position);
            shape.setFillColor(sf::Color::Black);
            window.draw(shape);
        }

        // FPS 텍스트 업데이트
        sf::Time elapsedTime = clock.restart();
        float fps = 1.0f / elapsedTime.asSeconds();
        fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));

        // FPS 텍스트 그리기
        window.draw(fpsText);

        window.display();
    }

    return 0;
}
