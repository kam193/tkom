// Copyright 2019 Kamil Mankowski

#include <boost/test/unit_test.hpp>

#include "../Value.h"

BOOST_AUTO_TEST_SUITE(ValueTest)

BOOST_AUTO_TEST_CASE(test_list_element_can_be_changed) {
  std::vector<std::shared_ptr<Value>> initlist{
      std::make_shared<Value>(std::string{"test"}),
      std::make_shared<Value>(15L)};
  Value val(initlist);

  auto list = val.getList();
  BOOST_TEST(list[0]->getStr() == "test");
  BOOST_TEST(list[1]->getInt() == 15L);

  list[0]->setStr("newval");
  list[1]->setInt(22L);

  auto newref = val.getList();
  BOOST_TEST(newref[0]->getStr() == "newval");
  BOOST_TEST(newref[1]->getInt() == 22L);
}

BOOST_AUTO_TEST_SUITE_END()
