// Copyright 2019 Kamil Mankowski

#ifndef SRC_SCANNER_SCANNER_H_
#define SRC_SCANNER_SCANNER_H_

#include <cctype>
#include <istream>
#include <map>
#include <stdexcept>
#include <string>
#include "Token.h"

class Scanner {
 public:
  explicit Scanner(std::istream &in);

  Token getNextToken();

 private:
  std::istream &in;
  struct ReadingState {
    bool newLine = true;  // Begin of the new line
    int currentLine = 0;
  } currentState;
  std::map<std::string, Token::Type> keywordsTokens;
  std::map<std::string, Token::Type> onlySinglePunct;

  char getNextChar();
  void moveForward();
  void skipWhitespaces();
  bool isValidIdentiferChar(char c);

  Token parseNewLine();
  Token parseSpace();
  Token parseAlpha();
  Token parseDigit();
  Token parsePunct();
};

#endif  // SRC_SCANNER_SCANNER_H_
