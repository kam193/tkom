// Copyright 2019 Kamil Mankowski

#include "BuiltInFunc.h"

// TODO: tests
std::shared_ptr<Value> PrintFunc::exec(std::shared_ptr<Context> ctx) {
  for (int i = 0; i < ctx->parametersSize(); ++i)
    out << ctx->getParameter(i)->toString() << " ";
  out << "\n";
  return std::make_shared<Value>();
}
