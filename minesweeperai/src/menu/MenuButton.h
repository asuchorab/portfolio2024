//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_MENUBUTTON_H
#define MINESWEEPER_MENUBUTTON_H

#include <functional>
#include "MenuItem.h"

/**
 * Menu item that does something on click
 */
class MenuButton : public MenuItem {
public:
  MenuButton(std::string content, std::function<void()> func);

  void display(
      sf::RenderWindow& window, sf::Vector2i rel_mouse_pos,
      bool focused) override;

  void onClick(sf::Vector2i rel_mouse_pos) override;

private:
  std::string content;
  std::function<void()> func;
};


#endif //MINESWEEPER_MENUBUTTON_H
