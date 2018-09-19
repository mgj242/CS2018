#ifndef GARAGECOMMAN_HPP
#define GARAGECOMMAND_HPP


/**
 *@desc Enum representation of garage commands.
 */
enum GarageDriverProtocol {
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


#endif
