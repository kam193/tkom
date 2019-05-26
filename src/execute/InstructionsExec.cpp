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
