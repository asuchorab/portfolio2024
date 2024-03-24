//
// Created by rutio on 2020-07-12.
//

#include "MenuList.h"
#include "../Core.h"

void MenuList::display(sf::RenderWindow& window, sf::Vector2i mouse_pos) {
  auto view = window.getView();
  auto view_backup = view;
  for (int i = 0; i < menu_items.size(); ++i) {
    auto& item = menu_items[i];
    mouse_pos.y -= Core::CELL_WIDTH;
    view.move({0, -Core::CELL_WIDTH});
    window.setView(view);
    item->display(window, mouse_pos, focused == i);
  }
  window.setView(view_backup);
}

void MenuList::resetFocus() {
  focused = -1;
}

void MenuList::onClick(sf::Vector2i mouse_pos) {
  int index = (mouse_pos.y - Core::CELL_WIDTH) / Core::CELL_WIDTH;
  if (index >= 0 && index < menu_items.size()) {
    if (mouse_pos.y <= Core::MENU_HEIGHT) {
      menu_items[index]->onClick(
          {mouse_pos.x, mouse_pos.y - (index + 1) * Core::CELL_WIDTH});
    }
    focused = index;
  }
}

void MenuList::onTextEntered(uint32_t code) {
  if (focused >= 0 && focused < menu_items.size()) {
    menu_items[focused]->onTextEntered(code);
  }
}

void MenuList::onKeyPressed(sf::Keyboard::Key key) {
  if (focused >= 0 && focused < menu_items.size()) {
    menu_items[focused]->onKeyPressed(key);
  }
}

void MenuList::addItem(std::unique_ptr<MenuItem> item) {
  menu_items.emplace_back(std::move(item));
}
