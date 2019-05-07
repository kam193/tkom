// Copyright 2019 Kamil Mankowski

#include "Parser.h"

#include <iostream>

std::unordered_map<ParseState, std::set<ttype>, std::hash<int>>
    Parser::expectedTokens = {
        {InstructionState, {ttype::def, ttype::nl, ttype::eof}},
        {FuncDefParams,
         {ttype::identifier, ttype::comma, ttype::closeBracket}}};

Program Parser::parse() {
  getNextToken(ttype::space);

  auto cb = parseCodeBlock(currentToken.getInteger());

  std::cout << "Parsing end" << std::endl;

  return Program();
}

std::unique_ptr<Instruction> Parser::parseFunctionDef(int width) {
  auto func = std::make_unique<Function>();

  getNextToken(ttype::identifier);
  func->name = currentToken.getString();
  getNextToken(ttype::openBracket);

  while (getNextToken(FuncDefParams)) {
    if (currentToken.getType() == ttype::identifier)
      func->argumentNames.push_back(currentToken.getString());
    else if (currentToken.getType() == ttype::closeBracket)
      break;
  }

  getNextToken(ttype::colon);
  getNextToken(ttype::nl);
  getNextToken(ttype::space);

  if (currentToken.getInteger() > width)
    func->code = parseCodeBlock(currentToken.getInteger());
  else
    throwError("Expected a new code block, but indend is incorrect");

  if (func->code->instructions.size() <= 0)
    throwError("Function needs code body");

  return func;
}

std::unique_ptr<CodeBlock> Parser::parseCodeBlock(int width) {
  int currentSpace = width;
  auto code = std::make_unique<CodeBlock>();

  while (currentSpace == width) {
    // parse & add instruction
    // uwazac na instrukcje, ktore maja wewnatrz sobie blok kodu
    // i zwracaja

    getNextToken(InstructionState);

    if (currentToken.getType() == ttype::def)
      code->instructions.push_back(parseFunctionDef(width));

    if (currentToken.getType() == ttype::eof) break;

    if (currentToken.getType() == ttype::nl) getNextToken(ttype::space);
    currentSpace = currentToken.getInteger();
  }

  return code;
}

bool Parser::getNextToken(ParseState state) {
  currentToken = scanner.getNextToken();

  auto findResult = expectedTokens[state].find(currentToken.getType());
  if (findResult != expectedTokens[state].end())
    return true;
  else
    throwError("Token type unexpected");
}

bool Parser::getNextToken(ttype expectedType) {
  currentToken = scanner.getNextToken();

  if (currentToken.getType() == expectedType)
    return true;
  else
    throwError("Token type unexpected");
}

void Parser::throwError(std::string msg) {
  std::cout << "Error on line " << currentToken.getLine() << " column "
            << currentToken.getColumn() << ":\n\t" << msg << std::endl;
  throw std::exception();
}
