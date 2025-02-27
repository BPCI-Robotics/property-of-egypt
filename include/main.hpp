#ifndef _PROS_MAIN_H_
#define _PROS_MAIN_H_

/**
 * If defined, some commonly used enums will have preprocessor macros which give
 * a shorter, more convenient naming pattern. If this isn't desired, simply
 * comment the following line out.
 *
 * For instance, E_CONTROLLER_MASTER has a shorter name: CONTROLLER_MASTER.
 * E_CONTROLLER_MASTER is pedantically correct within the PROS styleguide, but
 * not convenient for most student programmers.
 */
#define PROS_USE_SIMPLE_NAMES

/**
 * If defined, C++ literals will be available for use. All literals are in the
 * pros::literals namespace.
 *
 * For instance, you can do `4_mtr = 50` to set motor 4's target velocity to 50
 */
// #define PROS_USE_LITERALS

#include "api.h" // IWYU pragma: keep

/**
 * You should add more #includes here
 */
//#include "okapi/api.hpp"
//#include "pros/api_legacy.h"

/**
 * If you find doing pros::Motor() to be tedious and you'd prefer just to do
 * Motor, you can use the namespace with the following commented out line.
 *
 * IMPORTANT: Only the okapi or pros namespace may be used, not both
 * concurrently! The okapi namespace will export all symbols inside the pros
 * namespace.
 */
// using namespace pros;
// using namespace pros::literals;
// using namespace okapi;

/**
 * Prototypes for the competition control tasks are redefined here to ensure
 * that they can be called from user code (i.e. calling autonomous from a
 * button press in opcontrol() for testing purposes).
 */
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
/**
 * You can add C++-only headers here
 */
//#include <iostream>
#endif

#endif  // _PROS_MAIN_H_

/* Here I define all my namespaces. */

namespace auton_selector {
    void init(void (*enter_callback)(std::unordered_map<std::string, std::string>));
    void btn0_cb();
    void btn1_cb();
    void btn2_cb();
}

namespace wall_stake {
    void init();
    void pickup();
    void forward();
    void reverse();
    void stop();
    void reset();
}

#define REVERSE -1
#define FORWARD  1

#define REJECT_RED 1
#define REJECT_BLUE 2

enum class auton_direction { LEFT, RIGHT };

namespace lift_intake {
    void init(int color_to_reject);
    void init();
    void stop();
    void spin(int direction);
    
} /* namespace color_sort */

namespace auton {
    void set_config(std::unordered_map<std::string, std::string> config);
    void start();
}

using namespace pros;