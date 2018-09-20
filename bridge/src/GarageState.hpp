#ifndef GARAGESTATE_HPP
#define GARAGESTATE_HPP


#include <cstdint>
 
#include "MotorState.hpp"


/**
 * @ desc Garage controller state.
 */
class GarageState {
public:
    // Accessors

    uint8_t getStripePosition() const;
    void setStripePosition(uint8_t StripePosition);
    uint8_t getStripesCount() const;
    void setStripesCount(uint8_t stripePosition);

    MotorState getMotorState() const;
    void setMotorState(MotorState);

    bool getInteriorLights() const;
    void setInteriorLights(bool isOn);
    bool getExteriorLights() const;
    void setExteriorLights(bool isOn);

private:
    // Attributes

    uint8_t _doorStripePosition;
    uint8_t _doorStripesCount;

    MotorState _motorState;
    bool _interiorLights;
    bool _exteriorLights;
};


#endif // !defined GARAGESTATE_HPP
