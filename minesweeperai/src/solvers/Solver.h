//
// Created by rutio on 2020-07-12.
//

#ifndef MINESWEEPER_SOLVER_H
#define MINESWEEPER_SOLVER_H

#include "../GameInstance.h"
#include <random>
#include <functional>

/**
 * Abstract class for minesweeper game solvers,
 * contains some utility functions
 */
class Solver {
public:
  Solver();

  virtual ~Solver() = default;

  /**
   * Makes one move in the game
   * @return whether a move was made
   */
  virtual bool step(GameInstance& game) = 0;

  /**
   * Tells the ai what should be the first move
   */
  void setStart(int new_start_x, int new_start_y);

  /**
   * Counts amount of cells around (x, y) that meet criteria set by predicate
   * func with cell argument
   */
  int countAround(
      GameInstance& game, int x, int y,
      const std::function<bool(MinesweeperCell&)>& func);

  /**
   * Counts amount of cells around (x, y) that meet criteria set by predicate
   * func with game and coordinates argument
   */
  int countAround(
      GameInstance& game, int x, int y,
      const std::function<bool(GameInstance&, int, int)>& func);

  /**
   * Performs a function on cells around (x, y) with cell argument
   */
  void executeAround(
      GameInstance& game, int x, int y,
      const std::function<void(MinesweeperCell&)>& func);

  /**
   * Performs a function on cells around (x, y) with game and position argument
   */
  void executeAround(
      GameInstance& game, int x, int y,
      const std::function<void(GameInstance&, int, int)>& func);

  /**
   * Regurns given ai starting point
   */
  sf::Vector2i getStartingPoint(GameInstance& game);

  static constexpr auto is_flag = [] (MinesweeperCell& cell) {
    return cell.flagged;
  };

  static constexpr auto is_unknown = [] (MinesweeperCell& cell) {
    return !cell.uncovered && !cell.flagged;
  };

protected:
  std::default_random_engine random;
  int start_x = 0;
  int start_y = 0;
};


#endif //MINESWEEPER_SOLVER_H
