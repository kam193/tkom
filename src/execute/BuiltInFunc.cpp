// Copyright 2019 Kamil Mankowski

#include "BuiltInFunc.h"

std::shared_ptr<Value> PrintFunction::exec(std::shared_ptr<Context> ctx) {
  for (int i = 0; i < ctx->parametersSize(); ++i) {
    if (ctx->getParameter(i)->getType() == ValueType::Text)
      out << ctx->getParameter(i)->getStr() << " ";
    else
      out << ctx->getParameter(i)->toString() << " ";
  }
  out << "\n";
  return std::make_shared<Value>(ValueType::None);
}

std::shared_ptr<Value> RangeFunction::exec(std::shared_ptr<Context> ctx) {
  if (ctx->parametersSize() != PARAMS_SIZE)
    throw ParametersCountNotExpected(name, ctx->parametersSize(), PARAMS_SIZE);

  auto end = ctx->getParameter(0);
  if (end->getType() != ValueType::Int) throw TypeNotExpected("int");

  std::vector<std::shared_ptr<Value>> elements;
  for (int64_t i = 0; i < end->getInt(); ++i)
    elements.push_back(std::make_shared<Value>(i));

  return std::make_shared<Value>(elements);
}

std::shared_ptr<Value> LenFunction::exec(std::shared_ptr<Context> ctx) {
  if (ctx->parametersSize() != PARAMS_SIZE)
    throw ParametersCountNotExpected(name, ctx->parametersSize(), PARAMS_SIZE);

  auto input = ctx->getParameter(0);
  if (input->getType() != ValueType::List &&
      input->getType() != ValueType::Text)
    throw TypeNotExpected("list, string");

  int64_t size = 0;
  if (input->getType() == ValueType::List)
    size = input->getList().size();
  else
    size = input->getStr().size();

  return std::make_shared<Value>(size);
}
