//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_MENUITEM_H
#define MINESWEEPER_MENUITEM_H


#include <SFML/Graphics.hpp>

/**
 * Base abstract class for menu elements, default functions do nothing
 */
class MenuItem {
public:
  virtual void display(
      sf::RenderWindow& window, sf::Vector2i rel_mouse_pos, bool focused) = 0;

  virtual void onClick(sf::Vector2i rel_mouse_pos);

  virtual void onTextEntered(uint32_t code);

  virtual void onKeyPressed(sf::Keyboard::Key key);

  virtual ~MenuItem() = default;
};


#endif //MINESWEEPER_MENUITEM_H
