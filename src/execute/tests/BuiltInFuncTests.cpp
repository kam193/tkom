// Copyright 2019 Kamil Mankowski

#include <sstream>

#include <boost/test/unit_test.hpp>

#include "../BuiltInFunc.h"

BOOST_AUTO_TEST_SUITE(BuiltInFuncTest)

BOOST_AUTO_TEST_CASE(test_print) {
  std::stringstream stream;
  auto val = std::make_shared<Value>(std::string{"test"});
  auto ctx = std::make_shared<Context>();

  PrintFunction print(stream);
  ctx->addParameter(val);
  print.exec(ctx);

  BOOST_TEST(stream.str() == "test \n");
}

BOOST_AUTO_TEST_CASE(test_range) {
  auto val = std::make_shared<Value>(3l);
  auto ctx = std::make_shared<Context>();

  RangeFunction range;
  ctx->addParameter(val);
  auto result = range.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::List));
  auto list = result->getList();
  BOOST_TEST(list.size() == 3);
  BOOST_TEST(list[0]->getInt() == 0);
  BOOST_TEST(list[1]->getInt() == 1);
  BOOST_TEST(list[2]->getInt() == 2);
}

BOOST_AUTO_TEST_CASE(test_range_no_arg) {
  auto ctx = std::make_shared<Context>();

  RangeFunction range;
  BOOST_CHECK_THROW(range.exec(ctx), ParametersCountNotExpected);
}

BOOST_AUTO_TEST_CASE(test_range_wrong_arg) {
  auto val = std::make_shared<Value>(std::string{"nonumber"});
  auto ctx = std::make_shared<Context>();

  RangeFunction range;
  ctx->addParameter(val);
  BOOST_CHECK_THROW(range.exec(ctx), TypeNotExpected);
}

BOOST_AUTO_TEST_CASE(test_len_str) {
  auto val = std::make_shared<Value>(std::string{"123"});
  auto ctx = std::make_shared<Context>();

  LenFunction len;
  ctx->addParameter(val);
  auto result = len.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::Int));
  BOOST_TEST(result->getInt() == 3);
}

BOOST_AUTO_TEST_CASE(test_len_list) {
  auto ctx = std::make_shared<Context>();
  std::vector<std::shared_ptr<Value>> elements;
  elements.push_back(std::make_shared<Value>(std::string{"123"}));
  elements.push_back(std::make_shared<Value>(4L));
  auto val = std::make_shared<Value>(elements);

  LenFunction len;
  ctx->addParameter(val);
  auto result = len.exec(ctx);

  BOOST_TEST((result->getType() == ValueType::Int));
  BOOST_TEST(result->getInt() == 2);
}

BOOST_AUTO_TEST_CASE(test_len_no_arg) {
  auto ctx = std::make_shared<Context>();

  LenFunction len;
  BOOST_CHECK_THROW(len.exec(ctx), ParametersCountNotExpected);
}

BOOST_AUTO_TEST_CASE(test_len_wrong_arg) {
  auto val = std::make_shared<Value>(1L);
  auto ctx = std::make_shared<Context>();

  LenFunction len;
  ctx->addParameter(val);
  BOOST_CHECK_THROW(len.exec(ctx), TypeNotExpected);
}

BOOST_AUTO_TEST_SUITE_END()
