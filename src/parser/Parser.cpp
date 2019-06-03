// Copyright 2019 Kamil Mankowski

#include "Parser.h"

std::unordered_map<ExpectedTokens, std::set<ttype>, std::hash<int>>
    Parser::expectedTokens = {
        {ParamsDef, {ttype::identifier, ttype::comma, ttype::closeBracket}},
        {InstrEnd, {ttype::nl, ttype::eof}},
        {OperatorsAddSub, {ttype::add, ttype::sub}},
        {OperatorsMulDiv, {ttype::multipOp, ttype::divOp}},
        {SliceStart, {ttype::integerNumber, ttype::colon}}};

std::unique_ptr<CodeBlock> Parser::parse() {
  getNextToken(ttype::space);
  auto code = parseCodeBlock(currentToken.getInteger());
  if (!checkTokenType(ttype::eof)) throw IndentNotMatch(currentToken);
  return std::move(code);
}

std::unique_ptr<Instruction> Parser::tryParseFunctionDef(int width) {
  if (currentToken.getType() != ttype::def) return nullptr;

  getNextToken(ttype::identifier);
  auto func = std::make_unique<Function>(currentToken.getString());
  getNextToken(ttype::openBracket);

  while (getNextToken(ParamsDef)) {
    if (currentToken.getType() == ttype::identifier)
      func->addArgument(currentToken.getString());
    else if (currentToken.getType() == ttype::closeBracket)
      break;
  }

  getNextToken(ttype::colon);
  getNextToken(ttype::nl);
  getNextToken(ttype::space);

  if (currentToken.getInteger() > width) {
    auto codeBlock = parseCodeBlock(currentToken.getInteger(), true);
    func->setCode(std::move(codeBlock));
  } else {
    throw ExpectedCodeBlock(currentToken);
  }

  if (func->empty()) throw ExpectedCodeBlock(currentToken);

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
      code->addInstruction(std::move(instrPtr));
    } else if (inFunc && currentToken.getType() == ttype::returnT) {
      code->addInstruction(parseReturn());
      break;
    } else if (inLoop && currentToken.getType() == ttype::continueT) {
      code->addInstruction(std::make_unique<Continue>());
      getNextToken(InstrEnd);
    } else if (inLoop && currentToken.getType() == ttype::breakT) {
      code->addInstruction(std::make_unique<Break>());
      getNextToken(InstrEnd);
    } else if ((instrPtr = tryParseAssignExpr()) != nullptr) {
      code->addInstruction(std::move(instrPtr));
    } else if ((instrPtr = tryParseExpr()) != nullptr) {
      code->addInstruction(std::move(instrPtr));
    } else if ((instrPtr = tryParseIfExpr(width, inFunc, inLoop)) != nullptr) {
      code->addInstruction(std::move(instrPtr));
      // Support for else: try, check if previous is if, if is -> append else
      // in loop ignore empty lines (the same indent) and check first non-nl,
      // non-space token - if not else, restore
    } else if ((instrPtr = tryParseWhileLoop(width, inFunc)) != nullptr) {
      code->addInstruction(std::move(instrPtr));
    } else if ((instrPtr = tryParseForLoop(width, inFunc)) != nullptr) {
      code->addInstruction(std::move(instrPtr));
    }

    if (currentToken.getType() == ttype::eof) break;
    if (currentToken.getType() == ttype::nl) getNextToken(ttype::space);
    if (currentToken.getType() != ttype::space)
      throw UnexpectedToken(currentToken);
    currentSpace = currentToken.getInteger();
  }

  return code;
}

std::unique_ptr<Return> Parser::parseReturn() {
  auto returnInstr = std::make_unique<Return>();
  std::unique_ptr<Instruction> instrPtr;

  getNextToken();
  if (checkTokenType(InstrEnd)) {
    returnInstr->setValue(std::make_unique<Constant>(ValueType::None));
    getNextToken();
  } else if ((instrPtr = tryParseCmpExpr(ttype::nl)) != nullptr) {
    returnInstr->setValue(std::move(instrPtr));
  } else if ((instrPtr = tryParseExpr()) != nullptr) {
    returnInstr->setValue(std::move(instrPtr));
  } else {
    throw UnexpectedAfterReturn(currentToken);
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

std::unique_ptr<Constant> Parser::tryParseNumber() {
  std::unique_ptr<Constant> number = nullptr;
  bool negative = false;

  if (checkTokenType(ttype::sub)) {
    getNextToken();
    negative = true;
  }

  if (checkTokenType(ttype::realNumber))
    number = std::make_unique<Constant>(currentToken.getReal() *
                                        (negative ? -1.0 : 1.0));
  else if (checkTokenType(ttype::integerNumber))
    number = std::make_unique<Constant>(currentToken.getInteger() *
                                        (negative ? -1 : 1));

  if (number == nullptr && negative) restoreToken();
  if (number != nullptr) getNextToken();

  return number;
}

std::unique_ptr<Constant> Parser::tryParseConstant() {
  std::unique_ptr<Constant> constPtr = nullptr;

  constPtr = tryParseNumber();
  if (constPtr != nullptr) return constPtr;

  if (checkTokenType(ttype::none))
    constPtr = std::make_unique<Constant>(ValueType::None);
  else if (checkTokenType(ttype::trueT))
    constPtr = std::make_unique<Constant>(true);
  else if (checkTokenType(ttype::falseT))
    constPtr = std::make_unique<Constant>(false);
  else if (checkTokenType(ttype::stringT))
    constPtr = std::make_unique<Constant>(currentToken.getString());

  if (constPtr != nullptr) getNextToken();

  return constPtr;
}

std::unique_ptr<Expression> Parser::tryParseExpr() {
  auto leftMul = tryParseExprMul();
  if (leftMul == nullptr) return nullptr;
  auto expr = std::make_unique<Expression>();
  expr->setArgument(std::move(leftMul));

  while (checkTokenType(OperatorsAddSub)) {
    auto type = currentToken.getType() == ttype::add ? Expression::Type::Add
                                                     : Expression::Type::Sub;
    expr->setType(type);
    getNextToken();
    auto rightMul = tryParseExprMul();
    if (rightMul == nullptr) throw IncorrectExpression(currentToken);
    expr->setArgument(std::move(rightMul));
  }
  return std::move(expr);
}

std::unique_ptr<Expression> Parser::tryParseExprMul() {
  auto left = tryParseExprExp();
  if (left == nullptr) return nullptr;
  auto expr = std::make_unique<Expression>();
  expr->setArgument(std::move(left));

  while (checkTokenType(OperatorsMulDiv)) {
    auto type = currentToken.getType() == ttype::multipOp
                    ? Expression::Type::Mul
                    : Expression::Type::Div;
    expr->setType(type);
    getNextToken();
    auto right = tryParseExprExp();
    if (right == nullptr) throw IncorrectExpression(currentToken);
    expr->setArgument(std::move(right));
  }
  return std::move(expr);
}

std::unique_ptr<Expression> Parser::tryParseExprExp() {
  auto left = tryParseArgument();
  if (left == nullptr) return nullptr;
  auto expr = std::make_unique<Expression>();
  expr->setArgument(std::move(left));

  while (checkTokenType(ttype::expOp)) {
    expr->setType(Expression::Type::Exp);
    getNextToken();
    auto right = tryParseArgument();
    if (right == nullptr) throw IncorrectExpression(currentToken);
    expr->setArgument(std::move(right));
  }
  return std::move(expr);
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
    throw InvalidCompareExpression(currentToken);

  getNextToken();
  auto rightExpr = tryParseExpr();
  if (rightExpr == nullptr) throw InvalidCompareExpression(currentToken);
  if (currentToken.getType() != expectedEnd ||
      currentToken.getType() == ttype::eof)
    throw InvalidCompareExpression(currentToken);

  return std::make_unique<CompareExpr>(type, std::move(leftExprPtr),
                                       std::move(rightExpr));
}

std::unique_ptr<FunctionCall> Parser::tryParseFuncCall() {
  if (currentToken.getType() != ttype::identifier) return nullptr;
  std::string funcName = currentToken.getString();
  getNextToken();
  if (!checkTokenType(ttype::openBracket)) {
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
      throw InvalidFunctionCall(currentToken);
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

  getNextToken(SliceStart);
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
    throw NoEndOfSlice(currentToken);

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
    if (elem == nullptr) throw InvalidListElement(currentToken);
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
  if (rightExpr == nullptr) throw InvalidAssign(currentToken);
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
  if (blockSpace <= width) throw ExpectedCodeBlock(currentToken);
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
  if (blockSpace <= width) throw ExpectedCodeBlock(currentToken);
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
  if (sliced == nullptr) throw InvalidForLoop(currentToken);
  if (!checkTokenType(ttype::colon)) throw InvalidForLoop(currentToken);
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

bool Parser::getNextToken(ExpectedTokens state) {
  getNextToken();

  if (checkTokenType(state)) return true;
  throw UnexpectedToken(currentToken);
}

bool Parser::getNextToken(ttype expectedType) {
  getNextToken();

  if (currentToken.getType() == expectedType) return true;
  throw UnexpectedToken(currentToken);
}

bool Parser::checkTokenType(ExpectedTokens state) {
  auto findResult = expectedTokens[state].find(currentToken.getType());
  if (findResult != expectedTokens[state].end()) return true;
  return false;
}

bool Parser::checkTokenType(ttype expectedType) {
  return currentToken.getType() == expectedType;
}
