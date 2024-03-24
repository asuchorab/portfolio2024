//
// Created by rutio on 2020-07-12.
//

#include "MenuIntInput.h"
#include "../Core.h"
#include "../WindowManager.h"

MenuIntInput::MenuIntInput(std::string content, int* target)
    : content(std::move(content)),
      target(target) {
}

void MenuIntInput::display(
    sf::RenderWindow& window, sf::Vector2i rel_mouse_pos, bool focused) {
  sf::RectangleShape rect;
  rect.setSize(
      {Core::MENU_WIDTH * 0.5f - Core::CELL_WIDTH * 0.2f, Core::CELL_WIDTH * 0.8f});
  rect.setPosition({Core::MENU_WIDTH * 0.5f + Core::CELL_WIDTH * 0.1f, Core::CELL_WIDTH * 0.1f});
  if (focused) {
    rect.setFillColor(Core::SELECTED_COLOR);
  } else if (rel_mouse_pos.x >= 0 && rel_mouse_pos.y >= 0 &&
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
  text.setString(content);
  text.setOrigin({-Core::CELL_WIDTH / 8.f, 0});
  text.setPosition(0, 0);
  window.draw(text);
  text.setString(std::to_string(*target));
  auto text_rect = text.getLocalBounds();
  text.setOrigin(text_rect.width / 2.f, 0);
  text.setPosition(Core::MENU_WIDTH * 0.75f, 0);
  window.draw(text);
}

void MenuIntInput::onClick(sf::Vector2i rel_mouse_pos) {
  MenuItem::onClick(rel_mouse_pos);
}

void MenuIntInput::onTextEntered(uint32_t code) {
  int old_value = *target;
  if (code >= '0' && code <= '9') {
    *target *= 10;
    *target += code - '0';
  } else if (code == '-') {
    *target = -*target;
  }
}

void MenuIntInput::onKeyPressed(sf::Keyboard::Key key) {
  if (key == sf::Keyboard::BackSpace) {
    *target /= 10;
  }
}
