#include "Creature.h"
#include "utility.h"

Creature::Creature() {
}

void Creature::init(b2World* world, const sf::Vector2f& pos) {
    this->world = world;

    heart_beat = random_float(0.1, 1.1);
    // Create the first node
    nodes.emplace_back();
    nodes.back().init(world, pos, random_float(0, 1));

    int node_count = random_int(2, 3);
    for(int i = 0; i < node_count; ++i) addRandomNode();
}

void Creature::addRandomNode() {
    if(nodes.empty()) return;

    // Add New node
    int parent = random_int(0, nodes.size()-1);
    nodes.emplace_back();
    Node& new_node = nodes.back();

    float dist = 6;
    sf::Vector2f offset(random_float(-dist, dist), random_float(-dist, dist));
    new_node.init(world, nodes[parent].getPosition() + offset,
                  random_float(0, 1)); // Friction

    addMuscle(&nodes[parent], &new_node);

    // Connect one more muscle to the closest node
    unsigned closest_id = -1;
    float shortest_dist = 999999999;
    for(unsigned i = 0; i < nodes.size() - 1; ++i) {
        if(i == parent) continue;

        float dist = magnitude(new_node.getPosition() - nodes[i].getPosition());
        if(dist < shortest_dist) {
            shortest_dist = dist;
            closest_id = i;
        }
    }

    if(closest_id != -1) addMuscle(&nodes[closest_id], &new_node);
}

void Creature::addMuscle(Node* a, Node* b) {
    b2DistanceJointDef jointDef;
    jointDef.bodyA = a->body;
    jointDef.bodyB = b->body;

    sf::Vector2f dist_vec = b->getPosition() - a->getPosition();
    float dist = magnitude(dist_vec);

    sf::Vector2f a2b = normalize(dist_vec)*a->shape.m_radius;
    jointDef.localAnchorA = b2Vec2( a2b.x,  a2b.y);
    jointDef.localAnchorB = b2Vec2(-a2b.x, -a2b.y);
    jointDef.collideConnected = false;
    jointDef.frequencyHz = 0;

    float ratio = random_float(0.01, 0.2);
    muscles.push_back(Muscle(world, jointDef,
                             dist*(1.0f - ratio), // Short length
                             dist*(1.0f + ratio), // Long length
                             random_float(0, 1), // Extend time
                             random_float(0, 1), // Contract time
                             random_float(3, 12))); // Strength
}

void Creature::update(float dt) {
    if((timer += dt) > heart_beat) timer = 0;
    for(unsigned i = 0; i < muscles.size(); ++i) {
        muscles[i].update(timer/heart_beat, dt);
    }
}

void Creature::render(sf::RenderTarget& rt) {
    for(unsigned i = 0; i < muscles.size(); ++i) muscles[i].render(rt);

    for(unsigned i = 0; i < nodes.size(); ++i) nodes[i].render(rt);
}




const float MIN_STRENGTH = 3.0f;
const float MAX_STRENGTH = 12.0f;

Muscle::Muscle(b2World* world, b2DistanceJointDef& def,
               float short_len_, float long_len_,
               float extend_time_, float contract_time_,
               float strength_) :
        short_len(short_len_), long_len(long_len_), extend_time(extend_time_),
        contract_time(contract_time_), strength(strength_){
    def.length = long_len;
    target_len = &short_len;
    joint = (b2DistanceJoint*) world->CreateJoint( &def );
    joint->SetLength(long_len);

    float d = (strength-MIN_STRENGTH) / (MAX_STRENGTH - MIN_STRENGTH);
    c = sf::Color(255 - (255-0)*d, 255 - (255-0)*d, 255 - (255-0)*d);
}

void Muscle::update(float c_time, float dt) {
    target_len = (contract_time < extend_time && (c_time < contract_time || extend_time < c_time)) ||
                 (extend_time < contract_time && extend_time < c_time) ?
                &short_len : &long_len;

    joint->SetLength(joint->GetLength() + strength*((*target_len) - joint->GetLength())*dt);
}

void Muscle::render(sf::RenderTarget& rt) {
    sf::RectangleShape rect;

    sf::Vector2f a(joint->GetBodyA()->GetPosition().x, joint->GetBodyA()->GetPosition().y);
    sf::Vector2f b(joint->GetBodyB()->GetPosition().x, joint->GetBodyB()->GetPosition().y);
    rect.setSize(sf::Vector2f(0.5f - 0.2f*(joint->GetLength()-short_len)/(long_len-short_len), magnitude(a-b)));
    rect.setOrigin(rect.getSize().x*0.5f, rect.getSize().y);
    rect.setPosition(a);
    rect.rotate(vecToAngle(b-a));
    rect.setFillColor(c);

    rt.draw(rect);
}







Node::Node(){
}

Node::~Node() {
}


void Node::init(b2World* world, const sf::Vector2f& pos, float friction) {
    body_def.type = b2_dynamicBody; //this will be a dynamic body
    body_def.position.Set(-10, 20); //a little to the left
    body_def.fixedRotation = true;
    body = world->CreateBody(&body_def);

    shape.m_p.Set(0, 0); //position, relative to body position
    shape.m_radius = 0.5; //radius

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

sf::Vector2f Node::getPosition() const {
    return sf::Vector2f(body->GetPosition().x, body->GetPosition().y);
}

void Node::setPosition(const sf::Vector2f& pos) {
    body->SetTransform(b2Vec2(pos.x, pos.y), body->GetAngle());
}

void Node::render(sf::RenderTarget& rt) {
    sf::CircleShape circ;
    circ.setFillColor(c);
    circ.setRadius(shape.m_radius);
    circ.setOrigin(circ.getRadius(), circ.getRadius());
    circ.setPosition(body->GetPosition().x, body->GetPosition().y);
    rt.draw(circ);
}
