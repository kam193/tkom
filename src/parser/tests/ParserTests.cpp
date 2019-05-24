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

template <typename ExpectedException>
void assertExpectedException(const std::string &code) {
  std::stringstream input(code);
  Parser parser(input);
  BOOST_CHECK_THROW(parser.parse(), ExpectedException);
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

BOOST_AUTO_TEST_CASE(test_expression_multiple_times) {
  std::string program =
      "var = 15 * 5 * 4 / 3\n"
      "var = 1 + 3 + 4 - 2 - 8\n"
      "var = 1 ^ 2 ^ 2 ^ 2";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_while_loop) {
  std::string program = "while i < 17:\n  func(i * 24)\n  i += 1\n  continue";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_for_loop) {
  std::string program = "for i in var:\n  do_sth()\nfor e in fun():\n  break";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_list_and_slice) {
  std::string program =
      "val = [12, b, run()]\nv2 = val[:]\nv3 = val[1]\nv4 = fun()[:3]";
  std::string expected =
      "val = [12, b, run()]\nv2 = val[0:]\nv3 = val[1]\nv4 = fun()[0:3]";
  assertExpectedCode(program, expected);
}

BOOST_AUTO_TEST_CASE(test_list_complex) {
  std::string program =
      "val = [[1, 2, 3], "
      "15, run(23, \"a\"), "
      "9 * 23, a * 5 + rand(), "
      "[a, b][0:2]][5]";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_example_program) {
  std::string program =
      "def function_name(arg1):\n"
      "  x = arg1 / 3\n"
      "  return x + arg1\n"
      "function_name(23)";
  assertExpectedCode(program);
}

BOOST_AUTO_TEST_CASE(test_unexpected_token) {
  std::string program = "?";
  assertExpectedException<UnexpectedToken>(program);
}

BOOST_AUTO_TEST_CASE(test_no_expected_codeblock) {
  std::string program = "def fun():\n";
  assertExpectedException<ExpectedCodeBlock>(program);
}

BOOST_AUTO_TEST_CASE(test_invalid_use_of_return) {
  std::string program = "def fun():\n  return()\n";
  assertExpectedException<UnexpectedAfterReturn>(program);
}

BOOST_AUTO_TEST_CASE(test_invalid_expression) {
  std::string program = "a = 44 *";
  assertExpectedException<IncorrectExpression>(program);
}

BOOST_AUTO_TEST_CASE(test_invalid_compare_no_op) {
  std::string program = "if a b:";
  assertExpectedException<InvalidCompareExpression>(program);
}

BOOST_AUTO_TEST_CASE(test_invalid_funccall) {
  std::string program = "fun(ab if)";
  assertExpectedException<InvalidFunctionCall>(program);
}

BOOST_AUTO_TEST_CASE(test_invalid_slice) {
  std::string program = "a = t[1";
  assertExpectedException<NoEndOfSlice>(program);
}

BOOST_AUTO_TEST_CASE(test_invalid_list_element) {
  std::string program = "a = [1, +]";
  assertExpectedException<InvalidListElement>(program);
}

BOOST_AUTO_TEST_CASE(test_invalid_assign) {
  std::string program = "a = ";
  assertExpectedException<InvalidAssign>(program);
}

BOOST_AUTO_TEST_CASE(test_invalid_for) {
  std::string program = "for e in:";
  assertExpectedException<InvalidForLoop>(program);
}

BOOST_AUTO_TEST_SUITE_END()
