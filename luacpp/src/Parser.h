//
// Created by Rutio on 2021-09-23.
//

#ifndef LUAEXT_PARSER_H
#define LUAEXT_PARSER_H

#include "Lexer.h"
#include <vector>
//#include <Windows.h>

namespace luaext {

enum class NodeType {
  // for initialization
  INVALID,
  // control
  CHUNK, BLOCK, LABEL,
  // lists
  VAR_LIST, NAME_LIST, EXP_LIST, PAR_LIST,
  // statements
  NOOP, ASSIGN, LOC_ASSIGN, BREAK, WHILE, REPEAT, IF, FOR_NUM, FOR, RETURN,
  FUNC_CALL, METHOD_CALL, GOTO,
  // tables
  TABLE, FIELD, FIELD_SEQ,
  // values
  NIL, TRUE_, FALSE_, NUM_INT, NUM_FLOAT, LITERAL, TRIPLE_DOT, VAR_FIELD,
  METHOD_FIELD, FUNC_DEF, METHOD_DEF, NAME,
  // binary operators
  ADD, SUB, MUL, DIV, DIV_FLOOR, EXP, MOD, BIT_AND, BIT_XOR, BIT_OR, RSHIFT,
  LSHIFT, CONCAT, LT, LE, GT, GE, EQ, NE, LOGIC_AND, LOGIC_OR,
  // unary operators
  MINUS, LOGIC_NOT, LENGTH, BIT_NOT,
  // count of node types
  NodeType_COUNT
};

static const char* NODE_TYPE_NAMES[(int) NodeType::NodeType_COUNT] = {
    // for initialization
    "INVALID",
    // control
    "CHUNK", "BLOCK", "LABEL",
    // lists
    "VAR_LIST", "NAME_LIST", "EXP_LIST", "PAR_LIST",
    // statements
    "NOOP", "ASSIGN", "LOC_ASSIGN", "BREAK", "WHILE", "REPEAT", "IF", "FOR_NUM", "FOR", "RETURN",
    "FUNC_CALL", "METHOD_CALL", "GOTO",
    // tables
    "TABLE", "FIELD", "FIELD_SEQ",
    // values
    "NIL", "TRUE", "FALSE", "NUM_INT", "NUM_FLOAT", "LITERAL", "TRIPLE_DOT", "VAR_FIELD",
    "METHOD_FIELD", "FUNC_DEF", "METHOD_DEF", "NAME",
    // binary operators
    "ADD", "SUB", "MUL", "DIV", "DIV_FLOOR", "EXP", "MOD", "BIT_AND", "BIT_XOR", "BIT_OR", "RSHIFT",
    "LSHIFT", "CONCAT", "LT", "LE", "GT", "GE", "EQ", "NE", "LOGIC_AND", "LOGIC_OR",
    // unary operators
    "MINUS", "LOGIC_NOT", "LENGTH", "BIT_NOT",
};

struct ASTNode {
  explicit ASTNode(luaext::NodeType type, const Token& token);

  void addChild(ASTNode&& node);

  NodeType type;
  const Token* token;
  std::vector<ASTNode> children;
};

std::ostream& operator<<(std::ostream& os, const ASTNode& node);

ASTNode parse(const std::vector<Token>& tokens);

class ParsingError : public std::runtime_error {
public:
  explicit ParsingError(uint32_t line, uint32_t character, const std::string& str)
      : std::runtime_error(str), line(line), character(character) {}
  uint32_t line;
  uint32_t character;
};

namespace parser {

// Functions generally follow the grammar productions
class Parser {
public:
  explicit Parser(const std::vector<Token>& tokens);

  ASTNode parse();

private:
  void throwError(const std::string& err) const;

  void advanceCurrentToken();

  bool accept(TokenType t);

  void expect(TokenType t);

  TokenType lookAhead(int position);

  bool isLookAheadToken(TokenType t, int position);

  void block(ASTNode& parent);

  bool stat(ASTNode& parent);

  bool retstat(ASTNode& parent);

  bool prefixexpstat(ASTNode& parent);

  bool label(ASTNode& parent);

  bool funcname(ASTNode& parent);

  bool namelist(ASTNode& parent);

  bool explist(ASTNode& parent);

  bool nonopexp(ASTNode& parent);

  bool binopNoAdvance(ASTNode& node);

  bool unop(ASTNode& parent);

  bool prett_nud(ASTNode& left);

  void prett_led(ASTNode& left, ASTNode& op_node, int op_bp);

  bool exp(ASTNode& parent, int rbp = 0);

  bool prefixexp(ASTNode& parent);

  void prefixexpcont(ASTNode& parent);

  bool args(ASTNode& parent);

  bool functiondef(ASTNode& parent);

  bool funcbody(ASTNode& parent);

  void parlist(ASTNode& parent);

  bool tableconstructor(ASTNode& parent);

  bool field(ASTNode& parent);

  std::vector<Token>::const_iterator token;
  TokenType token_type;
};

}

}

#endif //LUAEXT_PARSER_H
