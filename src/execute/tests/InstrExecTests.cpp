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

template <typename ConstType>
std::unique_ptr<Constant> constant(ConstType value) {
  return std::make_unique<Constant>(value);
}

// [1, 2, 3]
std::unique_ptr<Constant> get_list_of_ints() {
  std::vector<std::unique_ptr<Instruction>> elements;
  elements.push_back(constant<int64_t>(1L));
  elements.push_back(constant<int64_t>(2L));
  elements.push_back(constant<int64_t>(3L));

  return std::make_unique<Constant>(std::move(elements));
}

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
  elements.push_back(constant<int64_t>(1L));
  elements.push_back(constant<std::string>("element2"));
  elements.push_back(constant<bool>(false));

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

BOOST_AUTO_TEST_CASE(test_slice_get_second_pos) {
  auto ctx = empty_context();
  auto baseList = get_list_of_ints();

  Slice slice(Slice::SliceType::Start, 1, 0);  // var[1]
  slice.setSource(std::move(baseList));

  auto result = slice.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::Int));
  BOOST_TEST(result->getInt() == 2);
}

BOOST_AUTO_TEST_CASE(test_slice_from_second_to_end) {
  auto ctx = empty_context();
  auto baseList = get_list_of_ints();

  Slice slice(Slice::SliceType::StartToEnd, 1, 0);  // var[1:]
  slice.setSource(std::move(baseList));

  auto result = slice.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::List));
  BOOST_TEST(result->getList().size() == 2);
  BOOST_TEST(result->getList()[0]->getInt() == 2);
  BOOST_TEST(result->getList()[1]->getInt() == 3);
}

BOOST_AUTO_TEST_CASE(test_slice_from_pos_to_pos) {
  auto ctx = empty_context();
  auto baseList = get_list_of_ints();

  Slice slice(Slice::SliceType::StartToSlice, 0, 2);  // var[0:2] == var[:2]
  slice.setSource(std::move(baseList));

  auto result = slice.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::List));
  BOOST_TEST(result->getList().size() == 2);
  BOOST_TEST(result->getList()[0]->getInt() == 1);
  BOOST_TEST(result->getList()[1]->getInt() == 2);
}

BOOST_AUTO_TEST_CASE(test_slice_pos_out_of_range) {
  auto ctx = empty_context();
  auto baseList = get_list_of_ints();

  Slice slice(Slice::SliceType::Start, 5, 0);
  slice.setSource(std::move(baseList));

  BOOST_CHECK_THROW(slice.exec(ctx), OutOfRange);
}

BOOST_AUTO_TEST_CASE(test_slice_end_out_of_range) {
  auto ctx = empty_context();
  auto baseList = get_list_of_ints();

  Slice slice(Slice::SliceType::StartToSlice, 0, 5);
  slice.setSource(std::move(baseList));

  BOOST_CHECK_THROW(slice.exec(ctx), OutOfRange);
}

BOOST_AUTO_TEST_CASE(test_slice_not_a_list) {
  auto ctx = empty_context();
  auto notlist = mock_instr();

  Slice slice(Slice::SliceType::StartToSlice, 0, 5);
  slice.setSource(std::move(notlist));

  BOOST_CHECK_THROW(slice.exec(ctx), NotList);
}

BOOST_AUTO_TEST_CASE(test_function_call) {
  class TestFunction : public Instruction {
   public:
    std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override {
      BOOST_TEST(ctx->parametersSize() == 2);
      BOOST_TEST(ctx->getParameter(0)->getInt() == 1);
      BOOST_TEST(ctx->getParameter(1)->getInt() == 2);
      return std::make_shared<Value>(std::string{"called"});
    }
  };
  auto ctx = empty_context();
  std::string name = "func_name";
  ctx->setFunction(name, std::make_unique<TestFunction>());

  FunctionCall call(name);
  call.addArgument(constant<int64_t>(1L));
  call.addArgument(constant<int64_t>(2L));

  BOOST_TEST(ctx->parametersSize() == 0);
  auto result = call.exec(ctx);

  BOOST_TEST(ctx->parametersSize() == 0);
  BOOST_TEST(result->getStr() == "called");
}

BOOST_AUTO_TEST_CASE(test_function_call_not_declared) {
  auto ctx = empty_context();
  std::string name = "not_declared";
  FunctionCall call(name);
  BOOST_CHECK_THROW(call.exec(ctx), FunctionNotDeclared);
}

BOOST_AUTO_TEST_SUITE_END()
