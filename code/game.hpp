#include "ground.hpp"
#include "ptclr.hpp"
#include "obst.hpp"
#include "trex.hpp"
#include "network.hpp"

#define TIME_SPEED 1
#define DINO_NUM 250

class Game
{
public:
    Game();
    void run();

private:
    int lastNum;
    bool isUpdate;
    std::vector<Trex> trexes;
    std::vector<int> order;
    std::list<int> bestTrexes;
    Obst obst;
    PtClr ptclr;
    Ground ground;
    NeuralNetwork *network;
    sf::View view;
    sf::Clock clock;
    sf::Sprite gameOver;
    sf::Sprite againBotton;
    sf::Texture bottomTexture;
    sf::Texture gameOverTexture;
    sf::RenderWindow *window;

    void draw();
    void render();
    void reset();
    void update(float &);
    void FPStest(sf::Clock &);
    void processEvents();
    void updateNodes();
    void recorgWeight();
};

inline Game::Game()
{
    window = new sf::RenderWindow(sf::VideoMode(1800, 450), "Dinosour");
    window->setPosition({0, 500});
    view.setCenter(300, 70);
    view.setSize(600, 200);
    window->setView(view);

    trexes.resize(DINO_NUM);

    std::vector<int> networkSize = NETWORK;
    network = new NeuralNetwork(networkSize);

    srand(time(0));
}

inline void Game::run()
{
    float frame = 1.f / 60;
    reset();
    while (window->isOpen())
    {
        // FPStest(clock);
        float time = clock.restart().asSeconds() * TIME_SPEED;

        while (time >= frame)
        {
            time -= frame;
            processEvents();
            if (isUpdate)
            {
                update(frame);
                render();
            }
            else
                reset();
        }
        processEvents();
        if (isUpdate)
        {
            update(time);
            render();
        }
        else
            reset();
    }
}

inline void Game::processEvents()
{
    sf::Event event;
    while (window->pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::EventType::Closed:
            window->close();
            break;
        case sf::Event::EventType::KeyPressed:
            switch (event.key.code)
            {
            case sf::Keyboard::Key::Escape:
                window->close();
                break;
            case sf::Keyboard::Key::Space:
            case sf::Keyboard::Key::Up:
                reset();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    for (auto &trex : trexes)
        trex.deside(ptclr.getLevel(), obst.getHitbox());
}

inline void Game::reset()
{
    if (ptclr.getLevel())
    {
        std::ofstream generate("generate.txt", std::ios::app);
        generate << trunc(ptclr.getLevel() * 100) << std::endl;
        generate.close();
    }

    isUpdate = true;
    obst.reset();
    ptclr.reset();
    ground.reset();
    clock.restart();
    for (auto &trex : trexes)
        trex.reset();
    trexes[0].reset(false);
    trexes[1].reset(false);
    for (int i = 0; i < DINO_NUM; i++)
        order.push_back(i);

    system("cls");
    std::cout << "Last Dino: " << DINO_NUM << std::endl;
}

inline void Game::update(float &time)
{
    int lastNum = order.size();
    for (int i = 0; i < (int)order.size(); i++)
        if (Entity::isTouch(trexes[order[i]].getHitbox(), obst.getHitbox()))
        {
            bestTrexes.push_back(order[i]);
            order.erase(order.begin() + i--);

            if (bestTrexes.size() > 1)
                bestTrexes.pop_front();
        }
    if ((int)order.size() != lastNum)
    {
        system("cls");
        std::cout << "Last Dino: " << order.size() << std::endl;
    }

    if (order.empty())
    {
        isUpdate = false;
        recorgWeight();
    }

    float level = ptclr.getLevel();
    for (auto &i : order)
        trexes[i].update(time);
    obst.update(time, level);
    ground.update(time, level);
    ptclr.update(time);
    network->update(trexes[order[0]].getNodes(), trexes[order[1]].getWeight());
}

inline void Game::draw()
{
    ground.draw(window);
    for (auto &i : order)
        trexes[i].draw(window);
    obst.draw(window);
    ptclr.draw(window);
    network->show();
}

inline void Game::FPStest(sf::Clock &clock)
{
    static float pastTime = 0;
    static int cnt = 0;

    pastTime += clock.getElapsedTime().asSeconds(), cnt++;
    if (pastTime > 1.f)
    {
        std::cout << cnt << std::endl;
        pastTime = 0;
        cnt = 0;
    }
}

inline void Game::render()
{
    window->clear(sf::Color::White);
    draw();
    window->display();
}

inline void Game::recorgWeight()
{
    auto weight1 = trexes[bestTrexes.front()].getWeight(), weight2 = trexes[bestTrexes.back()].getWeight();

    for (int i = 0; i < (int)weight1.size(); i++)
        for (int j = 0; j < (int)weight1[i].size(); j++)
            for (int k = 0; k < (int)weight1[i][j].size(); k++)
                if (rand() % 2 == 0)
                    weight1[i][j][k] = weight2[i][j][k];

    std::ofstream weightFile("weight.txt", std::ios::out);
    for (auto &i : weight1)
        for (auto &j : i)
            for (auto &k : j)
                weightFile << k << std::endl;
    weightFile.close();
}