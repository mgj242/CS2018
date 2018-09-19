#include "GarageState.hpp"


// Construction & destruction

GarageState::GarageState(uint8_t doorStripePosition, uint8_t doorStripesCount,     MotorState motorState, 
    bool interiorLights, bool exteriorLights) :
    _doorStripePosition(doorStripePosition)
,   _doorStripesCount(doorStripesCount)
,   _motorState(motorState)
,   _interiorLights(interiorLights)
,   _exteriorLights(exteriorLights)
{
}

GarageState::GarageState(){

}

// Accessors


uint8_t GarageState:: getStripePosition() const {
    return _doorStripePosition;
}

void GarageState:: setStripePosition(uint8_t stripePosition) {
    _doorStripePosition = stripePosition;
}

uint8_t GarageState::getStripesCount() const {
    return _doorStripesCount;
}
 
void GarageState::setStripesCount(uint8_t stripePosition) {
    _doorStripePosition = stripePosition;
}

MotorState GarageState::getMotorState() const {
    return _motorState;
}

void GarageState::setMotorState(MotorState motorState) {
    _motorState = motorState;
}

bool GarageState::getInteriorLights() const {
    return _interiorLights;
}

void GarageState::setInteriorLights(bool isOn) {
    _interiorLights = isOn;
} 

bool GarageState::getExteriorLights() const {
    return _exteriorLights;
}

void GarageState::setExteriorLights(bool isOn) {
    _exteriorLights = isOn;
}
