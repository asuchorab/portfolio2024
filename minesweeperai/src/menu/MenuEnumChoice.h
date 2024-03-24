//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_MENUENUMCHOICE_H
#define MINESWEEPER_MENUENUMCHOICE_H

#include "MenuItem.h"

/**
 * Menu item that controls an enum parameter
 */
class MenuEnumChoice : public MenuItem {
public:
  MenuEnumChoice(
      std::string content, std::vector<std::string> options, int* target);

  void display(
      sf::RenderWindow& window, sf::Vector2i rel_mouse_pos,
      bool focused) override;

  void onClick(sf::Vector2i rel_mouse_pos) override;

private:
  std::string content;
  std::vector<std::string> options;
  int* target;
};


#endif //MINESWEEPER_MENUENUMCHOICE_H
