//
// Created by rutio on 2020-07-13.
//

#ifndef MINESWEEPER_CELLGROUPSSOLVER_H
#define MINESWEEPER_CELLGROUPSSOLVER_H

#include "Solver.h"
#include "CellGroups.h"

/**
 * An advanced minesweeper AI that can flexibly group fields where a certain
 * amount of mines can be. Compared to DumbCellGroupsSolver, it handles any
 * mine group shape and also can make decisions based on any combination of
 * neighbouring groups that do not fully fit in cell's neighbourhood, taking
 * possible minimum and maximum number of neighbouring mines.
 * This AI should win expert sometimes and when it doesn't it's often
 * because the game requires you to make a guess.
 */
class CellGroupsSolver : public Solver {
public:
  bool step(GameInstance& game) override;

private:
  // Data of each cell, only holds whether it was visited in this pass
  // and groups, a vector of groups on that cell
  struct Data {
    bool visited = false;
    std::vector<CellGroup> groups;
  };

  // Move type for struct QueuedMove
  enum MoveType {
    UNCOVER, FLAG, QUESTION
  };

  // As the AI can decide many moves in one go, they are queued so the step
  // actually does one thing at once
  struct QueuedMove {
    QueuedMove(
        int x, int y, CellGroupsSolver::MoveType type, bool continueGroup);

    int x;
    int y;
    MoveType type;
    // if this move is part of a group and is not last
    bool continue_group;
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


#endif //MINESWEEPER_CELLGROUPSSOLVER_H
