build:
	g++ --std=c++11 src/*.cpp src/scanner/*.cpp -O2 -o tkom.out

debug:
	g++ -g --std=c++11 src/*.cpp src/scanner/*.cpp -O2 -o tkomd.out