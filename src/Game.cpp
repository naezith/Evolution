#include "Game.h"
#include "utility.h"
#include <iostream>

Game Game::m_instance;

void Game::init(float _dt) {
    setDeltaTime(_dt);
    m_view_size = sf::Vector2f(1280, 720);
    m_font.loadFromFile("resources/Ubuntu-C.ttf");
    viewDimensions.y = 50.0f;
    viewDimensions.x = viewDimensions.y*m_view_size.x/m_view_size.y;

    view.reset(sf::FloatRect(0.0f, m_view_size.y - viewDimensions.y, viewDimensions.x, viewDimensions.y));
    view.setViewport(sf::FloatRect(0,0,1,1));
    //view.zoom(1.5);
    m_window.setView(view);

    gravity = b2Vec2(0.0f, 9.81);
    m_world = std::make_unique<b2World>(gravity);


    // Make the ground
    groundDim = sf::Vector2f(m_view_size.x, 1.0f);
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, m_view_size.y - groundDim.y*0.5f);

    groundBody = m_world->CreateBody(&groundBodyDef);

    // Ground fixture
    b2PolygonShape groundBox;
    groundBox.SetAsBox(groundDim.x*0.5f, groundDim.y*0.5f);
    groundFixture = groundBody->CreateFixture(&groundBox, 0.0f);
    groundFixture->SetFriction(1.0);

    spawnCreature();
}

void Game::drawRect(sf::VertexArray& va, b2Body* b, b2Fixture* f, const sf::Vector2f& dim, const sf::Color& c) {
    sf::Vertex v;
    v.color = c;
    b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();

    for(int i = 0; i < polygonShape->GetVertexCount(); ++i) {
        const b2Vec2& b2v = b->GetWorldPoint(polygonShape->GetVertex(i));
        v.position.x = b2v.x;
        v.position.y = b2v.y;
        va.append(v);
    }
}

void Game::restart() {
    m_first_launch = false;
}

void Game::run() {
    sf::ContextSettings window_settings;
    window_settings.antialiasingLevel = 4;
    m_window.create(sf::VideoMode(m_view_size.x, m_view_size.y), "Evolution | 13011057 | Tolga Ay", sf::Style::Default, window_settings);
    sf::Clock clock;

    while(m_window.isOpen()){
        // Check events
        sf::Event event;
        while(m_window.pollEvent(event)){
            if(event.type == sf::Event::Closed) {
                m_window.close();
            }
        }

        // Update
        m_accumulator += clock.restart().asSeconds();
        while(m_accumulator > m_dt){
            m_accumulator -= m_dt;

            update();
        }

        // Render
        render();
    }
}

void Game::spawnCreature() {
    //if(m_creatures.size() > 0) return;
    m_creatures.emplace_back();
    m_creatures.back().init(m_world.get(), sf::Vector2f(viewDimensions.x*0.5f + random_float(-5.0f, 5.0f),
                                        m_view_size.y - 50.0f + random_float(-5.0f, 5.0f)));
}

void Game::update() {
    m_global_timer += m_dt;

    if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        spawnCreature();
    }
    for(unsigned i = 0; i < m_creatures.size(); ++i) {
        m_creatures[i].update(m_dt);
    }

    m_world->Step(m_dt, 6, 2);
}

void Game::render() {
    m_window.clear(sf::Color(135, 206, 250));
    m_window.setView(view);
    sf::VertexArray va(sf::Quads);


    for(unsigned i = 0; i < m_creatures.size(); ++i) {
        m_creatures[i].render(m_window);
    }

    drawRect(va, groundBody, groundFixture, groundDim, sf::Color(0, 104, 10));

    m_window.draw(va);

    m_window.display();
}


float Game::getDeltaTime() { return m_dt; }
void Game::setDeltaTime(float _dt) { m_dt = _dt; }
float Game::getTime() { return m_global_timer; }
sf::Vector2f Game::getViewSize() { return m_view_size; }
