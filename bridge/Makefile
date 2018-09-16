all: bin/bridge



bin/bridge: obj/Config.o
	g++ -o $@ $+ 

obj/Config.o: src/Config.cpp src/Config.hpp
	g++ -std=c++11 $< -o $@



.PHONY: all clean

clean:
	rm -f obj/*. bin/*. 
