.PHONY: all clean


CC=g++
CC_OPTIONS=-I src/ -std=c++11 -Wall -O3
# CC_OPTIONS=-I src/ -std=c++11 -Wall -O0 -g


all: bin/bridge


clean:
	rm -f obj/*. bin/*. 


bin/bridge: obj/Config.o obj/GarageState.o
	$(CC) -o $@ $+ 


obj/Config.o: src/Config.cpp src/Config.hpp
	$(CC) $(CC_OPTIONS) -c $< -o $@


obj/GarageState.o: src/GarageState.cpp src/GarageState.hpp
	$(CC) $(CC_OPTIONS) -c $< -o $@
