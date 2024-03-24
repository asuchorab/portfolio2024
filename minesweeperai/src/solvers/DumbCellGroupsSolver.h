//
// Created by rutio on 2020-07-13.
//

#ifndef MINESWEEPER_DUMBCELLGROUPSSOLVER_H
#define MINESWEEPER_DUMBCELLGROUPSSOLVER_H

#include "Solver.h"

/**
 * A bit more advanced AI than "obvious moves", It can group cells into groups
 * if it finds that two or three cells in a line and use it for later reasoning.
 * The limited grouping capabilities make it not great still.
 */
class DumbCellGroupsSolver : public Solver {
public:
  bool step(GameInstance& game) override;

private:
  // Data of each cell, holds whether it was visited in this pass
  // and groups, a bit field using enum Group
  struct Data {
    bool visited = false;
    uint8_t groups = 0;
  };

  // Move type for struct QueuedMove
  enum MoveType {
    UNCOVER, FLAG, QUESTION
  };

  // As the AI can decide many moves in one go, they are queued so the step
  // actually does one thing at once
  struct QueuedMove {
    QueuedMove(
        int x, int y, DumbCellGroupsSolver::MoveType type, bool continueGroup);

    int x;
    int y;
    MoveType type;
    // if this move is part of a group and is not last
    bool continue_group;
  };

  // Type of group that exists on this field, first number is number of mines,
  // second number is number of fields, H or V is horizontal or vertical
  enum Group {
    H_1_2 = 1 << 0,
    H_1_3 = 1 << 1,
    H_2_3 = 1 << 2,
    V_1_2 = 1 << 3,
    V_1_3 = 1 << 4,
    V_2_3 = 1 << 5
  };

  // Simplified access to data vector
  Data& getData(int x, int y);

  // Execute queued move if available
  bool executeQueuedMoves(GameInstance& game);

  // next positions to visit
  std::vector<sf::Vector2i> stack;

  // Additional data of each cell, indexed by y * width + x
  std::vector<Data> data;

  // Moves already decided to be made later
  std::vector<QueuedMove> queued_moves;

  // Current position in queue, in each executeQueuedMoves call it advances
  // through the moves, possibly multiple in group in one pass, didn't use
  // actual queue structure because i know queue will always be emptied after
  // a new batch of numbers is inserted so it's not a true queue
  int queue_pos = 0;

  // Minefield width
  int width;

  // If something changed that requires looking at things again besides stack
  bool should_retry = false;
};


#endif //MINESWEEPER_DUMBCELLGROUPSSOLVER_H
