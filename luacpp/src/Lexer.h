//
// Created by Rutio on 2021-09-22.
//

#ifndef LUAEXT_LEXER_H
#define LUAEXT_LEXER_H

#include <vector>
#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace luaext {

enum class TokenType {
  // keywords
  AND, BREAK, DO, ELSE, ELSEIF, END, FALSE_, FOR, FUNCTION, GOTO, IF, IN_,
  LOCAL, NIL, NOT, OR, REPEAT, RETURN, THEN, TRUE_, UNTIL, WHILE,
  // symbols
  PLUS, MINUS, ASTERISK, SLASH, DOUBLE_SLASH, CARET, PERCENT, AMPERSAND,
  TILDE, PIPE, RSHIFT, LSHIFT, CONCAT, LT, LE, GT, GE, EQ, NE, HASH,
  COMMA, SEMICOLON, COLON, DOT, TRIPLE_DOT, DOUBLE_COLON, LPAREN, RPAREN,
  LBRACKET, RBRACKET, LBRACE, RBRACE, ASSIGN,
  // literals
  NUM_INT, NUM_FLOAT, LITERAL,
  // identifiers (function/variable name)
  NAME,
  // end marker, signal for the parser that there is no more tokens
  END_MARKER,
  // count of token types
  TokenType_COUNT
};

static const char* TOKEN_TYPE_NAMES[(int) TokenType::TokenType_COUNT] = {
    // keywords
    "'and'", "'break'", "'do'", "'else'", "'elseif'", "'end'", "'false'", "'for'", "'function'", "GOTO", "IF", "IN",
    "'local'", "'nil'", "'not'", "'or'", "'repeat'", "'return'", "'then'", "'true'", "'until'", "'while'",
    // symbols
    "'+'", "'-'", "'*'", "'/'", "'//'", "'^'", "'%'", "'&'",
    "'~'", "'|'", "'>>'", "'<<'", "'..'", "'<'", "'<='", "'>'", "'>=", "'=='", "'~='", "'#'",
    "','", "';'", "':'", "'.'", "'...'", "'::'", "'('", "')'",
    "'['", "']'", "'{'", "'}'", "'='",
    // literals
    "NUM_INT", "NUM_FLOAT", "LITERAL",
    // identifiers (function/variable name)
    "ID",
    // end marker, signal for the parser that there is no more tokens
    "END_MARKER",
};

struct Token {
  union {
    double value_float;
    int64_t value_int;
  } value;
  std::string value_string;
  uint32_t line;
  uint32_t character;
  TokenType token_type;

  const char* getTypeString() const;
};

std::ostream& operator<<(std::ostream& os, const Token& token);

std::vector<Token> lex(const std::string& source);

const char* getTokenTypeString(TokenType type);

class LexingError : public std::runtime_error {
public:
  explicit LexingError(uint32_t line, uint32_t character, const std::string& str)
      : std::runtime_error(str), line(line), character(character) {}
  uint32_t line;
  uint32_t character;
};

namespace lexer {
class EOFException : public std::runtime_error {
public:
  explicit EOFException() : std::runtime_error("EOF") {}
};

class Lexer {
public:
  explicit Lexer(const std::string& source);

  std::vector<Token> getTokens();

private:
  char processWord();

  char processNumber();

  char processSpecial();

  char processStringLiteral();

  char processRawStringLiteral();

  char processComment();

  char processMultilineComment();

  char getNextCharacter();

  void beginToken();

  void throwError(const std::string& err) const;

  void pushToken(TokenType type);

  void pushIntNumeralToken(int64_t value);

  void pushFloatNumeralToken(double value);

  void pushStringToken(TokenType type, std::string str);

  const std::string& source;
  std::vector<Token> tokens;
  uint32_t index = 0;
  uint32_t line = 1;
  uint32_t character = 1;
  uint32_t token_index = 0;
  uint32_t token_line = 0;
  uint32_t token_character = 0;
  char last_char = '\0';
  bool began_multiline_comment = false;
  bool began_raw_string = false;
};

}

}

#endif //LUAEXT_LEXER_H
