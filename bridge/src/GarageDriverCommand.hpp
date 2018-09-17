#ifndef GARAGECOMMAND_HPP
#define GARAGECOMMAND_HPP


/**
 *@desc Garage controller commands.
 */
enum GarageDriverCommand{
	NoOperation,
	OpenToNextStrip,
	FullOpen,
	FullClose,
	CloseToNextStrip,
	SwitchInteriorsLightsOn,
	SwitchInteriorLightsOff,
	SwitchExteriorLightsOn,
	SwitchExteriorLightsOff
};


#endif // !defined GARAGECOMMAND_HPP
