// Copyright 2019 Kamil Mankowski

#ifndef SRC_PARSER_PROGRAM_H_
#define SRC_PARSER_PROGRAM_H_

#include <memory>
#include <string>
#include <utility>

#include "Instructions.h"

class Program {
 private:
  std::unique_ptr<CodeBlock> code;

 public:
  explicit Program(std::unique_ptr<CodeBlock> code) : code(std::move(code)) {}
  std::string codeToString() {
    std::string tmp = code->toString();
    return std::regex_replace(tmp, std::regex("(^|\n)  "), "$1");
  }
};

#endif  // SRC_PARSER_PROGRAM_H_
