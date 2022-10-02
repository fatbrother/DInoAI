#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>
#include "sfLine.hpp"


inline float regressFunction(float value)
{
    if (value >= 0)
        return std::pow(0.9f, -value) - 1.0f;
    else
        return 1.0f - std::pow(0.9f, -value);
}

class NeuralNetwork
{
public:
    NeuralNetwork();
    NeuralNetwork(std::vector<int>);
    void update(std::vector<std::vector<float>>, std::vector<std::vector<std::vector<float>>>);
    void show();

private:
    sf::RenderWindow window;
    std::vector<int> networkSize;
    std::vector<std::vector<sf::CircleShape>> nodes;
};

NeuralNetwork::NeuralNetwork()
{

}

NeuralNetwork::NeuralNetwork(std::vector<int> networkSize) : networkSize(networkSize)
{
    window.create(sf::VideoMode(1000, 500), "Neural Network");
    window.setPosition({0, 0});

    int n = networkSize.size();
    for (int i = 0; i < n; i++)
    {
        nodes.push_back(std::vector<sf::CircleShape>());
        for (int j = 0; j < networkSize[i]; j++)
        {
            nodes[i].push_back(sf::CircleShape(20));
            nodes[i].back().setPosition(400 / n + ((1000 / n) * i), 200 / networkSize[i] + (500 / networkSize[i]) * j);
            nodes[i].back().setFillColor(sf::Color::White);
        }
    }
}

void NeuralNetwork::update(std::vector<std::vector<float>> trexNodes, std::vector<std::vector<std::vector<float>>> trexWeights)
{
    window.clear();

    for (int layer = 0; layer + 1 < (int)nodes.size(); layer++)
        for (int i = 0; i < (int)nodes[layer].size(); i++)
            for (int j = 0; j < (int)nodes[layer + 1].size(); j++)
            {
                float weight = trexWeights[layer][i][j];
                sf::Color color = weight > 0 ? sf::Color::Green : sf::Color::Red;
                weight = weight < 0 ? -weight : weight;

                auto p1 = nodes[layer][i], p2 = nodes[layer + 1][j];

                sf::Vector2f point1(p1.getPosition().x + p1.getRadius(), p1.getPosition().y + p1.getRadius());
                sf::Vector2f point2(p2.getPosition().x + p2.getRadius(), p2.getPosition().y + p2.getRadius());
                
                window.draw(sfLine(point1, point2, color, 10 * weight));
            }

    for (int i = 0; i < (int)trexNodes.size(); i++)
        for (int j = 0; j < (int)trexNodes[i].size(); j++)
        {
            float data = 255.f * std::max(0.f, regressFunction(trexNodes[i][j]));
            nodes[i][j].setFillColor(sf::Color(data, data, data));

            window.draw(nodes[i][j]);
        }
}

void NeuralNetwork::show()
{
    window.display();
}