//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_GAMEINSTANCE_H
#define MINESWEEPER_GAMEINSTANCE_H


#include <SFML/Graphics.hpp>
#include <random>
#include "MinesweeperConfig.h"
#include "MinesweeperCell.h"
#include "menu/MenuList.h"

/**
 * Represents an instance of a minesweeper game
 * Capable of all minesweeper logic, generating and rendering
 */
class GameInstance {
public:
  GameInstance(MinesweeperConfig config, int seed);

  sf::VideoMode getVideoMode() const;

  bool generate(int initial_x, int initial_y);

  void render(sf::RenderWindow& window);

  void uncover(int x, int y);

  void rightClick(int x, int y);

  void flag(int x, int y);

  void question(int x, int y);

  void advanceTime(double dt);

  void requestRedraw();

  int getRemainingMines() const;

  int getTimer() const;

  int getUncoveredFields() const;

  int getWidth() const;

  int getHeight() const;

  bool isVictory() const;

  bool isGameOver() const;

  MinesweeperCell& getCell(int x, int y);

private:
  int seed;
  MinesweeperConfig config;
  int remaining_mines;
  int uncovered_fields = 0;
  int num_flags = 0;
  // Indexed by y * width + x
  std::vector<MinesweeperCell> field;
  double time_accumulator = 0;
  int timer = 0;
  bool game_over = false;
  bool victory = false;
  bool generated = false;
  bool redraw = true;
};


#endif //MINESWEEPER_GAMEINSTANCE_H
