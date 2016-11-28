#include "Game.h"
#include "utility.h"
#include <iostream>

Game Game::m_instance;
const unsigned POPULATION = 20;
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
    m_creatures[curr_creature_id = 0]->setActive(true);
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

        // Toggle fast simulation
        bool j_now = sf::Keyboard::isKeyPressed(sf::Keyboard::J);
        if(j_now && !j_last_frame) simulate_quick = !simulate_quick;
        j_last_frame = j_now;

        // Focus camera on the best one
        bool cam_key_now = sf::Keyboard::isKeyPressed(sf::Keyboard::H);
        if(cam_key_now && !cam_key_last_frame) show_best = !show_best;
        cam_key_last_frame = cam_key_now;

        if(simulate_quick) update(); // Just simulate as fast as possible
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
sf::Vector2f Game::getSpawnPos() { return sf::Vector2f(0, m_view_size.y - viewDimensions.y*0.2); }
void Game::spawnCreature() {
    m_creatures.push_back(std::make_unique<Creature>());
    m_creatures.back()->init(m_world.get(), getSpawnPos());
}

static inline float getFitness(float k) {
    return k*k*k;
}

int randomSelection(std::vector<std::unique_ptr<Creature>>& p) {
    float sum_fitness = 0;
    for(unsigned i = 0; i < p.size(); ++i) sum_fitness += getFitness(p[i]->fitness);

    float val = std::rand() % std::max(1, (int)sum_fitness);
    for(unsigned i = 0; i < p.size(); ++i) {
        val -= getFitness(p[i]->fitness);
        if(val <= 0) return i;
    }

    return p.size() - 1;
}

void Game::update() {
    m_global_timer += m_dt;

    if(best_creature != nullptr) best_creature->update(m_dt);
    m_creatures[curr_creature_id]->update(m_dt);
    m_world->Step(m_dt, 1, 1);

    // Create new creature after a while
    if(m_global_timer >= 15.0f) {
        m_global_timer = 0.0f;

        m_creatures[curr_creature_id]->setActive(false);

        sf::Vector2f spawn_pos = getSpawnPos();
        // Set the best creature if it's better
        if(best_creature == nullptr || best_creature->fitness < m_creatures[curr_creature_id]->fitness){
            best_c_gen = gen;
            best_c_id = curr_creature_id;
            best_creature = m_creatures[curr_creature_id]->copy();
            best_creature->setActive(true);
        }
        best_creature->setPosition(spawn_pos);

        // All creatures did the test,
        if(curr_creature_id >= POPULATION-1) {
            // Create a new generation
            std::sort(m_creatures.begin(), m_creatures.end(), [](const std::unique_ptr<Creature>& c1,
                                                                 const std::unique_ptr<Creature>& c2)
                                                                { return c1->fitness > c2->fitness; });
            std::cout << "> G#" << gen << std::endl;

            std::vector<std::unique_ptr<Creature>> new_creatures;

            int print_count = std::min(2, (int)m_creatures.size());
            for(int i = 0; i < print_count; ++i) {
                std::cout << m_creatures[i]->fitness << std::endl;
            }

            for(int i = 0; i < m_creatures.size(); ++i) {
                int chosen = randomSelection(m_creatures);
                new_creatures.push_back(m_creatures[chosen]->mutatedCopy());
                new_creatures.back()->setPosition(spawn_pos);
            }

            // Make the new population, current population
            for(unsigned i = 0; i < m_creatures.size(); ++i)
                m_creatures[i] = std::move(new_creatures[i]);


            // Reset stats
            curr_creature_id = 0;

            ++gen;
        }
        // Continue testing creatures
        else ++curr_creature_id;
        m_creatures[curr_creature_id]->setActive(true);
    }
}

void Game::render() {
    m_window.clear(sf::Color(135, 206, 250));

    float curr_x = m_creatures[curr_creature_id]->fitness;
    float focus_x = show_best && best_creature != nullptr ? best_creature->getPosition().x : curr_x;
    best_x = focus_x;


    view.reset(sf::FloatRect(focus_x - viewDimensions.x*0.5f, m_view_size.y - viewDimensions.y, viewDimensions.x, viewDimensions.y));
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
    for(int i = 0; i <= 300; ++i) {
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
    text.setString("Current Gen #" + std::to_string(gen) +
                   "  ID #" + std::to_string(curr_creature_id + 1) +
                   (best_creature != nullptr ?
                    ("                   Best Creature:  Gen #" + std::to_string(best_c_gen) +
                                                "  ID #" + std::to_string(best_c_id)  +
                                                "    Dist: " +  setPrecision(best_creature->fitness, 2) + " m")
                    : ""));
    text.setOrigin(text.getGlobalBounds().width*0.5f, text.getGlobalBounds().height*0.5f);
    text.setPosition(best_x - text.getCharacterSize()*0.01f - 1, text.getPosition().y - 1);
    text_back.setPosition(best_x - 1, text.getGlobalBounds().top + text.getGlobalBounds().height*0.5f);
    text_back.setSize(sf::Vector2f(text_back.getSize().x*6.0, text_back.getSize().y));
    text_back.setOrigin(0.2, text_back.getSize().y*0.5f);
    m_window.draw(text_back);
    m_window.draw(text);


    // Draw Creatures
    if(best_creature != nullptr) best_creature->render(m_window);
    m_creatures[curr_creature_id]->render(m_window);


    // Draw ground
    drawRect(va, groundBody, groundFixture, groundDim, sf::Color(0, 104, 10));

    m_window.draw(va);

    m_window.display();
}


float Game::getDeltaTime() { return m_dt; }
void Game::setDeltaTime(float _dt) { m_dt = _dt; }
float Game::getTime() { return m_global_timer; }
sf::Vector2f Game::getViewSize() { return m_view_size; }
