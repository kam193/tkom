// Copyright 2019 Kamil Mankowski

#ifndef SRC_SCANNER_TOKEN_H_
#define SRC_SCANNER_TOKEN_H_

#include <string>

class Token {
 public:
  enum class Type {
    space,  // For any whitespace, value is int and means number of white chars
    identifier,
    eof,
    nl,
    number,
    forT,
    NaT  // not a token
  };

  Token() {};
  Token(Type type, int value) : type(type), intValue(value) {}
  explicit Token(Type type) : type(type) {}
  explicit Token(std::string str) : type(Type::identifier), strValue(str) {}

  int getInteger() { return intValue; }
  std::string getString() { return strValue; }
  Type getType() { return type; }

 private:
  Type type;
  int intValue;
  std::string strValue;
};

#endif  // SRC_SCANNER_TOKEN_H_
