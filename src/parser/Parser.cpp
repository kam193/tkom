// Copyright 2019 Kamil Mankowski

#include "Parser.h"

#include <iostream>

std::unordered_map<ParseState, std::set<ttype>, std::hash<int>>
    Parser::expectedTokens = {
        {ParamsDef, {ttype::identifier, ttype::comma, ttype::closeBracket}},
        {InstrEnd, {ttype::nl, ttype::eof}},
        {OperatorsAddSub, {ttype::add, ttype::sub}},
        {OperatorsMulDiv, {ttype::multipOp, ttype::divOp}},
        {SliceStart, {ttype::integerNumber, ttype::colon}}};

Program Parser::parse() {
  getNextToken(ttype::space);
  auto code = parseCodeBlock(currentToken.getInteger());
  return Program(std::move(code));
}

std::unique_ptr<Instruction> Parser::tryParseFunctionDef(int width) {
  if (currentToken.getType() != ttype::def) return nullptr;

  auto func = std::make_unique<Function>();

  getNextToken(ttype::identifier);
  func->name = currentToken.getString();
  getNextToken(ttype::openBracket);

  while (getNextToken(ParamsDef)) {
    if (currentToken.getType() == ttype::identifier)
      func->argumentNames.push_back(currentToken.getString());
    else if (currentToken.getType() == ttype::closeBracket)
      break;
  }

  getNextToken(ttype::colon);
  getNextToken(ttype::nl);
  getNextToken(ttype::space);

  if (currentToken.getInteger() > width)
    func->code = parseCodeBlock(currentToken.getInteger(), true);
  else
    throwError("Expected a new code block, but indend is incorrect");

  if (func->code->instructions.size() <= 0)
    throwError("Function needs code body");

  return func;
}

std::unique_ptr<CodeBlock> Parser::parseCodeBlock(int width, bool inFunc,
                                                  bool inLoop) {
  int currentSpace = width;
  auto code = std::make_unique<CodeBlock>();

  while (currentSpace == width) {
    getNextToken();

    std::unique_ptr<Instruction> instrPtr;

    if ((instrPtr = tryParseFunctionDef(width)) != nullptr) {
      code->instructions.push_back(std::move(instrPtr));
    } else if (inFunc && currentToken.getType() == ttype::returnT) {
      code->instructions.push_back(parseReturn());
      break;
    } else if (inLoop && currentToken.getType() == ttype::continueT) {
      code->instructions.push_back(std::make_unique<Continue>());
      getNextToken(InstrEnd);
    } else if (inLoop && currentToken.getType() == ttype::breakT) {
      code->instructions.push_back(std::make_unique<Break>());
      getNextToken(InstrEnd);
    } else if ((instrPtr = tryParseAssignExpr()) != nullptr) {
      code->instructions.push_back(std::move(instrPtr));
    } else if ((instrPtr = tryParseExpr()) != nullptr) {
      code->instructions.push_back(std::move(instrPtr));
    } else if ((instrPtr = tryParseIfExpr(width, inFunc, inLoop)) != nullptr) {
      code->instructions.push_back(std::move(instrPtr));
      // Support for else: try, check if previous is if, if is -> append else
    } else if ((instrPtr = tryParseWhileLoop(width, inFunc)) != nullptr) {
      code->instructions.push_back(std::move(instrPtr));
    } else if ((instrPtr = tryParseForLoop(width, inFunc)) != nullptr) {
      code->instructions.push_back(std::move(instrPtr));
    }

    if (currentToken.getType() == ttype::eof) break;
    if (currentToken.getType() == ttype::nl) getNextToken(ttype::space);
    if (currentToken.getType() != ttype::space)
      throwError("Unexpected token inside code block.");
    currentSpace = currentToken.getInteger();
  }

  return code;
}

std::unique_ptr<Return> Parser::parseReturn() {
  auto returnInstr = std::make_unique<Return>();
  std::unique_ptr<Instruction> instrPtr;

  getNextToken();
  if (checkTokenType(InstrEnd)) {
    returnInstr->value = std::make_unique<Constant>(Constant::Type::None);
    getNextToken();
  } else if ((instrPtr = tryParseCmpExpr(ttype::nl)) != nullptr) {
    returnInstr->value = std::move(instrPtr);
  } else if ((instrPtr = tryParseExpr()) != nullptr) {
    returnInstr->value = std::move(instrPtr);
  } else {
    throwError("Unexpected token after 1'return'.");
  }

  return returnInstr;
}

std::unique_ptr<Instruction> Parser::tryParseArgument() {
  std::unique_ptr<Instruction> argPtr;

  if ((argPtr = tryParseConstant()) != nullptr)
    return argPtr;
  else if ((argPtr = tryParseSlice()) != nullptr)
    return argPtr;
  return nullptr;
}

std::unique_ptr<Constant> Parser::tryParseConstant() {
  std::unique_ptr<Constant> constPtr = nullptr;

  if (checkTokenType(ttype::none))
    constPtr = std::make_unique<Constant>(Constant::Type::None);
  else if (checkTokenType(ttype::trueT))
    constPtr = std::make_unique<Constant>(true);
  else if (checkTokenType(ttype::falseT))
    constPtr = std::make_unique<Constant>(false);
  else if (checkTokenType(ttype::realNumber))
    constPtr = std::make_unique<Constant>(currentToken.getReal());
  else if (checkTokenType(ttype::integerNumber))
    constPtr = std::make_unique<Constant>(currentToken.getInteger());
  else if (checkTokenType(ttype::stringT))
    constPtr = std::make_unique<Constant>(currentToken.getString());

  if (constPtr != nullptr) getNextToken();

  return constPtr;
}

std::unique_ptr<Expression> Parser::tryParseExpr() {
  auto leftMul = tryParseExprMul();
  if (leftMul == nullptr) return nullptr;

  if (checkTokenType(OperatorsAddSub)) {
    auto type = currentToken.getType() == ttype::add ? Expression::Type::Add
                                                     : Expression::Type::Sub;
    getNextToken();
    auto rightMul = tryParseExprMul();
    if (rightMul == nullptr) throwError("Expression needs a right side.");

    return std::make_unique<Expression>(std::move(leftMul), type,
                                        std::move(rightMul));
  }
  return std::make_unique<Expression>(std::move(leftMul));
}

std::unique_ptr<Expression> Parser::tryParseExprMul() {
  auto left = tryParseExprExp();
  if (left == nullptr) return nullptr;

  if (checkTokenType(OperatorsMulDiv)) {
    auto type = currentToken.getType() == ttype::multipOp
                    ? Expression::Type::Mul
                    : Expression::Type::Div;
    getNextToken();
    auto right = tryParseExprExp();
    if (right == nullptr) throwError("Expression needs a right side.");

    return std::make_unique<Expression>(std::move(left), type,
                                        std::move(right));
  }
  return std::make_unique<Expression>(std::move(left));
}

std::unique_ptr<Expression> Parser::tryParseExprExp() {
  auto left = tryParseArgument();
  if (left == nullptr) return nullptr;

  if (checkTokenType(ttype::expOp)) {
    getNextToken();
    auto right = tryParseArgument();
    if (right == nullptr) throwError("Expression needs a right side.");

    return std::make_unique<Expression>(std::move(left), Expression::Type::Exp,
                                        std::move(right));
  }
  return std::make_unique<Expression>(std::move(left));
}

std::unique_ptr<CompareExpr> Parser::tryParseCmpExpr(ttype expectedEnd) {
  auto leftExprPtr = tryParseExpr();
  if (leftExprPtr == nullptr) return nullptr;

  if (currentToken.getType() == expectedEnd ||
      currentToken.getType() == ttype::eof)
    return std::make_unique<CompareExpr>(std::move(leftExprPtr));

  CompareExpr::Type type;
  auto tokenType = currentToken.getType();
  if (tokenType == ttype::greater)
    type = CompareExpr::Type::Greater;
  else if (tokenType == ttype::greaterEq)
    type = CompareExpr::Type::GreaterEq;
  else if (tokenType == ttype::less)
    type = CompareExpr::Type::Less;
  else if (tokenType == ttype::lessEq)
    type = CompareExpr::Type::LessEq;
  else if (tokenType == ttype::diff)
    type = CompareExpr::Type::Different;
  else if (tokenType == ttype::equal)
    type = CompareExpr::Type::Equal;
  else
    throwError("Expected compare operator.");

  getNextToken();
  auto rightExpr = tryParseExpr();
  if (rightExpr == nullptr) throwError("Expected right side of compare.");
  if (currentToken.getType() != expectedEnd ||
      currentToken.getType() == ttype::eof)
    throwError("Expected ':' or new line after compare expression.");

  return std::make_unique<CompareExpr>(type, std::move(leftExprPtr),
                                       std::move(rightExpr));
}

std::unique_ptr<FunctionCall> Parser::tryParseFuncCall() {
  if (currentToken.getType() != ttype::identifier) return nullptr;
  std::string funcName = currentToken.getString();
  if (!getNextToken(ttype::openBracket, true)) {
    restoreToken();
    return nullptr;
  }

  auto funcPtr = std::make_unique<FunctionCall>(funcName);
  std::unique_ptr<Instruction> argumentPtr;

  getNextToken();
  while (currentToken.getType() != ttype::closeBracket) {
    if ((argumentPtr = tryParseExpr()) != nullptr)
      funcPtr->addArgument(std::move(argumentPtr));
    else if (!checkTokenType(ttype::comma))
      throwError("Unexpected token inside function call arguments");
    if (checkTokenType(ttype::comma)) getNextToken();
  }
  getNextToken();
  return funcPtr;
}

std::unique_ptr<Slice> Parser::tryParseSliceSt() {
  if (currentToken.getType() != ttype::openSquareBracket) return nullptr;

  int start = 0;
  int end = 0;
  auto state = Slice::SliceType::Start;

  getNextToken(ParseState::SliceStart);
  if (checkTokenType(ttype::integerNumber)) {
    start = currentToken.getInteger();
    getNextToken();
  }
  if (checkTokenType(ttype::colon)) {
    state = Slice::SliceType::StartToEnd;
    getNextToken();
  }
  if (state == Slice::SliceType::StartToEnd &&
      checkTokenType(ttype::integerNumber)) {
    state = Slice::SliceType::StartToSlice;
    end = currentToken.getInteger();
    getNextToken();
  }
  if (!checkTokenType(ttype::closeSquareBracket))
    throwError("Expected end of slice.");

  getNextToken();
  return std::make_unique<Slice>(state, start, end);
}

std::unique_ptr<Instruction> Parser::tryParseSlice() {
  auto instr = tryParseSlicedValue();
  if (instr == nullptr) return nullptr;

  auto sliceSt = tryParseSliceSt();
  if (sliceSt != nullptr) {
    sliceSt->setSource(std::move(instr));
    return sliceSt;
  }
  return instr;
}

std::unique_ptr<Instruction> Parser::tryParseSlicedValue() {
  std::unique_ptr<Instruction> value;

  if ((value = tryParseList()) != nullptr) {
    return value;
  } else if ((value = tryParseFuncCall()) != nullptr) {
    return value;
  } else if (checkTokenType(ttype::identifier)) {
    getNextToken();
    return std::make_unique<Variable>(savedToken.getString());
  }
  return nullptr;
}

std::unique_ptr<Constant> Parser::tryParseList() {
  if (!checkTokenType(ttype::openSquareBracket)) return nullptr;

  std::vector<std::unique_ptr<Instruction>> elements;
  std::unique_ptr<Instruction> elem;

  getNextToken();
  while (!checkTokenType(ttype::closeSquareBracket)) {
    elem = tryParseExpr();
    if (elem == nullptr)
      throwError(
          "Expected an expression as element of list, but no one found.");
    elements.push_back(std::move(elem));
    if (checkTokenType(ttype::comma)) getNextToken();
  }
  getNextToken();

  return std::make_unique<Constant>(std::move(elements));
}

std::unique_ptr<AssignExpr> Parser::tryParseAssignExpr() {
  if (currentToken.getType() != ttype::identifier) return nullptr;
  std::string variableName = currentToken.getString();
  getNextToken();
  AssignExpr::Type type;
  if (currentToken.getType() == ttype::assign) {
    type = AssignExpr::Type::Assign;
  } else if (currentToken.getType() == ttype::addAssign) {
    type = AssignExpr::Type::AddAssign;
  } else if (currentToken.getType() == ttype::subAssign) {
    type = AssignExpr::Type::SubAssign;
  } else {
    restoreToken();
    return nullptr;
  }

  getNextToken();
  auto rightExpr = tryParseExpr();
  if (rightExpr == nullptr)
    throwError("Assign operation needs an expression on the right side.");
  return std::make_unique<AssignExpr>(type, variableName, std::move(rightExpr));
}

std::unique_ptr<If> Parser::tryParseIfExpr(int width, bool inFunction,
                                           bool inLoop) {
  if (currentToken.getType() != ttype::ifT) return nullptr;
  getNextToken();
  auto comp = tryParseCmpExpr();
  getNextToken(ttype::nl);
  getNextToken(ttype::space);
  int blockSpace = currentToken.getInteger();
  if (blockSpace <= width) throwError("Expected a new code block.");
  auto code = parseCodeBlock(blockSpace, inFunction, inLoop);
  return std::make_unique<If>(std::move(comp), std::move(code));
}

std::unique_ptr<While> Parser::tryParseWhileLoop(int width, bool inFunction) {
  if (currentToken.getType() != ttype::whileT) return nullptr;
  getNextToken();
  auto comp = tryParseCmpExpr();
  getNextToken(ttype::nl);
  getNextToken(ttype::space);
  int blockSpace = currentToken.getInteger();
  if (blockSpace <= width) throwError("Expected a new code block.");
  auto code = parseCodeBlock(blockSpace, inFunction, true);
  return std::make_unique<While>(std::move(comp), std::move(code));
}

std::unique_ptr<For> Parser::tryParseForLoop(int width, bool inFunction) {
  if (!checkTokenType(ttype::forT)) return nullptr;
  getNextToken(ttype::identifier);
  std::string iterator = currentToken.getString();
  getNextToken(ttype::in);
  getNextToken();
  auto sliced = tryParseSlice();
  if (sliced == nullptr)
    throwError("For loop: expected a range to iterate on.");
  if (!checkTokenType(ttype::colon)) throwError("Expected ':' after range.");
  getNextToken(ttype::nl);
  getNextToken(ttype::space);
  auto block = parseCodeBlock(currentToken.getInteger(), inFunction, true);
  return std::make_unique<For>(iterator, std::move(sliced), std::move(block));
}

bool Parser::getNextToken() {
  if (tokenRestored) {
    std::swap(savedToken, currentToken);
    tokenRestored = false;
  } else {
    savedToken = currentToken;
    currentToken = scanner.getNextToken();
  }
  return true;
}

void Parser::restoreToken() {
  std::swap(savedToken, currentToken);
  tokenRestored = true;
}

bool Parser::getNextToken(ParseState state, bool no_except) {
  getNextToken();

  if (checkTokenType(state))
    return true;
  else if (no_except)
    return false;
  throwError("Token type unexpected");
}

bool Parser::getNextToken(ttype expectedType, bool no_except) {
  getNextToken();

  if (currentToken.getType() == expectedType)
    return true;
  else if (no_except)
    return false;
  throwError("Token type unexpected");
}

bool Parser::checkTokenType(ParseState state) {
  auto findResult = expectedTokens[state].find(currentToken.getType());
  if (findResult != expectedTokens[state].end()) return true;
  return false;
}

bool Parser::checkTokenType(ttype expectedType) {
  return currentToken.getType() == expectedType;
}

void Parser::throwError(std::string msg) {
  std::cout << "Error on line " << currentToken.getLine() << " column "
            << currentToken.getColumn() << ":\n\t" << msg << std::endl;
  throw std::exception();
}
