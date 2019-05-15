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

BOOST_AUTO_TEST_CASE(test_basic_variable_assign) {
  std::string program = "variable = False\nsecond -= \"aa\"\nthird += 15";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_function_calls) {
  std::string program = "func1()\nfunc2(a, b, 20 * 3, func1())";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_assign_with_expression) {
  std::string program = "var = 15 * 7 + 24 * 3 ^ 7";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_while_loop) {
  std::string program = "while i < 17:\n  func(i * 24)\n  i += 1\n  continue";
  assertExpectedCode(program);
}

// BOOST_AUTO_TEST_CASE(test_for_loop) {
//   std::string program = "for i in var:\n  do_sth()\nfor e in fun():\n  break";
//   assertExpectedCode(program);
// }

// BOOST_AUTO_TEST_CASE(test_list_and_slice) {
//   std::string program =
//       "val = [12, b, run()]\nv2 = val[:]\nv3 = val[1]\nv4 = fun()[-3]";
//   assertExpectedCode(program);
// }

BOOST_AUTO_TEST_SUITE_END()
