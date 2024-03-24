//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_OBVIOUSMOVESSOLVER_H
#define MINESWEEPER_OBVIOUSMOVESSOLVER_H

#include "Solver.h"

/**
 * Most basic minesweeper AI
 * Will only click on the obviously safe fields and will not always win
 * the easiest mode and usually do little in expert mode
 */
class ObviousMovesSolver : public Solver {
public:
  bool step(GameInstance& game) override;

private:
  // Data of each cell, only holds whether it was visited in this pass
  struct Data {
    bool visited = false;
  };

  // next positions to visit
  std::vector<sf::Vector2i> stack;

  // Additional data of each cell, indexed by y * width + x
  std::vector<Data> data;

  // If something changed that requires looking at things again besides stack
  bool should_retry = false;
};


#endif //MINESWEEPER_OBVIOUSMOVESSOLVER_H
