//
// Created by rutio on 2020-07-12.
//

#include <random>
#include <chrono>
#include "ObviousMovesSolver.h"

bool ObviousMovesSolver::step(GameInstance& game) {
  if (game.isGameOver() || game.isVictory()) {
    return false;
  }
  int width = game.getWidth();
  int height = game.getHeight();
  bool did_change = false;
  if (data.empty()) {
    data.resize(width * height);
  }
  if (stack.empty()) {
    auto pos = getStartingPoint(game);
    stack.push_back(pos);
  }
  while (!did_change && !stack.empty()) {
    auto pos = stack.back();
    stack.pop_back();
    auto& cell_data = data[pos.y * width + pos.x];
    if (!cell_data.visited) {
      int num_flags = countAround(game, pos.x, pos.y, is_flag);
      int adjacent = game.getCell(pos.x, pos.y).num_adjacent;
      if (num_flags == adjacent) {
        if (countAround(game, pos.x, pos.y, is_unknown) != 0) {
          executeAround(
              game, pos.x, pos.y,
              [this, width](GameInstance& game, int x, int y) {
                executeAround(
                    game, x, y,
                    [this, width](GameInstance& game, int x, int y) {
                      auto& cell = game.getCell(x, y);
                      if (cell.uncovered && !cell.flagged) {
                        data[y * width + x].visited = false;
                        stack.emplace_back(x, y);
                      }
                    });
                game.uncover(x, y);
              });
          did_change = true;
          should_retry = true;
        }
      } else if (num_flags < adjacent) {
        if (countAround(game, pos.x, pos.y, is_unknown) ==
            adjacent - num_flags) {
          executeAround(
              game, pos.x, pos.y,
              [this, width](GameInstance& game, int x, int y) {
                auto& cell = game.getCell(x, y);
                if (!cell.flagged && !cell.uncovered) {
                  executeAround(
                      game, x, y,
                      [this, width](GameInstance& game, int x, int y) {
                        auto& cell = game.getCell(x, y);
                        if (cell.uncovered && !cell.flagged) {
                          data[y * width + x].visited = false;
                          stack.emplace_back(x, y);
                        }
                      });
                  game.flag(x, y);
                }
              });
          did_change = true;
          should_retry = true;
        }
      }
      executeAround(
          game, pos.x, pos.y,
          [this, width](GameInstance& game, int x, int y) {
            if (game.getCell(x, y).uncovered && !data[y * width + x].visited) {
              stack.emplace_back(x, y);
            }
          });
      cell_data.visited = true;
    }
    if (stack.empty()) {
      int num_visited = (int) std::count_if(data.begin(), data.end(),
                                      [](auto& d) { return d.visited; });
      if (num_visited != game.getUncoveredFields()) {
        sf::Vector2i pos(0, 0);
        while (true) {
          if (game.getCell(pos.x, pos.y).uncovered &&
              !data[pos.y * width + pos.x].visited) {
            break;
          }
          pos.x++;
          if (pos.x == width) {
            pos.x = 0;
            pos.y++;
          }
        }
        stack.push_back(pos);
      } else if (should_retry) {
        should_retry = false;
        auto pos = getStartingPoint(game);
        stack.push_back(pos);
        for (auto& d : data) {
          d.visited = false;
        }
      }
    }
  }
  return did_change;
}
