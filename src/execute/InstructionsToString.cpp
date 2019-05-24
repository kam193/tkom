// Copyright 2019 Kamil Mankowski

#include "Instructions.h"

std::string CodeBlock::toString() {
  std::string outstr = "  ";
  for (auto &instr : instructions) {
    outstr += std::regex_replace(instr->toString(), std::regex("\n"), "\n  ");
    outstr += "\n  ";
  }
  return outstr.substr(0, outstr.size() - 3);
}

std::string Function::toString() {
  std::string out = "def " + name + "(";
  for (int i = 0; i < argumentNames.size(); ++i) {
    out += argumentNames[i];
    if (i != argumentNames.size() - 1) out += ", ";
  }
  out += "):\n";
  out += code->toString();
  return out;
}

std::string Constant::toString() {
  switch (type) {
    case ValueType::None:
      return "None";
    case ValueType::Bool:
      return boolValue ? "True" : "False";
    case ValueType::Int:
      return std::to_string(intValue);
    case ValueType::Real:
      return std::to_string(realValue);
    case ValueType::Text:
      return "\"" + strValue + "\"";
    case ValueType::List:
      return listToString();
    default:
      throw std::runtime_error("Constant type invalid.");
  }
}

std::string Constant::listToString() {
  std::string out = "[";
  for (int i = 0; i < listElements.size(); ++i) {
    out += listElements[i]->toString();
    if (i != listElements.size() - 1) out += ", ";
  }
  out += "]";
  return out;
}

std::string Slice::toString() {
  std::string out = source->toString() += "[";
  out += std::to_string(start);
  if (type != SliceType::Start) out += ":";
  if (type == SliceType::StartToSlice) out += std::to_string(end);
  out += "]";
  return out;
}

std::string FunctionCall::toString() {
  std::string out = name + "(";
  for (int i = 0; i < args.size(); ++i) {
    out += args[i]->toString();
    if (i != args.size() - 1) out += ", ";
  }
  out += ")";
  return out;
}

std::string Expression::toString() {
  std::string out = "";
  for (int i = 0; i < args.size(); ++i) {
    if (i != 0) out += typeToString(types[i - 1]);
    out += args[i]->toString();
  }
  return out;
}

std::string Expression::typeToString(Type _type) {
  switch (_type) {
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

std::string CompareExpr::toString() {
  std::string out = leftExpr->toString();
  out += operatorToString();
  if (rightExpr != nullptr) out += rightExpr->toString();
  return out;
}

std::string CompareExpr::operatorToString() {
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

std::string AssignExpr::toString() {
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

std::string If::toString() {
  std::string out = "if ";
  out += compare->toString();
  out += ":\n";
  out += ifCode->toString();
  return out;
}

std::string For::toString() {
  std::string out = "for " + iterator + " in " + range->toString() + ":\n";
  out += code->toString();
  return out;
}

std::string While::toString() {
  std::string out = "while " + compare->toString() += ":\n";
  out += code->toString();
  return out;
}
