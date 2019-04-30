all: synscanmake finscanmake udpscanmake
synscanmake: synscan.cpp
	g++ -std=c++11 synscan.cpp -o synscan
finscanmake: finscan.cpp
	g++ -std=c++11 -pthread finscan.cpp -o finscan
udpscanmake: udpscan.cpp
	g++ -std=c++11 -pthread udpscan.cpp -o udpscan