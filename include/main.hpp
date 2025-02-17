#pragma once

#define PROS_USE_SIMPLE_NAMES

#include "api.h"
#include "EZ-Template/api.hpp"

#include "autons.hpp"
#include "color_sort.hpp"

extern "C" {
    void autonomous(void);
    void initialize(void);
    void disabled(void);
    void competition_initialize(void);
    void opcontrol(void);
}

using namespace pros;

inline Controller controller (pros::E_CONTROLLER_MASTER);

/* EZ template uses the first motor for tracking */
inline ez::Drive chassis(
    {1, -2, 3},
    {-4, 5, -6},
    10,  // IMU Port

    3.35,   // Wheel Diameter (inches)
    360.0);  // Wheel RPM

inline Motor lift_intake (-7, v5::MotorGears::blue, v5::MotorUnits::degrees);
inline Motor wall_stake (8, v5::MotorGears::red, v5::MotorUnits::degrees); 

inline Vision vision_sensor (9, E_VISION_ZERO_CENTER);

inline adi::Pneumatics stake_piston ('A', false, false);
inline adi::Pneumatics doink_piston ('B', false, false);

inline adi::DigitalIn donut_presence_sensor ('C');