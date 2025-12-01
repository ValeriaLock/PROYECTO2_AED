#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <string>
#include "Entity.h"
#include "QuadTree.h"

// ------------------ utilidades matemáticas ------------------

float length(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f operator-(const sf::Vector2f& a, const sf::Vector2f& b) {
    return {a.x - b.x, a.y - b.y};
}

sf::Vector2f operator+(const sf::Vector2f& a, const sf::Vector2f& b) {
    return {a.x + b.x, a.y + b.y};
}

sf::Vector2f operator*(const sf::Vector2f& a, float s) {
    return {a.x * s, a.y * s};
}

sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = length(v);
    if (len == 0.f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
}

// ------------------ pantallas ------------------

enum class Screen {
    Menu,
    QuadDebug,
    Arcade
};

// tiempo formateado mm:ss:ms
std::string formatTime(float seconds) {
    if (seconds < 0.f) seconds = 0.f;
    int s = static_cast<int>(seconds);
    int ms = static_cast<int>((seconds - s) * 100.f);
    int m = s / 60;
    s = s % 60;
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", m, s, ms);
    return buffer;
}

// ------------------ dibujo ------------------

void drawAlien(sf::RenderWindow& window,
               const sf::Vector2f& center,
               float baseSize,
               sf::Color color) {
    const int W = 11;
    const int H = 8;
    const int pattern[H][W] = {
        {0,0,1,0,0,0,0,0,1,0,0},
        {0,0,0,1,0,0,0,1,0,0,0},
        {0,0,1,1,1,1,1,1,1,0,0},
        {0,1,1,0,1,1,1,0,1,1,0},
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,0,1,1,1,1,1,1,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,1},
        {0,0,0,1,1,0,1,1,0,0,0},
    };
    /*const int pattern[H][W] = {
        {0,0,1,1,1,1,1,0,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,0},
        {1,1,0,1,1,1,1,1,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,0,1,1,1,0,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,1,0,1,0,0,0,1,0,1,1},
        {0,0,0,0,1,1,1,0,0,0,0},
    };*/

    float scale = baseSize / static_cast<float>(H);
    sf::RectangleShape pixel;
    pixel.setSize({scale, scale});
    pixel.setFillColor(color);

    float totalWidth  = W * scale;
    float totalHeight = H * scale;
    sf::Vector2f topLeft(center.x - totalWidth / 2.f,
                         center.y - totalHeight / 2.f);

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (pattern[y][x] == 0) continue;
            pixel.setPosition(sf::Vector2f(
                topLeft.x + x * scale,
                topLeft.y + y * scale
            ));
            window.draw(pixel);
        }
    }
}

// ------------------ main ------------------

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const unsigned WIDTH  = 800;
    const unsigned HEIGHT = 600;
    const float RADIUS = 6.f;

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "QuadTree");
    window.setFramerateLimit(60);

    sf::FloatRect world({0.f, 0.f}, {static_cast<float>(WIDTH), static_cast<float>(HEIGHT)});

    Screen current = Screen::Menu;

    // ------------ modo debug ------------
    QuadTree quadDebug(world, 4);
    std::vector<Entity> debugEntities;
    bool debugInitialized = false;
    bool showGridDebug = false;

    // ------------ modo arcade ------------
    QuadTree quadArcade(world, 4);
    std::vector<Entity> arcadeEnemies;
    Entity player;
    bool arcadeInitialized = false;
    bool arcadeGameOver = false;
    bool showGridArcade = false;
    float arcadeSurvivalTime = 0.f;
    float arcadeSpawnTimer = 0.f;
    const int arcadeMaxEnemies = 80;

    // ------------ fuente ------------
    sf::Font font;
    bool hasFont = font.openFromFile("PIXEL.ttf");
    if (!hasFont) {
        hasFont = font.openFromFile("ARIAL.TTF");
    }

    // ------------ fondo ------------
    sf::Texture bgTexture;
    bool hasBg = bgTexture.loadFromFile("fondo.jpg");
    std::optional<sf::Sprite> bgSprite;
    if (hasBg) {
        bgSprite.emplace(bgTexture);
        auto size = bgTexture.getSize();
        float scaleX = WIDTH  / static_cast<float>(size.x);
        float scaleY = HEIGHT / static_cast<float>(size.y);
        float scale  = (scaleX > scaleY) ? scaleX : scaleY;
        bgSprite->setScale(sf::Vector2f(scale, scale));
        bgSprite->setPosition(sf::Vector2f(0.f, 0.f));
    }

    sf::Clock clock;

    while (window.isOpen()) {
        // ----------- eventos -----------
        while (std::optional<sf::Event> ev = window.pollEvent()) {
            const sf::Event& event = *ev;

            if (event.is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                auto key = keyPressed->code;

                if (key == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                if (current == Screen::Menu) {
                    if (key == sf::Keyboard::Key::Num1) {
                        current = Screen::QuadDebug;
                        debugInitialized = false;
                    } else if (key == sf::Keyboard::Key::Num2) {
                        current = Screen::Arcade;
                        arcadeInitialized = false;
                        arcadeGameOver = false;
                    }
                } else {
                    if (key == sf::Keyboard::Key::M) {
                        current = Screen::Menu;
                    }
                    if (current == Screen::Arcade && key == sf::Keyboard::Key::R) {
                        arcadeInitialized = false;
                        arcadeGameOver = false;
                    }
                    if (key == sf::Keyboard::Key::Space) {
                        if (current == Screen::QuadDebug)
                            showGridDebug = !showGridDebug;
                        else if (current == Screen::Arcade)
                            showGridArcade = !showGridArcade;
                    }
                }
            }
        }

        float dt = clock.restart().asSeconds();

        // ======================================================
        //                       DEBUG
        // ======================================================
        if (current == Screen::QuadDebug) {
            if (!debugInitialized) {
                debugEntities.clear();
                for (int i = 0; i < 80; ++i) {
                    sf::Vector2f pos(
                        static_cast<float>(std::rand() % WIDTH),
                        static_cast<float>(std::rand() % HEIGHT)
                    );
                    sf::Vector2f vel(
                        (std::rand() % 100 - 50) / 50.f,
                        (std::rand() % 100 - 50) / 50.f
                    );
                    debugEntities.emplace_back(EntityKind::Enemy, pos, RADIUS, vel);
                }
                debugInitialized = true;
            }

            for (auto& e : debugEntities) e.colliding = false;

            // mover
            for (auto& e : debugEntities) {
                sf::Vector2f pos = e.shape.getPosition();
                pos += e.velocity * 100.f * dt;

                if (pos.x - RADIUS < 0.f) {
                    pos.x = RADIUS;
                    e.velocity.x *= -1.f;
                } else if (pos.x + RADIUS > WIDTH) {
                    pos.x = WIDTH - RADIUS;
                    e.velocity.x *= -1.f;
                }

                if (pos.y - RADIUS < 0.f) {
                    pos.y = RADIUS;
                    e.velocity.y *= -1.f;
                } else if (pos.y + RADIUS > HEIGHT) {
                    pos.y = HEIGHT - RADIUS;
                    e.velocity.y *= -1.f;
                }

                e.shape.setPosition(pos);
            }

            // reconstruir QuadTree
            quadDebug.reset();
            for (auto& e : debugEntities) {
                quadDebug.insert(&e);
            }

            // detectar colisiones usando QuadTree
            for (auto& e : debugEntities) {
                sf::Vector2f pos = e.shape.getPosition();
                float querySize = RADIUS * 4.f;
                sf::FloatRect range(
                    {pos.x - querySize / 2.f, pos.y - querySize / 2.f},
                    {querySize, querySize}
                );

                std::vector<Entity*> candidates;
                quadDebug.queryRange(range, candidates);

                for (auto* other : candidates) {
                    if (&e == other) continue;
                    sf::Vector2f diff = other->shape.getPosition() - pos;
                    if (length(diff) < 2 * RADIUS) {
                        e.colliding = true;
                        other->colliding = true;
                    }
                }
            }

            // dibujar
            window.clear();
            if (bgSprite) window.draw(*bgSprite);

            if (showGridDebug) {
                quadDebug.draw(window);
            }

            for (auto& e : debugEntities) {
                e.shape.setFillColor(e.colliding ? sf::Color::Red : sf::Color::White);
                window.draw(e.shape);
            }

            if (hasFont) {
                std::string msg =
                    "QUADTREE\n"
                    "[ESPACE] Quadtree\n"
                    "[M] Menu | [ESC] Salir";
                sf::Text t(font, msg, 18);
                auto bounds = t.getLocalBounds();
                t.setOrigin(sf::Vector2f(bounds.size.x / 2.f, 0.f));
                t.setPosition(sf::Vector2f(WIDTH / 2.f, 20.f));
                t.setFillColor(sf::Color::Cyan);
                t.setOutlineColor(sf::Color::Black);
                t.setOutlineThickness(2.f);
                window.draw(t);
            }

            window.display();
        }

        // ======================================================
        //                       ARCADE
        // ======================================================
        else if (current == Screen::Arcade) {
            const sf::Vector2f center(static_cast<float>(WIDTH) / 2.f,
                                      static_cast<float>(HEIGHT) / 2.f);
            const float safeRadius = 100.f;

            if (!arcadeInitialized) {
                arcadeEnemies.clear();

                player = Entity(
                    EntityKind::Player,
                    center,
                    RADIUS * 1.5f,
                    sf::Vector2f(0.f, 0.f)
                );

                // enemigos iniciales
                for (int i = 0; i < 45; ++i) {
                    sf::Vector2f pos;
                    do {
                        pos = sf::Vector2f(
                            static_cast<float>(std::rand() % WIDTH),
                            static_cast<float>(std::rand() % HEIGHT)
                        );
                    } while (length(pos - center) < safeRadius);

                    sf::Vector2f vel(
                        (std::rand() % 100 - 50) / 80.f,
                        (std::rand() % 100 - 50) / 80.f
                    );
                    Entity e(EntityKind::Enemy, pos, RADIUS, vel);
                    e.fusionStage = 0; // verde
                    arcadeEnemies.push_back(e);
                }

                arcadeInitialized = true;
                arcadeGameOver = false;
                arcadeSurvivalTime = 0.f;
                arcadeSpawnTimer = 0.f;
            }

            player.colliding = false;
            for (auto& e : arcadeEnemies) e.colliding = false;

            if (!arcadeGameOver) {
                arcadeSurvivalTime += dt;
                arcadeSpawnTimer += dt;

                // movimiento jugador (con WASD o flechas)
                sf::Vector2f dir(0.f, 0.f);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
                    dir.x -= 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
                    dir.x += 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
                    dir.y -= 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
                    dir.y += 1.f;

                dir = normalize(dir);
                const float playerSpeed = 200.f;
                sf::Vector2f ppos = player.shape.getPosition();
                ppos += dir * playerSpeed * dt;

                float pR = player.shape.getRadius();
                if (ppos.x - pR < 0.f) ppos.x = pR;
                if (ppos.x + pR > WIDTH) ppos.x = WIDTH - pR;
                if (ppos.y - pR < 0.f) ppos.y = pR;
                if (ppos.y + pR > HEIGHT) ppos.y = HEIGHT - pR;
                player.shape.setPosition(ppos);

                // generación de nuevos enemigos
                if (arcadeSpawnTimer >= 3.f &&
                    static_cast<int>(arcadeEnemies.size()) < arcadeMaxEnemies) {
                    arcadeSpawnTimer = 0.f;
                    sf::Vector2f pos;
                    do {
                        pos = sf::Vector2f(
                            static_cast<float>(std::rand() % WIDTH),
                            static_cast<float>(std::rand() % HEIGHT)
                        );
                    } while (length(pos - center) < safeRadius);

                    sf::Vector2f vel(
                        (std::rand() % 100 - 50) / 80.f,
                        (std::rand() % 100 - 50) / 80.f
                    );
                    Entity e(EntityKind::Enemy, pos, RADIUS, vel);
                    e.fusionStage = 0;
                    arcadeEnemies.push_back(e);
                }

                // movimiento enemigos
                for (auto& e : arcadeEnemies) {
                    float r = e.shape.getRadius();
                    sf::Vector2f pos = e.shape.getPosition();
                    pos += e.velocity * 100.f * dt * (r / RADIUS);

                    if (pos.x - r < 0.f) {
                        pos.x = r;
                        e.velocity.x *= -1.f;
                    } else if (pos.x + r > WIDTH) {
                        pos.x = WIDTH - r;
                        e.velocity.x *= -1.f;
                    }

                    if (pos.y - r < 0.f) {
                        pos.y = r;
                        e.velocity.y *= -1.f;
                    } else if (pos.y + r > HEIGHT) {
                        pos.y = HEIGHT - r;
                        e.velocity.y *= -1.f;
                    }

                    e.shape.setPosition(pos);
                }

                // reconstruir QuadTree
                quadArcade.reset();
                for (auto& e : arcadeEnemies) {
                    quadArcade.insert(&e);
                }

                const int MAX_STAGE = 5;
                std::vector<bool> alive(arcadeEnemies.size(), true);

                // fusiones usando QuadTree
                for (std::size_t i = 0; i < arcadeEnemies.size(); ++i) {
                    if (!alive[i]) continue;
                    auto& e = arcadeEnemies[i];
                    sf::Vector2f pos = e.shape.getPosition();
                    float r = e.shape.getRadius();
                    float querySize = r * 4.f;

                    sf::FloatRect range(
                        {pos.x - querySize / 2.f, pos.y - querySize / 2.f},
                        {querySize, querySize}
                    );

                    std::vector<Entity*> candidates;
                    quadArcade.queryRange(range, candidates);

                    for (auto* otherPtr : candidates) {
                        if (&e == otherPtr) continue;

                        std::size_t j = arcadeEnemies.size();
                        for (std::size_t k = 0; k < arcadeEnemies.size(); ++k) {
                            if (&arcadeEnemies[k] == otherPtr) {
                                j = k;
                                break;
                            }
                        }
                        if (j == arcadeEnemies.size() || !alive[j]) continue;

                        auto& o = arcadeEnemies[j];
                        sf::Vector2f diff = o.shape.getPosition() - pos;
                        float dist = length(diff);
                        float rSum = r + o.shape.getRadius();

                        if (dist < rSum * 0.9f) {
                            float r2 = o.shape.getRadius();
                            float newArea = r * r + r2 * r2;
                            float newRadius = std::sqrt(newArea);

                            sf::Vector2f newPos = (pos + o.shape.getPosition()) * 0.5f;
                            sf::Vector2f newVel = (e.velocity + o.velocity) * 0.5f;

                            e.shape.setRadius(newRadius);
                            e.shape.setOrigin(sf::Vector2f(newRadius, newRadius));
                            e.shape.setPosition(newPos);
                            e.velocity = newVel;

                            int newStage = e.fusionStage;
                            if (o.fusionStage > newStage) newStage = o.fusionStage;
                            if (newStage < MAX_STAGE) newStage += 1;
                            e.fusionStage = newStage;

                            alive[j] = false;
                        }
                    }
                }

                std::vector<Entity> newEnemies;
                newEnemies.reserve(arcadeEnemies.size());
                for (std::size_t i = 0; i < arcadeEnemies.size(); ++i) {
                    if (alive[i]) newEnemies.push_back(arcadeEnemies[i]);
                }
                arcadeEnemies.swap(newEnemies);

                quadArcade.reset();
                for (auto& e : arcadeEnemies) {
                    quadArcade.insert(&e);
                }

                // muerte del jugador (colisión)
                sf::Vector2f p = player.shape.getPosition();
                float querySizeP = 200.f;
                sf::FloatRect rangeP(
                    {p.x - querySizeP / 2.f, p.y - querySizeP / 2.f},
                    {querySizeP, querySizeP}
                );

                std::vector<Entity*> candidatesP;
                quadArcade.queryRange(rangeP, candidatesP);

                for (auto* enemy : candidatesP) {
                    sf::Vector2f diff = enemy->shape.getPosition() - p;
                    float dist = length(diff);
                    float rP = player.shape.getRadius();
                    float rE = enemy->shape.getRadius();

                    if (dist < rP + rE) {
                        player.colliding = true;
                        arcadeGameOver = true;
                        break;
                    }
                }
            }

            // colores y dibujo
            window.clear();
            if (bgSprite) window.draw(*bgSprite);
            if (showGridArcade) quadArcade.draw(window);

            auto colorFromStage = [](int stage) -> sf::Color {
                switch (stage) {
                    case 0: return sf::Color(0, 255, 0);       // verde
                    case 1: return sf::Color(255, 255, 0);     // amarillo
                    case 2: return sf::Color(255, 165, 0);     // naranja
                    case 3: return sf::Color(255, 0, 0);       // rojo
                    case 4: return sf::Color(180, 0, 255);     // morado
                    case 5: return sf::Color(0, 150, 255);     // azul
                    default: return sf::Color::White;
                }
            };

            for (auto& e : arcadeEnemies) {
                float baseSize = e.shape.getRadius() * 3.f;
                sf::Color c = colorFromStage(e.fusionStage);
                drawAlien(window, e.shape.getPosition(), baseSize, c);
            }

            player.shape.setFillColor(player.colliding ? sf::Color::Red : sf::Color::Green);
            window.draw(player.shape);

            if (hasFont) {
                std::string msg;
                if (!arcadeGameOver) {
                    msg =
                        "MODO ARCADE\n"
                        "Tiempo: " + formatTime(arcadeSurvivalTime) + "\n"
                        "Move: WASD / Flechas\n"
                        "[ESPACE] Quadtree | [R] Reiniciar | [M] Menu | [ESC] Salir";
                } else {
                    msg =
                        "GAME OVER\n"
                        "Tiempo: " + formatTime(arcadeSurvivalTime) + "\n"
                        "[R] Reiniciar | [M] Menu | [ESC] Salir";
                }
                sf::Text t(font, msg, 16);
                auto bounds = t.getLocalBounds();
                t.setOrigin(sf::Vector2f(bounds.size.x / 2.f, 0.f));
                t.setPosition(sf::Vector2f(WIDTH / 2.f, 20.f));
                t.setFillColor(sf::Color::Yellow);
                t.setOutlineColor(sf::Color::Black);
                t.setOutlineThickness(2.f);
                window.draw(t);
            }

            window.display();
        }

        // ======================================================
        //                        MENU
        // ======================================================
        else {
            window.clear();
            if (bgSprite) window.draw(*bgSprite);

            if (hasFont) {
                sf::Text title(font, "QUAD TREE", 48);
                auto tb = title.getLocalBounds();
                title.setOrigin(sf::Vector2f(tb.size.x / 2.f, tb.size.y / 2.f));
                title.setPosition(sf::Vector2f(WIDTH / 2.f, HEIGHT / 2.f - 120.f));
                title.setFillColor(sf::Color(0, 255, 255));
                title.setOutlineColor(sf::Color(255, 0, 255));
                title.setOutlineThickness(2.f);
                window.draw(title);

                sf::Text opt1(font, "[1] Modo QuadTree", 24);
                auto b1 = opt1.getLocalBounds();
                opt1.setOrigin(sf::Vector2f(b1.size.x / 2.f, b1.size.y / 2.f));
                opt1.setPosition(sf::Vector2f(WIDTH / 2.f, HEIGHT / 2.f - 20.f));
                opt1.setFillColor(sf::Color::White);
                opt1.setOutlineColor(sf::Color::Black);
                opt1.setOutlineThickness(2.f);
                window.draw(opt1);

                sf::Text opt2(font, "[2] Modo Arcade", 24);
                auto b2 = opt2.getLocalBounds();
                opt2.setOrigin(sf::Vector2f(b2.size.x / 2.f, b2.size.y / 2.f));
                opt2.setPosition(sf::Vector2f(WIDTH / 2.f, HEIGHT / 2.f + 30.f));
                opt2.setFillColor(sf::Color::White);
                opt2.setOutlineColor(sf::Color::Black);
                opt2.setOutlineThickness(2.f);
                window.draw(opt2);

                sf::Text opt3(font, "[ESC] Salir", 20);
                auto b3 = opt3.getLocalBounds();
                opt3.setOrigin(sf::Vector2f(b3.size.x / 2.f, b3.size.y / 2.f));
                opt3.setPosition(sf::Vector2f(WIDTH / 2.f, HEIGHT / 2.f + 100.f));
                opt3.setFillColor(sf::Color(200, 200, 200));
                opt3.setOutlineColor(sf::Color::Black);
                opt3.setOutlineThickness(2.f);
                window.draw(opt3);
            }

            window.display();
        }
    }

    return 0;
}