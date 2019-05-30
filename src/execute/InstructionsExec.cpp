// Copyright 2019 Kamil Mankowski

#include "Instructions.h"

std::shared_ptr<Value> Constant::exec(std::shared_ptr<Context> ctx) {
  switch (type) {
    case ValueType::None:
      return std::make_shared<Value>(ValueType::None);
    case ValueType::Int:
      return std::make_shared<Value>(intValue);
    case ValueType::Real:
      return std::make_shared<Value>(realValue);
    case ValueType::Bool:
      return std::make_shared<Value>(boolValue);
    case ValueType::Text:
      return std::make_shared<Value>(strValue);
  }
  std::vector<std::shared_ptr<Value>> values;
  for (auto& elem : listElements) {
    auto val = elem->exec(ctx);
    values.push_back(val);
  }
  return std::make_shared<Value>(values);
}

std::shared_ptr<Value> Variable::exec(std::shared_ptr<Context> ctx) {
  auto val = ctx->getVariableValue(name);
  if (val == nullptr) throw ReadNotAssignVariable(name);
  return val;
}

std::shared_ptr<Value> Return::exec(std::shared_ptr<Context> ctx) {
  auto retValue = value->exec(ctx);
  return std::make_shared<Value>(retValue);
}

std::shared_ptr<Value> CodeBlock::exec(std::shared_ptr<Context> ctx) {
  for (int i = 0; i < instructions.size(); i++) {
    if (instructions[i]->getInstructionType() == FunctionT) {
      auto name = instructions[i]->instrName();
      ctx->setFunction(name, std::move(instructions[i]));
      continue;
    }
    auto result = instructions[i]->exec(ctx);
    if (isResultToReturn(result)) return result;
  }
  return std::make_shared<Value>(ValueType::None);
}

bool CodeBlock::isResultToReturn(std::shared_ptr<Value> result) {
  auto type = result->getType();
  return type == ValueType::T_BREAK || type == ValueType::T_CONTINUE ||
         type == ValueType::T_RETURN;
}

std::shared_ptr<Value> Slice::exec(std::shared_ptr<Context> ctx) {
  auto sourceValue = source->exec(ctx);
  if (sourceValue->getType() != ValueType::List)
    throw NotList(source->instrName());

  if (start < 0 || start > sourceValue->getList().size())
    throw OutOfRange(start);

  if (type == SliceType::Start) return sourceValue->getList()[start];
  if (type == SliceType::StartToEnd) end = sourceValue->getList().size();
  if (end < 0 || end > sourceValue->getList().size()) throw OutOfRange(end);

  std::vector<std::shared_ptr<Value>> resultElements;
  for (int i = start; i < end; ++i)
    resultElements.push_back(sourceValue->getList()[i]);
  return std::make_shared<Value>(resultElements);
}

std::shared_ptr<Value> FunctionCall::exec(std::shared_ptr<Context> ctx) {
  auto func = ctx->getFunction(name);
  if (func == nullptr) throw FunctionNotDeclared(name);

  auto callctx = std::make_shared<Context>(ctx);
  for (auto& arg : args) {
    auto argval = arg->exec(ctx);
    callctx->addParameter(argval);
  }

  return func->exec(callctx);
}

std::map<ValueType, std::map<Expression::Type, std::vector<ValueType>>>
    Expression::allowedOperands = {
        {ValueType::List,
         std::map<Expression::Type, std::vector<ValueType>>{
             {Expression::Type::Add, {ValueType::List}},
             {Expression::Type::Sub, {}},
             {Expression::Type::Mul, {ValueType::Int}},
             {Expression::Type::Div, {}},
             {Expression::Type::Exp, {}},
         }},
        {ValueType::Text,
         std::map<Expression::Type, std::vector<ValueType>>{
             {Expression::Type::Add, {ValueType::Text}},
             {Expression::Type::Sub, {}},
             {Expression::Type::Mul, {ValueType::Int}},
             {Expression::Type::Div, {}},
             {Expression::Type::Exp, {}},
         }},
        {ValueType::Int,
         std::map<Expression::Type, std::vector<ValueType>>{
             {Expression::Type::Add, {ValueType::Int, ValueType::Real}},
             {Expression::Type::Sub, {ValueType::Int, ValueType::Real}},
             {Expression::Type::Mul,
              {ValueType::Int, ValueType::Real, ValueType::List}},
             {Expression::Type::Div, {ValueType::Int, ValueType::Real}},
             {Expression::Type::Exp, {ValueType::Int, ValueType::Real}},
         }},
        {ValueType::Real,
         std::map<Expression::Type, std::vector<ValueType>>{
             {Expression::Type::Add, {ValueType::Int, ValueType::Real}},
             {Expression::Type::Sub, {ValueType::Int, ValueType::Real}},
             {Expression::Type::Mul, {ValueType::Int, ValueType::Real}},
             {Expression::Type::Div, {ValueType::Int, ValueType::Real}},
             {Expression::Type::Exp, {ValueType::Int, ValueType::Real}},
         }},
};

bool Expression::checkCompatibility(ValueType left, ValueType right,
                                    Expression::Type op) {
  if (left == ValueType::None) return false;
  auto findRight = std::find(allowedOperands[left][op].begin(),
                             allowedOperands[left][op].end(), right);
  return findRight != allowedOperands[left][op].end();
}

// TODO: FIX - no copy constr in Value
std::shared_ptr<Value> Expression::execExprList(std::shared_ptr<Value> list,
                                                std::shared_ptr<Value> right,
                                                Type op) {
  std::vector<std::shared_ptr<Value>> elements;
  if (op == Type::Mul) {
    for (int i = 0; i < right->getInt(); ++i) {
      for (auto& elem : list->getList())
        elements.push_back(std::make_shared<Value>(elem));
    }
  } else {
    for (auto& elem : list->getList())
      elements.push_back(std::make_shared<Value>(elem));
    for (auto& elem : right->getList())
      elements.push_back(std::make_shared<Value>(elem));
  }

  return std::make_shared<Value>(elements);
}

std::shared_ptr<Value> Expression::execExprStr(std::shared_ptr<Value> str,
                                               std::shared_ptr<Value> right,
                                               Type op) {
  std::string out = "";
  if (op == Type::Mul) {
    for (int i = 0; i < right->getInt(); ++i) out += str->getStr();
  } else {
    out = str->getStr() + right->getStr();
  }

  return std::make_shared<Value>(out);
}

std::shared_ptr<Value> Expression::execExprInt(int64_t left, int64_t right,
                                               Type op) {
  if (op == Expression::Type::Add) return std::make_shared<Value>(left + right);
  if (op == Expression::Type::Sub) return std::make_shared<Value>(left - right);
  if (op == Expression::Type::Mul) return std::make_shared<Value>(left * right);
  if (op == Expression::Type::Div) return std::make_shared<Value>(left / right);
  return std::make_shared<Value>((int64_t)std::pow(left, right));
}

std::shared_ptr<Value> Expression::execExprReal(double left, double right,
                                                Type op) {
  if (op == Expression::Type::Add) return std::make_shared<Value>(left + right);
  if (op == Expression::Type::Sub) return std::make_shared<Value>(left - right);
  if (op == Expression::Type::Mul) return std::make_shared<Value>(left * right);
  if (op == Expression::Type::Div) return std::make_shared<Value>(left / right);
  return std::make_shared<Value>(std::pow(left, right));
}

std::shared_ptr<Value> Expression::makeExpression(std::shared_ptr<Value> left,
                                                  std::shared_ptr<Value> right,
                                                  Expression::Type op) {
  auto leftType = left->getType();
  auto rightType = right->getType();

  if (leftType == ValueType::List || rightType == ValueType::List) {
    if (leftType == ValueType::List)
      return execExprList(left, right, op);
    else
      return execExprList(right, left, op);
  }
  if (leftType == ValueType::Text || rightType == ValueType::Text) {
    if (leftType == ValueType::Text)
      return execExprStr(left, right, op);
    else
      return execExprStr(right, left, op);
  }
  if (leftType == ValueType::Real || rightType == ValueType::Real) {
    if (leftType == ValueType::Int) left->setReal(left->getInt());
    if (rightType == ValueType::Int) right->setReal(right->getInt());
    return execExprReal(left->getReal(), right->getReal(), op);
  }
  return execExprInt(left->getInt(), right->getInt(), op);
}

std::shared_ptr<Value> Expression::exec(std::shared_ptr<Context> ctx) {
  auto left = args[0]->exec(ctx);
  int i = 1;
  for (auto op : types) {
    if (i >= args.size()) throw UnexpectedError();
    auto right = args[i]->exec(ctx);

    if (!checkCompatibility(left->getType(), right->getType(), op))
      throw OperandsTypesNotCompatible("", "", Expression::typeToString(op));

    left = makeExpression(left, right, op);
    ++i;
  }
  return left;
}

std::shared_ptr<Value> AssignExpr::exec(std::shared_ptr<Context> ctx) {
  if (type == Type::Assign) {
    auto value = expression->exec(ctx);
    ctx->setVariable(variableName, value);
    return value;
  } else {
    auto old = ctx->getVariableValue(variableName);
    if (old == nullptr) throw ReadNotAssignVariable(variableName);

    auto value = expression->exec(ctx);
    auto op =
        type == Type::AddAssign ? Expression::Type::Add : Expression::Type::Sub;

    if (!Expression::checkCompatibility(old->getType(), value->getType(), op))
      throw OperandsTypesNotCompatible("", "", Expression::typeToString(op));

    auto newvalue = Expression::makeExpression(old, value, op);
    ctx->setVariable(variableName, newvalue);
    return newvalue;
  }
}

std::shared_ptr<Value> For::exec(std::shared_ptr<Context> ctx) {
  auto rangeList = range->exec(ctx);
  if (rangeList->getType() != ValueType::List) throw IterableExpected();

  std::shared_ptr<Value> result;
  for (auto value : rangeList->getList()) {
    ctx->setVariable(iterator, value);
    result = code->exec(ctx);
    if (result->getType() == ValueType::T_BREAK) break;
    if (result->getType() == ValueType::T_CONTINUE) continue;
    if (result->getType() == ValueType::T_RETURN) return result->getValuePtr();
  }

  return std::make_shared<Value>(ValueType::None);
}
