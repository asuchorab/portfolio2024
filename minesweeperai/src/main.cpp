#include <iostream>

#include "Core.h"
#include "WindowManager.h"

int main() {
  WindowManager::getInstance();
  Core core;
  core.run();
  WindowManager::getInstance().unloadFont();
  return 0;
}