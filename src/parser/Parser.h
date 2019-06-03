// Copyright 2019 Kamil Mankowski

#ifndef SRC_PARSER_PARSER_H_
#define SRC_PARSER_PARSER_H_

#include <iostream>
#include <istream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "../execute/Instructions.h"
#include "../scanner/Scanner.h"
#include "../scanner/Token.h"
#include "ParserExceptions.h"

using ttype = Token::Type;

enum ExpectedTokens {
  InstructionState,
  ParamsDef,
  ReturnState,
  InstrEnd,
  OperatorsAddSub,
  OperatorsMulDiv,
  SliceStart
};

class Parser {
 public:
  explicit Parser(std::istream &in) : scanner(in) {}
  std::unique_ptr<CodeBlock> parse();

 private:
  Scanner scanner;
  std::list<Instruction> programCode;
  Token currentToken;
  Token savedToken;
  bool tokenRestored = false;

  bool getNextToken(ExpectedTokens state);
  bool getNextToken(ttype expectedType);
  bool getNextToken();
  void restoreToken();
  bool checkTokenType(ExpectedTokens state);
  bool checkTokenType(ttype expectedType);

  std::unique_ptr<Instruction> tryParseFunctionDef(int width);
  std::unique_ptr<CodeBlock> parseCodeBlock(int width, bool inFunction = false,
                                            bool inLoop = false);
  std::unique_ptr<Return> parseReturn();

  std::unique_ptr<Instruction> tryParseArgument();
  std::unique_ptr<FunctionCall> tryParseFuncCall();
  std::unique_ptr<Constant> tryParseConstant();
  std::unique_ptr<Constant> tryParseNumber();
  std::unique_ptr<Slice> tryParseSliceSt();
  std::unique_ptr<Instruction> tryParseSlicedValue();
  std::unique_ptr<Instruction> tryParseSlice();
  std::unique_ptr<Constant> tryParseList();

  std::unique_ptr<CompareExpr> tryParseCmpExpr(
      ttype expectedEnd = ttype::colon);
  std::unique_ptr<Expression> tryParseExpr();
  std::unique_ptr<Expression> tryParseExprMul();
  std::unique_ptr<Expression> tryParseExprExp();
  std::unique_ptr<AssignExpr> tryParseAssignExpr();
  std::unique_ptr<If> tryParseIfExpr(int width, bool inFunction, bool inLoop);
  std::unique_ptr<For> tryParseForLoop(int width, bool inFunction);
  std::unique_ptr<While> tryParseWhileLoop(int width, bool inFunction);

  static std::unordered_map<ExpectedTokens, std::set<ttype>, std::hash<int>>
      expectedTokens;
};

#endif  // SRC_PARSER_PARSER_H_
