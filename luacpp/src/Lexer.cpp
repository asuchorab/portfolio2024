//
// Created by Rutio on 2021-09-22.
//

#include "Lexer.h"
#include <regex>
#include <map>

namespace luaext {

inline bool isAlphaOrUnderscore(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

inline bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

inline bool isHexDigit(char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

inline bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == '\v';
}

const char* Token::getTypeString() const {
  return TOKEN_TYPE_NAMES[(int) token_type];
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << token.line << ':'
     << token.character << "; "
     << TOKEN_TYPE_NAMES[(int) token.token_type];
  if (token.token_type == TokenType::NUM_INT) {
    os << "; value: " << token.value.value_int;
  } else if (token.token_type == TokenType::NUM_FLOAT) {
    os << "; value: " << token.value.value_float;
  }
  if (!token.value_string.empty()) {
    os << "; value: " << std::quoted(token.value_string);
  }
  os << "\n";
  return os;
}

std::vector<Token> lex(const std::string& source) {
  lexer::Lexer lexer(source);
  return lexer.getTokens();
}

const char* getTokenTypeString(TokenType type) {
  return TOKEN_TYPE_NAMES[(int) type];
}

namespace lexer {

Lexer::Lexer(const std::string& source) : source(source) {
}

std::vector<Token> Lexer::getTokens() {
  try {
    char c = getNextCharacter();
    while (c) {
      if (isWhitespace(c)) {
        c = getNextCharacter();
        continue;
      }
      if (isAlphaOrUnderscore(c)) {
        beginToken();
        c = processWord();
      } else if (isDigit(c)) {
        beginToken();
        c = processNumber();
      } else if (c == '"' || c == '\'') {
        beginToken();
        c = processStringLiteral();
      } else {
        beginToken();
        c = processSpecial();
      }
    }
  } catch (const EOFException&) {
    pushToken(TokenType::END_MARKER);
  }
  return std::move(tokens);
}

char Lexer::processWord() {
  static std::map<std::string, TokenType> KEYWORD_MAP = {
      {"and",      TokenType::AND},
      {"break",    TokenType::BREAK},
      {"do",       TokenType::DO},
      {"else",     TokenType::ELSE},
      {"elseif",   TokenType::ELSEIF},
      {"end",      TokenType::END},
      {"false",    TokenType::FALSE_},
      {"for",      TokenType::FOR},
      {"function", TokenType::FUNCTION},
      {"goto",     TokenType::GOTO},
      {"if",       TokenType::IF},
      {"in",       TokenType::IN_},
      {"local",    TokenType::LOCAL},
      {"nil",      TokenType::NIL},
      {"not",      TokenType::NOT},
      {"or",       TokenType::OR},
      {"repeat",   TokenType::REPEAT},
      {"return",   TokenType::RETURN},
      {"then",     TokenType::THEN},
      {"true",     TokenType::TRUE_},
      {"until",    TokenType::UNTIL},
      {"while",    TokenType::WHILE}
  };
  char c;
  while (true) {
    c = getNextCharacter();
    if (!(isAlphaOrUnderscore(c) || isDigit(c))) {
      break;
    }
  }
  std::string word = source.substr(token_index, index - token_index - 1);
  auto keyword_found = KEYWORD_MAP.find(word);
  if (keyword_found != KEYWORD_MAP.end()) {
    pushToken(keyword_found->second);
  } else {
    pushStringToken(TokenType::NAME, std::move(word));
  }
  return c;
}

char Lexer::processNumber() {
  bool began_exponent = false;
  bool just_began_exponent = true;
  bool is_hex = false;
  bool is_fractional = false;
  char c;
  while (true) {
    c = getNextCharacter();
    if (began_exponent) {
      if (!isDigit(c)) {
        if (c != '-' || !just_began_exponent) {
          break;
        }
      }
      just_began_exponent = false;
    } else {
      if (c == 'x' || c == 'X') {
        is_hex = true;
      } else if (is_hex ? (c == 'p' || c == 'P') : (c == 'e' || c == 'E')) {
        began_exponent = true;
      } else if (c == '.') {
        is_fractional = true;
      } else if (is_hex ? !isHexDigit(c) : !isDigit(c)) {
        break;
      }
    }
  }
  std::string numeral = source.substr(token_index, index - token_index - 1);
  static std::regex hex_regex("0x([0-9A-Fa-f]+)\\.?([0-9A-Fa-f]*)([pP](-?[0-9]*))?");
  if (is_hex) {
    std::smatch match;
    if (std::regex_match(numeral, match, hex_regex)) {
      try {
        int64_t base = std::stoll(match[1].str(), nullptr, 16);
        std::string fractional = match[2];
        std::string exponent = match[4];
        if (fractional.empty() && exponent.empty()) {
          pushIntNumeralToken(base);
        } else {
          throwError("hex float numerals are not supported");
        }
      } catch (const std::invalid_argument&) {
        throwError("invalid hex numeral " + numeral);
      } catch (const std::out_of_range&) {
        throwError("hex numeral out of range " + numeral);
      }
    } else {
      throwError("invalid hex numeral " + numeral);
    }
  } else {
    if (is_fractional || began_exponent) {
      try {
        double val = std::stod(numeral);
        pushFloatNumeralToken(val);
      } catch (const std::invalid_argument&) {
        throwError("invalid float numeral: " + numeral);
      } catch (const std::out_of_range&) {
        throwError("float numeral out of range: " + numeral);
      }
    } else {
      try {
        int64_t val = std::stoll(numeral);
        pushIntNumeralToken(val);
      } catch (const std::invalid_argument&) {
        throwError("invalid int numeral: " + numeral);
      } catch (const std::out_of_range&) {
        throwError("int numeral out of range: " + numeral);
      }
    }
  }
  return c;
}

char Lexer::processSpecial() {
  char c;
  switch (last_char) {
    case '+':
      pushToken(TokenType::PLUS);
      return getNextCharacter();
    case '*':
      pushToken(TokenType::ASTERISK);
      return getNextCharacter();
    case '^':
      pushToken(TokenType::CARET);
      return getNextCharacter();
    case '%':
      pushToken(TokenType::PERCENT);
      return getNextCharacter();
    case '&':
      pushToken(TokenType::AMPERSAND);
      return getNextCharacter();
    case '|':
      pushToken(TokenType::PIPE);
      return getNextCharacter();
    case '#':
      pushToken(TokenType::HASH);
      return getNextCharacter();
    case ',':
      pushToken(TokenType::COMMA);
      return getNextCharacter();
    case ';':
      pushToken(TokenType::SEMICOLON);
      return getNextCharacter();
    case '(':
      pushToken(TokenType::LPAREN);
      return getNextCharacter();
    case ')':
      pushToken(TokenType::RPAREN);
      return getNextCharacter();
    case '{':
      pushToken(TokenType::LBRACE);
      return getNextCharacter();
    case '}':
      pushToken(TokenType::RBRACE);
      return getNextCharacter();
    case ']':
      pushToken(TokenType::RBRACKET);
      return getNextCharacter();
    case '[':
      c = getNextCharacter();
      if (c == '=' || c == '[') {
        return processRawStringLiteral();
      } else {
        pushToken(TokenType::LBRACKET);
        return c;
      }
    case '-':
      c = getNextCharacter();
      if (c == '-') {
        return processComment();
      } else {
        pushToken(TokenType::MINUS);
        return c;
      }
    case '=':
      c = getNextCharacter();
      if (c == '=') {
        pushToken(TokenType::EQ);
        return getNextCharacter();
      } else {
        pushToken(TokenType::ASSIGN);
        return c;
      }
    case '/':
      c = getNextCharacter();
      if (c == '/') {
        pushToken(TokenType::DOUBLE_SLASH);
        return getNextCharacter();
      } else {
        pushToken(TokenType::SLASH);
        return c;
      }
    case '~':
      c = getNextCharacter();
      if (c == '=') {
        pushToken(TokenType::NE);
        return getNextCharacter();
      } else {
        pushToken(TokenType::TILDE);
        return c;
      }
    case '>':
      c = getNextCharacter();
      if (c == '>') {
        pushToken(TokenType::RSHIFT);
        return getNextCharacter();
      } else if (c == '=') {
        pushToken(TokenType::GE);
        return getNextCharacter();
      } else {
        pushToken(TokenType::GT);
        return c;
      }
    case '<':
      c = getNextCharacter();
      if (c == '<') {
        pushToken(TokenType::LSHIFT);
        return getNextCharacter();
      } else if (c == '=') {
        pushToken(TokenType::LE);
        return getNextCharacter();
      } else {
        pushToken(TokenType::LT);
        return c;
      }
    case ':':
      c = getNextCharacter();
      if (c == ':') {
        pushToken(TokenType::DOUBLE_COLON);
        return getNextCharacter();
      } else {
        pushToken(TokenType::COLON);
        return c;
      }
    case '.':
      c = getNextCharacter();
      if (c == '.') {
        c = getNextCharacter();
        if (c == '.') {
          pushToken(TokenType::TRIPLE_DOT);
          return getNextCharacter();
        } else {
          pushToken(TokenType::CONCAT);
          return c;
        }
      } else {
        pushToken(TokenType::DOT);
        return c;
      }
    default:
      throwError("Unexpected character " + std::to_string((uint8_t) last_char));
  }
  return 0;
}

char Lexer::processStringLiteral() {
  char delimiter = last_char;
  std::string result;
  bool skipping_whitespace = false;
  bool skipLoadingCharacter = false;
  char c = 0;
  while (true) {
    if (skipLoadingCharacter) {
      skipLoadingCharacter = false;
    } else {
      c = getNextCharacter();
    }
    if (c == delimiter) {
      pushStringToken(TokenType::LITERAL, result);
      break;
    }
    if (c == '\\') {
      c = getNextCharacter();
      switch (c) {
        case 'a':
          result += '\a';
          break;
        case 'b':
          result += '\b';
          break;
        case 'f':
          result += '\f';
          break;
        case 'n':
          result += '\n';
          break;
        case 'r':
          result += '\r';
          break;
        case 't':
          result += '\t';
          break;
        case 'v':
          result += '\v';
          break;
        case '\\':
          result += '\\';
          break;
        case '"':
          result += '"';
          break;
        case '\'':
          result += '\'';
          break;
        case 'z':
          skipping_whitespace = true;
          break;
        case 'x': {
          std::string hex;
          hex += getNextCharacter();
          hex += getNextCharacter();
          try {
            int64_t val = std::stoll(hex, nullptr, 16);
            result += (char) val;
          } catch (const std::invalid_argument&) {
            throwError("Invalid hex code in \\x");
          }
          break;
        }
        case 'u':
          throwError("Unicode escape codes are not supported");
        default:
          if (isDigit(c)) {
            std::string numeral;
            numeral += c;
            c = getNextCharacter();
            if (isDigit(c)) {
              numeral += c;
              c = getNextCharacter();
              if (isDigit(c)) {
                numeral += c;
                c = getNextCharacter();
              }
            }
            skipLoadingCharacter = true;
            int64_t val = std::stoll(numeral);
            if (val > 255) {
              throwError(std::string("Invalid byte escape value") + numeral);
            }
            result += (char) val;
          } else {
            throwError(std::string("Invalid escape character ") + c);
          }
      }
    } else {
      if (skipping_whitespace) {
        if (!isWhitespace(c)) {
          skipping_whitespace = false;
          result += c;
        }
      } else {
        if (c == '\n' || c == 'r') {
          throwError("Unterminated string literal");
        }
        result += c;
      }
    }
  }
  return getNextCharacter();
}

char Lexer::processRawStringLiteral() {
  began_raw_string = true;
  char c = last_char;
  int level = 0;
  while (true) {
    if (c == '=') {
      level++;
    } else if (c == '[') {
      break;
    } else {
      throwError("Invalid raw string delimiter");
    }
    c = getNextCharacter();
  }
  uint32_t begin_index = index;
  uint32_t end_index = index;
  int consecutive_eq = 0;
  bool closing = false;
  while (true) {
    if (closing) {
      if (c == '=') {
        consecutive_eq++;
      } else if (c == ']' && consecutive_eq == level) {
        pushStringToken(
            TokenType::LITERAL,
            source.substr(begin_index, end_index - begin_index - 1));
        break;
      } else {
        consecutive_eq = 0;
        closing = false;
      }
    } else if (c == ']') {
      closing = true;
      end_index = index;
    }
    c = getNextCharacter();
  }
  began_raw_string = false;
  return getNextCharacter();
}

char Lexer::processComment() {
  char c = getNextCharacter();
  if (c == '[') {
    c = getNextCharacter();
    if (c == '[') {
      return processMultilineComment();
    }
  }
  while (c != '\n' && c != '\r') {
    c = getNextCharacter();
  }
  return getNextCharacter();
}

char Lexer::processMultilineComment() {
  began_multiline_comment = true;
  char previous_c = '[';
  char c = getNextCharacter();
  while (previous_c != ']' || c != ']') {
    previous_c = c;
    c = getNextCharacter();
  }
  began_multiline_comment = false;
  return getNextCharacter();
}

char Lexer::getNextCharacter() {
  if (index >= source.size()) {
    if (began_multiline_comment) {
      throwError("Unterminated multiline comment");
    } else if (began_raw_string) {
      throwError("Unterminated raw string");
    } else {
      throw EOFException();
    }
  }
  char ret = source[index];
  last_char = ret;
  index++;
  if (ret == '\n') {
    character = 1;
    line++;
  } else {
    character++;
  }
  return ret;
}

void Lexer::beginToken() {
  token_index = index - 1;
  token_line = line;
  token_character = character - 1;
}

void Lexer::throwError(const std::string& err) const {
  throw LexingError(token_line, token_character, err);
}

void Lexer::pushToken(TokenType type) {
  tokens.emplace_back();
  auto& token = tokens.back();
  token.line = token_line;
  token.character = token_character;
  token.token_type = type;
}

void Lexer::pushIntNumeralToken(int64_t value) {
  tokens.emplace_back();
  auto& token = tokens.back();
  token.line = token_line;
  token.character = token_character;
  token.token_type = TokenType::NUM_INT;
  token.value.value_int = value;
}

void Lexer::pushFloatNumeralToken(double value) {
  tokens.emplace_back();
  auto& token = tokens.back();
  token.line = token_line;
  token.character = token_character;
  token.token_type = TokenType::NUM_FLOAT;
  token.value.value_float = value;
}

void Lexer::pushStringToken(TokenType type, std::string str) {
  tokens.emplace_back();
  auto& token = tokens.back();
  token.line = token_line;
  token.character = token_character;
  token.token_type = type;
  token.value_string = std::move(str);
}
}

}