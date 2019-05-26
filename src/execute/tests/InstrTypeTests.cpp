// Copyright 2019 Kamil Mankowski

#include <boost/test/unit_test.hpp>

#include "../Instructions.h"

BOOST_AUTO_TEST_SUITE(InstructionsTypesTest)

BOOST_AUTO_TEST_CASE(test_function_override_type) {
  std::string name = "func_name";
  Function func(name);
  BOOST_TEST(func.getInstructionType() == FunctionT);
}

BOOST_AUTO_TEST_SUITE_END()
