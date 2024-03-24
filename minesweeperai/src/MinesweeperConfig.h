//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_MINESWEEPERCONFIG_H
#define MINESWEEPER_MINESWEEPERCONFIG_H

/**
 * Data structure of minesweeper game parameters
 */
struct MinesweeperConfig {
  MinesweeperConfig(int width, int height, int mines)
      : width(width), height(height), mines(mines) {}
  int width;
  int height;
  int mines;
};


#endif //MINESWEEPER_MINESWEEPERCONFIG_H
