#include <SFML/Graphics.hpp>
#include "entity.hpp"

#define DIE -1
#define RUN 0
#define JUMP 1
#define SHIFT 2
#define NETWORK {6, 6, 6, 3}

class Trex : public Entity
{
public:
    Trex();
    void deside(float, sf::FloatRect);
    void update(float &);
    void draw(sf::RenderWindow *);
    void reset(bool);
    std::vector<std::vector<std::vector<float>>> getWeight();
    std::vector<std::vector<float>> getNodes();

private:
    int type;     // control run or jump or shift
    int nextType; // store next type for ready
    bool state;   // control runing skin
    float gravity;
    float pastTime;
    float velocityDown;
    sf::Sprite object;
    sf::Texture jump;
    sf::Texture dead;
    std::vector<sf::Texture> run;
    std::vector<sf::Texture> shift;
    std::vector<int> networkSizes;
    std::vector<std::vector<float>> nodes;
    std::vector<std::vector<std::vector<float>>> weights;

    void setType();
    void updateRun(float);
    void updateJump(float);
    void updateShift(float);
    int computeDeside();
};

inline Trex::Trex()
{
    jump.loadFromFile("material.png", sf::IntRect(848, 2, 44, 45));
    dead.loadFromFile("material.png", sf::IntRect(1025, 2, 44, 45));
    run.resize(2);
    for (int i = 0; i < 2; i++)
        run[i].loadFromFile("material.png", sf::IntRect(936 + 44 * i, 2, 43, 45));
    shift.resize(2);
    for (int i = 0; i < 2; i++)
        shift[i].loadFromFile("material.png", sf::IntRect(1112 + 59 * i, 19, 59, 28));
    object.setPosition(0, 105);

    state = 0;
    pastTime = 0;
    type = RUN;
    nextType = RUN;
    setHitbox(object);

    networkSizes = NETWORK;

    nodes.resize(networkSizes.size());
    for (int i = 0; i < (int)nodes.size(); i++)
        nodes[i].resize(networkSizes[i]);

    weights.resize(networkSizes.size() - 1);
    for (int i = 0; i < (int)weights.size(); i++)
    {
        weights[i].resize(networkSizes[i]);
        for (auto &j : weights[i])
            j.resize(networkSizes[i + 1]);
    }
}

inline void Trex::update(float &time)
{
    if (nextType == SHIFT && type == JUMP)
        gravity = 13000;

    if (type != nextType && type != JUMP)
        setType();

    switch (type)
    {
    case SHIFT:
        updateShift(time);
        break;
    case RUN:
        updateRun(time);
        break;
    case JUMP:
        updateJump(time);
        break;
    }
}

inline void Trex::draw(sf::RenderWindow *window)
{
    window->draw(object);
    // drawHitbox(window);
}

inline void Trex::reset(bool isRand = true)
{
    state = 0;
    pastTime = 0;
    nextType = RUN;
    setType();

    std::ifstream weightFile("weight.txt");

    for (auto &i : weights)
        for (auto &j : i)
            for (auto &k : j)
                weightFile >> k;

    if (isRand)
    {
        for (int time = 0; time < 3; time++)
        {
            int layer = rand() % weights.size();
            int i = rand() % weights[layer].size();
            int j = rand() % weights[layer][i].size();
            int neg = rand() % 2 == 0 ? -1 : 1;

            weights[layer][i][j] = neg * (double)rand() / (RAND_MAX + 1.0);
        }
    }

    weightFile.close();
}

inline void Trex::deside(float level, sf::FloatRect obst)
{
    nodes[0][0] = (obst.left - hitbox.left);                                     // dx
    nodes[0][1] = (obst.top - hitbox.top);                                       // dy
    nodes[0][2] = obst.width;                                                    // width
    nodes[0][3] = obst.height;                                                   // height
    nodes[0][4] = 150 - object.getTextureRect().height - object.getPosition().y; // positionY
    nodes[0][5] = level;                                                         // speed

    nextType = computeDeside();
}

inline int Trex::computeDeside()
{
    for (int layer = 1; layer < (int)nodes.size(); layer++)
        for (int node = 0; node < (int)nodes[layer].size(); node++)
        {
            float nowNode = 0;
            for (int i = 0; i < (int)nodes[layer - 1].size(); i++)
                nowNode += nodes[layer - 1][i] * weights[layer - 1][i][node];

            nodes[layer][node] = nowNode;
        }

    // nodes[final][0] RUN, nodes[final][1] JUMP, nodes[final][2] SHIFT

    int final = nodes.size() - 1;
    if (nodes[final][0] >= nodes[final][1] && nodes[final][0] >= nodes[final][2])
        return RUN;
    else if (nodes[final][1] >= nodes[final][2])
        return JUMP;
    else
        return SHIFT;
}

inline void Trex::setType()
{
    type = nextType;
    switch (type)
    {
    case RUN:
        gravity = 0;
        velocityDown = 0;
        object.setTexture(run[state], true);
        object.setPosition(0, 105);
        setHitbox(object);
        break;
    case JUMP:
        velocityDown = -500;
        gravity = 1300;
        object.setTexture(jump, true);
        setHitbox(object);
        break;
    case SHIFT:
        object.setTexture(shift[state], true);
        object.setPosition(0, 122);
        setHitbox(object);
        break;
    }
}

inline void Trex::updateShift(float time)
{
    pastTime += time;
    if (pastTime >= 1.f / 6)
    {
        pastTime -= 1.f / 6;
        state = !state;
        object.setTexture(shift[state]);
    }
}

inline void Trex::updateJump(float time)
{
    velocityDown += time * gravity;
    object.move(0, velocityDown * time);
    moveHitbox(0, velocityDown * time);
    if (object.getPosition().y >= 105 && velocityDown >= 0)
        setType();
}

inline void Trex::updateRun(float time)
{
    pastTime += time;
    if (pastTime >= 1.f / 6)
    {
        pastTime -= 1.f / 6;
        state = !state;
        object.setTexture(run[state]);
    }
}

inline std::vector<std::vector<std::vector<float>>> Trex::getWeight()
{
    return weights;
}

inline std::vector<std::vector<float>> Trex::getNodes()
{
    return nodes;
}