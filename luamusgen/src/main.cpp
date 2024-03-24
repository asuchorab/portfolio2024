//
// Created by rutio on 14.08.2019.
//

#include <scriptloader/ExecutionManager.h>
#include <iostream>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: luamusgen <filename>\n";
    exit(EXIT_FAILURE);
  }
  ExecutionManager manager(argv[1]);
  manager.execute();
  return 0;
}