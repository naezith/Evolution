#include "Game.h"
#include "utility.h"
#include <iostream>

Game Game::m_instance;
const unsigned POPULATION = 1000;
void Game::init(float _dt) {
    setDeltaTime(_dt);
    m_view_size = sf::Vector2f(1280, 720);
    m_font.loadFromFile("resources/Ubuntu-C.ttf");
    viewDimensions.y = 15.0f;
    viewDimensions.x = viewDimensions.y*m_view_size.x/m_view_size.y;

    view.reset(sf::FloatRect(-viewDimensions.x*0.5f, m_view_size.y - viewDimensions.y, viewDimensions.x, viewDimensions.y));
    view.setViewport(sf::FloatRect(0,0,1,1));

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

    for(int i = 0; i < POPULATION; ++i) spawnCreature();
    m_creatures[curr_creature_id = 0].setActive(true);
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
        double elapsed = clock.restart().asSeconds();
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::H)) update(); // Just simulate as fast as possible
        else {
            // Update
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::K)) elapsed *= 4.0; // Simulate 4x
            m_accumulator += elapsed;

            // Check events
            sf::Event event;
            while(m_window.pollEvent(event)){
                if(event.type == sf::Event::Closed) {
                    m_window.close();
                }
            }
            while(m_accumulator > m_dt){
                m_accumulator -= m_dt;

                update();
            }

            // Render
            render();
        }
    }
}

void Game::spawnCreature() {
    m_creatures.emplace_back();
    m_creatures.back().init(m_world.get(), sf::Vector2f(0, m_view_size.y - viewDimensions.y*0.3));
}

void Game::update() {
    m_global_timer += m_dt;

    m_creatures[curr_creature_id].update(m_dt);

    m_world->Step(m_dt, 6, 2);

    best_x = m_creatures[curr_creature_id].fitness;
    if(best_x > overall_best_x) {
        overall_best_id = curr_creature_id;
        overall_best_x = best_x;
    }
    view.reset(sf::FloatRect(best_x - viewDimensions.x*0.5f, m_view_size.y - viewDimensions.y, viewDimensions.x, viewDimensions.y));
    m_window.setView(view);

    // Create new creature after a while
    if(m_global_timer >= 15.0f) {
        m_creatures[curr_creature_id].setActive(false);

        m_global_timer = 0.0f;

        // All creatures did the test,
        if(curr_creature_id >= POPULATION) {
            // Create a new generation

            // Reset stats
            curr_creature_id = overall_best_x = overall_best_id = 0;
            gen = 1;
        }
        // Continue testing creatures
        else ++curr_creature_id;

        m_creatures[curr_creature_id].setActive(true);
    }
}

void Game::render() {
    m_window.clear(sf::Color(135, 206, 250));
    m_window.setView(view);
    sf::VertexArray va(sf::Quads);

    // Draw background lines
    sf::Text text;
    text.setFont(m_font);
    text.setCharacterSize(12);
    text.setColor(sf::Color::Black);
    float scale = viewDimensions.y/m_view_size.y;
    text.setScale(scale, scale);

    sf::RectangleShape text_back(sf::Vector2f(0.7, 0.3));
    text_back.setOrigin(text_back.getSize()*0.5f);

    sf::RectangleShape line(sf::Vector2f(0.025, 4.0));
    line.setOrigin(line.getSize().x*0.5f, line.getSize().y);
    for(int i = -30; i <= 30; ++i) {
        line.setPosition(i, groundBody->GetPosition().y);
        m_window.draw(line);

        text.setString(std::to_string(i) + " m");
        text.setOrigin(text.getGlobalBounds().width*0.5f, text.getGlobalBounds().height*0.5f);
        text.setPosition(i - text.getCharacterSize()*0.01f, line.getPosition().y - line.getSize().y);

        text_back.setPosition(i, text.getGlobalBounds().top + text.getGlobalBounds().height*0.5f);
        m_window.draw(text_back);
        m_window.draw(text);
    }

    // Show the best
    text.setString(setPrecision(best_x, 2) + " m");
    text.setOrigin(text.getGlobalBounds().width*0.5f, text.getGlobalBounds().height*0.5f);
    text.setPosition(best_x - text.getCharacterSize()*0.01f, text.getPosition().y - 1);
    text_back.setPosition(best_x, text.getGlobalBounds().top + text.getGlobalBounds().height*0.5f);
    text_back.setSize(sf::Vector2f(text_back.getSize().x*2.0, text_back.getSize().y));
    text_back.setOrigin(text_back.getSize()*0.5f);
    m_window.draw(text_back);
    m_window.draw(text);

    // Show the timer
    text.setString("Time: " + setPrecision(m_global_timer, 2) + " sec");
    text.setOrigin(0, 0);
    text.setPosition(best_x, text.getPosition().y - 1);
    text_back.setPosition(best_x, text.getGlobalBounds().top + text.getGlobalBounds().height*0.5f);
    text_back.setSize(sf::Vector2f(text_back.getSize().x*1.4, text_back.getSize().y));
    text_back.setOrigin(0.2, text_back.getSize().y*0.5f);
    m_window.draw(text_back);
    m_window.draw(text);

    // Show the overall best
    text.setString("Current Creature: #" + std::to_string(curr_creature_id + 1) +
                   "           Best creature: #" + std::to_string(overall_best_id + 1) +
                   "       " + setPrecision(overall_best_x, 2) + " m");
    text.setOrigin(text.getGlobalBounds().width*0.5f, text.getGlobalBounds().height*0.5f);
    text.setPosition(best_x - text.getCharacterSize()*0.01f - 1, text.getPosition().y - 1);
    text_back.setPosition(best_x - 1, text.getGlobalBounds().top + text.getGlobalBounds().height*0.5f);
    text_back.setSize(sf::Vector2f(text_back.getSize().x*3.0, text_back.getSize().y));
    text_back.setOrigin(0.2, text_back.getSize().y*0.5f);
    m_window.draw(text_back);
    m_window.draw(text);


    // Draw Creatures
    m_creatures[curr_creature_id].render(m_window);


    // Draw ground
    drawRect(va, groundBody, groundFixture, groundDim, sf::Color(0, 104, 10));

    m_window.draw(va);

    m_window.display();
}


float Game::getDeltaTime() { return m_dt; }
void Game::setDeltaTime(float _dt) { m_dt = _dt; }
float Game::getTime() { return m_global_timer; }
sf::Vector2f Game::getViewSize() { return m_view_size; }
