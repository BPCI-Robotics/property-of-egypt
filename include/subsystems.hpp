#pragma once

#include "EZ-Template/api.hpp"
#include "api.h"

extern ez::Drive chassis;

// Your motors, sensors, etc. should go here.  Below are examples

// inline pros::Motor intake(1);
// inline pros::adi::DigitalIn limit_switch('A');

inline Controller controller (pros::E_CONTROLLER_MASTER);

inline MotorGroup left_motors ({1, -2, 3}, v5::MotorGears::blue, v5::MotorUnits::degrees);
inline MotorGroup right_motors ({-4, 5, -6}, v5::MotorGears::blue, v5::MotorUnits::degrees);

inline Motor lift_intake (-7, v5::MotorGears::blue, v5::MotorUnits::degrees);
inline Motor wall_stake (8, v5::MotorGears::red, v5::MotorUnits::degrees); 

inline Vision vision_sensor (9, E_VISION_ZERO_CENTER);

inline adi::Pneumatics stake_piston ('A', false, false);
inline adi::Pneumatics doink_piston ('B', false, false);

inline adi::Button donut_presence_sensor ('C');