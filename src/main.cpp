// Copyright 2019 Kamil Mankowski

#include <iostream>
#include <sstream>
#include "scanner/Scanner.h"

int main() {
  std::stringstream sss("   its a for  112  token ?? += \"aaaa\" \n   sss");

  Scanner scan(sss);

  Token t;
  while ((t = scan.getNextToken()).getType() != Token::Type::eof) {
    std::cout << static_cast<std::underlying_type<Token::Type>::type>(
                     t.getType())
              << " " << t.getInteger() << " " << t.getString() << std::endl;
  }
  return 0;
}
