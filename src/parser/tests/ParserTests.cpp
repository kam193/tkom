// Copyright 2019 Kamil Mankowski

#include <sstream>

#include <boost/test/unit_test.hpp>
#include "../Parser.h"

namespace tt = boost::test_tools;
using ttype = Token::Type;

BOOST_AUTO_TEST_SUITE(ParserTest)

BOOST_AUTO_TEST_CASE(test_function_def) {
  std::string program =
      "  def name(a, b, c):\n    if a == 4:\n      return True";
  std::stringstream input(program);

  Parser parser(input);
  parser.parse();

  // check expected code block
}

BOOST_AUTO_TEST_SUITE_END()
