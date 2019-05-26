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
  if (val == nullptr)
    throw ReadNotAssignVariable(name);
  return val;
}
