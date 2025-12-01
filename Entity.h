#pragma once
#include <SFML/Graphics.hpp>

enum class EntityKind {
    Enemy,
    Player
};

struct Entity {
    EntityKind kind = EntityKind::Enemy;
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool colliding = false;
    int fusionStage = 0; // 0: verde, 1: amarillo, 2: naranja, 3: rojo, 4: morado, 5: azul

    Entity() = default;

    Entity(EntityKind k, const sf::Vector2f& pos, float radius, const sf::Vector2f& vel)
        : kind(k), velocity(vel) {
        shape.setRadius(radius);
        shape.setOrigin({radius, radius});
        shape.setPosition(pos);
        if (kind == EntityKind::Player) {
            shape.setFillColor(sf::Color::Green);
        } else {
            shape.setFillColor(sf::Color::White);
        }
    }
};
