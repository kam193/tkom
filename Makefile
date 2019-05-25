SOURCE_CODE=src/scanner/*.cpp src/parser/*.cpp src/execute/*.cpp
TEST_CODE=src/tests_main.cpp src/scanner/tests/*.cpp src/parser/tests/*.cpp src/execute/tests/*.cpp
MAIN=src/main.cpp

build: test build-notest

build-notest:
	g++ --std=c++14 $(MAIN) $(SOURCE_CODE) -O2 -o tkom.out

debug:
	g++ -g --std=c++14 $(MAIN) $(SOURCE_CODE) -o tkomd.out

test:
	g++ -O2 --std=c++14 $(TEST_CODE) $(SOURCE_CODE) -o tests.out -lboost_unit_test_framework
	./tests.out

clean:
	rm tkom.out tkomd.out tests.out
