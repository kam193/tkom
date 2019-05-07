// Copyright 2019 Kamil Mankowski

#ifndef SRC_PARSER_INSTRUCTIONS_H_
#define SRC_PARSER_INSTRUCTIONS_H_

#include <list>
#include <string>
#include <memory>

class Instruction {
  // pozycja w tekście
  // metoda ewaluacji
  // metoda wypisania
};

class CodeBlock : public Instruction {  // public virtual?
 public:
  std::list<std::unique_ptr<Instruction>> instructions;  // NOT A PUBLIC!!!!
};

class Function : public Instruction {
 public:
  std::unique_ptr<CodeBlock> code;  // NOT A PUBLIC
  std::list<std::string> argumentNames;
  std::string name;

  // evaluate (dziedziczone) -> dodaje do listy funkcji
  // call -> woła funkcję
};

#endif  // SRC_PARSER_INSTRUCTIONS_H_
