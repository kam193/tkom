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
  std::vector<std::unique_ptr<Instruction>> listElements;

  explicit Constant(Type _type) : type(_type) {}
  explicit Constant(bool value) : type(Type::Bool), boolValue(value) {}
  explicit Constant(std::int64_t value) : type(Type::Int), intValue(value) {}
  explicit Constant(double value) : type(Type::Real), realValue(value) {}
  explicit Constant(std::string value) : type(Type::Text), strValue(value) {}
  explicit Constant(std::vector<std::unique_ptr<Instruction>> &&elements)
      : type(Type::List), listElements(std::move(elements)) {}

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
        return "\"" + strValue + "\"";
      case Type::List:
        return listToString();
      default:
        throw std::exception();
    }
  }

 private:
  std::string listToString() {
    std::string out = "[";
    for (int i = 0; i < listElements.size(); ++i) {
      out += listElements[i]->toString();
      if (i != listElements.size() - 1) out += ", ";
    }
    out += "]";
    return out;
  }
};

class Slice : public Instruction {
 public:
  enum SliceType { Start, StartToEnd, StartToSlice };
  Slice(SliceType type, int start, int end)
      : type(type), start(start), end(end) {}

  void setSource(std::unique_ptr<Instruction> _source) {
    source = std::move(_source);
  }

  TypeInstruction getInstructionType() override { return SliceT; }
  std::string toString() override {
    std::string out = source->toString() += "[";
    out += std::to_string(start);
    if (type != SliceType::Start) out += ":";
    if (type == SliceType::StartToSlice) out += std::to_string(end);
    out += "]";
    return out;
  }

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
  std::string toString() override {
    std::string out = name + "(";
    for (int i = 0; i < args.size(); ++i) {
      out += args[i]->toString();
      if (i != args.size() - 1) out += ", ";
    }
    out += ")";
    return out;
  }

 private:
  std::string name;
  std::vector<std::unique_ptr<Instruction>> args;
};

class Return : public Instruction {
 public:
  std::unique_ptr<Instruction> value;
  TypeInstruction getInstructionType() override { return ReturnT; }
  std::string toString() override { return "return " + value->toString(); }
};

class Expression : public Instruction {
 public:
  enum Type { None, Add, Sub, Mul, Div, Exp };

  explicit Expression(std::unique_ptr<Instruction> left)
      : leftExpr(std::move(left)), type(Type::None) {}
  Expression(std::unique_ptr<Instruction> left, Type type,
             std::unique_ptr<Instruction> right)
      : leftExpr(std::move(left)), type(type), rightExpr(std::move(right)) {}

  TypeInstruction getInstructionType() override { return ExpressionAddT; }
  std::string toString() override {
    std::string out = leftExpr->toString();
    if (type != Type::None) {
      out += typeToString();
      out += rightExpr->toString();
    }
    return out;
  }

 private:
  Type type;
  std::unique_ptr<Instruction> leftExpr;
  std::unique_ptr<Instruction> rightExpr = nullptr;
  std::string typeToString() {
    switch (type) {
      case Type::None:
        return "";
      case Type::Add:
        return " + ";
      case Type::Sub:
        return " - ";
      case Type::Mul:
        return " * ";
      case Type::Div:
        return " / ";
      case Type::Exp:
        return " ^ ";
    }
  }
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

  std::string toString() override {
    std::string out = leftExpr->toString();
    out += operatorToString();
    if (rightExpr != nullptr) out += rightExpr->toString();
    return out;
  }

 private:
  Type type;
  std::unique_ptr<Expression> leftExpr;
  std::unique_ptr<Expression> rightExpr;

  std::string operatorToString() {
    switch (type) {
      case Type::NoComp:
        return "";
      case Type::Greater:
        return " > ";
      case Type::GreaterEq:
        return " >= ";
      case Type::Less:
        return " < ";
      case Type::LessEq:
        return " <= ";
      case Type::Different:
        return " != ";
      case Type::Equal:
        return " == ";
    }
  }
};

class AssignExpr : public Instruction {
 public:
  enum Type { Assign, AddAssign, SubAssign };
  AssignExpr(Type type, std::string name, std::unique_ptr<Expression> expr)
      : type(type), variableName(name), expression(std::move(expr)) {}

  TypeInstruction getInstructionType() override { return AssignExprT; }
  std::string toString() override {
    std::string out = variableName;
    if (type == Type::Assign)
      out += " = ";
    else if (type == Type::AddAssign)
      out += " += ";
    else
      out += " -= ";
    out += expression->toString();
    return out;
  }

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
  For(std::string iterator, std::unique_ptr<Instruction> range,
      std::unique_ptr<CodeBlock> code)
      : iterator(iterator), range(std::move(range)), code(std::move(code)) {}
  TypeInstruction getInstructionType() override { return ForT; }
  std::string toString() override {
    std::string out = "for " + iterator + " in " + range->toString() + ":\n";
    out += code->toString();
    return out;
  }

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
  std::string toString() override {
    std::string out = "while " + compare->toString() += ":\n";
    out += code->toString();
    return out;
  }

 private:
  std::unique_ptr<CompareExpr> compare;
  std::unique_ptr<CodeBlock> code;
};

#endif  // SRC_PARSER_INSTRUCTIONS_H_
