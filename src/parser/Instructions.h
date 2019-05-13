// Copyright 2019 Kamil Mankowski

#ifndef SRC_PARSER_INSTRUCTIONS_H_
#define SRC_PARSER_INSTRUCTIONS_H_

#include <list>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

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
  ExpressionT,
  AssignExprT,
  ContinueT,
  BreakT,
  IfT,
  ForT,
  WhileT
};

class Instruction {
  // pozycja w tekście
  // metoda ewaluacji
  // metoda wypisania
 public:
  virtual TypeInstruction getInstructionType() { return GeneralT; }
  virtual std::string toString() { return "Instruction"; }
};

class CodeBlock : public Instruction {  // public virtual?
 public:
  std::list<std::unique_ptr<Instruction>> instructions;  // NOT A PUBLIC!!!!
  TypeInstruction getInstructionType() override { return CodeBlockT; }
  std::string toString() override {
    std::string outstr = "  ";
    for (auto &instr : instructions) {
      outstr += std::regex_replace(instr->toString(), std::regex("\n"), "\n  ");
      outstr += "\n  ";
    }
    return outstr.substr(0, outstr.size() - 3);
  }
};

class Function : public Instruction {
 public:
  std::unique_ptr<CodeBlock> code;  // NOT A PUBLIC
  std::vector<std::string> argumentNames;
  std::string name;

  TypeInstruction getInstructionType() override { return FunctionT; }

  std::string toString() override {
    std::string out = "def " + name + "(";
    for (int i = 0; i < argumentNames.size(); ++i) {
      out += argumentNames[i];
      if (i != argumentNames.size() - 1) out += ", ";
    }
    out += "):\n";
    out += code->toString();
    return out;
  }
  // evaluate (dziedziczone) -> dodaje do listy funkcji
  // call -> woła funkcję
};

class Variable : public Instruction {
 public:
  explicit Variable(std::string name) : name(name) {}
  TypeInstruction getInstructionType() override { return VariableT; }
  std::string toString() override { return name; }

 private:
  std::string name;
};

class Constant : public Instruction {
 public:
  enum class Type { None, Bool, Int, Real, Text, List };
  Type type;
  std::int64_t intValue;
  double realValue;
  bool boolValue;
  std::string strValue;
  std::unique_ptr<std::vector<Variable>> listValue;  // fix!

  explicit Constant(Type _type) : type(_type) {}
  explicit Constant(bool value) : type(Type::Bool), boolValue(value) {}
  explicit Constant(std::int64_t value) : type(Type::Int), intValue(value) {}
  explicit Constant(double value) : type(Type::Real), realValue(value) {}
  explicit Constant(std::string value) : type(Type::Real), strValue(value) {}
  explicit Constant(std::unique_ptr<std::vector<Variable>> values)
      : type(Type::List), listValue(std::move(values)) {}  // shared ptr?

  TypeInstruction getInstructionType() override { return ConstantT; }

  std::string toString() override {
    switch (type) {
      case Type::None:
        return "None";
      case Type::Bool:
        return boolValue ? "True" : "False";
      case Type::Int:
        return std::to_string(intValue);
      case Type::Real:
        return std::to_string(realValue);
      case Type::Text:
        return strValue;
      // case Type::List:
      default:
        throw std::exception();
    }
  }
};

class Slice : public Instruction {
 public:
  enum SliceType { Start, StartToEnd, StartToSlice };
  Slice(SliceType type, int start, int end)
      : type(type), start(Start), end(end) {}

  void setSource(std::unique_ptr<Instruction> _source) {
    source = std::move(_source);
  }

  TypeInstruction getInstructionType() override { return SliceT; }

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
    args->push_back(std::move(arg));
  }

  TypeInstruction getInstructionType() override { return FunctionCallT; }

 private:
  std::string name;
  std::unique_ptr<std::vector<std::unique_ptr<Instruction>>> args;
};

class Return : public Instruction {
 public:
  std::unique_ptr<Instruction> value;
  TypeInstruction getInstructionType() override { return ReturnT; }
  std::string toString() override { return "return " + value->toString(); }
};

class Expression : public Instruction {
 public:
  TypeInstruction getInstructionType() override { return ExpressionT; }
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

 private:
  Type type;
  std::unique_ptr<Expression> leftExpr;
  std::unique_ptr<Expression> rightExpr;
};

class AssignExpr : public Instruction {
 public:
  enum Type { Assign, AddAssign, SubAssign };
  AssignExpr(Type type, std::string name, std::unique_ptr<Expression> expr)
      : type(type), variableName(name), expression(std::move(expr)) {}

  TypeInstruction getInstructionType() override { return AssignExprT; }

 private:
  Type type;
  std::string variableName;
  std::unique_ptr<Expression> expression;
};

class Continue : public Instruction {
 public:
  TypeInstruction getInstructionType() override { return ContinueT; }
};

class Break : public Instruction {
 public:
  TypeInstruction getInstructionType() override { return BreakT; }
};

class If : public Instruction {
 public:
  If(std::unique_ptr<CompareExpr> compare, std::unique_ptr<CodeBlock> ifCode)
      : compare(std::move(compare)), ifCode(std::move(ifCode)) {}

  TypeInstruction getInstructionType() override { return IfT; }

  std::string toString() override {
    std::string out = "if ";
    out += compare->toString();
    out += ":\n";
    out += ifCode->toString();
    return out;
  }

 private:
  std::unique_ptr<CompareExpr> compare;
  std::unique_ptr<CodeBlock> ifCode;
  std::unique_ptr<CodeBlock> elseCode = nullptr;  // TD
};

class For : public Instruction {
 public:
  TypeInstruction getInstructionType() override { return ForT; }

 private:
  std::string varName;
  std::unique_ptr<Slice> slice;
};

class While : public Instruction {
 public:
  While(std::unique_ptr<CompareExpr> compare, std::unique_ptr<CodeBlock> code)
      : compare(std::move(compare)), code(std::move(code)) {}
  TypeInstruction getInstructionType() override { return WhileT; }

 private:
  std::unique_ptr<CompareExpr> compare;
  std::unique_ptr<CodeBlock> code;
};

// class NoneConst : public Instruction {
//  public:
//   int value;
// };

#endif  // SRC_PARSER_INSTRUCTIONS_H_
