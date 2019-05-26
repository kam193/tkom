// Copyright 2019 Kamil Mankowski

#include <boost/test/unit_test.hpp>

#include "../Context.h"
#include "../ExecuteExceptions.h"
#include "../Instructions.h"

BOOST_AUTO_TEST_SUITE(InstrExecTest)

class MockInstruction : public Instruction {
 public:
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override {
    ++executedCount;
    return std::make_shared<Value>();
  }
  static int getExecutedCount() { return executedCount; }
  static void resetExecutedCount() { executedCount = 0; }

 private:
  static int executedCount;
};

int MockInstruction::executedCount = 0;

std::unique_ptr<MockInstruction> mock_instr() {
  return std::make_unique<MockInstruction>();
}
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

BOOST_AUTO_TEST_CASE(test_return_exec) {
  auto ctx = empty_context();
  auto instr = mock_instr();

  MockInstruction::resetExecutedCount();
  Return ret;
  ret.setValue(std::move(instr));

  auto result = ret.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::T_RETURN));
  BOOST_TEST((result->getValuePtr()->getType() == ValueType::None));
  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
}

BOOST_AUTO_TEST_CASE(test_break_exec) {
  auto ctx = empty_context();

  Break br;
  auto result = br.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::T_BREAK));
}

BOOST_AUTO_TEST_CASE(test_continue_exec) {
  auto ctx = empty_context();

  Continue cn;
  auto result = cn.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::T_CONTINUE));
}

BOOST_AUTO_TEST_CASE(test_code_block_simple) {
  auto ctx = empty_context();
  CodeBlock cb;
  cb.addInstruction(mock_instr());
  cb.addInstruction(mock_instr());
  cb.addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  auto result = cb.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::None));
  BOOST_TEST(MockInstruction::getExecutedCount() == 3);
}

BOOST_AUTO_TEST_CASE(test_code_block_break) {
  auto ctx = empty_context();
  CodeBlock cb;
  cb.addInstruction(mock_instr());
  cb.addInstruction(std::make_unique<Break>());
  cb.addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  auto result = cb.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::T_BREAK));
  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
}

BOOST_AUTO_TEST_CASE(test_code_block_continue) {
  auto ctx = empty_context();
  CodeBlock cb;
  cb.addInstruction(mock_instr());
  cb.addInstruction(std::make_unique<Continue>());
  cb.addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  auto result = cb.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::T_CONTINUE));
  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
}

BOOST_AUTO_TEST_CASE(test_code_block_return) {
  auto ctx = empty_context();
  auto retinstr = std::make_unique<Return>();
  retinstr->setValue(std::make_unique<Constant>(ValueType::None));
  CodeBlock cb;
  cb.addInstruction(mock_instr());
  cb.addInstruction(std::move(retinstr));
  cb.addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  auto result = cb.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::T_RETURN));
  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
}

BOOST_AUTO_TEST_CASE(test_code_block_declare_func) {
  auto ctx = empty_context();
  std::string name = "func_name";
  auto func = std::make_unique<Function>(name);

  CodeBlock cb;
  cb.addInstruction(std::move(func));
  cb.exec(ctx);

  BOOST_TEST(ctx->getFunction(name) != nullptr);
}

BOOST_AUTO_TEST_SUITE_END()
