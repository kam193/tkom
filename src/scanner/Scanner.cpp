// Copyright 2019 Kamil Mankowski

#include "Scanner.h"

Scanner::Scanner(std::istream &in) : in(in) {
  keywordsTokens.insert(std::make_pair("True", Token::Type::trueT));
  keywordsTokens.insert(std::make_pair("False", Token::Type::falseT));
  keywordsTokens.insert(std::make_pair("None", Token::Type::none));
  keywordsTokens.insert(std::make_pair("while", Token::Type::whileT));
  keywordsTokens.insert(std::make_pair("for", Token::Type::forT));
  keywordsTokens.insert(std::make_pair("if", Token::Type::ifT));
  keywordsTokens.insert(std::make_pair("else", Token::Type::elseT));
  keywordsTokens.insert(std::make_pair("continue", Token::Type::continueT));
  keywordsTokens.insert(std::make_pair("def", Token::Type::def));
  keywordsTokens.insert(std::make_pair("return", Token::Type::returnT));

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
    if (nextChar == EOF) return makeToken(Token::Type::eof);
    if (nextChar == '\n') return parseNewLine();
    if (isspace(nextChar)) return parseSpace();
    if (isdigit(nextChar)) return parseDigit();
    if (isValidIdentiferChar(nextChar)) return parseAlpha();
    if (nextChar == '"') return parseQuotationMark();
    if (ispunct(nextChar)) return parsePunct();
    return unvalidToken("" + nextChar);
  }
}

char Scanner::getNextChar() {
  if (in.eof()) return EOF;
  if (!in) throw std::runtime_error("Error on source reading!");
  return in.peek();
}

void Scanner::moveForward() {
  in.get();
  currentColumn += 1;
}

void Scanner::skipWhitespaces() {
  // Don't skip whitespaces on begin of line: they are used
  // for define code blocks
  if (isNewLine) {
    isNewLine = false;
    return;
  }

  char c;
  while ((c = getNextChar()) != '\n' && isspace(c)) moveForward();
}

bool Scanner::isValidIdentiferChar(char c) { return isalnum(c) || c == '_'; }

bool Scanner::isValidRealNumber(const std::string &value, int pointerPosition) {
  for (int i = 0; i < value.size(); ++i) {
    if (i != pointerPosition && !isdigit(value[i])) return false;
  }
  return value[pointerPosition] == '.';
}

bool Scanner::isValidIntegerNumber(const std::string &value) {
  for (auto &digit : value) {
    if (!isdigit(digit)) return false;
  }
  return true;
}

bool Scanner::isValidHexNumber(const std::string &value) {
  for (auto &digit : value) {
    if (!isxdigit(digit)) return false;
  }
  return true;
}

Token Scanner::unvalidToken(const std::string &value) {
  return Token(Token::Type::NaT, value, currentLine, currentColumn);
}

Token Scanner::makeToken(Token::Type type, std::int64_t value) {
  return Token(type, value, currentLine, currentColumn);
}

Token Scanner::makeToken(Token::Type type) {
  return Token(type, currentLine, currentColumn);
}

Token Scanner::makeToken(double value) {
  return Token(value, currentLine, currentLine);
}

Token Scanner::makeToken(Token::Type type, std::string str) {
  return Token(type, str, currentLine, currentColumn);
}

Token Scanner::parseNewLine() {
  isNewLine = true;
  moveForward();
  auto token = makeToken(Token::Type::nl);
  currentLine += 1;
  currentColumn = 0;
  return token;
}

Token Scanner::parseSpace() {
  int spacesCount = 0;
  char c;

  while ((c = getNextChar()) != '\n' && isspace(c)) {
    ++spacesCount;
    moveForward();
  }

  return makeToken(Token::Type::space, spacesCount);
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
    return makeToken(findedKeyword->second);

  return makeToken(Token::Type::identifier, identifer);
}

Token Scanner::parseDigit() {
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
    if (isValidRealNumber(tmp, pointerPosition))
      return makeToken(std::stod(tmp));
  }
  if (tmp.size() > 2 && tmp.substr(0, 2) == "0x") {
    std::string onlyNumber = tmp.substr(2, tmp.size());
    if (isValidHexNumber(onlyNumber))
      return makeToken(Token::Type::integerNumber,
                       std::stoll(onlyNumber, 0, 16));
  }
  if (isValidIntegerNumber(tmp))
    return makeToken(Token::Type::integerNumber, std::stoi(tmp));
  return unvalidToken(tmp);
}

Token Scanner::parsePunct() {
  std::string tmp = "";
  tmp += getNextChar();
  moveForward();

  auto findedToken = onlySinglePunct.find(tmp);
  if (findedToken != onlySinglePunct.end())
    return makeToken(findedToken->second);

  if (getNextChar() == '=') {
    tmp += '=';
    moveForward();
  }

  findedToken = multiCharOperators.find(tmp);
  if (findedToken != multiCharOperators.end()) {
    return makeToken(findedToken->second);
  }

  return unvalidToken(tmp);
}

Token Scanner::parseQuotationMark() {
  std::string tmp = "";
  char c;

  moveForward();
  while ((c = getNextChar()) != '"' && c != '\n' && c != EOF) {
    tmp += c;
    moveForward();
  }

  if (c != '"') return unvalidToken(tmp);

  moveForward();
  return makeToken(Token::Type::stringT, tmp);
}
