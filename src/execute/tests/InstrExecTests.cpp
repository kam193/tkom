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

template <typename ValueType>
std::shared_ptr<Value> get_value(ValueType value) {
  return std::make_shared<Value>(value);
}

// [1, 2, 3]
std::unique_ptr<Constant> get_list_of_ints() {
  std::vector<std::unique_ptr<Instruction>> elements;
  elements.push_back(constant<int64_t>(1L));
  elements.push_back(constant<int64_t>(2L));
  elements.push_back(constant<int64_t>(3L));

  return std::make_unique<Constant>(std::move(elements));
}

void test_expr_bad_operands(std::unique_ptr<Constant> (*left_operand)(void),
                            std::vector<std::unique_ptr<Constant>> bad_operands,
                            Expression::Type operation) {
  auto ctx = empty_context();
  for (int i = 0; i < bad_operands.size(); ++i) {
    auto left = left_operand();
    Expression expr;
    expr.setArgument(std::move(left));
    expr.setType(operation);
    expr.setArgument(std::move(bad_operands[i]));
    BOOST_CHECK_THROW(expr.exec(ctx), OperandsTypesNotCompatible);
  }
}

template <typename LeftType, typename RightType>
std::shared_ptr<Value> exec_expression(LeftType left, RightType right,
                                       Expression::Type op) {
  auto ctx = empty_context();
  Expression expr;
  expr.setArgument(constant<LeftType>(left));
  expr.setType(op);
  expr.setArgument(constant<RightType>(right));
  return expr.exec(ctx);
}

std::unique_ptr<Expression> expression_const_5() {
  auto expr = std::make_unique<Expression>();
  expr->setArgument(constant<int64_t>(5));
  return std::move(expr);
}

std::unique_ptr<Expression> constant_expr(std::unique_ptr<Instruction> cst) {
  auto expr = std::make_unique<Expression>();
  expr->setArgument(std::move(cst));
  return std::move(expr);
}

void test_compare(CompareExpr::Type type, std::unique_ptr<Constant> left_const,
                  std::unique_ptr<Constant> right_const, bool expected) {
  auto ctx = empty_context();
  CompareExpr cmp(type, constant_expr(std::move(left_const)),
                  constant_expr(std::move(right_const)));
  auto result = cmp.exec(ctx);
  BOOST_TEST((result->getType() == ValueType::Bool));
  BOOST_TEST(result->getBool() == expected);
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

BOOST_AUTO_TEST_CASE(test_declare_func) {
  auto ctx = empty_context();
  std::string name = "func_name";
  auto code = std::make_unique<CodeBlock>();
  code->addInstruction(mock_instr());

  Function func(name);
  func.setCode(std::move(code));
  func.exec(ctx);

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

BOOST_AUTO_TEST_CASE(test_expr_list_bad_operands_throw) {
  auto lef_operand_creator = get_list_of_ints;

  std::vector<std::unique_ptr<Constant>> bad_operands;
  bad_operands.push_back(constant<std::string>("test"));
  bad_operands.push_back(constant<int64_t>(1));
  bad_operands.push_back(constant<double>(1.0));
  bad_operands.push_back(constant<ValueType>(ValueType::None));

  test_expr_bad_operands(lef_operand_creator, std::move(bad_operands),
                         Expression::Type::Add);

  bad_operands = std::vector<std::unique_ptr<Constant>>();
  bad_operands.push_back(get_list_of_ints());
  bad_operands.push_back(constant<std::string>("test"));
  bad_operands.push_back(constant<int64_t>(1));
  bad_operands.push_back(constant<double>(1.0));
  bad_operands.push_back(constant<ValueType>(ValueType::None));

  test_expr_bad_operands(lef_operand_creator, std::move(bad_operands),
                         Expression::Type::Sub);

  bad_operands = std::vector<std::unique_ptr<Constant>>();
  bad_operands.push_back(get_list_of_ints());
  bad_operands.push_back(constant<std::string>("test"));
  bad_operands.push_back(constant<double>(1.0));
  bad_operands.push_back(constant<ValueType>(ValueType::None));

  test_expr_bad_operands(lef_operand_creator, std::move(bad_operands),
                         Expression::Type::Mul);

  bad_operands = std::vector<std::unique_ptr<Constant>>();

  bad_operands.push_back(get_list_of_ints());
  bad_operands.push_back(constant<std::string>("test"));
  bad_operands.push_back(constant<int64_t>(1));
  bad_operands.push_back(constant<double>(1.0));
  bad_operands.push_back(constant<ValueType>(ValueType::None));

  test_expr_bad_operands(lef_operand_creator, std::move(bad_operands),
                         Expression::Type::Div);
}

BOOST_AUTO_TEST_CASE(test_expr_string_bad_operands_throw) {
  auto lef_operand_creator = [] { return constant<std::string>("test"); };

  std::vector<std::unique_ptr<Constant>> bad_operands;
  bad_operands.push_back(get_list_of_ints());
  bad_operands.push_back(constant<int64_t>(1));
  bad_operands.push_back(constant<double>(1.0));
  bad_operands.push_back(constant<ValueType>(ValueType::None));

  test_expr_bad_operands(lef_operand_creator, std::move(bad_operands),
                         Expression::Type::Add);

  bad_operands = std::vector<std::unique_ptr<Constant>>();
  bad_operands.push_back(get_list_of_ints());
  bad_operands.push_back(constant<std::string>("test"));
  bad_operands.push_back(constant<int64_t>(1));
  bad_operands.push_back(constant<double>(1.0));
  bad_operands.push_back(constant<ValueType>(ValueType::None));

  test_expr_bad_operands(lef_operand_creator, std::move(bad_operands),
                         Expression::Type::Sub);

  bad_operands = std::vector<std::unique_ptr<Constant>>();
  bad_operands.push_back(get_list_of_ints());
  bad_operands.push_back(constant<std::string>("test"));
  bad_operands.push_back(constant<double>(1.0));
  bad_operands.push_back(constant<ValueType>(ValueType::None));

  test_expr_bad_operands(lef_operand_creator, std::move(bad_operands),
                         Expression::Type::Mul);

  bad_operands = std::vector<std::unique_ptr<Constant>>();

  bad_operands.push_back(get_list_of_ints());
  bad_operands.push_back(constant<std::string>("test"));
  bad_operands.push_back(constant<int64_t>(1));
  bad_operands.push_back(constant<double>(1.0));
  bad_operands.push_back(constant<ValueType>(ValueType::None));

  test_expr_bad_operands(lef_operand_creator, std::move(bad_operands),
                         Expression::Type::Div);
}

BOOST_AUTO_TEST_CASE(test_list_mul) {
  auto ctx = empty_context();
  auto list = get_list_of_ints();
  auto mulCount = constant<int64_t>(3);

  Expression expr;
  expr.setArgument(std::move(list));
  expr.setType(Expression::Type::Mul);
  expr.setArgument(std::move(mulCount));
  auto result = expr.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::List));
  BOOST_TEST(result->getList().size() == 9);
}

BOOST_AUTO_TEST_CASE(test_list_mul_negative) {
  auto ctx = empty_context();
  auto list = get_list_of_ints();
  auto mulCount = constant<int64_t>(-2);

  Expression expr;
  expr.setArgument(std::move(list));
  expr.setType(Expression::Type::Mul);
  expr.setArgument(std::move(mulCount));
  auto result = expr.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::List));
  BOOST_TEST(result->getList().size() == 0);
}

BOOST_AUTO_TEST_CASE(test_str_mul) {
  auto ctx = empty_context();
  auto str = constant<std::string>("test");
  auto mulCount = constant<int64_t>(2);

  Expression expr;
  expr.setArgument(std::move(str));
  expr.setType(Expression::Type::Mul);
  expr.setArgument(std::move(mulCount));
  auto result = expr.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::Text));
  BOOST_TEST(result->getStr() == "testtest");
}

BOOST_AUTO_TEST_CASE(test_str_mul_negative) {
  auto ctx = empty_context();
  auto str = constant<std::string>("test");
  auto mulCount = constant<int64_t>(-1);

  Expression expr;
  expr.setArgument(std::move(str));
  expr.setType(Expression::Type::Mul);
  expr.setArgument(std::move(mulCount));
  auto result = expr.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::Text));
  BOOST_TEST(result->getStr() == "");
}

BOOST_AUTO_TEST_CASE(test_list_add) {
  auto ctx = empty_context();
  auto list = get_list_of_ints();
  auto otherlist = get_list_of_ints();

  Expression expr;
  expr.setArgument(std::move(list));
  expr.setType(Expression::Type::Add);
  expr.setArgument(std::move(otherlist));
  auto result = expr.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::List));
  BOOST_TEST(result->getList().size() == 6);
}

BOOST_AUTO_TEST_CASE(test_str_add) {
  auto ctx = empty_context();
  auto str = constant<std::string>("test");
  auto otherStr = constant<std::string>("second");

  Expression expr;
  expr.setArgument(std::move(str));
  expr.setType(Expression::Type::Add);
  expr.setArgument(std::move(otherStr));
  auto result = expr.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::Text));
  BOOST_TEST(result->getStr() == "testsecond");
}

BOOST_AUTO_TEST_CASE(test_int_int_all_operators) {
  auto add = exec_expression<int64_t, int64_t>(4, 3, Expression::Type::Add);
  BOOST_TEST((add->getType() == ValueType::Int));
  BOOST_TEST(add->getInt() == 7);

  auto sub = exec_expression<int64_t, int64_t>(4, 3, Expression::Type::Sub);
  BOOST_TEST((sub->getType() == ValueType::Int));
  BOOST_TEST(sub->getInt() == 1);

  auto mul = exec_expression<int64_t, int64_t>(4, 3, Expression::Type::Mul);
  BOOST_TEST((mul->getType() == ValueType::Int));
  BOOST_TEST(mul->getInt() == 12);

  auto div = exec_expression<int64_t, int64_t>(4, 3, Expression::Type::Div);
  BOOST_TEST((div->getType() == ValueType::Int));
  BOOST_TEST(div->getInt() == 1);

  auto exp = exec_expression<int64_t, int64_t>(4, 3, Expression::Type::Exp);
  BOOST_TEST((exp->getType() == ValueType::Int));
  BOOST_TEST(exp->getInt() == 64);
}

BOOST_AUTO_TEST_CASE(test_double_double_all_operators) {
  auto add = exec_expression<double, double>(4.0, 3.0, Expression::Type::Add);
  BOOST_TEST((add->getType() == ValueType::Real));
  BOOST_TEST(add->getReal() == 7);

  auto sub = exec_expression<double, double>(4.0, 3.0, Expression::Type::Sub);
  BOOST_TEST((sub->getType() == ValueType::Real));
  BOOST_TEST(sub->getReal() == 1.0);

  auto mul = exec_expression<double, double>(4.0, 3.0, Expression::Type::Mul);
  BOOST_TEST((mul->getType() == ValueType::Real));
  BOOST_TEST(mul->getReal() == 12.0);

  auto div = exec_expression<double, double>(3.0, 2.0, Expression::Type::Div);
  BOOST_TEST((div->getType() == ValueType::Real));
  BOOST_TEST(div->getReal() == 1.5);

  auto exp = exec_expression<double, double>(4.0, 3.0, Expression::Type::Exp);
  BOOST_TEST((exp->getType() == ValueType::Real));
  BOOST_TEST(exp->getReal() == 64.0);
}

BOOST_AUTO_TEST_CASE(test_double_int_all_operators) {
  auto add = exec_expression<double, int64_t>(4.0, 3, Expression::Type::Add);
  BOOST_TEST((add->getType() == ValueType::Real));
  BOOST_TEST(add->getReal() == 7);

  auto sub = exec_expression<int64_t, double>(4, 3.0, Expression::Type::Sub);
  BOOST_TEST((sub->getType() == ValueType::Real));
  BOOST_TEST(sub->getReal() == 1.0);

  auto mul = exec_expression<double, int64_t>(4.0, 3, Expression::Type::Mul);
  BOOST_TEST((mul->getType() == ValueType::Real));
  BOOST_TEST(mul->getReal() == 12.0);

  auto div = exec_expression<int64_t, double>(3, 2.0, Expression::Type::Div);
  BOOST_TEST((div->getType() == ValueType::Real));
  BOOST_TEST(div->getReal() == 1.5);

  auto exp = exec_expression<double, int64_t>(4.0, 3, Expression::Type::Exp);
  BOOST_TEST((exp->getType() == ValueType::Real));
  BOOST_TEST(exp->getReal() == 64.0);
}

BOOST_AUTO_TEST_CASE(test_expression_multiple_op) {
  auto ctx = empty_context();

  Expression expr;
  expr.setArgument(constant<int64_t>(2));
  expr.setType(Expression::Type::Mul);
  expr.setArgument(constant<int64_t>(4));
  expr.setType(Expression::Type::Mul);
  expr.setArgument(constant<int64_t>(3));
  auto result = expr.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::Int));
  BOOST_TEST(result->getInt() == 24);
}

BOOST_AUTO_TEST_CASE(test_expression_multiple_op_throw) {
  auto ctx = empty_context();

  Expression expr;
  expr.setArgument(constant<std::string>("test"));
  expr.setType(Expression::Type::Mul);
  expr.setArgument(constant<int64_t>(4));
  expr.setType(Expression::Type::Mul);
  expr.setArgument(constant<std::string>("bad"));

  BOOST_CHECK_THROW(expr.exec(ctx), OperandsTypesNotCompatible);
}

BOOST_AUTO_TEST_CASE(test_new_assign_expr) {
  auto ctx = empty_context();
  std::string name = "var";

  AssignExpr expr(AssignExpr::Type::Assign, name, expression_const_5());
  expr.exec(ctx);

  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 5);
}

BOOST_AUTO_TEST_CASE(test_add_assign_expr) {
  auto ctx = empty_context();
  std::string name = "var";
  ctx->setVariable(name, std::make_shared<Value>(3L));

  AssignExpr expr(AssignExpr::Type::AddAssign, name, expression_const_5());
  expr.exec(ctx);

  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 8);
}

BOOST_AUTO_TEST_CASE(test_sub_assign_expr) {
  auto ctx = empty_context();
  std::string name = "var";
  ctx->setVariable(name, std::make_shared<Value>(3L));

  AssignExpr expr(AssignExpr::Type::SubAssign, name, expression_const_5());
  expr.exec(ctx);

  BOOST_TEST(ctx->getVariableValue(name)->getInt() == -2);
}

BOOST_AUTO_TEST_CASE(test_override_assign_expr) {
  auto ctx = empty_context();
  std::string name = "var";
  ctx->setVariable(name, std::make_shared<Value>(3L));

  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 3);
  AssignExpr expr(AssignExpr::Type::Assign, name, expression_const_5());
  expr.exec(ctx);
  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 5);
}

BOOST_AUTO_TEST_CASE(test_sub_assign_expr_no_var_throw) {
  auto ctx = empty_context();
  std::string name = "var";

  AssignExpr expr(AssignExpr::Type::SubAssign, name, expression_const_5());
  BOOST_CHECK_THROW(expr.exec(ctx), ReadNotAssignVariable);
}

BOOST_AUTO_TEST_CASE(test_add_assign_expr_incompatibile_throw) {
  auto ctx = empty_context();
  std::string name = "var";
  ctx->setVariable(name, std::make_shared<Value>(true));

  AssignExpr expr(AssignExpr::Type::SubAssign, name, expression_const_5());
  BOOST_CHECK_THROW(expr.exec(ctx), OperandsTypesNotCompatible);
}

BOOST_AUTO_TEST_CASE(test_for_execute) {
  auto ctx = empty_context();
  std::string name = "i";
  auto cb = std::make_unique<CodeBlock>();
  cb->addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  For forinstr(name, get_list_of_ints(), std::move(cb));
  forinstr.exec(ctx);

  BOOST_TEST(MockInstruction::getExecutedCount() == 3);
  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 3);
}

BOOST_AUTO_TEST_CASE(test_for_not_iterable_throw) {
  auto ctx = empty_context();
  std::string rangename = "var";
  ctx->setVariable(rangename, std::make_shared<Value>(1L));
  auto cb = std::make_unique<CodeBlock>();
  cb->addInstruction(mock_instr());

  For forinstr("i", std::make_unique<Variable>(rangename), std::move(cb));
  BOOST_CHECK_THROW(forinstr.exec(ctx), IterableExpected);
}

BOOST_AUTO_TEST_CASE(test_for_break) {
  auto ctx = empty_context();
  std::string name = "i";
  auto cb = std::make_unique<CodeBlock>();
  cb->addInstruction(mock_instr());
  cb->addInstruction(std::make_unique<Break>());

  MockInstruction::resetExecutedCount();
  For forinstr(name, get_list_of_ints(), std::move(cb));
  forinstr.exec(ctx);

  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 1);
}

BOOST_AUTO_TEST_CASE(test_for_continue) {
  auto ctx = empty_context();
  std::string name = "i";
  auto cb = std::make_unique<CodeBlock>();
  cb->addInstruction(std::make_unique<Continue>());
  cb->addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  For forinstr(name, get_list_of_ints(), std::move(cb));
  forinstr.exec(ctx);

  BOOST_TEST(MockInstruction::getExecutedCount() == 0);
  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 3);
}

BOOST_AUTO_TEST_CASE(test_compare_equal_true) {
  test_compare(CompareExpr::Type::Equal, constant<int64_t>(3),
               constant<int64_t>(3), true);
  test_compare(CompareExpr::Type::Equal, constant<double>(3.30),
               constant<double>(3.30), true);
  test_compare(CompareExpr::Type::Equal, constant<int64_t>(3),
               constant<double>(3.0), true);
  test_compare(CompareExpr::Type::Equal, constant<std::string>("test"),
               constant<std::string>("test"), true);
  test_compare(CompareExpr::Type::Equal, get_list_of_ints(), get_list_of_ints(),
               true);
  test_compare(CompareExpr::Type::Equal, constant<ValueType>(ValueType::None),
               constant<ValueType>(ValueType::None), true);
}

BOOST_AUTO_TEST_CASE(test_compare_diff_false) {
  test_compare(CompareExpr::Type::Different, constant<int64_t>(3),
               constant<int64_t>(3), false);
  test_compare(CompareExpr::Type::Different, constant<double>(3.30),
               constant<double>(3.30), false);
  test_compare(CompareExpr::Type::Different, constant<int64_t>(3),
               constant<double>(3.0), false);
  test_compare(CompareExpr::Type::Different, constant<std::string>("test"),
               constant<std::string>("test"), false);

  test_compare(CompareExpr::Type::Different, get_list_of_ints(),
               get_list_of_ints(), false);

  test_compare(CompareExpr::Type::Different,
               constant<ValueType>(ValueType::None),
               constant<ValueType>(ValueType::None), false);
}

BOOST_AUTO_TEST_CASE(test_compare_diff_true) {
  test_compare(CompareExpr::Type::Different, constant<int64_t>(3),
               constant<int64_t>(5), true);
  test_compare(CompareExpr::Type::Different, constant<double>(3.30),
               constant<double>(3.10), true);
  test_compare(CompareExpr::Type::Different, constant<int64_t>(3),
               constant<double>(3.30), true);
  test_compare(CompareExpr::Type::Different, constant<std::string>("test"),
               constant<std::string>("noest"), true);

  test_compare(CompareExpr::Type::Different,
               constant<ValueType>(ValueType::None), get_list_of_ints(), true);
}

BOOST_AUTO_TEST_CASE(test_compare_equal_false) {
  test_compare(CompareExpr::Type::Equal, constant<int64_t>(3),
               constant<int64_t>(5), false);
  test_compare(CompareExpr::Type::Equal, constant<double>(3.30),
               constant<double>(3.99), false);
  test_compare(CompareExpr::Type::Equal, constant<int64_t>(3),
               constant<double>(3.99), false);
  test_compare(CompareExpr::Type::Equal, constant<std::string>("test"),
               constant<std::string>("other"), false);
}

BOOST_AUTO_TEST_CASE(test_compare_equal_false_types) {
  test_compare(CompareExpr::Type::Equal, constant<int64_t>(3),
               constant<std::string>("str"), false);
  test_compare(CompareExpr::Type::Equal, constant<double>(3.30),
               constant<bool>(true), false);
  test_compare(CompareExpr::Type::Equal, constant<int64_t>(3),
               get_list_of_ints(), false);
  test_compare(CompareExpr::Type::Equal, constant<std::string>("test"),
               constant<ValueType>(ValueType::None), false);
}

BOOST_AUTO_TEST_CASE(test_compare_less_true) {
  test_compare(CompareExpr::Type::Less, constant<int64_t>(3),
               constant<int64_t>(5), true);
  test_compare(CompareExpr::Type::Less, constant<double>(3.30),
               constant<double>(3.99), true);
  test_compare(CompareExpr::Type::Less, constant<int64_t>(3),
               constant<double>(3.99), true);
  test_compare(CompareExpr::Type::Less, constant<std::string>("test"),
               constant<std::string>("tfst"), true);
  test_compare(CompareExpr::Type::Less, constant<std::string>("aa"),
               constant<std::string>("aaa"), true);

  test_compare(CompareExpr::Type::LessEq, constant<int64_t>(3),
               constant<int64_t>(3), true);
  test_compare(CompareExpr::Type::LessEq, constant<double>(3.30),
               constant<double>(3.99), true);
  test_compare(CompareExpr::Type::LessEq, constant<int64_t>(3),
               constant<double>(3.0), true);
  test_compare(CompareExpr::Type::LessEq, constant<std::string>("test"),
               constant<std::string>("test"), true);
  test_compare(CompareExpr::Type::LessEq, constant<std::string>("aa"),
               constant<std::string>("aaa"), true);
}

BOOST_AUTO_TEST_CASE(test_compare_less_false) {
  for (auto type : {CompareExpr::Type::Less, CompareExpr::Type::LessEq}) {
    test_compare(type, constant<int64_t>(3), constant<int64_t>(2), false);
    test_compare(type, constant<double>(3.30), constant<double>(2.22), false);
    test_compare(type, constant<int64_t>(3), constant<double>(1.5), false);
    test_compare(type, constant<std::string>("test"),
                 constant<std::string>("afst"), false);
    test_compare(type, constant<std::string>("aaa"),
                 constant<std::string>("aa"), false);
  }
}

BOOST_AUTO_TEST_CASE(test_compare_greater_true) {
  test_compare(CompareExpr::Type::Greater, constant<int64_t>(5),
               constant<int64_t>(3), true);
  test_compare(CompareExpr::Type::Greater, constant<double>(4.50),
               constant<double>(3.99), true);
  test_compare(CompareExpr::Type::Greater, constant<int64_t>(8),
               constant<double>(3.99), true);
  test_compare(CompareExpr::Type::Greater, constant<std::string>("zest"),
               constant<std::string>("tfst"), true);
  test_compare(CompareExpr::Type::Greater, constant<std::string>("aaa"),
               constant<std::string>("aa"), true);

  test_compare(CompareExpr::Type::GreaterEq, constant<int64_t>(3),
               constant<int64_t>(3), true);
  test_compare(CompareExpr::Type::GreaterEq, constant<double>(3.30),
               constant<double>(3.10), true);
  test_compare(CompareExpr::Type::GreaterEq, constant<int64_t>(3),
               constant<double>(3.0), true);
  test_compare(CompareExpr::Type::GreaterEq, constant<std::string>("test"),
               constant<std::string>("test"), true);
  test_compare(CompareExpr::Type::GreaterEq, constant<std::string>("aaa"),
               constant<std::string>("aa"), true);
}

BOOST_AUTO_TEST_CASE(test_compare_greater_false) {
  for (auto type : {CompareExpr::Type::Greater, CompareExpr::Type::GreaterEq}) {
    test_compare(type, constant<int64_t>(1), constant<int64_t>(2), false);
    test_compare(type, constant<double>(1.30), constant<double>(2.22), false);
    test_compare(type, constant<int64_t>(1), constant<double>(1.5), false);
    test_compare(type, constant<std::string>("afst"),
                 constant<std::string>("test"), false);
    test_compare(type, constant<std::string>("aa"),
                 constant<std::string>("aaa"), false);
  }
}

BOOST_AUTO_TEST_CASE(test_compare_nocomp) {
  auto ctx = empty_context();
  int64_t value = 11;
  auto expr = constant_expr(constant<int64_t>(value));

  CompareExpr cmp(std::move(expr));
  auto result = cmp.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::Int));
  BOOST_TEST(result->getInt() == value);
}

BOOST_AUTO_TEST_CASE(test_compare_incomparable_types) {
  for (auto type : {CompareExpr::Type::Greater, CompareExpr::Type::GreaterEq,
                    CompareExpr::Type::Less, CompareExpr::Type::LessEq}) {
    BOOST_CHECK_THROW(test_compare(type, constant<ValueType>(ValueType::None),
                                   constant<ValueType>(ValueType::None), false),
                      TypesNotComparable);

    BOOST_CHECK_THROW(test_compare(type, get_list_of_ints(),
                                   constant<ValueType>(ValueType::None), false),
                      TypesNotComparable);

    BOOST_CHECK_THROW(test_compare(type, constant<std::string>("test"),
                                   constant<int64_t>(11), false),
                      TypesNotComparable);

    BOOST_CHECK_THROW(test_compare(type, constant<bool>(false),
                                   constant<ValueType>(ValueType::None), false),
                      TypesNotComparable);

    BOOST_CHECK_THROW(
        test_compare(type, constant<bool>(true), constant<double>(1.5), false),
        TypesNotComparable);
  }
}

BOOST_AUTO_TEST_CASE(test_compare_false_equivalent) {
  std::vector<std::shared_ptr<Value>> empty_elements;
  auto empty_list = std::make_shared<Value>(empty_elements);

  std::vector<std::shared_ptr<Value>> elements;
  elements.push_back(get_value<int64_t>(1));
  auto list = std::make_shared<Value>(elements);

  BOOST_TEST(CompareExpr::isFalseEquivalent(empty_list) == true);
  BOOST_TEST(CompareExpr::isFalseEquivalent(
                 get_value<ValueType>(ValueType::None)) == true);
  BOOST_TEST(CompareExpr::isFalseEquivalent(get_value<int64_t>(0)) == true);
  BOOST_TEST(CompareExpr::isFalseEquivalent(get_value<double>(0.0)) == true);
  BOOST_TEST(CompareExpr::isFalseEquivalent(get_value<std::string>("")) ==
             true);

  BOOST_TEST(CompareExpr::isFalseEquivalent(list) == false);
  BOOST_TEST(CompareExpr::isFalseEquivalent(get_value<int64_t>(11)) == false);
  BOOST_TEST(CompareExpr::isFalseEquivalent(get_value<double>(2.0)) == false);
  BOOST_TEST(CompareExpr::isFalseEquivalent(get_value<std::string>("test")) ==
             false);
}

BOOST_AUTO_TEST_CASE(test_if_not_execute) {
  auto ctx = empty_context();
  auto cmp = std::make_unique<CompareExpr>(
      constant_expr(constant<ValueType>(ValueType::None)));
  auto code = std::make_unique<CodeBlock>();
  code->addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  If test_if(std::move(cmp), std::move(code));
  test_if.exec(ctx);
  BOOST_TEST(MockInstruction::getExecutedCount() == 0);
}

BOOST_AUTO_TEST_CASE(test_if_execute) {
  auto ctx = empty_context();
  auto cmp = std::make_unique<CompareExpr>(constant_expr(constant<bool>(true)));
  auto code = std::make_unique<CodeBlock>();
  code->addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  If test_if(std::move(cmp), std::move(code));
  test_if.exec(ctx);
  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
}

BOOST_AUTO_TEST_CASE(test_while_execute) {
  auto ctx = empty_context();
  std::string name = "i";
  ctx->setVariable(name, get_value<int64_t>(0));

  auto cmp = std::make_unique<CompareExpr>(
      CompareExpr::Type::Less, constant_expr(std::make_unique<Variable>(name)),
      constant_expr(constant<int64_t>(2)));
  auto increment_i = std::make_unique<AssignExpr>(
      AssignExpr::AddAssign, name, constant_expr(constant<int64_t>(1)));
  auto cb = std::make_unique<CodeBlock>();
  cb->addInstruction(mock_instr());
  cb->addInstruction(std::move(increment_i));

  MockInstruction::resetExecutedCount();
  While whileinstr(std::move(cmp), std::move(cb));
  whileinstr.exec(ctx);

  BOOST_TEST(MockInstruction::getExecutedCount() == 2);
  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 2);
}

BOOST_AUTO_TEST_CASE(test_while_break) {
  auto ctx = empty_context();
  std::string name = "i";
  ctx->setVariable(name, get_value<int64_t>(0));

  auto cmp = std::make_unique<CompareExpr>(
      CompareExpr::Type::Less, constant_expr(std::make_unique<Variable>(name)),
      constant_expr(constant<int64_t>(2)));
  auto increment_i = std::make_unique<AssignExpr>(
      AssignExpr::AddAssign, name, constant_expr(constant<int64_t>(1)));
  auto cb = std::make_unique<CodeBlock>();
  cb->addInstruction(mock_instr());
  cb->addInstruction(std::make_unique<Break>());
  cb->addInstruction(std::move(increment_i));

  MockInstruction::resetExecutedCount();
  While whileinstr(std::move(cmp), std::move(cb));
  whileinstr.exec(ctx);

  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 0);
}

BOOST_AUTO_TEST_CASE(test_while_continue) {
  auto ctx = empty_context();
  std::string name = "i";
  ctx->setVariable(name, get_value<int64_t>(0));

  // while i < 2:
  //   i += 1
  //   if i == 2:
  //     continue
  //   MockInstruction
  auto cmp = std::make_unique<CompareExpr>(
      CompareExpr::Type::Less, constant_expr(std::make_unique<Variable>(name)),
      constant_expr(constant<int64_t>(2)));
  auto increment_i = std::make_unique<AssignExpr>(
      AssignExpr::AddAssign, name, constant_expr(constant<int64_t>(1)));
  auto if_code = std::make_unique<CodeBlock>();
  if_code->addInstruction(std::make_unique<Continue>());
  auto if_cmp = std::make_unique<CompareExpr>(
      CompareExpr::Type::Equal, constant_expr(std::make_unique<Variable>(name)),
      constant_expr(constant<int64_t>(2)));
  auto if_instr = std::make_unique<If>(std::move(if_cmp), std::move(if_code));
  auto cb = std::make_unique<CodeBlock>();
  cb->addInstruction(std::move(increment_i));
  cb->addInstruction(std::move(if_instr));
  cb->addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  While whileinstr(std::move(cmp), std::move(cb));
  whileinstr.exec(ctx);

  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
  BOOST_TEST(ctx->getVariableValue(name)->getInt() == 2);
}

BOOST_AUTO_TEST_CASE(test_function_incorrect_params_throw) {
  auto ctx = empty_context();
  auto code = std::make_unique<CodeBlock>();
  auto args_names = std::vector<std::string>{"arg1", "arg2"};

  ctx->addParameter(get_value<int64_t>(1));
  FunctionPointer func("name", args_names, code.get());

  BOOST_CHECK_THROW(func.exec(ctx), ParametersCountNotExpected);
}

BOOST_AUTO_TEST_CASE(test_function_simple_call) {
  auto ctx = empty_context();
  auto code = std::make_unique<CodeBlock>();
  code->addInstruction(mock_instr());

  MockInstruction::resetExecutedCount();
  FunctionPointer func("name", std::vector<std::string>(), code.get());
  auto result = func.exec(ctx);

  BOOST_TEST(MockInstruction::getExecutedCount() == 1);
  BOOST_TEST((result->getType() == ValueType::None));
}

BOOST_AUTO_TEST_CASE(test_function_return_arg) {
  auto ctx = empty_context();
  std::string name = "arg1";
  auto args_names = std::vector<std::string>{name};
  auto return_instr = std::make_unique<Return>();
  return_instr->setValue(std::make_unique<Variable>(name));
  auto code = std::make_unique<CodeBlock>();
  code->addInstruction(std::move(return_instr));

  ctx->addParameter(get_value<std::string>("argument_test"));
  FunctionPointer func("name", args_names, code.get());
  auto result = func.exec(ctx);

  BOOST_TEST(result->getStr() == "argument_test");
}

BOOST_AUTO_TEST_SUITE_END()
