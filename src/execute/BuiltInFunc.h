// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_BUILTINFUNC_H_
#define SRC_EXECUTE_BUILTINFUNC_H_

#include <iostream>
#include <memory>
#include <string>

#include "Instructions.h"

class PrintFunc : public Instruction {
 public:
  explicit PrintFunc(std::ostream &out) : out(out) {}

  TypeInstruction getInstructionType() override { return BuiltInFuncT; }
  std::string instrName() override { return "print"; }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::ostream &out;
};

#endif  // SRC_EXECUTE_BUILTINFUNC_H_
