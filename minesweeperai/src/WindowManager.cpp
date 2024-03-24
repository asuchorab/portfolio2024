//
// Created by rutio on 2020-07-12.
//

#include "WindowManager.h"
#include "Core.h"


WindowManager& WindowManager::getInstance() {
  static WindowManager instance;
  return instance;
}

void WindowManager::updateWindow(sf::VideoMode video_mode) {
  if (video_mode.width < Core::MENU_WIDTH) {
    video_mode.width = Core::MENU_WIDTH;
  }
  if (video_mode.height < Core::MENU_HEIGHT) {
    video_mode.height = Core::MENU_HEIGHT;
  }
  if (!window.isOpen()) {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    window.create(video_mode, "Minozamiatacz",
                  sf::Style::Titlebar | sf::Style::Close, settings);
    font.loadFromFile("OpenSans-Regular.ttf");
  } else {
    window.setSize({video_mode.width, video_mode.height});
    window.setView(
        sf::View({0, 0, (float) video_mode.width, (float) video_mode.height}));
  }
}

sf::RenderWindow& WindowManager::getWindow() {
  return window;
}

sf::Font& WindowManager::getFont() {
  return font;
}

WindowManager::WindowManager() {
  //font = new sf::Font();
  //font.loadFromFile("OpenSans-Regular.ttf");
}

void WindowManager::unloadFont() {
  //font = sf::Font();
}
