#include "Creature.h"
#include "utility.h"

/// CREATURE
Creature::Creature() {
}

float r(){
    return pow(random_float(-1, 1), 19);
}

void Creature::init(b2World* world_, const sf::Vector2f& position) {
    world = world_;

    heart_beat = random_float(0.1, 1.1);
    // Create the first node
    nodes.push_back(std::make_unique<Node>());
    nodes.back()->init(world, sf::Vector2f(0,0), random_float(0, 1));

    int node_count = random_int(2, 3);
    for(int i = 0; i < node_count; ++i) addRandomNode();

    setPosition(position);
    setActive(false);
}

void Creature::setPosition(const sf::Vector2f& position) {
    updatePosition(); // Update the current center
    sf::Vector2f move_vec = position - pos;

    // Move all nodes
    for(unsigned i = 0; i < nodes.size(); ++i)
        nodes[i]->setPosition(nodes[i]->getPosition() + move_vec);

    pos = position; // Center is changed
}

std::unique_ptr<Creature> Creature::mutatedCopy() {
    std::unique_ptr<Creature> mutated = std::make_unique<Creature>();
    mutated->world = world;
    mutated->heart_beat = heart_beat + 0.2f*r();

    for(unsigned i = 0; i < nodes.size(); ++i)
        mutated->nodes.push_back(nodes[i]->mutatedCopy());

    for(unsigned i = 0; i < muscles.size(); ++i)
        mutated->muscles.push_back(muscles[i]->mutatedCopy(mutated->nodes));

    if(random_int(0, 100) < 5) mutated->addRandomNode();
    if(random_int(0, 100) < 5) mutated->addMuscle();

    // Remove random node
    if(random_int(0, 100) < 5) mutated->removeRandomNode();

    // Remove random muscle
    if(random_int(0, 100) < 5) mutated->removeRandomMuscle();

    // Last checks
    mutated->checkMuscleOverlap();
    mutated->checkLoneNodes();
    mutated->setActive(false);

    return std::move(mutated);
}

const float NODE_DIST = 1.5f;
void Creature::removeRandomNode() {
    if(nodes.size() > 3) {
        int victim_id = random_int(0, nodes.size() - 1);

        // Remove the muscles which hold this node
        int i = 0;
        while(i < muscles.size()) {
            if(muscles[i]->a == victim_id || muscles[i]->b == victim_id)
                muscles.erase(muscles.begin() + i);
            else ++i;
        }

        // Lower other
        for(int i = 0; i < muscles.size(); i++){
            Muscle* m = muscles[i].get();

            if(m->a >= victim_id) --m->a;
            if(m->b >= victim_id) --m->b;
        }

        nodes.erase(nodes.begin() + victim_id);
    }
}

void Creature::removeRandomMuscle() {
    if(muscles.size() > 1) muscles.erase(muscles.begin() + random_int(0, muscles.size() - 1));
}

void Creature::checkMuscleOverlap() {
    std::vector<Muscle*> muscles_to_remove;
    for(unsigned i = 0; i < muscles.size(); ++i) {
        for(unsigned j = i + 1; j < muscles.size(); ++j) {
            Muscle* first = muscles[i].get();
            Muscle* second = muscles[j].get();
            if((first->a == second->a && first->b == second->b) ||
               (first->a == second->b && first->b == second->a) ||
               (first->a == first->b)) {
                   if(!contains(muscles_to_remove, first)) muscles_to_remove.push_back(first);
            }
        }
    }
    for(unsigned i = 0; i < muscles_to_remove.size(); ++i) {
        Muscle* curr = muscles_to_remove[i];
        for(unsigned j = 0; j < muscles.size(); ++j) {
            if(curr == muscles[j].get()) {
                muscles.erase(muscles.begin() + j);
                break;
            }
        }
    }
}

void Creature::checkLoneNodes() {
    if(nodes.size() >= 3) {
        for(unsigned i = 0; i < nodes.size(); ++i) {
            unsigned connected_node_count = 0;
            int connected_node = -1;

            // Find connected nodes
            for(unsigned j = 0; j < muscles.size(); ++j) {
                Muscle* musc = muscles[j].get();
                if(musc->a == i) {
                    ++connected_node_count;
                    connected_node = musc->b;
                }
                else if(musc->b == i) {
                    ++connected_node_count;
                    connected_node = musc->b;
                }
            }

            // Get some friends
            if(connected_node_count < 1) {
                unsigned buddy;
                do { buddy = random_int(0, nodes.size() - 1);
                } while(buddy == i || i == connected_node); // It's me or someone who is already a friend

                addMuscle(i, buddy);
            }
        }
    }
}

void Creature::addRandomNode() {
    if(nodes.empty()) return;

    // Add New node
    int parent = random_int(0, nodes.size()-1);
    sf::Vector2f offset(random_float(-NODE_DIST, NODE_DIST), random_float(-NODE_DIST, NODE_DIST));
    nodes.push_back(std::make_unique<Node>());
    Node* new_node = nodes.back().get();
    new_node->init(world, nodes[parent]->getPosition() + offset,
                  random_float(0, 1)); // Friction

    addMuscle(parent, nodes.size()-1);

    // Connect one more muscle to the closest node
    unsigned closest_id = -1;
    float shortest_dist = 999999999;
    for(unsigned i = 0; i < nodes.size() - 1; ++i) {
        if(i == parent) continue;

        float dist = magnitude(new_node->getPosition() - nodes[i]->getPosition());
        if(dist < shortest_dist) {
            shortest_dist = dist;
            closest_id = i;
        }
    }

    if(closest_id != -1) addMuscle(closest_id, nodes.size()-1);
}

void Creature::addMuscle(int a, int b) {
    // Pick random two nodes
    if(a == -1) {
        if(nodes.size() < 3) return;

        a = b = random_int(0, nodes.size() - 1);

        while(a == b) {
            b = random_int(0, nodes.size() - 1);
        }
    }

    // Create the muscle
    float ratio = random_float(0.01, 0.2);
    muscles.push_back(std::make_unique<Muscle>());
    float dist = magnitude(nodes[b]->getPosition() - nodes[a]->getPosition());
    muscles.back()->init(world, nodes, a, b,
                             dist*(1.0f - ratio), // Short length
                             dist*(1.0f + ratio), // Long length
                             random_float(0, 1), // Extend time
                             random_float(0, 1), // Contract time
                             random_float(3, 12)); // Strength
}

void Creature::updatePosition() {
    sf::Vector2f sum(0, 0);
    for(unsigned i = 0; i < nodes.size(); ++i) sum += nodes[i]->getPosition();
    pos = sum / (float) nodes.size();
}

void Creature::update(float dt) {
    if((timer += dt) > heart_beat) timer = 0;
    for(unsigned i = 0; i < muscles.size(); ++i) {
        muscles[i]->update(timer/heart_beat, dt);
    }

    // Set position to center of nodes
    updatePosition();

    // His distance is his fitness
    if(pos.x > fitness) fitness = pos.x;
}

void Creature::setActive(bool active) {
    for(unsigned i = 0; i < nodes.size(); ++i) nodes[i]->setActive(active);
    for(unsigned i = 0; i < muscles.size(); ++i) muscles[i]->setActive(active);
}

void Creature::render(sf::RenderTarget& rt) {
    for(unsigned i = 0; i < muscles.size(); ++i) muscles[i]->render(rt);
    for(unsigned i = 0; i < nodes.size(); ++i) nodes[i]->render(rt);
}




const float MIN_STRENGTH = 3.0f;
const float MAX_STRENGTH = 12.0f;
const float MUSCLE_THICKNESS = 0.2f;

/// MUSCLE
Muscle::Muscle(){}

Muscle::~Muscle() {
    world->DestroyJoint(joint);
    joint = nullptr;
}

void Muscle::init(b2World* world_, std::vector<std::unique_ptr<Node>>& nodes, int a_, int b_,
               float short_len_, float long_len_,
               float extend_time_, float contract_time_,
               float strength_) {
    a = a_;
    b = b_;


    b2DistanceJointDef jointDef;
    jointDef.bodyA = nodes[a]->body;
    jointDef.bodyB = nodes[b]->body;

    sf::Vector2f a2b = normalize(nodes[b]->getPosition() - nodes[a]->getPosition())*nodes[a]->shape.m_radius;
    jointDef.localAnchorA = b2Vec2( a2b.x,  a2b.y);
    jointDef.localAnchorB = b2Vec2(-a2b.x, -a2b.y);
    jointDef.collideConnected = false;
    jointDef.frequencyHz = 0;


    short_len = short_len_;
    long_len = long_len_;
    extend_time = extend_time_;
    contract_time = contract_time_;
    strength = strength_;

    world = world_;
    jointDef.length = long_len;
    target_len = &short_len;
    joint = (b2DistanceJoint*) world->CreateJoint( &jointDef );
    joint->SetLength(long_len);

    float d = (strength-MIN_STRENGTH) / (MAX_STRENGTH - MIN_STRENGTH);
    c = sf::Color(255 - (255-0)*d, 255 - (255-0)*d, 255 - (255-0)*d);
}

std::unique_ptr<Muscle> Muscle::mutatedCopy(std::vector<std::unique_ptr<Node>>& nodes) {
    std::unique_ptr<Muscle> mutated = std::make_unique<Muscle>();
    float len1 = short_len + short_len*0.15f*r();
    float len2 = long_len  + long_len *0.15f*r();
    mutated->init(world, nodes, a, b,
                     std::min(len1, len2), // Short length
                     std::max(len1, len2), // Long length
                     std::min(std::max(extend_time   + 0.2f*r(), 0.0f), 1.0f), // Extend time
                     std::min(std::max(contract_time + 0.2f*r(), 0.0f), 1.0f), // Contract time
                     strength + strength*0.2*r()); // Strength
    return std::move(mutated);
}

void Muscle::update(float c_time, float dt) {
    if(joint == nullptr) return;

    target_len = (contract_time < extend_time && (c_time < contract_time || extend_time < c_time)) ||
                 (extend_time < contract_time && extend_time < c_time) ?
                &short_len : &long_len;

    joint->SetLength(joint->GetLength() + strength*((*target_len) - joint->GetLength())*dt);
}

void Muscle::setActive(bool active) {

}

void Muscle::render(sf::RenderTarget& rt) {
    if(joint == nullptr) return;

    sf::RectangleShape rect;

    sf::Vector2f a(joint->GetBodyA()->GetPosition().x, joint->GetBodyA()->GetPosition().y);
    sf::Vector2f b(joint->GetBodyB()->GetPosition().x, joint->GetBodyB()->GetPosition().y);
    rect.setSize(sf::Vector2f(MUSCLE_THICKNESS * (1.0 - 0.5*(joint->GetLength()-short_len)/(long_len-short_len)), magnitude(a-b)));
    rect.setOrigin(rect.getSize().x*0.5f, rect.getSize().y);
    rect.setPosition(a);
    rect.rotate(vecToAngle(b-a));
    rect.setFillColor(c);

    rt.draw(rect);
}






/// NODE
Node::Node(){
}

Node::~Node() {
    world->DestroyBody(body);
    body = nullptr;
}

std::unique_ptr<Node> Node::mutatedCopy() {
    std::unique_ptr<Node> mutated = std::make_unique<Node>();
    mutated->init(world, sf::Vector2f(getPosition().x + 0.333f*NODE_DIST*r(),
                                      getPosition().y + 0.333f*NODE_DIST*r()),
                 std::min(std::max(fixture_def.friction + 0.1f*r(), 0.0f), 1.0f));
    return std::move(mutated);
}

void Node::init(b2World* world_, const sf::Vector2f& pos, float friction) {
    world = world_;
    body_def.type = b2_dynamicBody; //this will be a dynamic body
    body_def.position.Set(-10, 20); //a little to the left
    body_def.fixedRotation = true;
    body = world->CreateBody(&body_def);
    if(body == nullptr) return;

    shape.m_p.Set(0, 0); //position, relative to body position
    shape.m_radius = MUSCLE_THICKNESS*0.8; //radius

    fixture_def.shape = &shape; //this is a pointer to the shape above
    fixture_def.friction = friction;

    b2Filter filter;
    filter.groupIndex = -2;
    fixture_def.filter = filter;
    body->CreateFixture(&fixture_def); //add a fixture to the body
    setPosition(pos);
    float d = fixture_def.friction;
    c = sf::Color(255 - (255-139)*d, 255 - (255-0)*d, 255 - (255-0)*d);
}

void Node::setActive(bool active) {
    if(body == nullptr) return;
    body->SetActive(active);
}

sf::Vector2f Node::getPosition() const {
    if(body == nullptr) return sf::Vector2f(0, 0);
    return sf::Vector2f(body->GetPosition().x, body->GetPosition().y);
}

void Node::setPosition(const sf::Vector2f& pos) {
    if(body == nullptr) return;
    body->SetTransform(b2Vec2(pos.x, pos.y), body->GetAngle());
}

void Node::render(sf::RenderTarget& rt) {
    if(body == nullptr) return;
    sf::CircleShape circ;
    circ.setFillColor(c);
    circ.setRadius(shape.m_radius);
    circ.setOrigin(circ.getRadius(), circ.getRadius());
    circ.setPosition(body->GetPosition().x, body->GetPosition().y);
    rt.draw(circ);
}
