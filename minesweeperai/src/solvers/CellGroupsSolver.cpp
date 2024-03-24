//
// Created by rutio on 2020-07-13.
//

#include "CellGroupsSolver.h"
#include <optional>

bool CellGroupsSolver::step(GameInstance& game) {
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
      CellNeighbourMask known;
      int off_x = pos.x - 1;
      int off_y = pos.y - 1;
      int num_mines_flagged = 0;

      // if it's near borders, mark cells outside the field as known
      if (pos.x == 0) {
        known.set(0, 0);
        known.set(0, 1);
        known.set(0, 2);
      }
      if (pos.y == 0) {
        known.set(0, 0);
        known.set(1, 0);
        known.set(2, 0);
      }
      if (pos.x == width - 1) {
        known.set(2, 0);
        known.set(2, 1);
        known.set(2, 2);
      }
      if (pos.y == height - 1) {
        known.set(0, 2);
        known.set(1, 2);
        known.set(2, 2);
      }

      // mark all flags and uncovered cells and count guaranteed mines
      executeAround(
          game, pos.x, pos.y,
          [this, &known, &num_mines_flagged, off_x, off_y](
              GameInstance& game, int x, int y) {
            auto& cell = game.getCell(x, y);
            if (cell.flagged) {
              num_mines_flagged++;
              known.set(x - off_x, y - off_y);
            } else if (cell.uncovered) {
              known.set(x - off_x, y - off_y);
            }
          });

      // if there at least one unknown field
      int num_unknown = known.countFalse();
      if (num_unknown > 0) {

        // if all neighboring mines are flagged, uncover all the other cells
        if (num_mines_flagged == adjacent) {
          executeAround(
              game, pos.x, pos.y,
              [this, &known, off_x, off_y](GameInstance& game, int x, int y) {
                if (!known.get(x - off_x, y - off_y)) {
                  queued_moves.emplace_back(x, y, UNCOVER, true);
                }
              });
          did_change = true;
          should_retry = true;

          // if every uncovered cell must be a mine
        } else if (num_unknown == adjacent - num_mines_flagged) {
          executeAround(
              game, pos.x, pos.y,
              [this, &known, off_x, off_y](GameInstance& game, int x, int y) {
                if (!known.get(x - off_x, y - off_y)) {
                  queued_moves.emplace_back(x, y, FLAG, true);
                }
              });
          did_change = true;
          should_retry = true;

          // if there is no obvious solution, search groups
        } else {

          // structure for searching groups
          struct SearchGroup {
            SearchGroup(
                int x, int y, CellNeighbourMask::CheckResult stats,
                CellGroup group)
                : x(x), y(y), stats(stats), group(group) {}

            int x;
            int y;
            CellNeighbourMask::CheckResult stats;
            CellGroup group;
          };
          std::vector<SearchGroup> groups;

          // function for searching groups
          auto gather_groups =
              [this, &groups, &game, &known, off_x, off_y](int x, int y) {
                for (auto& group : getData(x, y).groups) {
                  auto check_result = known.check(x - off_x, y - off_y, group);
                  if (check_result.mines_min > 0) {
                    groups.emplace_back(x, y, check_result, group);
                  }
                }
              };

          // search groups in neighboring cells that can be useful
          if (pos.y > 0) {
            gather_groups(pos.x, pos.y - 1);
            if (pos.y > 1) {
              gather_groups(pos.x, pos.y - 2);
              if (pos.y > 2) {
                gather_groups(pos.x, pos.y - 3);
              }
            }
          }
          if (pos.y < height - 1) {
            gather_groups(pos.x, pos.y + 1);
          }
          if (pos.x > 0) {
            gather_groups(pos.x - 1, pos.y);
            if (pos.y > 0) {
              gather_groups(pos.x - 1, pos.y - 1);
              if (pos.y > 1) {
                gather_groups(pos.x - 1, pos.y - 2);
                if (pos.y > 2) {
                  gather_groups(pos.x - 1, pos.y - 3);
                }
              }
            }
            if (pos.y < height - 1) {
              gather_groups(pos.x - 1, pos.y + 1);
            }
            if (pos.x > 1) {
              gather_groups(pos.x - 2, pos.y);
              if (pos.y > 0) {
                gather_groups(pos.x - 2, pos.y - 1);
                if (pos.y > 1) {
                  gather_groups(pos.x - 2, pos.y - 2);
                }
              }
              if (pos.y < height - 1) {
                gather_groups(pos.x - 2, pos.y + 1);
              }
              if (pos.x > 2) {
                gather_groups(pos.x - 3, pos.y);
                if (pos.y > 0) {
                  gather_groups(pos.x - 3, pos.y - 1);
                }
                if (pos.y < height - 1) {
                  gather_groups(pos.x - 3, pos.y + 1);
                }
              }
            }
          }
          if (pos.x < width - 1) {
            gather_groups(pos.x + 1, pos.y);
            if (pos.y > 0) {
              gather_groups(pos.x + 1, pos.y - 1);
              if (pos.y > 1) {
                gather_groups(pos.x + 1, pos.y - 2);
                if (pos.y > 2) {
                  gather_groups(pos.x + 1, pos.y - 3);
                }
              }
            }
            if (pos.y < height - 1) {
              gather_groups(pos.x + 1, pos.y + 1);
            }
          }

          // structures for generating combinations
          struct Combination {
            Combination(CellNeighbourMask mask)
                : stats({0, 0, 0}),
                  mask(mask) {}

            CellNeighbourMask::CheckResult stats;
            std::vector<SearchGroup*> groups;
            CellNeighbourMask mask;
          };
          struct CombinationBranch {
            CombinationBranch(int index, Combination combination)
                : index(index), combination(std::move(combination)) {}

            int index;
            Combination combination;
          };

          // list of generated combinations
          std::vector<Combination> combinations;

          // best combination is one that has uncertainty of number of mines
          // and excludes the most fields
          Combination* best_combination = nullptr;

          // whether one combination determines a definitive solution
          bool found_solution = false;

          // generate all combinations
          if (!groups.empty()) {
            std::vector<CombinationBranch> combination_stack;
            CombinationBranch temp(0, Combination(known));
            while (true) {

              // try to check if the group applies
              auto& group = groups[temp.index];
              auto check_result = temp.combination.mask.check(
                  group.x - off_x, group.y - off_y, group.group);

              // if applies, branch by pushing on stack and applying to temp
              if (check_result.covered_fields > 0) {

                // check if the combination would not cover all fields
                if (temp.combination.stats.covered_fields +
                    check_result.covered_fields < num_unknown) {
                  combination_stack.emplace_back(temp);
                  temp.combination.mask.apply(group.x - off_x, group.y - off_y,
                                              group.group);
                  temp.combination.groups.push_back(&group);
                  temp.combination.stats.covered_fields += check_result.covered_fields;
                  temp.combination.stats.mines_min += check_result.mines_min;
                  temp.combination.stats.mines_max += check_result.mines_max;
                }
              }
              // move onto next
              temp.index++;

              // if processed all groups
              if (temp.index >= groups.size()) {

                // if the group contains anything, add it to found combinations
                if (temp.combination.stats.mines_min > 0) {
                  combinations.emplace_back(std::move(temp.combination));
                }

                // try to grab a combination from stack
                if (combination_stack.empty()) {
                  break;
                } else {
                  temp = std::move(combination_stack.back());
                  temp.index++;
                  combination_stack.pop_back();
                }
              }
            }
          }

          // function for marking groups with question marks
          auto question_groups = [this, pos](
              const std::vector<SearchGroup*>& groups) {
            for (auto& group : groups) {
              for (int i = 0; i < group->group.length; ++i) {
                auto g_off = group->group.offsets[i];
                queued_moves.emplace_back(
                    group->x + g_off.x, group->y + g_off.y, QUESTION, true);
              }
              queued_moves.back().continue_group = false;
            }
          };

          // go through all combinations
          for (auto& combination : combinations) {
            int num_mines_min = num_mines_flagged + combination.stats.mines_min;
            int num_mines_max = num_mines_flagged + combination.stats.mines_max;
            int combination_num_unknown =
                num_unknown - combination.stats.covered_fields;

            // if one combination guarantees to contain at least all of the
            // cell's given amount of mines, uncover all remaining
            if (num_mines_min >= adjacent) {
              question_groups(combination.groups);
              executeAround(
                  game, pos.x, pos.y,
                  [this, &combination, off_x, off_y](
                      GameInstance& game, int x, int y) {
                    if (!combination.mask.get(x - off_x, y - off_y)) {
                      queued_moves.emplace_back(x, y, UNCOVER, true);
                    }
                  });
              found_solution = true;
              did_change = true;
              should_retry = true;
              break;
            }
            // if the combination has a definitive amount of mines
            if (num_mines_min == num_mines_max) {

              // if the number of mines that must be in unknown cells
              // is equal to the number of unknown cells, flag all unknown cells
              if (combination_num_unknown == adjacent - num_mines_min) {
                question_groups(combination.groups);
                executeAround(
                    game, pos.x, pos.y,
                    [this, &combination, off_x, off_y](
                        GameInstance& game, int x, int y) {
                      if (!combination.mask.get(x - off_x, y - off_y)) {
                        queued_moves.emplace_back(x, y, FLAG, true);
                      }
                    });
                found_solution = true;
                did_change = true;
                should_retry = true;
                break;
              }

              // if the comhbination has between 2 and 4 unknown cells
              if (combination_num_unknown >= 2 &&
                  combination_num_unknown <= 4) {

                // if the combination is better than the best combination,
                // update best combination
                if (!best_combination ||
                    best_combination->stats.covered_fields <
                    combination.stats.covered_fields) {
                  best_combination = &combination;
                }
              }
            }
          }

          // if no defintive solution was found, create a new group out
          // of unknown cells
          if (!found_solution) {
            std::optional<std::pair<sf::Vector2<int8_t>, CellGroup>> opt_this_group;
            if (best_combination) {
              opt_this_group = best_combination->mask.getFalseGroup(
                  adjacent - num_mines_flagged -
                  best_combination->stats.mines_min);
            } else if (num_unknown >= 2 && num_unknown <= 4) {
              opt_this_group = known.getFalseGroup(
                  adjacent - num_mines_flagged);
            }
            if (opt_this_group) {
              auto this_group = opt_this_group.value();
              int group_off_x = off_x + this_group.first.x;
              int group_off_y = off_y + this_group.first.y;
              auto& data_cell = getData(group_off_x, group_off_y);
              bool do_insert = true;
              for (auto& group : data_cell.groups) {
                if (this_group.second == group) {
                  do_insert = false;
                  break;
                }
              }
              if (do_insert) {
                for (int i = 0; i < this_group.second.length; ++i) {
                  auto cell_off = this_group.second.offsets[i];
                  executeAround(
                      game, group_off_x + cell_off.x, group_off_y + cell_off.y,
                      [this](GameInstance& game, int x, int y) {
                        auto& cell = game.getCell(x, y);
                        if (cell.uncovered) {
                          getData(x, y).visited = false;
                          stack.emplace_back(x, y);
                        }
                      });
                }
                data_cell.groups.emplace_back(this_group.second);
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
          d.groups.clear();
        }
      }
    }
  }

  executeQueuedMoves(game);
  return did_change;
}

bool CellGroupsSolver::executeQueuedMoves(GameInstance& game) {
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
              if (cell.uncovered) {
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
              if (cell.uncovered) {
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

CellGroupsSolver::Data& CellGroupsSolver::getData(int x, int y) {
  return data[y * width + x];
}

CellGroupsSolver::QueuedMove::QueuedMove(
    int x, int y, CellGroupsSolver::MoveType type, bool continueGroup)
    : x(x), y(y), type(type), continue_group(continueGroup) {}
