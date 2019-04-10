// Copyright 2019 Kamil Mankowski

#include "Validation.h"

namespace validation {

bool isValidIdentiferChar(char c) { return isalnum(c) || c == '_'; }

bool isValidRealNumber(const std::string &value, int pointerPosition) {
  for (int i = 0; i < value.size(); ++i) {
    if (i != pointerPosition && !isdigit(value[i])) return false;
  }
  return value[pointerPosition] == '.';
}

bool isValidIntegerNumber(const std::string &value) {
  for (auto &digit : value) {
    if (!isdigit(digit)) return false;
  }
  return true;
}

bool isValidHexNumber(const std::string &value) {
  for (auto &digit : value) {
    if (!isxdigit(digit)) return false;
  }
  return true;
}

}  // namespace validation
