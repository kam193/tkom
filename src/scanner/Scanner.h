// Copyright 2019 Kamil Mankowski

#ifndef SRC_SCANNER_SCANNER_H_
#define SRC_SCANNER_SCANNER_H_

#include <cctype>
#include <istream>
#include <map>
#include <stdexcept>
#include <string>

#include "Token.h"
#include "Validation.h"

class Scanner {
 public:
  explicit Scanner(std::istream &in);

  Token getNextToken();

 private:
  std::istream &in;
  bool isNewLine = true;  // Begin of the new line
  int currentLine = 1;
  int currentColumn = 0;
  std::map<std::string, Token::Type> keywordsTokens;
  std::map<std::string, Token::Type> onlySinglePunct;
  std::map<std::string, Token::Type> multiCharOperators;

  char getNextChar();
  void moveForward();
  void skipWhitespaces();
  void skipComment();

  Token unvalidToken(const std::string &value);
  Token makeToken(Token::Type type, std::int64_t value);
  Token makeToken(Token::Type type);
  Token makeToken(double value);
  Token makeToken(Token::Type type, std::string str);

  Token parseNewLine();
  Token parseSpace();
  Token parseAlpha();
  Token parseDigit();
  Token parsePunct();
  Token parseQuotationMark();
  Token parseUnexpectedChar(std::string start = "");
};

#endif  // SRC_SCANNER_SCANNER_H_
