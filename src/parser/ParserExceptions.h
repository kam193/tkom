// Copyright 2019 Kamil Mankowski

#ifndef SRC_PARSER_PARSEREXCEPTIONS_H_
#define SRC_PARSER_PARSEREXCEPTIONS_H_

#include <exception>
#include <string>

#include "../scanner/Token.h"

class ParserExceptionBase : public std::exception {
 public:
  explicit ParserExceptionBase(const Token& token) {
    message = "Error on line " + std::to_string(token.getLine()) + " column " +
              std::to_string(token.getColumn()) + ": " + token.getString() +
              "\n\t";
  }
  const char* what() const throw() { return message.c_str(); }

 protected:
  std::string message;
};

class UnexpectedToken : public ParserExceptionBase {
 public:
  explicit UnexpectedToken(const Token& token) : ParserExceptionBase(token) {
    message += "Token type invalid or unexpected here.";
  }
};

class ExpectedCodeBlock : public ParserExceptionBase {
 public:
  explicit ExpectedCodeBlock(const Token& token) : ParserExceptionBase(token) {
    message +=
        "Expected a new code block, but indend is incorrect or block is empty.";
  }
};

class UnexpectedAfterReturn : public ParserExceptionBase {
 public:
  explicit UnexpectedAfterReturn(const Token& token)
      : ParserExceptionBase(token) {
    message += "Unexpected token after 'return'.";
  }
};

class IncorrectExpression : public ParserExceptionBase {
 public:
  explicit IncorrectExpression(const Token& token)
      : ParserExceptionBase(token) {
    message += "Expression needs a right side.";
  }
};

class InvalidCompareExpression : public ParserExceptionBase {
 public:
  explicit InvalidCompareExpression(const Token& token)
      : ParserExceptionBase(token) {
    message +=
        "Invalid compare expression. Possible reasons:\n\t"
        " (-) no compare operator,\n\t"
        " (-) no right side of compare,\n\t"
        " (-) invalid token after expression: expected is ':' or new line.";
  }
};

class InvalidFunctionCall : public ParserExceptionBase {
 public:
  explicit InvalidFunctionCall(const Token& token)
      : ParserExceptionBase(token) {
    message += "Unexpected token inside function call arguments.";
  }
};

class NoEndOfSlice : public ParserExceptionBase {
 public:
  explicit NoEndOfSlice(const Token& token) : ParserExceptionBase(token) {
    message += "Expected ']' as end of slice.";
  }
};

class InvalidListElement : public ParserExceptionBase {
 public:
  explicit InvalidListElement(const Token& token) : ParserExceptionBase(token) {
    message += "Expected an expression as element of list, but no one found.";
  }
};

class InvalidAssign : public ParserExceptionBase {
 public:
  explicit InvalidAssign(const Token& token) : ParserExceptionBase(token) {
    message += "Assign operation needs an expression on the right side.";
  }
};

class InvalidForLoop : public ParserExceptionBase {
 public:
  explicit InvalidForLoop(const Token& token) : ParserExceptionBase(token) {
    message +=
        "Invalid for loop declaration. Possible reasons:\n\t"
        " (-) no range to iterate on,\n\t"
        " (-) no ':' after range.";
  }
};

#endif  // SRC_PARSER_PARSEREXCEPTIONS_H_
