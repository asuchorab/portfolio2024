#include <iostream>
#include <fstream>
#include "Lexer.h"
#include "Parser.h"

int main(int argc, const char** args) {
  if (argc < 2) {
    std::cerr << "usage: " << args[0] << " <filename>\n";
    return 1;
  }
  std::ifstream ifs(args[1]);
  if (!ifs.good()) {
    std::cerr << "Error while opening source file\n";
    return 1;
  }
  std::string str;

  ifs.seekg(0, std::ios::end);
  str.reserve((unsigned int) ifs.tellg() + 1);
  ifs.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(ifs)),
             std::istreambuf_iterator<char>());
  str += '\n';
  try {
    auto tokens = luaext::lex(str);
    std::cout<<"LEXER OUTPUT\n";
    for (auto& t : tokens) {
      std::cout << t;
    }
    std::cout<<"\n----------------------\nPARSER OUTPUT\n";
    auto ast = luaext::parse(tokens);
    std::cout<<ast;
  } catch (const luaext::LexingError& e) {
    std::cerr << args[1] << ':' << e.line << ":" << e.character
    << ": Lexing error: " << e.what();
  } catch (const luaext::ParsingError& e) {
    std::cerr << args[1] << ':' << e.line << ":" << e.character
              << ": Parsing error: " << e.what();
  }
  return 0;
}
