#include "Config.hpp"
#include <iostream>
using namespace std;

int main() 
{
 	Config cfg;
	const char* c = cfg.getSerialPortDevicePath();
	cout <<"This is path" <<c << endl;   
	return 0;
}
