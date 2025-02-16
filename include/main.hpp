#pragma once

#define PROS_USE_SIMPLE_NAMES

#include "api.h"
#include "EZ-Template/api.hpp"

#include "autons.hpp"


#ifdef __cplusplus
extern "C" {
#endif
void autonomous(void);
void initialize(void);
void disabled(void);
void competition_initialize(void);
void opcontrol(void);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// Add STL (standard template library) includes here if necessary (you probably don't need it.)
#endif

using namespace pros;

inline Controller controller (pros::E_CONTROLLER_MASTER);

inline MotorGroup left_motors ({1, -2, 3}, v5::MotorGears::blue, v5::MotorUnits::degrees);
inline MotorGroup right_motors ({-4, 5, -6}, v5::MotorGears::blue, v5::MotorUnits::degrees);

inline ez::Drive chassis ({1, -2, 3}, {-4, 5, -6}, 10, 4.0, 600.0, 600.0 / 360.0);

inline Motor lift_intake (-7, v5::MotorGears::blue, v5::MotorUnits::degrees);
inline Motor wall_stake (8, v5::MotorGears::red, v5::MotorUnits::degrees); 

inline Vision vision_sensor (9, E_VISION_ZERO_CENTER);

inline adi::Pneumatics stake_piston ('A', false, false);
inline adi::Pneumatics doink_piston ('B', false, false);

inline adi::DigitalIn donut_presence_sensor ('C');