//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_WINDOWMANAGER_H
#define MINESWEEPER_WINDOWMANAGER_H


#include <SFML/Graphics.hpp>

/**
 * Singleton responsible for managing the application window
 */
class WindowManager {
public:
  static WindowManager& getInstance();

  void updateWindow(sf::VideoMode video_mode);

  sf::RenderWindow& getWindow();

  sf::Font& getFont();

  void unloadFont();

private:
  WindowManager();
  WindowManager(const WindowManager& other) = delete;
  WindowManager(WindowManager&& other) noexcept = delete;
  sf::RenderWindow window;
  uint64_t padding_because_sfml_linux_bugged;
  sf::Font font;
};


#endif //MINESWEEPER_WINDOWMANAGER_H
