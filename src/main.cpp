// Copyright 2019 Kamil Mankowski

#include <iomanip>
#include <iostream>

#include "scanner/Scanner.h"

int main() {
  Scanner scan(std::cin);

  std::cout << " LINE | COL | TOKEN | STR_VALUE | DOUBLE_VALUE | INT_VALUE\n";
  Token token;
  while ((token = scan.getNextToken()).getType() != Token::Type::eof) {
    std::cout << std::setw(5) << token.getLine() << " | " << std::setw(3)
              << token.getColumn() << " | " << std::setw(5)
              << static_cast<std::underlying_type<Token::Type>::type>(
                     token.getType())
              << " | " << std::setw(9) << token.getString() << " | "
              << token.getReal() << " | " << token.getInteger() << std::endl;
  }
  return 0;
}
