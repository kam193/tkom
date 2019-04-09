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

BOOST_AUTO_TEST_CASE(test_positions) {
  std::string program = "a1 \n   a2 a3";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  int expected[][2] = {{1, 2}, {1, 4}, {2, 3}, {2, 5}, {2, 8}};
  for (auto& expPosition : expected) {
    token = scanner.getNextToken();
    BOOST_TEST(token.getLine() == expPosition[0]);
    BOOST_TEST(token.getColumn() == expPosition[1]);
  }
}

BOOST_AUTO_TEST_CASE(test_space_counting) {
  std::string program = "   \n  \t  \n\r\r\n";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  int expected[] = {3, 5, 2};
  for (auto& expSize : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == ttype::space));
    BOOST_TEST(token.getInteger() == expSize);
    scanner.getNextToken();  // ignore NL
  }
}

BOOST_AUTO_TEST_CASE(test_eof_after_eof) {
  std::string program = "token";
  std::stringstream input(program);

  Scanner scanner(input);

  scanner.getNextToken();
  BOOST_TEST((scanner.getNextToken().getType() == ttype::eof));
  BOOST_TEST((scanner.getNextToken().getType() == ttype::eof));
}

BOOST_AUTO_TEST_CASE(test_single_punct_recognize) {
  std::string program = "( ) [ ] : , =";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  ttype expected[] = {ttype::openBracket,
                      ttype::closeBracket,
                      ttype::openSquareBracket,
                      ttype::closeSquareBracket,
                      ttype::colon,
                      ttype::comma,
                      ttype::assign};
  for (auto& expType : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == expType));
  }
}

BOOST_AUTO_TEST_CASE(test_math_operator_recognize) {
  std::string program = "+ - / ^ * += -=";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  ttype expected[] = {ttype::add,      ttype::sub,      ttype::divOp,
                      ttype::expOp,    ttype::multipOp, ttype::addAssign,
                      ttype::subAssign};
  for (auto& expType : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == expType));
  }
}

BOOST_AUTO_TEST_CASE(test_comparation_operator_recognize) {
  std::string program = "< > >= <= != ==";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  ttype expected[] = {ttype::less,   ttype::greater, ttype::greaterEq,
                      ttype::lessEq, ttype::diff,    ttype::equal};
  for (auto& expType : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == expType));
  }
}

BOOST_AUTO_TEST_CASE(test_string_recognize) {
  std::string program =
      "\"lorem\" \"ips ?? 1234 e $ um \" \" 34 == 2 + 1 return \" ";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  std::string expected[] = {"lorem", "ips ?? 1234 e $ um ",
                            " 34 == 2 + 1 return "};
  for (auto& expStr : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == ttype::stringT));
    BOOST_TEST(token.getString() == expStr);
  }
}

BOOST_AUTO_TEST_CASE(test_invalid_token) {
  std::string program = "?&* 123abs 0x12Q \"oh no \n";
  std::stringstream input(program);
  Token token;

  Scanner scanner(input);

  std::string expected[] = {"?&*", "123abs", "0x12Q", "oh no "};
  for (auto& expStr : expected) {
    token = scanner.getNextToken();
    BOOST_TEST((token.getType() == ttype::NaT));
    BOOST_TEST(token.getString() == expStr);
  }
}

BOOST_AUTO_TEST_SUITE_END()
