build:
	g++ --std=c++11 src/*.cpp src/scanner/*.cpp -O2 -o tkom.out

debug:
	g++ -g --std=c++11 src/*.cpp src/scanner/*.cpp -o tkomd.out

test:
	g++ -g --std=c++11 src/scanner/*.cpp src/scanner/tests/*.cpp -o tests.out -lboost_unit_test_framework
	./tests.out
