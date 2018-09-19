.PHONY: all clean


CC=g++
CC_COMMON_OPTIONS=-I src/ -std=c++11 -Wall
CC_OPTIONS=$(CC_COMMON_OPTIONS) -O3
# CC_OPTIONS=-$(CC_COMMON_OPTIONS) -O0 -g


all: bin/bridge


clean:
	rm -f obj/*. bin/*. 


bin/bridge: obj/Config.o obj/GarageState.o
	$(CC) -o $@ $+ 


obj/Config.o: src/Config.cpp src/Config.hpp
	$(CC) $(CC_OPTIONS) -c $< -o $@

obj/GarageState.o: src/GarageState.cpp src/GarageState.hpp
	$(CC) $(CC_OPTIONS) -c $< -o $@

obj/main.o: src/main.cpp src/Config.hpp
	$(CC) $(CC_OPTIONS) -c $< -o $@
