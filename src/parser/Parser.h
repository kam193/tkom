// Copyright 2019 Kamil Mankowski

#ifndef SRC_PARSER_PARSER_H_
#define SRC_PARSER_PARSER_H_

#include <istream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "../scanner/Scanner.h"
#include "../scanner/Token.h"
#include "Instructions.h"
#include "Program.h"

using ttype = Token::Type;

enum ParseState { InstructionState, ParamsDef, ReturnState };

class Parser {
 public:
  explicit Parser(std::istream &in) : scanner(in) {}
  Program parse();

 private:
  Scanner scanner;
  std::list<Instruction> programCode;
  Token currentToken;
  Token savedToken;
  bool tokenRestored = false;

  bool getNextToken(ParseState state, bool no_except = false);
  bool getNextToken(ttype expectedType, bool no_except = false);
  bool getNextToken();
  void restoreToken();

  std::unique_ptr<Instruction> tryParseFunctionDef(int width);
  std::unique_ptr<CodeBlock> parseCodeBlock(int width, bool inFunction = false,
                                            bool inLoop = false);
  std::unique_ptr<Return> parseReturn();

  std::unique_ptr<Instruction> tryParseArgument();  // popr. slice
  std::unique_ptr<Slice> tryParseSlice();
  std::unique_ptr<FunctionCall> tryParseFuncCall();
  std::unique_ptr<Constant> tryParseValue();  // wywalic
  std::unique_ptr<Constant> tryParseConstant();
  // tryParseSliced ???
  std::unique_ptr<Constant> tryParseList() { return nullptr; }  // td

  std::unique_ptr<CompareExpr> tryParseCmpExpr(ttype expectedEnd = ttype::colon);
  std::unique_ptr<Expression> tryParseExpr();  // td
  std::unique_ptr<AssignExpr> tryParseAssignExpr();
  std::unique_ptr<If> tryParseIfExpr(int width, bool inFunction, bool inLoop);
  std::unique_ptr<For> tryParseForLoop(int width, bool inFunction);  // td
  std::unique_ptr<While> tryParseWhileLoop(int width, bool inFunction);

  void throwError(std::string msg);

  static std::unordered_map<ParseState, std::set<ttype>, std::hash<int>>
      expectedTokens;
};

#endif  // SRC_PARSER_PARSER_H_
