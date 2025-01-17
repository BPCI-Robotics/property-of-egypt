#include "main.h"
#include "pros/vision.hpp"
#include "pros/vision.h"

using namespace pros;

Controller controller(CONTROLLER_MASTER);

Motor liftIntake (7, MotorGears::blue, MotorUnits::counts);
adi::Pneumatics left_pissed_in ('a', false, false);
adi::Pneumatics right_pissed_in ('b', false, false);

MotorGroup left_motors ({1, -2, -3}, v5::MotorGears::blue, v5::MotorUnits::counts);
MotorGroup right_motors ({-4, 5, -6}, v5::MotorGears::blue, v5::MotorUnits::counts);

v5::Vision color_sensor (9);

void initialize() {
    lcd::initialize();

    static vision_signature_s_t BLUE_SIG = Vision::signature_from_utility (1, -3775, -3259, -3517, 4809, 7525, 6167, 3.0, 0);
    static vision_signature_s_t RED_SIG =  Vision::signature_from_utility (2,  7457,  9721,  8589, -611,    1, -305, 3.0, 0);

    color_sensor.set_signature(1, &BLUE_SIG);
    color_sensor.set_signature(2, &RED_SIG);
}

void disabled() {}

void autonomous() {}

void opcontrol() {
    int target_signature = 1;
    const int speed = 600;

    while (true) {

        int forwardpower = controller.get_analog(E_CONTROLLER_ANALOG_LEFT_Y);
        int turnpower = controller.get_analog(E_CONTROLLER_ANALOG_RIGHT_X);

        int left = forwardpower + turnpower;
        int right = forwardpower - turnpower;

        left = left * speed / 100;
        right = right * speed / 100;

        left_motors.move_velocity(left);
        right_motors.move_velocity(right);

        if (controller.get_digital(DIGITAL_L1)) 
            liftIntake.move_velocity(speed);

        else if (controller.get_digital(DIGITAL_L2))
            liftIntake.move_velocity(-1 * speed);

        else 
            liftIntake.move_velocity(0);

        if (controller.get_digital(DIGITAL_R1)) {
            left_pissed_in.toggle();
            right_pissed_in.toggle();
        }


        vision_object_s_t obj = color_sensor.get_by_sig(0, target_signature);

        delay(20);
    }
}