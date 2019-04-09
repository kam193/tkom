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
    realNumber,
    forT,
    continueT,
    none,
    returnT,
    trueT,
    falseT,
    whileT,
    ifT,
    elseT,
    def
  };

  Token() {}
  Token(Type type, std::int64_t value, int line, int column)
      : type(type), numValue({.integer = value}), line(line), column(column) {}
  Token(Type type, int line, int column)
      : type(type), line(line), column(column) {}
  Token(double value, int line, int column)
      : type(Type::realNumber),
        numValue({.real = value}),
        line(line),
        column(column) {}
  Token(Type type, std::string str, int line, int column)
      : type(type), strValue(str), line(line), column(column) {}

  std::int64_t getInteger() { return numValue.integer; }
  double getReal() { return numValue.real; }
  std::string getString() { return strValue; }
  Type getType() { return type; }

  int getLine() { return line; }
  int getColumn() { return column; }

 private:
  Type type = Type::NaT;
  union {
    std::int64_t integer;
    double real;
  } numValue = {.integer = 0};
  std::string strValue = "";
  int line = 0;
  int column = 0;
};

#endif  // SRC_SCANNER_TOKEN_H_
