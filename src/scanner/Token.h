// Copyright 2019 Kamil Mankowski

#ifndef SRC_SCANNER_TOKEN_H_
#define SRC_SCANNER_TOKEN_H_

#include <string>

class Token {
 public:
  enum class Type {
    NaT,    // not a token
    space,  // For any whitespace, value is int and means number of white chars
    openBracket,         // (
    closeBracket,        // )
    openSquareBracket,   // [
    closeSquareBracket,  // ]
    colon,               // :
    comma,               // ,
    multipOp,            // *
    divOp,               // /
    expOp,               // ^
    assign,              // =
    addAssign,           // +=
    subAssign,           // -=
    add,                 // +
    sub,                 // -
    equal,               // ==
    greater,             // >
    less,                // <
    greaterEq,           // >=
    lessEq,              // <=
    diff,                // !=
    stringT,
    identifier,
    eof,
    nl,
    integerNumber,
    forT
  };

  Token() {}
  Token(Type type, int value) : type(type), intValue(value) {}
  explicit Token(Type type) : type(type) {}
  explicit Token(std::string str) : type(Type::identifier), strValue(str) {}
  Token(Type type, std::string str) : type(type), strValue(str) {}

  int getInteger() { return intValue; }
  std::string getString() { return strValue; }
  Type getType() { return type; }

 private:
  Type type = Type::NaT;
  int intValue = 0;
  std::string strValue = "";
};

#endif  // SRC_SCANNER_TOKEN_H_
