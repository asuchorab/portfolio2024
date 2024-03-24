//
// Created by rutio on 2020-07-12.
//

#include <thread>
#include <iostream>
#include "Core.h"
#include "WindowManager.h"
#include "TimeUtil.h"
#include "menu/MenuButton.h"
#include "menu/MenuIntInput.h"
#include "solvers/ObviousMovesSolver.h"
#include "menu/MenuEnumChoice.h"
#include "solvers/DumbCellGroupsSolver.h"
#include "solvers/CellGroupsSolver.h"

const sf::Color Core::BACKGROUND_COLOR = {160, 160, 160};
const sf::Color Core::UNCOVERED_COLOR = {210, 210, 210};
const sf::Color Core::COVERED_COLOR = {110, 110, 110};
const sf::Color Core::SELECTED_COLOR = {255, 255, 255};

Core::Core()
    : config(9, 9, 10),
      config_being_changed({9, 9, 10}),
      game({9, 9, 10}, 0),
      random((uint32_t) std::chrono::high_resolution_clock::now()
          .time_since_epoch().count()) {
  seed = random();
  game = GameInstance(config, seed);
  menu_main.addItem(std::make_unique<MenuButton>(
      "Game settings",
      [this]() {
        config_being_changed = config;
        changeMenu(&menu_game);
      }));
  menu_main.addItem(std::make_unique<MenuButton>(
      "Choose AI",
      [this]() {
        changeMenu(&menu_ai);
      }));
  menu_main.addItem(std::make_unique<MenuButton>(
      "AI Settings",
      [this]() {
        changeMenu(&menu_ai_settings);
      }));
  menu_main.addItem(std::make_unique<MenuButton>(
      "Exit",
      [this]() {
        running = false;
      }));
  menu_game.addItem(
      std::make_unique<MenuIntInput>("Width", &config_being_changed.width));
  menu_game.addItem(
      std::make_unique<MenuIntInput>("Height", &config_being_changed.height));
  menu_game.addItem(
      std::make_unique<MenuIntInput>("Mines", &config_being_changed.mines));
  menu_game.addItem(std::make_unique<MenuButton>(
      "Beginner",
      [this]() {
        config_being_changed = {9, 9, 10};
      }));
  menu_game.addItem(std::make_unique<MenuButton>(
      "Intermediate",
      [this]() {
        config_being_changed = {16, 16, 40};
      }));
  menu_game.addItem(std::make_unique<MenuButton>(
      "Expert",
      [this]() {
        config_being_changed = {30, 16, 99};
      }));
  menu_game.addItem(std::make_unique<MenuIntInput>("Seed: ", &seed));
  menu_game.addItem(std::make_unique<MenuButton>(
      "Generate (new seed)",
      [this]() {
        config = config_being_changed;
        if (config.width < 1) {
          config.width = 1;
        }
        if (config.height < 1) {
          config.height = 1;
        }
        changeGameInstanceNewSeed(config);
        current_menu = nullptr;
      }));
  menu_game.addItem(std::make_unique<MenuButton>(
      "Generate (this seed)",
      [this]() {
        config = config_being_changed;
        if (config.width < 1) {
          config.width = 1;
        }
        if (config.height < 1) {
          config.height = 1;
        }
        changeGameInstance(config);
        current_menu = nullptr;
      }));
  menu_ai.addItem(std::make_unique<MenuButton>(
      "None",
      [this]() {
        if (solver) {
          menu_header = "Unloaded AI";
        }
        ai_type = NONE;
        updateAI();
      }));
  menu_ai.addItem(std::make_unique<MenuButton>(
      "Obvious moves",
      [this]() {
        ai_type = OBVIOUS_MOVES;
        updateAI();
        menu_header = "AI loaded";
      }));
  menu_ai.addItem(std::make_unique<MenuButton>(
      "Dumb cell groups",
      [this]() {
        ai_type = DUMB_CELL_GROUPS;
        updateAI();
        menu_header = "AI loaded";
      }));
  menu_ai.addItem(std::make_unique<MenuButton>(
      "Cell groups",
      [this]() {
        ai_type = CELL_GROUPS;
        updateAI();
        menu_header = "AI loaded";
      }));
  menu_ai_settings.addItem(std::make_unique<MenuEnumChoice>(
      "AI mode: ",
      std::vector<std::string>{"step", "fast forward", "instant"},
      reinterpret_cast<int*>(&ai_mode)));
  menu_ai_settings.addItem(std::make_unique<MenuEnumChoice>(
      "Random start: ",
      std::vector<std::string>{"never", "new seed", "always"},
      reinterpret_cast<int*>(&ai_start_random)));
  menu_ai_settings.addItem(
      std::make_unique<MenuIntInput>("AI start X: ", &ai_start_x));
  menu_ai_settings.addItem(
      std::make_unique<MenuIntInput>("AI start Y: ", &ai_start_y));
}

void Core::run() {
  auto& wm = WindowManager::getInstance();
  auto& window = wm.getWindow();
  wm.updateWindow(game.getVideoMode());
  sf::Event event;
  auto last_update_time = timeutil::now();
  sf::Vector2i mouse_pos;
  int holding_x = 0;
  int holding_y = 0;
  int fast_forward_delay = 2;
  int fast_forward_accumulator = 0;

  while (running) {
    auto window_size = window.getSize();
    auto previous_update_time = last_update_time;
    last_update_time = timeutil::now();
    double dt = timeutil::duration(previous_update_time, last_update_time);
    if (!current_menu) {
      game.advanceTime(dt);
    }
    mouse_pos = sf::Mouse::getPosition(window);
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          running = false;
          break;
        case sf::Event::GainedFocus:
          game.requestRedraw();
          break;
        case sf::Event::MouseButtonPressed:
          WindowManager::getInstance().unloadFont();
          if (mouse_pos.x < CELL_WIDTH && mouse_pos.y < CELL_WIDTH) {
            if (!current_menu) {
              changeMenu(&menu_main);
            } else {
              menuBack();
            }
            break;
          }
          if (current_menu) {
            current_menu->onClick(mouse_pos);
          } else {
            holding_x = mouse_pos.x / CELL_WIDTH;
            holding_y = mouse_pos.y / CELL_WIDTH - 1;
            if (event.mouseButton.button == sf::Mouse::Button::Left) {
              if (holding_x == mouse_pos.x / CELL_WIDTH &&
                  holding_y == mouse_pos.y / CELL_WIDTH - 1) {
                game.uncover(holding_x, holding_y);
              }
              if (button.contains((float) mouse_pos.x, (float) mouse_pos.y)) {
                changeGameInstanceNewSeed(config);
              }
            } else if (event.mouseButton.button == sf::Mouse::Button::Right) {
              game.rightClick(holding_x, holding_y);
            }
          }
          break;
        case sf::Event::KeyPressed:
          if (event.key.code == sf::Keyboard::Space) {
            switch (ai_mode) {
              case STEP:
                if (ai_type != NONE) {
                  if (!solver) {
                    updateAI();
                  }
                  if (!solver->step(game)) {
                    solver = nullptr;
                  }
                }
                break;
              case FAST_FORWARD:
                if (ai_type != NONE) {
                  if (!solver) {
                    updateAI();
                  }
                  fast_forwarding_ai = !fast_forwarding_ai;
                }
                break;
              case INSTANT:
                if (ai_type != NONE) {
                  if (!solver) {
                    updateAI();
                  }
                  while (solver->step(game));
                  solver = nullptr;
                }
                break;
            }
          } else if (event.key.code == sf::Keyboard::R) {
            changeGameInstance(config);
          } else if (event.key.code == sf::Keyboard::N) {
            changeGameInstanceNewSeed(config);
          }
          if (current_menu) {
            current_menu->onKeyPressed(event.key.code);
          }
          break;
        case sf::Event::TextEntered:
          if (current_menu) {
            current_menu->onTextEntered(event.text.unicode);
          }
          break;
      }
    }
    if (fast_forwarding_ai) {
      fast_forward_accumulator++;
      if (fast_forward_accumulator >= fast_forward_delay) {
        fast_forward_accumulator -= fast_forward_delay;
        if (solver && ai_mode == FAST_FORWARD) {
          if (!solver->step(game)) {
            solver = nullptr;
            fast_forwarding_ai = false;
          }
        } else {
          fast_forwarding_ai = false;
        }
      }
    }

    if (current_menu) {
      window.clear(Core::BACKGROUND_COLOR);
    } else {
      sf::RectangleShape background;
      background.setSize(
          {(float) window_size.x, (float) Core::CELL_WIDTH});
      background.setPosition(0, 0);
      background.setFillColor(Core::BACKGROUND_COLOR);
      window.draw(background);
    }

    sf::RectangleShape rect;
    rect.setSize({Core::CELL_WIDTH * 0.8f, Core::CELL_WIDTH * 0.8f});
    rect.setPosition({Core::CELL_WIDTH * 0.1f, Core::CELL_WIDTH * 0.1f});
    if (mouse_pos.x >= 0 && mouse_pos.y >= 0 &&
        mouse_pos.x < Core::CELL_WIDTH && mouse_pos.y < Core::CELL_WIDTH) {
      rect.setFillColor(Core::COVERED_COLOR);
    } else {
      rect.setFillColor(Core::UNCOVERED_COLOR);
    }
    window.draw(rect);

    // draw the menu
    if (current_menu) {
      sf::CircleShape triangle(Core::CELL_WIDTH / 3.f, 3);
      triangle.setFillColor(BACKGROUND_COLOR);
      triangle.setRotation(-90);
      triangle.setPosition(Core::CELL_WIDTH * 0.25f, Core::CELL_WIDTH * 0.8f);
      window.draw(triangle);
      current_menu->display(window, mouse_pos);

      // draw the game
    } else {
      sf::Text text;
      text.setFont(wm.getFont());
      text.setCharacterSize(Core::CELL_WIDTH * 3 / 4);
      text.setFillColor({255, 0, 0});

      text.setString(std::to_string(game.getRemainingMines()));
      const float TEXT_OFFSET = Core::CELL_WIDTH / 8.f;
      text.setOrigin({-TEXT_OFFSET, 0});
      text.setPosition({Core::CELL_WIDTH, 0});
      window.draw(text);

      text.setString(std::to_string(game.getTimer()));
      auto text_rect = text.getLocalBounds();
      text.setOrigin({text_rect.width + TEXT_OFFSET, 0});
      text.setPosition((float) window_size.x, 0);
      window.draw(text);

      rect.setFillColor(Core::BACKGROUND_COLOR);
      rect.setSize({Core::CELL_WIDTH * 0.6f, Core::CELL_WIDTH * 0.1333f});
      rect.setPosition({Core::CELL_WIDTH * 0.2f, Core::CELL_WIDTH * 0.2f});
      window.draw(rect);
      rect.setPosition({Core::CELL_WIDTH * 0.2f, Core::CELL_WIDTH * 0.4333f});
      window.draw(rect);
      rect.setPosition({Core::CELL_WIDTH * 0.2f, Core::CELL_WIDTH * 0.6667f});
      window.draw(rect);

      if (game.isVictory()) {
        text.setFillColor({0, 127, 0});
        text.setString("Victory");
      } else if (game.isGameOver()) {
        text.setString("Failure");
      } else {
        text.setString("Restart");
      }
      text_rect = text.getLocalBounds();
      text.setOrigin({text_rect.width / 2.f, 0});
      text.setPosition(window_size.x / 2.f, 0);
      button.left = window_size.x / 2.f - Core::CELL_WIDTH * 1.5f + 1.f;
      button.top = 1.f;
      button.width = Core::CELL_WIDTH * 3.f - 2.f;
      button.height = Core::CELL_WIDTH - 2.f;
      sf::RectangleShape button_rect({button.width, button.height});
      button_rect.setPosition({button.left, button.top});
      button_rect.setFillColor(Core::UNCOVERED_COLOR);
      window.draw(button_rect);
      window.draw(text);
      game.render(window);
    }

    if (ai_type != NONE && !current_menu) {
      sf::Text text;
      text.setFont(wm.getFont());
      text.setCharacterSize(Core::CELL_WIDTH * 3 / 4);
      if (solver) {
        text.setFillColor({0, 128, 0});
      } else {
        text.setFillColor({255, 0, 0});
      }
      text.setString("AI");
      text.setPosition(
          {Core::CELL_WIDTH * 1.7f + window_size.x / 2.f, 0});
      window.draw(text);
    } else if (current_menu && !menu_header.empty()) {
      sf::Text text;
      text.setFont(wm.getFont());
      text.setCharacterSize(Core::CELL_WIDTH * 3 / 4);
      text.setFillColor(sf::Color::Black);
      text.setString(menu_header);
      text.setPosition({Core::MENU_WIDTH * 0.12f, 0});
      window.draw(text);
    }

    window.display();

    double sleep_duration = 1 / 60.0 - dt;
    if (sleep_duration > 0) {
      std::this_thread::sleep_for(
          std::chrono::nanoseconds((int64_t) (1e9 * sleep_duration)));
    }
  }
}

void Core::changeGameInstance(MinesweeperConfig new_config) {
  game = GameInstance(new_config, seed);
  solver = nullptr;
  auto& wm = WindowManager::getInstance();
  auto& window = wm.getWindow();
  wm.updateWindow(game.getVideoMode());
  if (ai_start_random == ALWAYS) {
    ai_start_x = random() % game.getWidth();
    ai_start_y = random() % game.getHeight();
  }
  updateAI();
}

void Core::changeGameInstanceNewSeed(MinesweeperConfig new_config) {
  seed = random();
  if (ai_start_random != NEVER) {
    ai_start_x = random() % game.getWidth();
    ai_start_y = random() % game.getHeight();
  }
  changeGameInstance(new_config);
}

void Core::changeMenu(MenuList* menu) {
  menu_header.clear();
  if (current_menu) {
    current_menu->resetFocus();
  }
  previous_menu.emplace_back(current_menu);
  current_menu = menu;
}

void Core::menuBack() {
  game.requestRedraw();
  menu_header.clear();
  if (current_menu) {
    current_menu->resetFocus();
  }
  if (previous_menu.empty()) {
    current_menu = nullptr;
  } else {
    current_menu = previous_menu.back();
    previous_menu.pop_back();
  }
}

void Core::updateAI() {
  fast_forwarding_ai = false;
  switch (ai_type) {
    case NONE:
      solver = nullptr;
      break;
    case OBVIOUS_MOVES:
      solver = std::make_unique<ObviousMovesSolver>();
      break;
    case DUMB_CELL_GROUPS:
      solver = std::make_unique<DumbCellGroupsSolver>();
      break;
    case CELL_GROUPS:
      solver = std::make_unique<CellGroupsSolver>();
      break;
  }
  if (solver) {
    solver->setStart(ai_start_x, ai_start_y);
  }
}
