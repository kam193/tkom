// Copyright 2019 Kamil Mankowski

#include <sstream>

#include <boost/test/included/unit_test.hpp>
#include "../Scanner.h"

namespace tt = boost::test_tools;
using ttype = Token::Type;

BOOST_AUTO_TEST_SUITE(ScannerTest)

BOOST_AUTO_TEST_CASE(test_integer_numbers_recognize) {
  std::string program = "123 0xab9 0 0xD 0123456789";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  int64_t expected[] = {123, 2745, 0, 13, 123456789};
  for (auto& expValue : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == ttype::integerNumber));
    BOOST_TEST(token.getInteger() == expValue);
  }
}

BOOST_AUTO_TEST_CASE(test_real_numbers_recognize) {
  std::string program = "12.3 0.5 9. 0. 0123.6";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  double expected[] = {12.3, 0.5, 9.0, 0.0, 123.6};
  for (auto& expValue : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == ttype::realNumber));
    BOOST_TEST(token.getReal() == expValue, tt::tolerance(0.00001));
  }
}

BOOST_AUTO_TEST_CASE(test_boolean_none_recognize) {
  std::string program = "True False None";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  ttype expected[] = {ttype::trueT, ttype::falseT, ttype::none};
  for (auto& expType : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == expType));
  }
}

BOOST_AUTO_TEST_CASE(test_loop_conditional_recognize) {
  std::string program = "for while if else";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  ttype expected[] = {ttype::forT, ttype::whileT, ttype::ifT, ttype::elseT};
  for (auto& expType : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == expType));
  }
}

BOOST_AUTO_TEST_CASE(test_controlkeywords_def_recognize) {
  std::string program = "return continue def";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  ttype expected[] = {ttype::returnT, ttype::continueT, ttype::def};
  for (auto& expType : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == expType));
  }
}

BOOST_AUTO_TEST_CASE(test_whitespace_recognize_and_ignore) {
  std::string program = "   \n    a \t b\n";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  ttype expected[] = {ttype::space,      ttype::nl,         ttype::space,
                      ttype::identifier, ttype::identifier, ttype::nl,
                      ttype::eof};
  for (auto& expType : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == expType));
  }
}

// TODO:
// space counting
// after EOF, return EOF when getNextToken() called
// operators
// assigment
// strings
// multiline
// not a token, erros like 123abc, "aaa, ...
// line & position counting

BOOST_AUTO_TEST_SUITE_END()
