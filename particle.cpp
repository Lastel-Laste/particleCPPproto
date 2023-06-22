class Particle {
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float radius;
    float mass;

    Particle(float x, float y)
    {
        position.x = x;
        position.y = y;
        velocity.x = (std::rand() % 100 - 50) * 0.1f;
        velocity.y = (std::rand() % 100 - 50) * 0.1f;
        acceleration = sf::Vector2f(0.0f, 0.0f);
        radius = 2.5;
        mass = radius * radius * 0.01;
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

std::vector<Particle> particles;