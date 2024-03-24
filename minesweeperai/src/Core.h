//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_CORE_H
#define MINESWEEPER_CORE_H


#include "MinesweeperConfig.h"
#include "GameInstance.h"
#include "solvers/Solver.h"

/**
 * Main object that has the main game loop, manages game instances
 * and AIs
 */
class Core {
public:
  enum AIMode {
    STEP, FAST_FORWARD, INSTANT
  };
  enum AIType {
    NONE, OBVIOUS_MOVES, DUMB_CELL_GROUPS, CELL_GROUPS
  };
  enum AIStartRandom {
    NEVER, NEW_SEED, ALWAYS
  };

  Core();

  void run();

  static const int CELL_WIDTH = 32;
  static const int MENU_WIDTH = CELL_WIDTH * 9;
  static const int MENU_HEIGHT = CELL_WIDTH * 10;
  static const sf::Color BACKGROUND_COLOR;
  static const sf::Color UNCOVERED_COLOR;
  static const sf::Color COVERED_COLOR;
  static const sf::Color SELECTED_COLOR;
private:
  void changeGameInstance(MinesweeperConfig new_config);
  void changeGameInstanceNewSeed(MinesweeperConfig new_config);
  void changeMenu(MenuList* menu);
  void menuBack();
  void updateAI();

  std::mt19937 random;
  MinesweeperConfig config;
  MinesweeperConfig config_being_changed;
  GameInstance game;
  int seed;
  bool running = true;

  AIMode ai_mode = STEP;
  AIType ai_type = NONE;
  AIStartRandom ai_start_random = NEW_SEED;
  std::unique_ptr<Solver> solver;
  bool fast_forwarding_ai = false;
  int ai_start_x = 0;
  int ai_start_y = 0;

  sf::FloatRect button;

  std::string menu_header;
  MenuList* current_menu = nullptr;
  std::vector<MenuList*> previous_menu;
  MenuList menu_main;
  MenuList menu_game;
  MenuList menu_ai;
  MenuList menu_ai_settings;
};


#endif //MINESWEEPER_CORE_H
