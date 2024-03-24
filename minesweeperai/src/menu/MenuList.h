//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_MENULIST_H
#define MINESWEEPER_MENULIST_H


#include <vector>
#include "MenuItem.h"

/**
 * Menu view class that contains menu items
 */
class MenuList {
public:
  void display(sf::RenderWindow& window, sf::Vector2i mouse_pos);

  void addItem(std::unique_ptr<MenuItem> item);

  void resetFocus();

  void onClick(sf::Vector2i mouse_pos);

  void onTextEntered(uint32_t code);

  void onKeyPressed(sf::Keyboard::Key key);

private:
  std::vector<std::unique_ptr<MenuItem>> menu_items;
  int focused = -1;
};


#endif //MINESWEEPER_MENULIST_H
