// Copyright 2019 Kamil Mankowski

#include "Program.h"

void Program::run() {
  Parser parser(in);
  auto code = parser.parse();

  auto global = makeGlobalContext();
  auto result = code->exec(global);
}

std::shared_ptr<Context> Program::makeGlobalContext() {
  auto ctx = std::make_shared<Context>();

  auto print = std::make_shared<PrintFunction>(out);
  ctx->setFunction(print->instrName(), print);

  auto range = std::make_shared<RangeFunction>();
  ctx->setFunction(range->instrName(), range);

  return ctx;
}
