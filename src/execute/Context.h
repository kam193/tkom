// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_CONTEXT_H_
#define SRC_EXECUTE_CONTEXT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Instructions.h"
#include "Value.h"

class Context {
 public:
  Context() {}
  explicit Context(std::shared_ptr<Context> parentContext)
      : parent(parentContext) {}
  std::shared_ptr<Instruction> getFunction(std::string name);
  void setFunction(std::string name, std::shared_ptr<Instruction> func);
  std::shared_ptr<Value> getVariableValue(std::string name);
  void setVariable(std::string name, std::shared_ptr<Value> value) {
    vars[name] = value;
  }
  std::shared_ptr<Value> getParameter(size_t index);
  void addParameter(std::shared_ptr<Value> param) { params.push_back(param); }
  size_t parametersSize() { return params.size(); }

 private:
  std::shared_ptr<Context> parent = nullptr;
  std::vector<std::shared_ptr<Value>> params;
  std::map<std::string, std::shared_ptr<Instruction>> funcs;
  std::map<std::string, std::shared_ptr<Value>> vars;
};

#endif  // SRC_EXECUTE_CONTEXT_H_
