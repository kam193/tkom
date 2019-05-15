// Copyright 2019 Kamil Mankowski

#include <iomanip>
#include <iostream>

#include <sstream>
#include <string>

#include "parser/Parser.h"
#include "scanner/Scanner.h"

int main() {
  std::string program = "while i < 17:\n  func(i * 24)\n  i += 1\n  continue\n";
  std::cout << program << std::endl;
  std::stringstream input(program);

  Parser parser(input);
  auto parsed = parser.parse();

  std::cout << "Parsing end" << std::endl;
  std::cout << parsed.codeToString();

  // Scanner scan(input);

  // std::cout << " LINE | COL | TOKEN | STR_VALUE | DOUBLE_VALUE | INT_VALUE\n";
  // Token token;
  // while ((token = scan.getNextToken()).getType() != Token::Type::eof) {
  //   std::cout << std::setw(5) << token.getLine() << " | " << std::setw(3)
  //             << token.getColumn() << " | " << std::setw(5)
  //             << static_cast<std::underlying_type<Token::Type>::type>(
  //                    token.getType())
  //             << " | " << std::setw(9) << token.getString() << " | "
  //             << token.getReal() << " | " << token.getInteger() << std::endl;
  // }
  return 0;
}
