//
// Created by rutio on 2020-07-12.
//

#include <chrono>
#include "Solver.h"

Solver::Solver()
    : random(
    (uint32_t) std::chrono::high_resolution_clock::now().time_since_epoch().count()) {
}

int Solver::countAround(
    GameInstance& game, int x, int y,
    const std::function<bool(MinesweeperCell&)>& func) {
  int count = 0;
  int x_begin = std::max(x - 1, 0);
  int x_end = std::min(x + 2, game.getWidth());
  int y_begin = std::max(y - 1, 0);
  int y_end = std::min(y + 2, game.getHeight());
  for (int j = y_begin; j < y_end; ++j) {
    for (int i = x_begin; i < x_end; ++i) {
      if (i != x || j != y) {
        if (func(game.getCell(i, j))) {
          count++;
        }
      }
    }
  }
  return count;
}

int Solver::countAround(
    GameInstance& game, int x, int y,
    const std::function<bool(GameInstance&, int, int)>& func) {
  int count = 0;
  int x_begin = std::max(x - 1, 0);
  int x_end = std::min(x + 2, game.getWidth());
  int y_begin = std::max(y - 1, 0);
  int y_end = std::min(y + 2, game.getHeight());
  for (int j = y_begin; j < y_end; ++j) {
    for (int i = x_begin; i < x_end; ++i) {
      if (i != x || j != y) {
        if (func(game, i, j)) {
          count++;
        }
      }
    }
  }
  return count;
}

void Solver::executeAround(
    GameInstance& game, int x, int y,
    const std::function<void(MinesweeperCell&)>& func) {
  int x_begin = std::max(x - 1, 0);
  int x_end = std::min(x + 2, game.getWidth());
  int y_begin = std::max(y - 1, 0);
  int y_end = std::min(y + 2, game.getHeight());
  for (int j = y_begin; j < y_end; ++j) {
    for (int i = x_begin; i < x_end; ++i) {
      if (i != x || j != y) {
        func(game.getCell(i, j));
      }
    }
  }
}

void Solver::executeAround(
    GameInstance& game, int x, int y,
    const std::function<void(GameInstance&, int, int)>& func) {
  int x_begin = std::max(x - 1, 0);
  int x_end = std::min(x + 2, game.getWidth());
  int y_begin = std::max(y - 1, 0);
  int y_end = std::min(y + 2, game.getHeight());
  for (int j = y_begin; j < y_end; ++j) {
    for (int i = x_begin; i < x_end; ++i) {
      if (i != x || j != y) {
        func(game, i, j);
      }
    }
  }
}

sf::Vector2i Solver::getStartingPoint(GameInstance& game) {
  int x, y;
  if (game.getUncoveredFields() == 0) {
    if (start_x >= 0 && start_x < game.getWidth() &&
        start_y >= 0 && start_y < game.getHeight()) {
      x = start_x;
      y = start_y;
    } else {
      x = random() % game.getWidth();
      y = random() % game.getHeight();
    }
    game.uncover(x, y);
  } else {
    x = 0;
    y = 0;
    int width = game.getWidth();
    while (true) {
      if (game.getCell(x, y).uncovered) {
        break;
      }
      x++;
      if (x == width) {
        x = 0;
        y++;
      }
    }
  }
  return {x, y};
}

void Solver::setStart(int new_start_x, int new_start_y) {
  start_x = new_start_x;
  start_y = new_start_y;
}
