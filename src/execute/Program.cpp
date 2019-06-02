// Copyright 2019 Kamil Mankowski

#include "Program.h"

// TODO: tests & exeptions
void Program::run() {
  Parser parser(in);
  auto code = parser.parse();

  auto global = makeGlobalContext();
  auto result = code->exec(global);

  out << "\nEXECUTING END.";
}

std::shared_ptr<Context> Program::makeGlobalContext() {
  auto ctx = std::make_shared<Context>();

  auto print = std::make_shared<PrintFunc>(out);
  ctx->setFunction(print->instrName(), print);

  return ctx;
}
