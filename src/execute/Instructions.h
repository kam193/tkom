// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_INSTRUCTIONS_H_
#define SRC_EXECUTE_INSTRUCTIONS_H_

#include <list>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "Value.h"
#include "ExecuteExceptions.h"

enum TypeInstruction {
  GeneralT,
  CodeBlockT,
  FunctionT,
  VariableT,
  ConstantT,
  SliceT,
  FunctionCallT,
  ReturnT,
  CompareExprT,
  ExpressionAddT,
  ExpressionMulT,
  ExpressionExpT,
  AssignExprT,
  ContinueT,
  BreakT,
  IfT,
  ForT,
  WhileT
};

class Context;
class Instruction {
 public:
  virtual TypeInstruction getInstructionType() { return GeneralT; }
  virtual std::string toString() { return "Instruction"; }
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

  TypeInstruction getInstructionType() override { return CodeBlockT; }
  std::string toString() override;

 private:
  std::list<std::unique_ptr<Instruction>> instructions;
};

class Function : public Instruction {
 public:
  explicit Function(const std::string &name) : name(name) {}

  void addArgument(const std::string &arg) { argumentNames.push_back(arg); }
  void setCode(std::unique_ptr<CodeBlock> cb) { code = std::move(cb); }

  bool empty() { return code == nullptr || code->empty(); }

  TypeInstruction getInstructionType() override { return FunctionT; }

  std::string toString() override;

 private:
  std::unique_ptr<CodeBlock> code = nullptr;
  std::vector<std::string> argumentNames;
  std::string name;
};

class Variable : public Instruction {
 public:
  explicit Variable(std::string name) : name(name) {}
  TypeInstruction getInstructionType() override { return VariableT; }
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

  TypeInstruction getInstructionType() override { return ConstantT; }
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

  TypeInstruction getInstructionType() override { return SliceT; }
  std::string toString() override;

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

  TypeInstruction getInstructionType() override { return FunctionCallT; }
  std::string toString() override;

 private:
  std::string name;
  std::vector<std::unique_ptr<Instruction>> args;
};

class Return : public Instruction {
 public:
  void setValue(std::unique_ptr<Instruction> val) { value = std::move(val); }
  TypeInstruction getInstructionType() override { return ReturnT; }
  std::string toString() override { return "return " + value->toString(); }

 private:
  std::unique_ptr<Instruction> value;
};

class Expression : public Instruction {
 public:
  enum Type { None, Add, Sub, Mul, Div, Exp };

  Expression() {}

  TypeInstruction getInstructionType() override { return ExpressionAddT; }
  std::string toString() override;

  void setArgument(std::unique_ptr<Instruction> arg) {
    args.push_back(std::move(arg));
  }
  void setType(Type type) { types.push_back(type); }

 private:
  Type type;
  std::vector<Type> types;
  std::vector<std::unique_ptr<Instruction>> args;
  std::string typeToString(Type _type);
};

class CompareExpr : public Instruction {
 public:
  enum Type { NoComp, Greater, GreaterEq, Less, LessEq, Different, Equal };
  explicit CompareExpr(std::unique_ptr<Expression> left)
      : type(Type::NoComp), leftExpr(std::move(left)), rightExpr(nullptr) {}
  CompareExpr(Type type, std::unique_ptr<Expression> left,
              std::unique_ptr<Expression> right)
      : type(type), leftExpr(std::move(left)), rightExpr(std::move(right)) {}

  TypeInstruction getInstructionType() override { return CompareExprT; }
  std::string toString() override;

 private:
  Type type;
  std::unique_ptr<Expression> leftExpr;
  std::unique_ptr<Expression> rightExpr;

  std::string operatorToString();
};

class AssignExpr : public Instruction {
 public:
  enum Type { Assign, AddAssign, SubAssign };
  AssignExpr(Type type, std::string name, std::unique_ptr<Expression> expr)
      : type(type), variableName(name), expression(std::move(expr)) {}

  TypeInstruction getInstructionType() override { return AssignExprT; }
  std::string toString() override;

 private:
  Type type;
  std::string variableName;
  std::unique_ptr<Expression> expression;
};

class Continue : public Instruction {
 public:
  TypeInstruction getInstructionType() override { return ContinueT; }
  std::string toString() override { return "continue"; }
};

class Break : public Instruction {
 public:
  TypeInstruction getInstructionType() override { return BreakT; }
  std::string toString() override { return "break"; }
};

class If : public Instruction {
 public:
  If(std::unique_ptr<CompareExpr> compare, std::unique_ptr<CodeBlock> ifCode)
      : compare(std::move(compare)), ifCode(std::move(ifCode)) {}

  TypeInstruction getInstructionType() override { return IfT; }
  std::string toString() override;

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
  TypeInstruction getInstructionType() override { return ForT; }
  std::string toString() override;

 private:
  std::string iterator;
  std::unique_ptr<Instruction> range;
  std::unique_ptr<CodeBlock> code;
};

class While : public Instruction {
 public:
  While(std::unique_ptr<CompareExpr> compare, std::unique_ptr<CodeBlock> code)
      : compare(std::move(compare)), code(std::move(code)) {}
  TypeInstruction getInstructionType() override { return WhileT; }
  std::string toString() override;

 private:
  std::unique_ptr<CompareExpr> compare;
  std::unique_ptr<CodeBlock> code;
};

#include "Context.h"

#endif  // SRC_EXECUTE_INSTRUCTIONS_H_
