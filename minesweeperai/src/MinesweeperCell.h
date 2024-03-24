//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_MINESWEEPERCELL_H
#define MINESWEEPER_MINESWEEPERCELL_H

/**
 * Data structure for a minesweeper cell
 */
struct MinesweeperCell {
  bool has_mine = false;
  bool uncovered = false;
  bool flagged = false;
  bool question = false;
  int num_adjacent = 0;
};


#endif //MINESWEEPER_MINESWEEPERCELL_H
