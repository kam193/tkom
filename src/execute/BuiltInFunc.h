// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_BUILTINFUNC_H_
#define SRC_EXECUTE_BUILTINFUNC_H_

#include <iostream>
#include <memory>
#include <string>

#include "Instructions.h"

class PrintFunction : public Instruction {
 public:
  explicit PrintFunction(std::ostream &out) : out(out) {}

  std::string instrName() override { return "print"; }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::ostream &out;
};

class RangeFunction : public Instruction {
 public:
  RangeFunction() {}

  std::string instrName() override { return name; }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  const int PARAMS_SIZE = 1;
  std::string name = "range";
};

class LenFunction : public Instruction {
 public:
  LenFunction() {}

  std::string instrName() override { return name; }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  const int PARAMS_SIZE = 1;
  std::string name = "len";
};

#endif  // SRC_EXECUTE_BUILTINFUNC_H_
