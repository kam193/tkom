// Copyright 2019 Kamil Mankowski

#include <sstream>

#include <boost/test/unit_test.hpp>
#include "../Scanner.h"

BOOST_AUTO_TEST_SUITE(ScannerTest)

BOOST_AUTO_TEST_CASE(initial) {
  int a = 1;
  BOOST_CHECK(a == 1);
}

BOOST_AUTO_TEST_SUITE_END()
