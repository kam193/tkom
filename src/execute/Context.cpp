// Copyright 2019 Kamil Mankowski

#include "Context.h"

std::shared_ptr<Instruction> Context::getFunction(std::string name) {
  if (funcs.count(name) == 1) return funcs[name];
  if (parent == nullptr) return nullptr;
  return parent->getFunction(name);
}

void Context::setFunction(std::string name, std::shared_ptr<Instruction> func) {
  if (funcs.count(name) != 0)
    throw std::runtime_error("Try to redefine function");
  funcs[name] = func;
}

std::shared_ptr<Value> Context::getVariableValue(std::string name) {
  if (vars.count(name) == 1) return vars[name];
  if (parent == nullptr) return nullptr;
  return parent->getVariableValue(name);
}

std::shared_ptr<Value> Context::getParameter(size_t index) {
  if (index < params.size()) return params[index];
  return nullptr;
}
