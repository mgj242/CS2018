/*
 @desc Header GarageState Class used for setting and getting Garage attributes
*/ 


#ifndef GARAGE_STATE_HPP
#define GARAGE_STATE_HPP


#include <cstdint>
 

class GarageState {

public:
    uint8_t getStripePosition() const;
    void setStripePosition(uint8_t StripePosition);
    uint8_t getStripesCount() const;
    void setStripesCount(uint8_t stripePosition);
    bool getInteriorLights() const;
    void setInteriorLights(bool isOn);
    bool getExteriorLights() const;
    void setExteriorLights(bool isOn);
private:

    uint8_t _doorStripePosition;
    uint8_t _doorStripesCount;
    // MotorState _motorState;
    bool _interiorLights;
    bool _exteriorLights;
};

#endif
