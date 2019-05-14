// Copyright 2019 Kamil Mankowski

#include <sstream>

#include <boost/test/unit_test.hpp>
#include "../Parser.h"

namespace tt = boost::test_tools;
using ttype = Token::Type;

// to do: prepare tests for toString()

BOOST_AUTO_TEST_SUITE(ParserTest)

void assertExpectedCode(const std::string &code, const std::string &expected) {
  std::stringstream input(code);
  Parser parser(input);
  auto parsed = parser.parse();
  BOOST_TEST(parsed.codeToString() == expected);
}

void assertExpectedCode(const std::string &code) {
  assertExpectedCode(code, code);
}

BOOST_AUTO_TEST_CASE(test_dumm_function_def) {
  std::string program = "def name(a, b, c):\n  return";
  std::string expected = "def name(a, b, c):\n  return None";
  assertExpectedCode(program, expected);
}

BOOST_AUTO_TEST_CASE(test_function_def) {
  std::string program = "def name(a, b, c):\n  if a * 5 == 4:\n    return True";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_variable_assign) {
  std::string program = "variable = 15\nsecond = \"aaaaa\"";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_SUITE_END()
