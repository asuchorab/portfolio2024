//
// Created by rutio on 2020-07-12.
//

#include "GameInstance.h"
#include "WindowManager.h"
#include "Core.h"
#include <iostream>
#include <array>

/**
 * Colors for various adjacency numbers, like in original game
 */
const sf::Color LABELS_COLORS[9] = {
    {0,   0,   0},
    {0,   0,   255},
    {0,   127, 0},
    {255, 0,   0},
    {0,   0,   127},
    {127, 0,   0},
    {0,   127, 127},
    {0,   0,   0},
    {127, 127, 127}
};

GameInstance::GameInstance(MinesweeperConfig config, int seed)
    : config(config),
      field(config.width * config.height),
      remaining_mines(config.mines),
      //random(seed),
      seed(seed) {
  if (remaining_mines > (int) field.size() - 9) {
    this->config.mines = remaining_mines = (int) field.size() - 9;
  }
}

sf::VideoMode GameInstance::getVideoMode() const {
  return sf::VideoMode(config.width * Core::CELL_WIDTH,
                       config.height * Core::CELL_WIDTH + Core::CELL_WIDTH);
}

bool GameInstance::generate(int initial_x, int initial_y) {
  std::mt19937 random(seed);
  if (initial_x < 0 || initial_y < 0 ||
      initial_x >= config.width || initial_y >= config.height) {
    throw std::runtime_error("wtf");
  }
  uint32_t length = (uint32_t) field.size();
  for (int i = 0; i < remaining_mines; ++i) {
    int index;
    int x, y;
    int attempts = 0;
    while (true) {
      index = (int) (random() % length);
      x = index % config.width;
      y = index / config.width;
      if (x >= initial_x - 1 && x <= initial_x + 1 && y >= initial_y - 1 &&
          y <= initial_y + 1) {
        continue;
      }
      if (!field[index].has_mine) {
        break;
      }
      if (attempts > 100000) {
        return false;
      }
    }
    field[index].has_mine = true;
    for (int j = -1; j < 2; ++j) {
      for (int k = -1; k < 2; ++k) {
        if (x + k >= 0 && x + k < config.width &&
            y + j >= 0 && y + j < config.height) {
          if (k != 0 || j != 0) {
            field[index + config.width * j + k].num_adjacent++;
          }
        }
      }
    }
  }
  uncover(initial_x, initial_y);
  return true;
}

template<size_t N>
inline void add_vertices(
    sf::VertexArray& va, sf::Vector2f offset, int& counter,
    std::array<sf::Vector2f, N> array, sf::Color color) {
  for (size_t i = 0; i < N; ++i) {
    va[counter].position = offset + array[i];
    va[counter].color = color;
    counter++;
  }
}

void GameInstance::render(sf::RenderWindow& window) {
  if (!redraw) {
    return;
  }
  redraw = false;

  const float W = Core::CELL_WIDTH;

  sf::VertexArray field_vertices(sf::Quads, field.size() * 4);
  int field_vertices_counter = 0;
  const std::array<sf::Vector2f, 4> FIELD_VERTICES = {
      sf::Vector2f(1, 1),
      sf::Vector2f(1, W - 1),
      sf::Vector2f(W - 1, W - 1),
      sf::Vector2f(W - 1, 1),
  };

  sf::VertexArray mine_vertices(sf::Quads, config.mines * 4);
  int mine_vertices_counter = 0;
  const std::array<sf::Vector2f, 4> MINE_VERTICES = {
      sf::Vector2f(W * 0.25f, W * 0.25f),
      sf::Vector2f(W * 0.25f, W * 0.75f),
      sf::Vector2f(W * 0.75f, W * 0.75f),
      sf::Vector2f(W * 0.75f, W * 0.25f),
  };

  sf::VertexArray flag_vertices(sf::Triangles, num_flags * 3);
  int flag_vertices_counter = 0;
  const std::array<sf::Vector2f, 3> FLAG_VERTICES = {
      sf::Vector2f(W * 0.43f, W * 0.2f),
      sf::Vector2f(W * 0.7f, W * 0.35f),
      sf::Vector2f(W * 0.43f, W * 0.5f)
  };

  sf::VertexArray pole_vertices(sf::Quads, num_flags * 4);
  int pole_vertices_counter = 0;
  const std::array<sf::Vector2f, 4> POLE_VERTICES = {
      sf::Vector2f(W * 0.37f, W * 0.2f),
      sf::Vector2f(W * 0.43f, W * 0.2f),
      sf::Vector2f(W * 0.43f, W * 0.8f),
      sf::Vector2f(W * 0.37f, W * 0.8f),
  };

  WindowManager& wm = WindowManager::getInstance();
  sf::Font& font = wm.getFont();
  auto window_size = window.getSize();

  sf::RectangleShape background;
  background.setSize(
      {(float) window_size.x, (float) window_size.y - Core::CELL_WIDTH});
  background.setPosition(0, Core::CELL_WIDTH);
  background.setFillColor(Core::BACKGROUND_COLOR);
  window.draw(background);

  for (int j = 0; j < config.height; ++j) {
    for (int i = 0; i < config.width; ++i) {
      sf::Vector2f offset = {(float) i * Core::CELL_WIDTH,
                             (float) j * Core::CELL_WIDTH + Core::CELL_WIDTH};
      MinesweeperCell& cell = field[j * config.width + i];
      if (cell.uncovered) {
        add_vertices(field_vertices, offset, field_vertices_counter,
                     FIELD_VERTICES, Core::UNCOVERED_COLOR);
        if (cell.has_mine) {
          add_vertices(mine_vertices, offset, mine_vertices_counter,
                       MINE_VERTICES, sf::Color::Black);
        } else if (cell.num_adjacent > 0) {
        }
      } else {
        add_vertices(field_vertices, offset, field_vertices_counter,
                     FIELD_VERTICES, Core::COVERED_COLOR);
        if (cell.flagged) {
          if (game_over && !cell.has_mine) {
          } else {
            add_vertices(flag_vertices, offset, flag_vertices_counter,
                         FLAG_VERTICES, sf::Color::Red);
            add_vertices(pole_vertices, offset, pole_vertices_counter,
                         POLE_VERTICES, sf::Color::Black);
          }
        } else if (cell.question) {
        }
      }
    }
  }
  if (field_vertices_counter > field.size() * 4) {
    std::cout << "Field vertices exceeded\n";
  }
  if (mine_vertices_counter > config.mines * 4) {
    std::cout << "Mine vertices exceeded\n";
  }
  if (flag_vertices_counter > num_flags * 3) {
    std::cout << "Flag vertices exceeded\n";
  }
  if (pole_vertices_counter > num_flags * 4) {
    std::cout << "Pole vertices exceeded\n";
  }
  window.draw(field_vertices);
  if (mine_vertices_counter > 0) {
    window.draw(mine_vertices);
  }
  window.draw(flag_vertices);
  window.draw(pole_vertices);

  sf::Text text;
  text.setFont(font);
  text.setCharacterSize(Core::CELL_WIDTH * 3 / 4);
  const float CELL_WIDTH_F = Core::CELL_WIDTH;
  for (int j = 0; j < config.height; ++j) {
    for (int i = 0; i < config.width; ++i) {
      MinesweeperCell& cell = field[j * config.width + i];
      if (cell.uncovered) {
        if (cell.has_mine) {
        } else if (cell.num_adjacent > 0) {
          text.setString(std::to_string(cell.num_adjacent));
          auto text_rect2 = text.getLocalBounds();
          text.setOrigin(text_rect2.width / 2, 0);
          text.setPosition(CELL_WIDTH_F * i + Core::CELL_WIDTH / 2.f,
                           CELL_WIDTH_F * j + Core::CELL_WIDTH);
          text.setFillColor(LABELS_COLORS[cell.num_adjacent]);
          window.draw(text);
        }
      } else {
        if (cell.flagged) {
          if (game_over && !cell.has_mine) {
            text.setString("X");
            auto text_rect2 = text.getLocalBounds();
            text.setFillColor(sf::Color::Red);
            text.setOrigin(text_rect2.width / 2, 0);
            text.setPosition(CELL_WIDTH_F * i + Core::CELL_WIDTH / 2.f,
                             CELL_WIDTH_F * j + Core::CELL_WIDTH);
            window.draw(text);
          } else {
          }
        } else if (cell.question) {
          text.setString("?");
          auto text_rect2 = text.getLocalBounds();
          text.setFillColor(sf::Color::Black);
          text.setOrigin(text_rect2.width / 2, 0);
          text.setPosition(CELL_WIDTH_F * i + Core::CELL_WIDTH / 2.f,
                           CELL_WIDTH_F * j + Core::CELL_WIDTH);
          window.draw(text);
        }
      }
    }
  }
}

void GameInstance::uncover(int x, int y) {
  if (x < 0 || y < 0 || x >= config.width || y >= config.height) {
    return;
  }
  if (game_over || victory) {
    return;
  }
  if (!generated) {
    generated = true;
    generate(x, y);
  }
  int index = y * config.width + x;
  if (field[index].uncovered || field[index].flagged) {
    return;
  }
  redraw = true;
  if (field[index].has_mine) {
    game_over = true;
    remaining_mines = 0;
    for (auto& cell : field) {
      if (cell.has_mine) {
        cell.uncovered = true;
      }
    }
    return;
  }
  field[index].uncovered = true;
  uncovered_fields++;
  if (uncovered_fields == field.size() - config.mines) {
    victory = true;
    for (auto& cell : field) {
      if (!cell.flagged && !cell.uncovered) {
        cell.flagged = true;
        cell.question = false;
        num_flags++;
      }
    }
  }
  if (field[index].num_adjacent == 0) {
    for (int j = -1; j < 2; ++j) {
      for (int k = -1; k < 2; ++k) {
        if (x + k >= 0 && x + k < config.width &&
            y + j >= 0 && y + j < config.height) {
          if (k != 0 || j != 0) {
            if (!field[index + config.width * j + k].uncovered) {
              uncover(x + k, y + j);
            }
          }
        }
      }
    }
  }
}

void GameInstance::rightClick(int x, int y) {
  if (x < 0 || y < 0 || x >= config.width || y >= config.height) {
    return;
  }
  if (game_over || victory) {
    return;
  }
  int index = y * config.width + x;
  auto& cell = field[index];
  if (cell.uncovered) {
    return;
  }
  redraw = true;
  if (cell.flagged) {
    cell.flagged = false;
    cell.question = true;
    num_flags--;
    remaining_mines++;
  } else if (cell.question) {
    cell.question = false;
  } else {
    cell.flagged = true;
    num_flags++;
    remaining_mines--;
  }
}

void GameInstance::flag(int x, int y) {
  if (x < 0 || y < 0 || x >= config.width || y >= config.height) {
    return;
  }
  if (game_over || victory) {
    return;
  }
  int index = y * config.width + x;
  auto& cell = field[index];
  if (cell.uncovered) {
    return;
  }
  if (!cell.flagged) {
    redraw = true;
    cell.flagged = true;
    cell.question = false;
    num_flags++;
    remaining_mines--;
  }
}

void GameInstance::question(int x, int y) {
  if (x < 0 || y < 0 || x >= config.width || y >= config.height) {
    return;
  }
  if (game_over || victory) {
    return;
  }
  int index = y * config.width + x;
  auto& cell = field[index];
  if (cell.uncovered) {
    return;
  }
  if (!cell.question) {
    redraw = true;
    if (cell.flagged) {
      cell.flagged = false;
      num_flags--;
      remaining_mines++;
    }
    cell.question = true;
  }
}

void GameInstance::advanceTime(double dt) {
  if (victory || game_over) {
    return;
  }
  time_accumulator += dt;
  while (time_accumulator > 1.0) {
    time_accumulator -= 1.0;
    timer++;
  }
}

void GameInstance::requestRedraw() {
  redraw = true;
}

int GameInstance::getRemainingMines() const {
  return remaining_mines;
}

int GameInstance::getTimer() const {
  return timer;
}

int GameInstance::getUncoveredFields() const {
  return uncovered_fields;
}

int GameInstance::getWidth() const {
  return config.width;
}

int GameInstance::getHeight() const {
  return config.height;
}

bool GameInstance::isVictory() const {
  return victory;
}

bool GameInstance::isGameOver() const {
  return game_over;
}

MinesweeperCell& GameInstance::getCell(int x, int y) {
  return field[y * config.width + x];
}
