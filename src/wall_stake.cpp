#include "main.hpp" // IWYU pragma: keep

using namespace pros;

// 8. wall stake red hold motor reversed
static Motor motor {-8, v5::MotorGears::red, v5::MotorEncoderUnits::degrees};

static const int vel = 75; // RPM

namespace wall_stake {

    void init() {
        motor.set_brake_mode(MOTOR_BRAKE_HOLD);
    }

    void pickup() {
        motor.move_absolute(50, vel);
    }

    void hold() {
        motor.move_absolute(140, vel);
    }

    void score() {
        motor.move_absolute(300, vel);
    }

    void reset() {
        motor.move_absolute(0, vel);
    }
}