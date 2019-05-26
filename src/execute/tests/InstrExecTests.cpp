// Copyright 2019 Kamil Mankowski

#include <boost/test/unit_test.hpp>

#include "../Context.h"
#include "../ExecuteExceptions.h"
#include "../Instructions.h"

BOOST_AUTO_TEST_SUITE(InstrExecTest)

std::shared_ptr<Context> empty_context() { return std::make_shared<Context>(); }

BOOST_AUTO_TEST_CASE(test_simple_constants_exec_value) {
  Constant none(ValueType::None);
  auto val = none.exec(empty_context());
  BOOST_TEST((val->getType() == ValueType::None));

  Constant boolean(true);
  val = boolean.exec(empty_context());
  BOOST_TEST((val->getType() == ValueType::Bool));
  BOOST_TEST(val->getBool() == true);

  Constant integer(23L);
  val = integer.exec(empty_context());
  BOOST_TEST((val->getType() == ValueType::Int));
  BOOST_TEST(val->getInt() == 23);

  Constant real(1.55);
  val = real.exec(empty_context());
  BOOST_TEST((val->getType() == ValueType::Real));
  BOOST_TEST(val->getReal() == 1.55);

  Constant string(std::string{"test"});
  val = string.exec(empty_context());
  BOOST_TEST((val->getType() == ValueType::Text));
  BOOST_TEST(val->getStr() == "test");
}

BOOST_AUTO_TEST_CASE(test_list_constant_exec_values) {
  std::vector<std::unique_ptr<Instruction>> elements;
  elements.push_back(std::make_unique<Constant>(1L));
  elements.push_back(std::make_unique<Constant>(std::string{"element2"}));
  elements.push_back(std::make_unique<Constant>(false));

  Constant list(std::move(elements));
  auto val = list.exec(empty_context());

  auto list_vals = val->getList();
  BOOST_TEST(list_vals.size() == 3);
  BOOST_TEST(list_vals[0]->getInt() == 1);
  BOOST_TEST(list_vals[1]->getStr() == "element2");
  BOOST_TEST(list_vals[2]->getBool() == false);
}

BOOST_AUTO_TEST_CASE(test_variable_get_from_ctx) {
  auto ctx = empty_context();
  std::string name = "var";
  auto value = std::make_shared<Value>();

  ctx->setVariable(name, value);
  Variable var(name);
  auto result = var.exec(ctx);

  BOOST_TEST(result == value);
}

BOOST_AUTO_TEST_CASE(test_variable_throw_when_not_exists) {
  auto ctx = empty_context();
  std::string name = "var";

  Variable var(name);

  BOOST_CHECK_THROW(var.exec(ctx), ReadNotAssignVariable);
}

BOOST_AUTO_TEST_SUITE_END()
