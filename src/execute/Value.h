// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_VALUE_H_
#define SRC_EXECUTE_VALUE_H_

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class ValueType {
  None,
  Bool,
  Int,
  Real,
  Text,
  List,
  T_CONTINUE,
  T_BREAK,
  T_RETURN
};

class Value {
 public:
  Value() : type(ValueType::None) {}
  explicit Value(std::shared_ptr<Value> val)
      : type(ValueType::T_RETURN), val_ptr(val) {}
  explicit Value(ValueType type) : type(type) {}
  explicit Value(bool value) : type(ValueType::Bool), boolValue(value) {}
  explicit Value(std::int64_t value) : type(ValueType::Int), intValue(value) {}
  explicit Value(double value) : type(ValueType::Real), realValue(value) {}
  explicit Value(std::string value) : type(ValueType::Text), strValue(value) {}
  explicit Value(std::vector<std::shared_ptr<Value>> &elements)
      : type(ValueType::List), listElements(elements) {}

  ValueType getType() { return type; }
  void setType(ValueType newType) { type = newType; }
  std::int64_t getInt() { return intValue; }
  void setInt(std::int64_t val) { intValue = val; }
  double getReal() { return realValue; }
  void setReal(double val) { realValue = val; }
  std::string getStr() { return strValue; }
  void setStr(std::string str) { strValue = str; }
  std::vector<std::shared_ptr<Value>> getList() { return listElements; }
  void setBool(bool val) { boolValue = val; }
  bool getBool() { return boolValue; }
  std::shared_ptr<Value> getValuePtr() { return val_ptr; }

  std::string toString();

 private:
  ValueType type;
  std::int64_t intValue;
  double realValue;
  bool boolValue;
  std::string strValue;
  std::vector<std::shared_ptr<Value>> listElements;
  std::shared_ptr<Value> val_ptr;

  std::string listToString();
};

#endif  // SRC_EXECUTE_VALUE_H_
