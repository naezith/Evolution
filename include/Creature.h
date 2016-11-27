#ifndef CREATURE_H
#define CREATURE_H

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

class Muscle {
    public:
        Muscle(b2World* world, b2DistanceJointDef& def, float short_len_, float long_len_,
               float extend_time_, float contract_time_, float strength_);

        void update(float c_time, float dt);
        void setActive(bool active);
        void render(sf::RenderTarget& rt);

        b2DistanceJoint* joint = nullptr;

        float short_len;
        float long_len;
        float* target_len = nullptr;

        float extend_time;
        float contract_time;

        float strength;
        sf::Color c;
};

class Node{
    public:
        Node();
        virtual ~Node();

        void init(b2World* world, const sf::Vector2f& pos, float friction);
        void setActive(bool active);
        sf::Vector2f getPosition() const;
        void setPosition(const sf::Vector2f& pos);
        void render(sf::RenderTarget& rt);
        sf::Color c;
        b2CircleShape shape;
        b2FixtureDef fixture_def;
        b2BodyDef body_def;
        b2Body* body = nullptr;
};

class Creature {
    public:
        Creature();

        void init(b2World* world, const sf::Vector2f& pos);
        void update(float dt);
        void render(sf::RenderTarget& rt);
        const sf::Vector2f& getPosition() const { return pos; };

        void addRandomNode();
        void addMuscle(Node* a, Node* b);
        void setActive(bool active);

        std::vector<Node> nodes;
        std::vector<Muscle> muscles;

        float timer = 0.0f;
        float heart_beat = 0.5f;

        b2World* world;
        float fitness = 0.0f;
    private:
        sf::Vector2f pos;
};

#endif // CREATURE_H
