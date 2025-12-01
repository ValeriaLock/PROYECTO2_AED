#pragma once
#include <vector>
#include <optional>
#include <SFML/Graphics.hpp>
#include "Entity.h"

class QuadTree {
private:
    struct Node {
        sf::FloatRect boundary;
        int capacity;
        bool divided;
        std::vector<Entity*> objects;
        Node* nw;
        Node* ne;
        Node* sw;
        Node* se;

        Node(const sf::FloatRect& bounds, int cap)
            : boundary(bounds), capacity(cap), divided(false),
              nw(nullptr), ne(nullptr), sw(nullptr), se(nullptr) {}
    };

    Node* root = nullptr;
    int capacity;
    sf::FloatRect worldBounds;

    void clear(Node* node) {
        if (!node) return;
        if (node->divided) {
            clear(node->nw);
            clear(node->ne);
            clear(node->sw);
            clear(node->se);
        }
        delete node;
    }

    bool insert(Node* node, Entity* e) {
        if (!node) return false;

        if (!node->boundary.contains(e->shape.getPosition()))
            return false;

        if ((int)node->objects.size() < node->capacity && !node->divided) {
            node->objects.push_back(e);
            return true;
        }

        if (!node->divided) subdivide(node);

        if (insert(node->nw, e)) return true;
        if (insert(node->ne, e)) return true;
        if (insert(node->sw, e)) return true;
        if (insert(node->se, e)) return true;

        return false;
    }

    void subdivide(Node* node) {
        const float x = node->boundary.position.x;
        const float y = node->boundary.position.y;
        const float w = node->boundary.size.x / 2.f;
        const float h = node->boundary.size.y / 2.f;

        node->nw = new Node(sf::FloatRect({x,      y},      {w, h}), node->capacity);
        node->ne = new Node(sf::FloatRect({x + w,  y},      {w, h}), node->capacity);
        node->sw = new Node(sf::FloatRect({x,      y + h},  {w, h}), node->capacity);
        node->se = new Node(sf::FloatRect({x + w,  y + h},  {w, h}), node->capacity);

        node->divided = true;

        auto oldObjects = node->objects;
        node->objects.clear();
        for (auto* obj : oldObjects) {
            insert(node->nw, obj) || insert(node->ne, obj) ||
            insert(node->sw, obj) || insert(node->se, obj);
        }
    }

    void query(Node* node, const sf::FloatRect& range, std::vector<Entity*>& found) const {
        if (!node) return;

        if (!node->boundary.findIntersection(range).has_value())
            return;

        for (auto* obj : node->objects) {
            if (range.contains(obj->shape.getPosition()))
                found.push_back(obj);
        }

        if (node->divided) {
            query(node->nw, range, found);
            query(node->ne, range, found);
            query(node->sw, range, found);
            query(node->se, range, found);
        }
    }

    void draw(Node* node, sf::RenderWindow& window) const {
        if (!node) return;

        sf::RectangleShape rect;
        rect.setPosition(node->boundary.position);
        rect.setSize(node->boundary.size);
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineThickness(1.f);
        rect.setOutlineColor(sf::Color(100, 100, 255, 80));
        window.draw(rect);

        if (node->divided) {
            draw(node->nw, window);
            draw(node->ne, window);
            draw(node->sw, window);
            draw(node->se, window);
        }
    }

public:
    QuadTree(const sf::FloatRect& bounds, int cap = 4)
        : capacity(cap), worldBounds(bounds) {
        root = new Node(worldBounds, capacity);
    }

    ~QuadTree() {
        clear(root);
    }

    void reset() {
        clear(root);
        root = new Node(worldBounds, capacity);
    }

    void insert(Entity* e) {
        insert(root, e);
    }

    void queryRange(const sf::FloatRect& range, std::vector<Entity*>& found) const {
        query(root, range, found);
    }

    void draw(sf::RenderWindow& window) const {
        draw(root, window);
    }
};
