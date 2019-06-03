// Copyright 2019 Kamil Mankowski

#include "Program.h"

void Program::run() {
  try {
    Parser parser(in);
    auto code = parser.parse();

    auto global = makeGlobalContext();
    auto result = code->exec(global);
  } catch (ParserExceptionBase e) {
    std::cout << e.what() << std::endl;
  } catch (ExecuteExceptionBase e) {
    std::cout << e.what() << std::endl;
  }
}

std::shared_ptr<Context> Program::makeGlobalContext() {
  auto ctx = std::make_shared<Context>();

  auto print = std::make_shared<PrintFunction>(out);
  ctx->setFunction(print->instrName(), print);

  auto range = std::make_shared<RangeFunction>();
  ctx->setFunction(range->instrName(), range);

  auto len = std::make_shared<LenFunction>();
  ctx->setFunction(len->instrName(), len);

  return ctx;
}
