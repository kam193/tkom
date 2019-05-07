// Copyright 2019 Kamil Mankowski

#ifndef SRC_PARSER_PARSER_H_
#define SRC_PARSER_PARSER_H_

#include <istream>
#include <list>
#include <memory>
#include <set>
#include <unordered_map>
#include <string>

#include "../scanner/Scanner.h"
#include "../scanner/Token.h"
#include "Instructions.h"
#include "Program.h"

using ttype = Token::Type;

enum ParseState { InstructionState, FuncDefParams };

class Parser {
 public:
  explicit Parser(std::istream &in) : scanner(in) {}
  Program parse();

 private:
  Scanner scanner;
  std::list<Instruction> programCode;
  Token currentToken;

  bool getNextToken(ParseState state);
  bool getNextToken(ttype expectedType);

  std::unique_ptr<Instruction> parseFunctionDef(int width);
  std::unique_ptr<CodeBlock> parseCodeBlock(int width);

  void throwError(std::string msg);

  static std::unordered_map<ParseState, std::set<ttype>, std::hash<int>>
      expectedTokens;
};

#endif  // SRC_PARSER_PARSER_H_
