//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_MENUINTINPUT_H
#define MINESWEEPER_MENUINTINPUT_H

#include "MenuItem.h"

/**
 * Menu item that controls an int parameter
 */
class MenuIntInput : public MenuItem {
public:
  MenuIntInput(std::string text, int* target);

  void display(sf::RenderWindow& window, sf::Vector2i rel_mouse_pos, bool focused) override;

  void onClick(sf::Vector2i rel_mouse_pos) override;

  void onTextEntered(uint32_t code) override;

  void onKeyPressed(sf::Keyboard::Key key) override;

private:
  std::string content;
  int* target;
};


#endif //MINESWEEPER_MENUINTINPUT_H
