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

#endif  // SRC_EXECUTE_EXECUTEEXCEPTIONS_H_
