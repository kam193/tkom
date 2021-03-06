// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_INSTRUCTIONS_H_
#define SRC_EXECUTE_INSTRUCTIONS_H_

#include <cmath>
#include <list>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "ExecuteExceptions.h"
#include "Value.h"

class Context;
class Instruction {
 public:
  virtual std::string toString() { return "Instruction"; }
  virtual std::string instrName() { return "__UNNAMED_INSTR"; }
  virtual std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) {
    return std::make_shared<Value>();
  }
};

class CodeBlock : public Instruction {
 public:
  void addInstruction(std::unique_ptr<Instruction> instr) {
    instructions.push_back(std::move(instr));
  }
  bool empty() { return instructions.empty(); }

  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::vector<std::unique_ptr<Instruction>> instructions;
  bool isResultToReturn(std::shared_ptr<Value> result);
};

class Function : public Instruction {
 public:
  explicit Function(const std::string &name) : name(name) {}

  void addArgument(const std::string &arg) { argumentNames.push_back(arg); }
  void setCode(std::unique_ptr<CodeBlock> cb) { code = std::move(cb); }

  bool empty() { return code == nullptr || code->empty(); }

  std::string instrName() override { return name; }
  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::unique_ptr<CodeBlock> code = nullptr;
  std::vector<std::string> argumentNames;
  std::string name;
};

class Variable : public Instruction {
 public:
  explicit Variable(std::string name) : name(name) {}
  std::string toString() override { return name; }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::string name;
};

class Constant : public Instruction {
 public:
  explicit Constant(ValueType _type) : type(_type) {}
  explicit Constant(bool value) : type(ValueType::Bool), boolValue(value) {}
  explicit Constant(std::int64_t value)
      : type(ValueType::Int), intValue(value) {}
  explicit Constant(double value) : type(ValueType::Real), realValue(value) {}
  explicit Constant(std::string value)
      : type(ValueType::Text), strValue(value) {}
  explicit Constant(std::vector<std::unique_ptr<Instruction>> &&elements)
      : type(ValueType::List), listElements(std::move(elements)) {}

  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  ValueType type;
  std::int64_t intValue;
  double realValue;
  bool boolValue;
  std::string strValue;
  std::vector<std::unique_ptr<Instruction>> listElements;

  std::string listToString();
};

class Slice : public Instruction {
 public:
  enum SliceType { Start, StartToEnd, StartToSlice };
  Slice(SliceType type, int start, int end)
      : type(type), start(start), end(end) {}

  void setSource(std::unique_ptr<Instruction> src) { source = std::move(src); }

  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  SliceType type;
  int start;
  int end;
  std::unique_ptr<Instruction> source;
};

class FunctionCall : public Instruction {
 public:
  explicit FunctionCall(std::string name) : name(name) {}
  void addArgument(std::unique_ptr<Instruction> arg) {
    args.push_back(std::move(arg));
  }

  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::string name;
  std::vector<std::unique_ptr<Instruction>> args;
};

class Return : public Instruction {
 public:
  void setValue(std::unique_ptr<Instruction> val) { value = std::move(val); }
  std::string toString() override { return "return " + value->toString(); }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::unique_ptr<Instruction> value;
};

class Expression : public Instruction {
 public:
  enum Type { None, Add, Sub, Mul, Div, Exp };

  Expression() {}

  std::string toString() override;

  void setArgument(std::unique_ptr<Instruction> arg) {
    args.push_back(std::move(arg));
  }
  void setType(Type type) { types.push_back(type); }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

  static std::string typeToString(Type _type);
  static bool checkCompatibility(ValueType left, ValueType right,
                                 Expression::Type op);
  static std::shared_ptr<Value> makeExpression(std::shared_ptr<Value> left,
                                               std::shared_ptr<Value> right,
                                               Expression::Type op);

 private:
  Type type;
  std::vector<Type> types;
  std::vector<std::unique_ptr<Instruction>> args;
  static std::map<ValueType, std::map<Expression::Type, std::vector<ValueType>>>
      allowedOperands;

  static std::shared_ptr<Value> execExprList(std::shared_ptr<Value> list,
                                             std::shared_ptr<Value> right,
                                             Type op);
  static std::shared_ptr<Value> execExprStr(std::shared_ptr<Value> str,
                                            std::shared_ptr<Value> right,
                                            Type op);
  static std::shared_ptr<Value> execExprInt(int64_t left, int64_t right,
                                            Type op);
  static std::shared_ptr<Value> execExprReal(double left, double right,
                                             Type op);
};

class CompareExpr : public Instruction {
 public:
  enum Type { NoComp, Greater, GreaterEq, Less, LessEq, Different, Equal };
  explicit CompareExpr(std::unique_ptr<Expression> left)
      : type(Type::NoComp), leftExpr(std::move(left)), rightExpr(nullptr) {}
  CompareExpr(Type type, std::unique_ptr<Expression> left,
              std::unique_ptr<Expression> right)
      : type(type), leftExpr(std::move(left)), rightExpr(std::move(right)) {}

  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

  static bool isFalseEquivalent(std::shared_ptr<Value> val);

 private:
  Type type;
  std::unique_ptr<Expression> leftExpr;
  std::unique_ptr<Expression> rightExpr;
  bool checkEqual(std::shared_ptr<Value> left, std::shared_ptr<Value> right);
  bool checkEqualList(std::shared_ptr<Value> left,
                      std::shared_ptr<Value> right);
  bool checkTypeCompatibility(ValueType left, ValueType right);
  bool compare(std::shared_ptr<Value> left, std::shared_ptr<Value> right,
               CompareExpr::Type cmp);
  template <typename T>
  bool compare(T left, T right, CompareExpr::Type cmp);
  bool compareList(std::shared_ptr<Value> left, std::shared_ptr<Value> right,
                   CompareExpr::Type cmp);

  std::string operatorToString();
};

class AssignExpr : public Instruction {
 public:
  enum Type { Assign, AddAssign, SubAssign };
  AssignExpr(Type type, std::string name, std::unique_ptr<Expression> expr)
      : type(type), variableName(name), expression(std::move(expr)) {}

  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  Type type;
  std::string variableName;
  std::unique_ptr<Expression> expression;
};

class Continue : public Instruction {
 public:
  std::string toString() override { return "continue"; }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) {
    return std::make_shared<Value>(ValueType::T_CONTINUE);
  }
};

class Break : public Instruction {
 public:
  std::string toString() override { return "break"; }
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) {
    return std::make_shared<Value>(ValueType::T_BREAK);
  }
};

class If : public Instruction {
 public:
  If(std::unique_ptr<CompareExpr> compare, std::unique_ptr<CodeBlock> ifCode)
      : compare(std::move(compare)), ifCode(std::move(ifCode)) {}

  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::unique_ptr<CompareExpr> compare;
  std::unique_ptr<CodeBlock> ifCode;
  std::unique_ptr<CodeBlock> elseCode = nullptr;  // TD
};

class For : public Instruction {
 public:
  For(std::string iterator, std::unique_ptr<Instruction> range,
      std::unique_ptr<CodeBlock> code)
      : iterator(iterator), range(std::move(range)), code(std::move(code)) {}
  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::string iterator;
  std::unique_ptr<Instruction> range;
  std::unique_ptr<CodeBlock> code;
};

class While : public Instruction {
 public:
  While(std::unique_ptr<CompareExpr> compare, std::unique_ptr<CodeBlock> code)
      : compare(std::move(compare)), code(std::move(code)) {}
  std::string toString() override;
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  std::unique_ptr<CompareExpr> compare;
  std::unique_ptr<CodeBlock> code;
};

class FunctionPointer : public Instruction {
 public:
  FunctionPointer(std::string name, std::vector<std::string> args,
                  CodeBlock *code_ptr)
      : name(name), argumentNames(args), code(code_ptr) {}
  std::shared_ptr<Value> exec(std::shared_ptr<Context> ctx) override;

 private:
  CodeBlock *code;
  std::vector<std::string> argumentNames;
  std::string name;
};

#include "Context.h"

#endif  // SRC_EXECUTE_INSTRUCTIONS_H_
