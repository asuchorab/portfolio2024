//
// Created by rutio on 2020-07-13.
//

#include "DumbCellGroupsSolver.h"

bool DumbCellGroupsSolver::step(GameInstance& game) {
  if (executeQueuedMoves(game)) {
    return true;
  }
  if (game.isGameOver() || game.isVictory()) {
    return false;
  }
  width = game.getWidth();
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
    queued_moves.clear();
    auto pos = stack.back();
    stack.pop_back();
    auto& cell_data = getData(pos.x, pos.y);
    if (!cell_data.visited) {
      int adjacent = game.getCell(pos.x, pos.y).num_adjacent;
      bool known[3][3] = {{false, false, false},
                          {false, true,  false},
                          {false, false, false}};
      int off_x = pos.x - 1;
      int off_y = pos.y - 1;
      int num_mines = 0;
      executeAround(
          game, pos.x, pos.y,
          [this, &known, &num_mines, off_x, off_y](
              GameInstance& game, int x, int y) {
            auto& cell = game.getCell(x, y);
            if (cell.flagged) {
              num_mines++;
              known[x - off_x][y - off_y] = true;
            } else if (cell.uncovered) {
              known[x - off_x][y - off_y] = true;
            }
          });
      if (pos.x > 0) {
        if (!known[0][1]) {
          if (getData(pos.x - 1, pos.y).groups & V_1_2 && !known[0][2]) {
            known[0][1] = true;
            known[0][2] = true;
            queued_moves.emplace_back(pos.x - 1, pos.y, QUESTION, true);
            queued_moves.emplace_back(pos.x - 1, pos.y + 1, QUESTION, false);
            num_mines++;
          } else if (pos.y > 0 && !known[0][0]) {
            auto groups_1_1 = getData(pos.x - 1, pos.y - 1).groups;
            if (groups_1_1 & V_1_2) {
              known[0][0] = true;
              known[0][1] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x - 1, pos.y, QUESTION, false);
              num_mines++;
            } else if (groups_1_1 & V_1_3 && !known[0][2]) {
              known[0][0] = true;
              known[0][1] = true;
              known[0][2] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x - 1, pos.y, QUESTION, true);
              queued_moves.emplace_back(pos.x - 1, pos.y + 1, QUESTION, false);
              num_mines++;
            } else if (groups_1_1 & V_2_3 && !known[0][2]) {
              known[0][0] = true;
              known[0][1] = true;
              known[0][2] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x - 1, pos.y, QUESTION, true);
              queued_moves.emplace_back(pos.x - 1, pos.y + 1, QUESTION, false);
              num_mines += 2;
            }
          }
        }
      } else {
        known[0][0] = true;
        known[0][1] = true;
        known[0][2] = true;
      }
      if (pos.y > 0) {
        if (!known[1][0]) {
          if (getData(pos.x, pos.y - 1).groups & H_1_2 && !known[2][0]) {
            known[1][0] = true;
            known[2][0] = true;
            queued_moves.emplace_back(pos.x, pos.y - 1, QUESTION, true);
            queued_moves.emplace_back(pos.x + 1, pos.y - 1, QUESTION, false);
            num_mines++;
          } else if (pos.x > 0 && !known[0][0]) {
            auto groups_1_1 = getData(pos.x - 1, pos.y - 1).groups;
            if (groups_1_1 & H_1_2) {
              known[0][0] = true;
              known[1][0] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x, pos.y - 1, QUESTION, false);
              num_mines++;
            } else if (groups_1_1 & H_1_3 && !known[2][0]) {
              known[0][0] = true;
              known[1][0] = true;
              known[2][0] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y - 1, QUESTION, false);
              num_mines++;
            } else if (groups_1_1 & H_2_3 && !known[2][0]) {
              known[0][0] = true;
              known[1][0] = true;
              known[2][0] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y - 1, QUESTION, false);
              num_mines += 2;
            }
          }
        }
      } else {
        known[0][0] = true;
        known[1][0] = true;
        known[2][0] = true;
      }
      if (pos.x < width - 1) {
        if (!known[2][1]) {
          if (getData(pos.x + 1, pos.y).groups & V_1_2 && !known[2][2]) {
            known[2][1] = true;
            known[2][2] = true;
            queued_moves.emplace_back(pos.x + 1, pos.y, QUESTION, true);
            queued_moves.emplace_back(pos.x + 1, pos.y + 1, QUESTION, false);
            num_mines++;
          } else if (pos.y > 0 && !known[2][0]) {
            auto groups_1_1 = getData(pos.x + 1, pos.y - 1).groups;
            if (groups_1_1 & V_1_2) {
              known[2][0] = true;
              known[2][1] = true;
              queued_moves.emplace_back(pos.x + 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y, QUESTION, false);
              num_mines++;
            } else if (groups_1_1 & V_1_3 && !known[2][2]) {
              known[2][0] = true;
              known[2][1] = true;
              known[2][2] = true;
              queued_moves.emplace_back(pos.x + 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y + 1, QUESTION, false);
              num_mines++;
            } else if (groups_1_1 & V_2_3 && !known[2][2]) {
              known[2][0] = true;
              known[2][1] = true;
              known[2][2] = true;
              queued_moves.emplace_back(pos.x + 1, pos.y - 1, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y + 1, QUESTION, false);
              num_mines += 2;
            }
          }
        }
      } else {
        known[2][0] = true;
        known[2][1] = true;
        known[2][2] = true;
      }
      if (pos.y < height - 1) {
        if (!known[1][2]) {
          if (getData(pos.x, pos.y + 1).groups & H_1_2 && !known[2][2]) {
            known[1][2] = true;
            known[2][2] = true;
            queued_moves.emplace_back(pos.x, pos.y + 1, QUESTION, true);
            queued_moves.emplace_back(pos.x + 1, pos.y + 1, QUESTION, false);
            num_mines++;
          } else if (pos.x > 0 && !known[0][2]) {
            auto groups_1_1 = getData(pos.x - 1, pos.y + 1).groups;
            if (groups_1_1 & H_1_2) {
              known[0][2] = true;
              known[1][2] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y + 1, QUESTION, true);
              queued_moves.emplace_back(pos.x, pos.y + 1, QUESTION, false);
              num_mines++;
            } else if (groups_1_1 & H_1_3 && !known[2][2]) {
              known[0][2] = true;
              known[1][2] = true;
              known[2][2] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y + 1, QUESTION, true);
              queued_moves.emplace_back(pos.x, pos.y + 1, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y + 1, QUESTION, false);
              num_mines++;
            } else if (groups_1_1 & H_2_3 && !known[2][2]) {
              known[0][2] = true;
              known[1][2] = true;
              known[2][2] = true;
              queued_moves.emplace_back(pos.x - 1, pos.y + 1, QUESTION, true);
              queued_moves.emplace_back(pos.x, pos.y + 1, QUESTION, true);
              queued_moves.emplace_back(pos.x + 1, pos.y + 1, QUESTION, false);
              num_mines += 2;
            }
          }
        }
      } else {
        known[0][2] = true;
        known[1][2] = true;
        known[2][2] = true;
      }
      int num_unknown = 8;
      num_unknown -= known[0][0];
      num_unknown -= known[0][1];
      num_unknown -= known[0][2];
      num_unknown -= known[1][0];
      num_unknown -= known[1][2];
      num_unknown -= known[2][0];
      num_unknown -= known[2][1];
      num_unknown -= known[2][2];
      if (num_unknown > 0) {
        if (num_mines == adjacent) {
          executeAround(
              game, pos.x, pos.y,
              [this, &known, off_x, off_y](GameInstance& game, int x, int y) {
                if (!known[x - off_x][y - off_y]) {
                  queued_moves.emplace_back(x, y, UNCOVER, true);
                }
              });
          did_change = true;
          should_retry = true;
        } else if (num_mines < adjacent) {
          auto propagate = [this](GameInstance& game, int x, int y) {
            auto& cell = game.getCell(x, y);
            if (cell.uncovered && !cell.flagged) {
              getData(x, y).visited = false;
              stack.emplace_back(x, y);
            }
          };

          if (num_unknown == adjacent - num_mines) {
            executeAround(
                game, pos.x, pos.y,
                [this, &known, off_x, off_y](GameInstance& game, int x, int y) {
                  if (!known[x - off_x][y - off_y]) {
                    queued_moves.emplace_back(x, y, FLAG, true);
                  }
                });
            did_change = true;
            should_retry = true;
          } else if (num_unknown == 2) {
            if (adjacent - num_mines == 1) {
              if (pos.x > 0) {
                if (!known[0][1]) {
                  if (!known[0][2]) {
                    getData(pos.x - 1, pos.y).groups |= V_1_2;
                    propagate(game, pos.x, pos.y);
                  } else if (pos.y > 0 && !known[0][0]) {
                    getData(pos.x - 1, pos.y - 1).groups |= V_1_2;
                    propagate(game, pos.x, pos.y);
                  }
                }
              }
              if (pos.y > 0) {
                if (!known[1][0]) {
                  if (!known[2][0]) {
                    getData(pos.x, pos.y - 1).groups |= H_1_2;
                    propagate(game, pos.x, pos.y);
                  } else if (pos.x > 0 && !known[0][0]) {
                    getData(pos.x - 1, pos.y - 1).groups |= H_1_2;
                    propagate(game, pos.x, pos.y);
                  }
                }
              }
              if (pos.x < width - 1) {
                if (!known[2][1]) {
                  if (!known[2][2]) {
                    getData(pos.x + 1, pos.y).groups |= V_1_2;
                    propagate(game, pos.x, pos.y);
                  } else if (pos.y > 0 && !known[2][0]) {
                    getData(pos.x + 1, pos.y - 1).groups |= V_1_2;
                    propagate(game, pos.x, pos.y);
                  }
                }
              }
              if (pos.y < height - 1) {
                if (!known[1][2]) {
                  if (!known[2][2]) {
                    getData(pos.x, pos.y + 1).groups |= H_1_2;
                    propagate(game, pos.x, pos.y);
                  } else if (pos.x > 0 && !known[0][2]) {
                    getData(pos.x - 1, pos.y + 1).groups |= H_1_2;
                    propagate(game, pos.x, pos.y);
                  }
                }
              }
            }
          } else if (num_unknown == 3) {
            int mines_remaining = adjacent - num_mines;
            if (mines_remaining == 1 || mines_remaining == 2) {
              if (pos.x > 0 && pos.y > 0) {
                if (!known[0][0]) {
                  if (!known[0][1] && !known[0][2]) {
                    getData(pos.x - 1, pos.y - 1).groups |=
                        mines_remaining == 1 ? V_1_3 : V_2_3;
                    propagate(game, pos.x, pos.y);
                  } else if (!known[1][0] && !known[2][0]) {
                    getData(pos.x - 1, pos.y - 1).groups |=
                        mines_remaining == 1 ? H_1_3 : H_2_3;
                    propagate(game, pos.x, pos.y);
                  }
                }
              }
              if (pos.x < width - 1 && pos.y < height - 1) {
                if (!known[2][2]) {
                  if (!known[2][0] && !known[2][1] && pos.y > 0) {
                    getData(pos.x + 1, pos.y - 1).groups |=
                        mines_remaining == 1 ? V_1_3 : V_2_3;
                    propagate(game, pos.x, pos.y);
                  } else if (!known[0][2] && !known[1][2] && pos.x > 0) {
                    getData(pos.x - 1, pos.y + 1).groups |=
                        mines_remaining == 1 ? H_1_3 : H_2_3;
                    propagate(game, pos.x, pos.y);
                  }
                }
              }
            }
          }
        }
      }
      executeAround(
          game, pos.x, pos.y,
          [this](GameInstance& game, int x, int y) {
            if (game.getCell(x, y).uncovered &&
                !getData(x, y).visited) {
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
              !getData(pos.x, pos.y).visited) {
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
  return executeQueuedMoves(game);
}

bool DumbCellGroupsSolver::executeQueuedMoves(GameInstance& game) {
  bool did_queued_move = false;
  while (true) {
    if (queued_moves.size() == queue_pos) {
      queued_moves.clear();
      queue_pos = 0;
      break;
    }
    auto& move = queued_moves[queue_pos];
    switch (move.type) {
      case UNCOVER:
        executeAround(
            game, move.x, move.y,
            [this](GameInstance& game, int x, int y) {
              auto& cell = game.getCell(x, y);
              if (cell.uncovered && !cell.flagged) {
                getData(x, y).visited = false;
                stack.emplace_back(x, y);
              }
            });
        game.uncover(move.x, move.y);
        break;
      case FLAG:
        executeAround(
            game, move.x, move.y,
            [this](GameInstance& game, int x, int y) {
              auto& cell = game.getCell(x, y);
              if (cell.uncovered && !cell.flagged) {
                getData(x, y).visited = false;
                stack.emplace_back(x, y);
              }
            });
        game.flag(move.x, move.y);
        break;
      case QUESTION:
        game.question(move.x, move.y);
        break;
    }
    queue_pos++;
    did_queued_move = true;
    if (!move.continue_group) {
      break;
    }
  }
  return did_queued_move;
}

DumbCellGroupsSolver::Data& DumbCellGroupsSolver::getData(int x, int y) {
  return data[y * width + x];
}

DumbCellGroupsSolver::QueuedMove::QueuedMove(
    int x, int y, DumbCellGroupsSolver::MoveType type, bool continueGroup)
    : x(x), y(y), type(type), continue_group(continueGroup) {}
