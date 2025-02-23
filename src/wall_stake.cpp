#include "main.hpp" // IWYU pragma: keep

using namespace pros;

// 8. wall stake red hold motor reversed
static Motor motor {-8, v5::MotorGears::red, v5::MotorEncoderUnits::degrees};

/*
    def pickup(self):
        self.motor.spin_to_position(50, DEGREES, 60, RPM)
            
    def hold(self):
        self.motor.spin_to_position(140, DEGREES, 60, RPM)
        
    def score(self):
        self.motor.spin_to_position(300, DEGREES, 70, RPM)

    def reset(self):
        self.motor.spin_to_position(self.absolute0Position, DEGREES, 70, RPM)
*/

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