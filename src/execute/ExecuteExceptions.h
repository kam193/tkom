// Copyright 2019 Kamil Mankowski

#ifndef SRC_EXECUTE_EXECUTEEXCEPTIONS_H_
#define SRC_EXECUTE_EXECUTEEXCEPTIONS_H_

#include <exception>
#include <string>

class ExecuteExceptionBase : public std::exception {
 public:
  ExecuteExceptionBase() { message = "Error on line <TODO>:\n\t"; }
  const char* what() const throw() { return message.c_str(); }

 protected:
  std::string message;
};

class ReadNotAssignVariable : public ExecuteExceptionBase {
 public:
  explicit ReadNotAssignVariable(std::string varname) : ExecuteExceptionBase() {
    message += "Variable '" + varname + "' never assign, but try to get value.";
  }
};

class OutOfRange : public ExecuteExceptionBase {
 public:
  explicit OutOfRange(int index) : ExecuteExceptionBase() {
    message += "Index '" + std::to_string(index) + "' is out of range.";
  }
};

class NotList : public ExecuteExceptionBase {
 public:
  explicit NotList(std::string name) : ExecuteExceptionBase() {
    message += "Slice need a list, '" + name + "' is not a list.";
  }
};

class FunctionNotDeclared : public ExecuteExceptionBase {
 public:
  explicit FunctionNotDeclared(std::string name) : ExecuteExceptionBase() {
    message += "Unable to find function '" + name + "'.";
  }
};

class OperandsTypesNotCompatible : public ExecuteExceptionBase {
 public:
  OperandsTypesNotCompatible(std::string ltyp, std::string rtyp,
                             std::string expr)
      : ExecuteExceptionBase() {
    message += "Operands types are not supported for expression: " + expr;
    // TODO: types to string
  }
};

class UnexpectedError : public ExecuteExceptionBase {
 public:
  UnexpectedError() : ExecuteExceptionBase() {
    message += "Unexpected error occured.";
  }
};

class IterableExpected : public ExecuteExceptionBase {
 public:
  IterableExpected() : ExecuteExceptionBase() {
    message +=
        "For expected slice or list as a range, but something else is given.";
  }
};

class TypesNotComparable : public ExecuteExceptionBase {
 public:
  TypesNotComparable() : ExecuteExceptionBase() {
    message += "Types in compare expression are not comparable.";
  }
};

class ParametersCountNotExpected : public ExecuteExceptionBase {
 public:
  ParametersCountNotExpected(std::string funcname, int given, int expected)
      : ExecuteExceptionBase() {
    message += "Function '" + funcname + "' expects " +
               std::to_string(expected) + " arguments, but " +
               std::to_string(given) + " given.";
  }
};

class TypeNotExpected : public ExecuteExceptionBase {
 public:
  explicit TypeNotExpected(std::string expected) : ExecuteExceptionBase() {
    message += "Variable type not expected. It should be '" + expected + "'.";
  }
};

#endif  // SRC_EXECUTE_EXECUTEEXCEPTIONS_H_
