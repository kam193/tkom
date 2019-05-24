// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_VALUE_H_
#define SRC_EXECUTE_VALUE_H_

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class ValueType { None, Bool, Int, Real, Text, List };

class Value {
 public:
  explicit Value(ValueType _type) : type(_type) {}
  explicit Value(bool value) : type(ValueType::Bool), boolValue(value) {}
  explicit Value(std::int64_t value) : type(ValueType::Int), intValue(value) {}
  explicit Value(double value) : type(ValueType::Real), realValue(value) {}
  explicit Value(std::string value) : type(ValueType::Text), strValue(value) {}
  explicit Value(std::vector<Value> &elements)
      : type(ValueType::List), listElements(elements) {}

  ValueType getType() { return type; }
  void setType(ValueType newType) { type = newType; }
  std::int64_t getInt() { return intValue; }
  void setInt(std::int64_t val) { intValue = val; }
  double getReal() { return realValue; }
  void setReal(double val) { realValue = val; }
  std::string getStr() { return strValue; }
  void setStr(std::string str) { strValue = str; }
  std::vector<Value> &getListReference() { return listElements; }

 private:
  ValueType type;
  std::int64_t intValue;
  double realValue;
  bool boolValue;
  std::string strValue;
  std::vector<Value> listElements;
};

#endif  // SRC_EXECUTE_VALUE_H_
