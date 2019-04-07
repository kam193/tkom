// Copyright 2019 Kamil Mankowski

#include "Scanner.h"

Scanner::Scanner(std::istream &in) : in(in) {
  keywordsTokens.insert(std::make_pair("for", Token::Type::forT));
  // rest of keywords
}

Token Scanner::getNextToken() {
  char nextChar;

  skipWhitespaces();
  while ((nextChar = getNextChar())) {
    if (nextChar == EOF) return Token(Token::Type::eof);
    if (nextChar == '\n') return parseNewLine();
    if (isspace(nextChar)) return parseSpace();
    if (isValidIdentiferChar(nextChar)) return parseAlpha();
    if (isdigit(nextChar)) return parseDigit();
    if (ispunct(nextChar)) return parsePunct();
    return Token(Token::Type::NaT);
  }
}

char Scanner::getNextChar() {
  if (in.eof()) return EOF;
  if (!in) throw std::runtime_error("Error on source reading!");
  return in.peek();
}

void Scanner::moveForward() { in.get(); }  // TODO: test error

void Scanner::skipWhitespaces() {
  // Don't skip whitespaces on begin of line: they are used
  // for define code blocks
  if (currentState.newLine) {
    currentState.newLine = false;
    return;
  }

  char c;
  while ((c = getNextChar()) != '\n' && isspace(c)) moveForward();
}

bool Scanner::isValidIdentiferChar(char c) { return isalpha(c) || c == '_'; }

Token Scanner::parseNewLine() {
  currentState.newLine = true;
  moveForward();
  return Token(Token::Type::nl);
}

Token Scanner::parseSpace() {
  int spacesCount = 0;
  char c;

  while ((c = getNextChar()) != '\n' && isspace(c)) {
    ++spacesCount;
    moveForward();
  }

  return Token(Token::Type::space, spacesCount);
}

Token Scanner::parseAlpha() {
  std::string identifer;
  char c;

  while (isValidIdentiferChar((c = getNextChar()))) {
    identifer += c;
    moveForward();
  }

  auto findedKeyword = keywordsTokens.find(identifer);
  if (findedKeyword != keywordsTokens.end())
    return Token(findedKeyword->second);

  return Token(identifer);
}
