#pragma once

#include "Maths.h"
#include <SFML/Window/Window.hpp>

class Keyboard;
class World;

class Player
{
  public:
    Player();

    void input(const Keyboard& keyboard, const sf::Window& window);

    void update(const sf::Time& dt, const World& world);

    const Transform& getTransform() const;

  private:
    Transform m_transform;
    glm::vec3 m_velocity;
};
