// Copyright 2019 Kamil Mankowski

#ifndef SRC_SCANNER_VALIDATION_H_
#define SRC_SCANNER_VALIDATION_H_

#include <cctype>
#include <string>

namespace validation {

bool isValidIdentiferChar(char c);
bool isValidRealNumber(const std::string &value, int pointerPosition);
bool isValidIntegerNumber(const std::string &value);
bool isValidHexNumber(const std::string &value);

}  // namespace validation

#endif  // SRC_SCANNER_VALIDATION_H_
