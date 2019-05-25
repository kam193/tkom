// Copyright 2019 Kamil Mankowski

#include <boost/test/unit_test.hpp>

#include "../Context.h"

BOOST_AUTO_TEST_SUITE(ContextTest)

BOOST_AUTO_TEST_CASE(test_add_function) {
  Context cxt;
  auto func = std::make_shared<Instruction>();
  std::string name = "myfunc";

  cxt.setFunction(name, func);

  BOOST_TEST(cxt.getFunction(name) == func);
}

BOOST_AUTO_TEST_CASE(test_cannot_redefine_function) {
  Context cxt;
  auto func1 = std::make_shared<Instruction>();
  auto func2 = std::make_shared<Instruction>();
  std::string name = "myfunc";

  cxt.setFunction(name, func1);

  BOOST_CHECK_THROW(cxt.setFunction(name, func2), std::exception);
}

BOOST_AUTO_TEST_CASE(test_add_variable) {
  Context cxt;
  auto val = std::make_shared<Value>(18L);
  std::string name = "myval";

  cxt.setVariable(name, val);

  auto cxt_val = cxt.getVariableValue(name);
  BOOST_TEST(cxt_val == val);
  BOOST_TEST(cxt_val->getInt() == 18);
}

BOOST_AUTO_TEST_CASE(test_variable_changed) {
  Context cxt;
  auto val = std::make_shared<Value>(18L);
  std::string name = "myval";

  cxt.setVariable(name, val);
  auto cxt_val = cxt.getVariableValue(name);
  BOOST_TEST(cxt_val->getInt() == 18);

  cxt_val->setInt(22L);
  BOOST_TEST(cxt.getVariableValue(name)->getInt() == 22);
}

BOOST_AUTO_TEST_CASE(test_add_params) {
  Context cxt;
  auto param1 = std::make_shared<Value>(18L);
  auto param2 = std::make_shared<Value>(18L);

  BOOST_TEST(cxt.parametersSize() == 0);
  cxt.addParameter(param1);
  cxt.addParameter(param2);

  BOOST_TEST(cxt.parametersSize() == 2);
  BOOST_TEST(cxt.getParameter(0) == param1);
  BOOST_TEST(cxt.getParameter(1) == param2);
}

BOOST_AUTO_TEST_CASE(test_get_not_existed_function) {
  Context cxt;
  std::string name = "none";

  BOOST_TEST(cxt.getFunction(name) == nullptr);
}

BOOST_AUTO_TEST_CASE(test_get_not_existed_variable) {
  Context cxt;
  std::string name = "none";

  BOOST_TEST(cxt.getVariableValue(name) == nullptr);
}

BOOST_AUTO_TEST_CASE(test_get_not_existed_param) {
  Context cxt;

  BOOST_TEST(cxt.getParameter(10) == nullptr);
}

BOOST_AUTO_TEST_CASE(test_get_inherited_function) {
  auto parent = std::make_shared<Context>();
  auto func = std::make_shared<Instruction>();
  std::string name = "myfunc";
  parent->setFunction(name, func);

  Context cxt(parent);

  BOOST_TEST(cxt.getFunction(name) == func);
}

BOOST_AUTO_TEST_CASE(test_get_inherited_variable) {
  auto parent = std::make_shared<Context>();
  auto val = std::make_shared<Value>(18L);
  std::string name = "myval";
  parent->setVariable(name, val);

  Context cxt(parent);

  auto cxt_val = cxt.getVariableValue(name);
  BOOST_TEST(cxt_val == val);
  BOOST_TEST(cxt_val->getInt() == 18);
}

BOOST_AUTO_TEST_CASE(test_params_not_inherited) {
  auto parent = std::make_shared<Context>();
  auto val = std::make_shared<Value>(18L);
  parent->addParameter(val);

  Context cxt(parent);

  BOOST_TEST(cxt.parametersSize() == 0);
}

BOOST_AUTO_TEST_CASE(test_change_inherited_variable) {
  auto parent = std::make_shared<Context>();
  auto val = std::make_shared<Value>(18L);
  std::string name = "myval";
  parent->setVariable(name, val);

  Context cxt(parent);
  auto cxt_val = cxt.getVariableValue(name);
  BOOST_TEST(cxt_val->getInt() == 18);
  BOOST_TEST(parent->getVariableValue(name)->getInt() == 18);

  cxt_val->setInt(22L);

  BOOST_TEST(cxt.getVariableValue(name)->getInt() == 22);
  BOOST_TEST(parent->getVariableValue(name)->getInt() == 22);
}

BOOST_AUTO_TEST_CASE(test_cover_inherited_variable) {
  auto parent = std::make_shared<Context>();
  auto val1 = std::make_shared<Value>(18L);
  auto val2 = std::make_shared<Value>(22L);
  std::string name = "myval";
  parent->setVariable(name, val1);

  Context cxt(parent);
  BOOST_TEST(cxt.getVariableValue(name) == val1);
  BOOST_TEST(parent->getVariableValue(name) == val1);

  cxt.setVariable(name, val2);

  BOOST_TEST(cxt.getVariableValue(name) == val2);
  BOOST_TEST(parent->getVariableValue(name) == val1);
}

BOOST_AUTO_TEST_CASE(test_cover_inherited_function) {
  auto parent = std::make_shared<Context>();

  auto func1 = std::make_shared<Instruction>();
  auto func2 = std::make_shared<Instruction>();
  std::string name = "myfunc";

  parent->setFunction(name, func1);
  Context cxt(parent);
  BOOST_TEST(cxt.getFunction(name) == func1);

  cxt.setFunction(name, func2);
  BOOST_TEST(cxt.getFunction(name) == func2);
  BOOST_TEST(parent->getFunction(name) == func1);
}

BOOST_AUTO_TEST_SUITE_END()
