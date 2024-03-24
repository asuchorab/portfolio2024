//
// Created by Rutio on 2021-09-23.
//

#include "Parser.h"
#include <string>

namespace luaext {

ASTNode::ASTNode(luaext::NodeType type, const luaext::Token& token)
    : type(type), token(&token) {}

void ASTNode::addChild(luaext::ASTNode&& node) {
  children.emplace_back(std::move(node));
}

inline void printNode(std::ostream& os, const ASTNode& node,
                      std::vector<int>& display_modes) {
  int last_mode = display_modes.back();
  display_modes.back() = 2;
  for (int i : display_modes) {
    if (i == 0) {
      os << "  ";
    } else if (i == 1) {
      os << "| ";
    } else if (i == 2) {
      os << "+-";
    }
  }
  display_modes.back() = last_mode;
  os << NODE_TYPE_NAMES[(int) node.type];
  if (node.type == NodeType::NUM_INT) {
    os << ": " << node.token->value.value_int;
  } else if (node.type == NodeType::NUM_FLOAT) {
    os << ": " << node.token->value.value_float;
  } else if (node.type == NodeType::LITERAL || node.type == NodeType::NAME
             || node.type == NodeType::GOTO || node.type == NodeType::LABEL) {
    os << ": " << node.token->value_string;
  }
  os << "\n";
  display_modes.push_back(1);
  for (size_t i = 0; i < node.children.size(); ++i) {
    auto& child = node.children[i];
    if (i == node.children.size() - 1) {
      display_modes.back() = 0;
    }
    printNode(os, child, display_modes);
  }
  display_modes.pop_back();
}

std::ostream& operator<<(std::ostream& os, const ASTNode& node) {
  std::vector<int> display_modes(1, 0);
  printNode(os, node, display_modes);
  return os;
}

ASTNode parse(const std::vector<Token>& tokens) {
  parser::Parser parser(tokens);
  return parser.parse();
}

namespace parser {

Parser::Parser(const std::vector<Token>& tokens)
    : token(tokens.begin()),
      token_type(token->token_type) {}

ASTNode Parser::parse() {
  ASTNode ret(NodeType::CHUNK, *token);
  block(ret);
  if (token_type != TokenType::END_MARKER) {
    throwError(std::string("chunk: unexpected token of type ")
               + token->getTypeString());
  }
  return ret;
}

void Parser::throwError(const std::string& err) const {
  throw ParsingError(token->line, token->character, err);
}

void Parser::advanceCurrentToken() {
  ++token;
  token_type = token->token_type;
}

bool Parser::accept(TokenType t) {
  if (token_type == t) {
    advanceCurrentToken();
    return true;
  } else {
    return false;
  }
}

void Parser::expect(TokenType t) {
  if (token_type == t) {
    advanceCurrentToken();
  } else {
    throwError(std::string("Expected ")
               + getTokenTypeString(t));
  }
}

TokenType Parser::lookAhead(int position) {
  auto it = token;
  while (position > 0) {
    if (token->token_type == TokenType::END_MARKER) {
      return TokenType::END_MARKER;
    } else {
      ++it;
    }
    position--;
  }
  return it->token_type;
}

bool Parser::isLookAheadToken(TokenType t, int position) {
  return lookAhead(position) == t;
}

void Parser::block(ASTNode& parent) {
  ASTNode node(NodeType::BLOCK, *token);
  while (stat(node));
  retstat(node);
  parent.addChild(std::move(node));
}

bool Parser::stat(ASTNode& parent) {
  auto first_token = token;
  if (prefixexpstat(parent) || label(parent)) {
    return true;
  } else if (accept(TokenType::SEMICOLON)) {
    parent.addChild(ASTNode(NodeType::NOOP, *first_token));
    return true;
  } else if (accept(TokenType::BREAK)) {
    ASTNode break_node(NodeType::BREAK, *token);
    parent.addChild(std::move(break_node));
    return true;
  } else if (accept(TokenType::GOTO)) {
    ASTNode goto_node(NodeType::GOTO, *token);
    expect(TokenType::NAME);
    parent.addChild(std::move(goto_node));
    return true;
  } else if (accept(TokenType::DO)) {
    block(parent);
    expect(TokenType::END);
    return true;
  } else if (accept(TokenType::WHILE)) {
    ASTNode while_node(NodeType::WHILE, *first_token);
    if (!exp(while_node)) {
      throwError("Expected an expression after 'while");
    }
    expect(TokenType::DO);
    block(while_node);
    expect(TokenType::END);
    parent.addChild(std::move(while_node));
    return true;
  } else if (accept(TokenType::REPEAT)) {
    ASTNode repeat_node(NodeType::REPEAT, *first_token);
    block(repeat_node);
    expect(TokenType::UNTIL);
    if (!exp(repeat_node)) {
      throwError("Expected an expression after 'until'");
    }
    parent.addChild(std::move(repeat_node));
    return true;
  } else if (accept(TokenType::IF)) {
    ASTNode if_node(NodeType::IF, *first_token);
    if (!exp(if_node)) {
      throwError("Expected an expression after 'if'");
    }
    expect(TokenType::THEN);
    block(if_node);
    while (accept(TokenType::ELSEIF)) {
      if (!exp(if_node)) {
        throwError("Expected an expression after 'elseif'");
      }
      expect(TokenType::THEN);
      block(if_node);
    }
    if (accept(TokenType::ELSE)) {
      block(if_node);
    }
    expect(TokenType::END);
    parent.addChild(std::move(if_node));
    return true;
  } else if (accept(TokenType::FOR)) {
    if (isLookAheadToken(TokenType::ASSIGN, 1)) {
      ASTNode for_node(NodeType::FOR_NUM, *first_token);
      ASTNode for_name(NodeType::NAME, *token);
      expect(TokenType::NAME);
      expect(TokenType::ASSIGN);
      for_node.addChild(std::move(for_name));
      if (!exp(for_node)) {
        throwError("Expected an expression after '='");
      }
      expect(TokenType::COMMA);
      if (!exp(for_node)) {
        throwError("Expected an expression after ','");
      }
      if (accept(TokenType::COMMA)) {
        if (!exp(for_node)) {
          throwError("Expected an expression after ','");
        }
      }
      expect(TokenType::DO);
      block(for_node);
      expect(TokenType::END);
      parent.addChild(std::move(for_node));
      return true;
    } else {
      ASTNode for_node(NodeType::FOR, *first_token);
      if (!namelist(for_node)) {
        throwError("Expected a name list");
      }
      expect(TokenType::IN_);
      if (!explist(for_node)) {
        throwError("Expected an expression list");
      }
      expect(TokenType::DO);
      block(for_node);
      expect(TokenType::END);
      return true;
    }
  } else if (accept(TokenType::FUNCTION)) {
    ASTNode assign(NodeType::ASSIGN, *first_token);
    ASTNode var_list(NodeType::VAR_LIST, *first_token);
    if (!funcname(var_list)) {
      throwError("Expected function name");
    }
    bool is_method = var_list.children.back().type == NodeType::METHOD_FIELD;
    var_list.children.back().type = NodeType::VAR_FIELD;
    assign.addChild(std::move(var_list));
    ASTNode exp_list(NodeType::EXP_LIST, *token);
    if (!funcbody(exp_list)) {
      throwError("Expected function body");
    }
    if (is_method) {
      exp_list.children.back().type = NodeType::METHOD_DEF;
    }
    assign.addChild(std::move(exp_list));
    parent.addChild(std::move(assign));
    return true;
  } else if (accept(TokenType::LOCAL)) {
    ASTNode assign(NodeType::LOC_ASSIGN, *first_token);
    if (accept(TokenType::FUNCTION)) {
      ASTNode name_list(NodeType::NAME_LIST, *token);
      ASTNode name(NodeType::NAME, *token);
      expect(TokenType::NAME);
      name_list.addChild(std::move(name));
      assign.addChild(std::move(name_list));
      ASTNode exp_list(NodeType::EXP_LIST, *token);
      if (!funcbody(exp_list)) {
        throwError("Expected function body");
      }
      assign.addChild(std::move(exp_list));
      parent.addChild(std::move(assign));
      return true;
    } else {
      if (!namelist(assign)) {
        throwError("Expected a name list");
      }
      if (accept(TokenType::ASSIGN)) {
        if (!explist(assign)) {
          throwError("Expected an expression list");
        }
      } else {
        assign.addChild(ASTNode(NodeType::EXP_LIST, *token));
      }
      parent.addChild(std::move(assign));
      return true;
    }
  }
  return false;
}

bool Parser::retstat(ASTNode& parent) {
  auto first_token = token;
  if (accept(TokenType::RETURN)) {
    parent.addChild(ASTNode(NodeType::RETURN, *first_token));
    explist(parent);
    accept(TokenType::SEMICOLON);
    return true;
  }
  return false;
}

bool Parser::prefixexpstat(ASTNode& parent) {
  ASTNode assign(NodeType::ASSIGN, *token);
  ASTNode var_list(NodeType::VAR_LIST, *token);
  if (prefixexp(var_list)) {
    auto& node = var_list.children.back();
    if (node.type == NodeType::FUNC_CALL
        || node.type == NodeType::METHOD_CALL) {
      parent.addChild(std::move(node));
      return true;
    } else if (node.type == NodeType::VAR_FIELD
               || node.type == NodeType::NAME) {
      while (true) {
        if (token_type != TokenType::COMMA) {
          break;
        }
        advanceCurrentToken();
        if (!prefixexp(var_list)) {
          throwError("Expected a variable in varlist");
        }
        auto& var_node = var_list.children.back();
        if (var_node.type != NodeType::VAR_FIELD
            && var_node.type != NodeType::NAME) {
          throwError("Expression in varlist isn't a variable");
        }
      }
      assign.addChild(std::move(var_list));
      if (token_type != TokenType::ASSIGN) {
        throwError("Expected '=' after varlist");
      }
      advanceCurrentToken();
      if (!explist(assign)) {
        throwError("Expected an expression list after '='");
      }
      parent.addChild(std::move(assign));
      return true;
    }
  }
  return false;
}

bool Parser::label(ASTNode& parent) {
  if (!accept(TokenType::DOUBLE_COLON)) {
    return false;
  }
  ASTNode ret(NodeType::LABEL, *token);
  expect(TokenType::NAME);
  expect(TokenType::DOUBLE_COLON);
  parent.addChild(std::move(ret));
  return true;
}

bool Parser::funcname(ASTNode& parent) {
  auto first_token = token;
  if (accept(TokenType::NAME)) {
    ASTNode ret(NodeType::NAME, *first_token);
    while (accept(TokenType::DOT)) {
      ASTNode new_root(NodeType::VAR_FIELD, *first_token);
      ASTNode field_name(NodeType::NAME, *token);
      expect(TokenType::NAME);
      new_root.addChild(std::move(ret));
      new_root.addChild(std::move(field_name));
      ret = std::move(new_root);
    }
    if (accept(TokenType::COLON)) {
      ASTNode new_root(NodeType::METHOD_FIELD, *first_token);
      ASTNode field_name(NodeType::NAME, *token);
      expect(TokenType::NAME);
      new_root.addChild(std::move(ret));
      new_root.addChild(std::move(field_name));
      ret = std::move(new_root);
    }
    parent.addChild(std::move(ret));
    return true;
  }
  return false;
}

bool Parser::namelist(ASTNode& parent) {
  auto first_token = token;
  if (accept(TokenType::NAME)) {
    ASTNode name_list(NodeType::NAME_LIST, *first_token);
    name_list.addChild(ASTNode(NodeType::NAME, *first_token));
    while (accept(TokenType::COMMA)) {
      name_list.addChild(ASTNode(NodeType::NAME, *token));
      expect(TokenType::NAME);
    }
    parent.addChild(std::move(name_list));
    return true;
  }
  return false;
}

bool Parser::explist(ASTNode& parent) {
  auto first_token = token;
  ASTNode exp_list(NodeType::EXP_LIST, *first_token);
  if (exp(exp_list)) {
    while (accept(TokenType::COMMA)) {
      if (!exp(exp_list)) {
        throwError("Expected an expression");
      }
    }
    parent.addChild(std::move(exp_list));
    return true;
  }
  return false;
}

static int BP[(int) NodeType::NodeType_COUNT] = {
    // for initialization
    0,
    // control
    0, 0, 0,
    // lists
    0, 0, 0, 0,
    // statements
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,
    // tables
    0, 0, 0,
    // values
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0,
    // binary operators
    90, 90, 100, 100, 100, 120, 100, 60, 50, 40, 70,
    70, 80, 30, 30, 30, 30, 30, 30, 20, 10,
    // unary operators
    110, 110, 110, 110,
};

static NodeType NONOP_MAP[(int) TokenType::TokenType_COUNT] = {
    // keywords
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::FALSE_, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::NIL, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::TRUE_, NodeType::INVALID, NodeType::INVALID,
    // symbols
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::TRIPLE_DOT, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    // literals
    NodeType::NUM_INT, NodeType::NUM_FLOAT, NodeType::LITERAL,
    // identifiers (function/variable name(
    NodeType::INVALID,
    // end marker, signal for the parser that there is no more tokens
    NodeType::INVALID,
};

static NodeType BINOP_MAP[(int) TokenType::TokenType_COUNT] = {
    // keywords
    NodeType::LOGIC_AND, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::LOGIC_OR, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    // symbols
    NodeType::ADD, NodeType::SUB, NodeType::MUL, NodeType::DIV, NodeType::DIV_FLOOR, NodeType::EXP, NodeType::MOD,
    NodeType::BIT_AND,
    NodeType::BIT_XOR, NodeType::BIT_OR, NodeType::RSHIFT, NodeType::LSHIFT, NodeType::CONCAT, NodeType::LT,
    NodeType::LE, NodeType::GT, NodeType::GE, NodeType::EQ, NodeType::NE, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    // literals
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    // identifiers (function/variable name(
    NodeType::INVALID,
    // end marker, signal for the parser that there is no more tokens
    NodeType::INVALID,
};

static NodeType UNOP_MAP[(int) TokenType::TokenType_COUNT] = {
    // keywords
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::LOGIC_NOT, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    // symbols
    NodeType::INVALID, NodeType::MINUS, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID,
    NodeType::BIT_NOT, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::LENGTH,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID,
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    // literals
    NodeType::INVALID, NodeType::INVALID, NodeType::INVALID,
    // identifiers (function/variable name(
    NodeType::INVALID,
    // end marker, signal for the parser that there is no more tokens
    NodeType::INVALID,
};

bool Parser::nonopexp(ASTNode& parent) {
  NodeType node_type = NONOP_MAP[(int) token_type];
  if (node_type != NodeType::INVALID) {
    parent.addChild(ASTNode(node_type, *token));
    advanceCurrentToken();
    return true;
  } else if (functiondef(parent)
             || prefixexp(parent)
             || tableconstructor(parent)) {
    return true;
  }
  return false;
}

bool Parser::binopNoAdvance(ASTNode& node) {
  auto node_type = BINOP_MAP[(int) token_type];
  if (node_type != NodeType::INVALID) {
    node.type = node_type;
    return true;
  }
  return false;
}

bool Parser::unop(ASTNode& parent) {
  auto node_type = UNOP_MAP[(int) token_type];
  if (node_type != NodeType::INVALID) {
    parent.addChild(ASTNode(node_type, *token));
    advanceCurrentToken();
    return true;
  }
  return false;
}

bool Parser::prett_nud(ASTNode& parent) {
  if (nonopexp(parent)) {
    return true;
  } else if (unop(parent)) {
    auto& node = parent.children.back();
    int bp = BP[(int) node.type];
    if (!exp(node, bp)) {
      throwError("Expected an expression after an unary operator");
    }
    return true;
  } else {
    return false;
  }
}

void Parser::prett_led(ASTNode& left, ASTNode& op_node, int op_bp) {
  op_node.addChild(std::move(left));
  left = std::move(op_node);
  if (left.type == NodeType::CONCAT || left.type == NodeType::EXP) {
    op_bp--;
  }
  if (!exp(left, op_bp)) {
    throwError("Expected an expression after a binary operator");
  }
}

bool Parser::exp(ASTNode& parent, int rbp) {
  if (prett_nud(parent)) {
    while (true) {
      ASTNode op_node(NodeType::INVALID, *token);
      if (binopNoAdvance(op_node)) {
        //auto op_node = std::move(parent.children.back());
        int bp = BP[(int) op_node.type];
        if (bp > rbp) {
          //parent.children.pop_back();
          auto& left = parent.children.back();
          advanceCurrentToken();
          prett_led(left, op_node, bp);
        } else {
          //parent.children.pop_back();
          break;
        }
      } else {
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}

bool Parser::prefixexp(ASTNode& parent) {
  auto first_token = token;
  if (accept(TokenType::NAME)) {
    parent.addChild(ASTNode(NodeType::NAME, *first_token));
    prefixexpcont(parent.children.back());
    return true;
  } else if (accept(TokenType::LPAREN)) {
    if (!exp(parent)) {
      throwError("Expected an expression");
    }
    expect(TokenType::RPAREN);
    prefixexpcont(parent.children.back());
    return true;
  }
  return false;
}

void Parser::prefixexpcont(ASTNode& parent) {
  auto first_token = token;
  if (accept(TokenType::LBRACKET)) {
    auto left(std::move(parent));
    parent = ASTNode(NodeType::VAR_FIELD, *first_token);
    parent.addChild(std::move(left));
    if (!exp(parent)) {
      throwError("Expected an expression");
    }
    expect(TokenType::RBRACKET);
    prefixexpcont(parent);
  } else if (accept(TokenType::DOT)) {
    auto left(std::move(parent));
    parent = ASTNode(NodeType::VAR_FIELD, *first_token);
    parent.addChild(std::move(left));
    parent.addChild(ASTNode(NodeType::LITERAL, *token));
    expect(TokenType::NAME);
    prefixexpcont(parent);
  } else if (args(parent)) {
    auto right(std::move(parent.children.back()));
    parent.children.pop_back();
    auto left(std::move(parent));
    parent = ASTNode(NodeType::FUNC_CALL, *first_token);
    parent.addChild(std::move(left));
    parent.addChild(std::move(right));
    prefixexpcont(parent);
  } else if (accept(TokenType::COLON)) {
    auto left(std::move(parent));
    parent = ASTNode(NodeType::METHOD_CALL, *first_token);
    parent.addChild(std::move(left));
    parent.addChild(ASTNode(NodeType::NAME, *token));
    advanceCurrentToken();
    if (!args(parent)) {
      throwError("Expected arguments for the method call");
    }
    prefixexpcont(parent);
  }
}

bool Parser::args(ASTNode& parent) {
  auto first_token = token;
  if (accept(TokenType::LPAREN)) {
    if (!explist(parent)) {
      parent.addChild(ASTNode(NodeType::EXP_LIST, *token));
    }
    expect(TokenType::RPAREN);
    return true;
  } else if (tableconstructor(parent)) {
    return true;
  } else if (accept(TokenType::LITERAL)) {
    parent.addChild(ASTNode(NodeType::LITERAL, *first_token));
    return true;
  }
  return false;
}

bool Parser::functiondef(ASTNode& parent) {
  if (accept(TokenType::FUNCTION)) {
    if (!funcbody(parent)) {
      throwError("Expected a function body");
    }
    return true;
  }
  return false;
}

bool Parser::funcbody(ASTNode& parent) {
  if (accept(TokenType::LPAREN)) {
    ASTNode func_def(NodeType::FUNC_DEF, *token);
    parlist(func_def);
    expect(TokenType::RPAREN);
    block(func_def);
    expect(TokenType::END);
    parent.addChild(std::move(func_def));
    return true;
  }
  return false;
}

void Parser::parlist(ASTNode& parent) {
  ASTNode par_list(NodeType::PAR_LIST, *token);
  auto first_token = token;
  if (accept(TokenType::NAME)) {
    ASTNode first_name(NodeType::NAME, *first_token);
    par_list.addChild(std::move(first_name));
    while (accept(TokenType::COMMA)) {
      auto temp_token = token;
      if (accept(TokenType::NAME)) {
        par_list.addChild(ASTNode(NodeType::NAME, *temp_token));
      } else if (accept(TokenType::TRIPLE_DOT)) {
        par_list.addChild(ASTNode(NodeType::TRIPLE_DOT, *temp_token));
        break;
      } else {
        throwError("Expected an identifier or '...'");
      }
    }
  } else if (accept(TokenType::TRIPLE_DOT)) {
    ASTNode triple_dot(NodeType::TRIPLE_DOT, *first_token);
    par_list.addChild(std::move(triple_dot));
  }
  parent.addChild(std::move(par_list));
}

bool Parser::tableconstructor(ASTNode& parent) {
  auto first_token = token;
  if (accept(TokenType::LBRACE)) {
    ASTNode table(NodeType::TABLE, *first_token);
    while (field(table)) {
      if (!(accept(TokenType::COMMA) || accept(TokenType::SEMICOLON))) {
        break;
      }
    }
    expect(TokenType::RBRACE);
    parent.addChild(std::move(table));
    return true;
  }
  return false;
}

bool Parser::field(ASTNode& parent) {
  auto first_token = token;
  if (accept(TokenType::LBRACKET)) {
    ASTNode field(NodeType::FIELD, *first_token);
    if (!exp(field)) {
      throwError("Expected an expression");
    }
    expect(TokenType::RBRACKET);
    expect(TokenType::ASSIGN);
    if (!exp(field)) {
      throwError("Expected an expression");
    }
    parent.addChild(std::move(field));
    return true;
  } else if (accept(TokenType::NAME)) {
    ASTNode field(NodeType::FIELD, *first_token);
    field.addChild(ASTNode(NodeType::LITERAL, *first_token));
    expect(TokenType::ASSIGN);
    if (!exp(field)) {
      throwError("Expected an expression");
    }
    parent.addChild(std::move(field));
    return true;
  } else {
    ASTNode field_seq(NodeType::FIELD_SEQ, *first_token);
    if (exp(field_seq)) {
      parent.addChild(std::move(field_seq));
      return true;
    }
  }
  return false;
}

}

}