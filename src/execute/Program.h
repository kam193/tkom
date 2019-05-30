// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_PROGRAM_H_
#define SRC_EXECUTE_PROGRAM_H_

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "../parser/Parser.h"
#include "BuiltInFunc.h"
#include "Context.h"

class Program {
 private:
  std::istream &in;
  std::ostream &out;
  std::shared_ptr<Context> makeGlobalContext();

 public:
  explicit Program(std::istream &in, std::ostream &out) : in(in), out(out) {}
  void run();
};

#endif  // SRC_EXECUTE_PROGRAM_H_
