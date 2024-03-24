//
// Created by rutio on 2020-07-13.
//

#ifndef MINESWEEPER_CELLGROUPS_H
#define MINESWEEPER_CELLGROUPS_H

#include <SFML/System.hpp>
#include <array>

/**
 * Auxiliary class for CellGroupsSolver representing one group
 * up to maximum 4 fields
 */
struct CellGroup {
  int8_t length;
  int8_t num_mines;
  std::array<sf::Vector2<int8_t>, 4> offsets;

  bool operator==(const CellGroup& other) const {
    if (length != other.length || num_mines != other.num_mines) {
      return false;
    }
    for (int i = 0; i < length; ++i) {
      if (offsets[i] != other.offsets[i]) {
        return false;
      }
    }
    return true;
  }
};

struct CellNeighbourMask {
  inline bool get(int x, int y) {
    return mask[y][x];
  }

  inline void set(int x, int y) {
    mask[y][x] = true;
  }

  inline int countFalse() {
    int ret = 8;
    for (int j = 0; j < 3; ++j) {
      for (int i = 0; i < 3; ++i) {
        if (i != 1 || j != 1) {
          ret -= mask[j][i];
        }
      }
    }
    return ret;
  }

  inline std::pair<sf::Vector2<int8_t>, CellGroup>
  getFalseGroup(int8_t num_mines) {
    std::pair<sf::Vector2<int8_t>, CellGroup> ret;
    ret.second.num_mines = num_mines;
    ret.second.length = 0;
    for (int j = 0; j < 3; ++j) {
      for (int i = 0; i < 3; ++i) {
        if (i != 1 || j != 1) {
          if (!mask[j][i]) {
            ret.second.offsets[ret.second.length] = {(int8_t) i, (int8_t) j};
            ret.second.length++;
          }
        }
      }
    }
    ret.first = {2, 2};
    for (int i = 0; i < ret.second.length; ++i) {
      if (ret.first.x > ret.second.offsets[i].x) {
        ret.first.x = ret.second.offsets[i].x;
      }
      if (ret.first.y > ret.second.offsets[i].y) {
        ret.first.y = ret.second.offsets[i].y;
      }
    }
    for (int i = 0; i < ret.second.length; ++i) {
      ret.second.offsets[i] -= ret.first;
    }
    return ret;
  }

  struct CheckResult {
    int8_t covered_fields;
    int8_t mines_min;
    int8_t mines_max;
  };

  inline CheckResult check(
      int mask_offset_x, int mask_offset_y, CellGroup& group) {
    CheckResult ret;
    ret.covered_fields = 0;
    bool success = true;
    for (int i = 0; i < group.length; ++i) {
      auto& offset = group.offsets[i];
      int x = offset.x + mask_offset_x;
      int y = offset.y + mask_offset_y;
      if (x >= 0 && y >= 0 && x <= 2 && y <= 2) {
        if (get(x, y)) {
          success = false;
        } else {
          ret.covered_fields++;
        }
      }
    }
    if (success && ret.covered_fields > 1) {
      int outside_fields = group.length - ret.covered_fields;
      ret.mines_min = std::max(0, group.num_mines - outside_fields);
      int empty_fields = group.length - group.num_mines;
      ret.mines_max =
          ret.covered_fields - std::max(0, empty_fields - outside_fields);
    } else {
      ret.covered_fields = 0;
      ret.mines_min = 0;
      ret.mines_max = 0;
    }
    return ret;
  }

  inline uint8_t apply(int mask_offset_x, int mask_offset_y, CellGroup& group) {
    for (int i = 0; i < group.length; ++i) {
      auto& offset = group.offsets[i];
      int x = offset.x + mask_offset_x;
      int y = offset.y + mask_offset_y;
      if (x >= 0 && y >= 0 && x <= 2 && y <= 2) {
        set(x, y);
      }
    }
    return group.num_mines;
  }


  bool mask[3][3] = {{false, false, false},
                     {false, true,  false},
                     {false, false, false}};
};

#endif //MINESWEEPER_CELLGROUPS_H
