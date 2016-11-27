#ifndef GAME_H
#define GAME_H

#include <Box2D/Box2D.h>
#include "SFML/Graphics.hpp"
#include <memory>
#include <vector>
#include "Creature.h"

class Game {
        static Game m_instance;
    public:
        static Game& i() { return m_instance; }

        void init(float _dt);
        void run();
        void restart();
        void update();
        void render();

        float getDeltaTime();
        void setDeltaTime(float _dt);
        float getTime();
        sf::Vector2f getViewSize();


        void drawRect(sf::VertexArray& va, b2Body* b, b2Fixture* f, const sf::Vector2f& dim, const sf::Color& c = sf::Color::Magenta);
        void spawnCreature();
        sf::Vector2f getSpawnPos();
    private:
        float m_accumulator = 0.0f;
        float m_dt = 1 / 125.0f;
        sf::RenderWindow m_window;
        sf::Vector2f m_view_size;
        float m_global_timer = 0.0f;
        bool m_first_launch = true;
        sf::Font m_font;

        sf::View view;
        b2Vec2 gravity;
        std::unique_ptr<b2World> m_world;
        std::vector<std::unique_ptr<Creature>> m_creatures;

        b2Body* groundBody;
        b2Fixture* groundFixture;
        sf::Vector2f groundDim;

        sf::Vector2f viewDimensions;

        float best_x = 0.0f;
        unsigned overall_best_id = 0;
        float overall_best_x = 0.0f;
        // Genetic Algorithm
        unsigned gen = 1;
        unsigned curr_creature_id = 0;
};

#endif // GAME_H
