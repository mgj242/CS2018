#include <iostream>

#include "Config.hpp"
#include "GarageState.hpp"
#include "GarageDriverProtocol.hpp"

using namespace std;


int main() 
{
 	

    GarageState gs;
    GarageDriverProtocol gdp; 

    gdp.recieveState(gs);




    
    
	return 0;
}
