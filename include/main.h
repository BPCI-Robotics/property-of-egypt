#pragma once

#define PROS_USE_SIMPLE_NAMES

#include "api.h"
#include "EZ-Template/api.hpp"

#include "autons.hpp"
#include "subsystems.hpp"


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