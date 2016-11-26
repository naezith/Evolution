#ifndef CREATURE_H
#define CREATURE_H

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

class Muscle {
    public:
        Muscle(b2World* world, b2DistanceJointDef& def, float short_len_, float long_len_,
               float extend_time_, float contract_time_, float strength_);

        void update(float c_time, float dt);
        void render(sf::RenderTarget& rt);

        b2DistanceJoint* joint = nullptr;

        float short_len = 1.5f;
        float long_len = 3.0f;
        float* target_len = nullptr;

        float extend_time = 0.75f;
        float contract_time = 0.25f;

        float strength = 1.0f;
        sf::Color c;
};

class Node{
    public:
        Node();
        virtual ~Node();

        void init(b2World* world, const sf::Vector2f& pos, float friction);
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


        void addRandomNode();
        void addMuscle(Node* a, Node* b);

        std::vector<Node> nodes;
        std::vector<Muscle> muscles;

        float timer = 0.0f;
        float heart_beat = 0.5f;

        b2World* world;
};

#endif // CREATURE_H
