SOURCE_CODE=src/scanner/*.cpp
TEST_CODE=src/scanner/tests/*.cpp
MAIN=src/main.cpp

build: test
	g++ --std=c++11 $(MAIN) $(SOURCE_CODE) -O2 -o tkom.out

debug:
	g++ -g --std=c++11 $(MAIN) $(SOURCE_CODE) -o tkomd.out

test:
	g++ -O2 --std=c++11 $(TEST_CODE) $(SOURCE_CODE) -o tests.out -lboost_unit_test_framework
	./tests.out
