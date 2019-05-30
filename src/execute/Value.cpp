// Copyright 2019 Kamil Mankowski

#include "Value.h"

std::string Value::toString() {
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
      return "CONTROL VARIABLE";
  }
}

std::string Value::listToString() {
  std::string out = "[";
  for (int i = 0; i < listElements.size(); ++i) {
    out += listElements[i]->toString();
    if (i != listElements.size() - 1) out += ", ";
  }
  out += "]";
  return out;
}
