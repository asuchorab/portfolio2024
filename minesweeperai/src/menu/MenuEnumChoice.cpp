//
// Created by rutio on 2020-07-12.
//

#include "MenuEnumChoice.h"
#include "../Core.h"
#include "../WindowManager.h"

MenuEnumChoice::MenuEnumChoice(
    std::string content, std::vector<std::string> options, int* target)
    : content(std::move(content)),
      options(std::move(options)),
      target(target) {
}

void MenuEnumChoice::display(
    sf::RenderWindow& window, sf::Vector2i rel_mouse_pos, bool focused) {
  sf::RectangleShape rect;
  rect.setSize(
      {Core::MENU_WIDTH - Core::CELL_WIDTH * 0.2f, Core::CELL_WIDTH * 0.8f});
  rect.setPosition({Core::CELL_WIDTH * 0.1f, Core::CELL_WIDTH * 0.1f});
  if (rel_mouse_pos.x >= 0 && rel_mouse_pos.y >= 0 &&
      rel_mouse_pos.x < Core::MENU_WIDTH && rel_mouse_pos.y < Core::CELL_WIDTH) {
    rect.setFillColor(Core::COVERED_COLOR);
  } else {
    rect.setFillColor(Core::UNCOVERED_COLOR);
  }
  window.draw(rect);
  auto& wm = WindowManager::getInstance();
  sf::Text text;
  text.setFont(wm.getFont());
  text.setCharacterSize(Core::CELL_WIDTH * 3 / 4);
  text.setFillColor({0, 0, 0});
  text.setString(content + options[*target]);
  auto text_rect = text.getLocalBounds();
  text.setOrigin({text_rect.width / 2.f, 0});
  text.setPosition(Core::MENU_WIDTH / 2.f, 0);
  window.draw(text);
}

void MenuEnumChoice::onClick(sf::Vector2i rel_mouse_pos) {
  (*target)++;
  if (*target >= (int) options.size()) {
    *target = 0;
  }
}
