// Copyright 2019 Kamil Mankowski

#include "Scanner.h"

Scanner::Scanner(std::istream &in) : in(in) {
  keywordsTokens.insert(std::make_pair("for", Token::Type::forT));
  // rest of keywords

  onlySinglePunct.insert(std::make_pair("(", Token::Type::openBracket));
  onlySinglePunct.insert(std::make_pair(")", Token::Type::closeBracket));
  onlySinglePunct.insert(std::make_pair("[", Token::Type::openSquareBracket));
  onlySinglePunct.insert(std::make_pair("]", Token::Type::closeSquareBracket));
  onlySinglePunct.insert(std::make_pair(":", Token::Type::colon));
  onlySinglePunct.insert(std::make_pair(",", Token::Type::comma));
  onlySinglePunct.insert(std::make_pair("*", Token::Type::multipOp));
  onlySinglePunct.insert(std::make_pair("/", Token::Type::divOp));
  onlySinglePunct.insert(std::make_pair("^", Token::Type::expOp));

  multiCharOperators.insert(std::make_pair("=", Token::Type::assign));
  multiCharOperators.insert(std::make_pair("+=", Token::Type::addAssign));
  multiCharOperators.insert(std::make_pair("-=", Token::Type::subAssign));
  multiCharOperators.insert(std::make_pair("+", Token::Type::add));
  multiCharOperators.insert(std::make_pair("-", Token::Type::sub));
  multiCharOperators.insert(std::make_pair("==", Token::Type::equal));
  multiCharOperators.insert(std::make_pair(">", Token::Type::greater));
  multiCharOperators.insert(std::make_pair("<", Token::Type::less));
  multiCharOperators.insert(std::make_pair(">=", Token::Type::greaterEq));
  multiCharOperators.insert(std::make_pair("<=", Token::Type::lessEq));
  multiCharOperators.insert(std::make_pair("!=", Token::Type::diff));
}

Token Scanner::getNextToken() {
  char nextChar;

  skipWhitespaces();
  while ((nextChar = getNextChar())) {
    if (nextChar == EOF) return Token(Token::Type::eof);
    if (nextChar == '\n') return parseNewLine();
    if (isspace(nextChar)) return parseSpace();
    if (isdigit(nextChar)) return parseDigit();
    if (isValidIdentiferChar(nextChar)) return parseAlpha();
    if (nextChar == '"') return parseQuotationMark();
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

bool Scanner::isValidIdentiferChar(char c) { return isalnum(c) || c == '_'; }

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

Token Scanner::parseDigit() {
  // TODO: other types of numbers

  std::string tmp = "";
  char c;
  while (isalnum(c = getNextChar())) {
    tmp += c;
    moveForward();
  }
  if (c == '.') {
    int pointerPosition = tmp.size();
    tmp += c;
    moveForward();
    while (isalnum(c = getNextChar())) {
      tmp += c;
      moveForward();
    }
    // is valid realnumber
    return Token(std::stod(tmp));
  }
  if (tmp.size() > 2 && tmp.substr(0, 2) == "0x") {
    // is valid hex
    double t = std::stoll(tmp.substr(2, tmp.size()), 0, 16);
    return Token(Token::Type::integerNumber,
                 std::stoll(tmp.substr(2, tmp.size()), 0, 16));
  }
  // is valid Int
  return Token(Token::Type::integerNumber, std::stoi(tmp));
}

Token Scanner::parsePunct() {
  std::string tmp = "";
  tmp += getNextChar();
  moveForward();

  auto findedToken = onlySinglePunct.find(tmp);
  if (findedToken != onlySinglePunct.end()) return Token(findedToken->second);

  if (getNextChar() == '=') {
    tmp += '=';
    moveForward();
  }

  findedToken = multiCharOperators.find(tmp);
  if (findedToken != multiCharOperators.end()) {
    return Token(findedToken->second);
  }

  return Token(Token::Type::NaT, tmp);
}

Token Scanner::parseQuotationMark() {
  std::string tmp = "";
  char c;

  moveForward();
  while ((c = getNextChar()) != '"' && c != '\n' && c != EOF) {
    tmp += c;
    moveForward();
  }

  if (c != '"') return Token(Token::Type::NaT, tmp);

  moveForward();
  return Token(Token::Type::stringT, tmp);
}
