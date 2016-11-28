#ifndef CREATURE_H
#define CREATURE_H

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>

class Node;
class Muscle {
    public:
        Muscle();
        virtual ~Muscle();

        void init(b2World* world_, std::vector<std::unique_ptr<Node>>& nodes, int a_, int b_, float short_len_, float long_len_,
               float extend_time_, float contract_time_, float strength_);
        void update(float c_time, float dt);
        void setActive(bool active);
        void render(sf::RenderTarget& rt);
        std::unique_ptr<Muscle> mutatedCopy(std::vector<std::unique_ptr<Node>>& node);
        std::unique_ptr<Muscle> copy(std::vector<std::unique_ptr<Node>>& node);

        b2DistanceJoint* joint = nullptr;

        float short_len;
        float long_len;
        float* target_len = nullptr;

        float extend_time;
        float contract_time;

        float strength;
        sf::Color c;
        b2World* world = nullptr;
        int a = -1, b = -1;
};

class Node{
    public:
        Node();
        virtual ~Node();

        void init(b2World* world_, const sf::Vector2f& pos, float friction);
        void setActive(bool active);
        sf::Vector2f getPosition() const;
        void setPosition(const sf::Vector2f& pos);
        void render(sf::RenderTarget& rt);
        std::unique_ptr<Node> mutatedCopy();
        std::unique_ptr<Node> copy();

        sf::Color c;
        b2CircleShape shape;
        b2FixtureDef fixture_def;
        b2BodyDef body_def;
        b2Body* body = nullptr;
        b2World* world = nullptr;
};

class Creature {
    public:
        Creature();

        void init(b2World* world_, const sf::Vector2f& position);
        void updatePosition();
        void setPosition(const sf::Vector2f& position);
        void update(float dt);
        void render(sf::RenderTarget& rt);
        const sf::Vector2f& getPosition() const { return pos; };

        void addRandomNode();
        void addMuscle(int a = -1, int b = -1);
        void setActive(bool active);
        std::unique_ptr<Creature> mutatedCopy();
        std::unique_ptr<Creature> copy();
        void removeRandomNode();
        void removeRandomMuscle();
        void checkMuscleOverlap();
        void checkLoneNodes();

        std::vector<std::unique_ptr<Node>> nodes;
        std::vector<std::unique_ptr<Muscle>> muscles;

        float timer = 0.0f;
        float heart_beat = 0.5f;

        b2World* world = nullptr;
        float fitness = 0.0f;
    private:
        sf::Vector2f pos;
};

#endif // CREATURE_H
