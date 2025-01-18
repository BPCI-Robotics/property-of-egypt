#include "main.h"
#include "lemlib/api.hpp"
#include "pros/vision.hpp"
#include "pros/vision.h"

using namespace pros;

Controller controller(CONTROLLER_MASTER);

Motor lift_intake (7, MotorGears::blue, MotorUnits::counts);

adi::Pneumatics stake_grab_left ('a', false, false);
adi::Pneumatics stake_grab_right ('b', false, false);

MotorGroup left_motors ({1, -2, -3}, v5::MotorGears::blue, v5::MotorUnits::counts);
MotorGroup right_motors ({-4, 5, -6}, v5::MotorGears::blue, v5::MotorUnits::counts);

v5::Vision vision_sensor (9);

// drivetrain settings
lemlib::Drivetrain drivetrain(&left_motors,
                              &right_motors,
                              10, // TODO: set track width (inches)
                              lemlib::Omniwheel::NEW_325,
                              360,
                              2 // higher = faster, less accurate
);

// lateral PID controller
lemlib::ControllerSettings lateral_controller( 10, // proportional gain (kP)
                                                0, // integral gain (kI)
                                                3, // derivative gain (kD)
                                                3, // anti windup
                                                1, // small error range, in inches
                                              100, // small error range timeout, in milliseconds
                                                3, // large error range, in inches
                                              500, // large error range timeout, in milliseconds
                                               20  // maximum acceleration (slew)
);

// angular PID controller
lemlib::ControllerSettings angular_controller(  2, // proportional gain (kP)
                                                0, // integral gain (kI)
                                               10, // derivative gain (kD)
                                                3, // anti windup
                                                1, // small error range, in degrees
                                              100, // small error range timeout, in milliseconds
                                                3, // large error range, in degrees
                                              500, // large error range timeout, in milliseconds
                                                0  // maximum acceleration (slew)
);

// odometry sensors
lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel 1
                            nullptr, // vertical tracking wheel 2
                            nullptr, // horizontal tracking wheel 1
                            nullptr, // horizontal tracking wheel 2
                            nullptr  // inertial sensor
);

lemlib::Chassis chassis(drivetrain, 
                        lateral_controller, 
                        angular_controller, 
                        sensors);

#define BLUE_SIG_ID 1
#define RED_SIG_ID  2

void initialize() {
    lcd::initialize();
    chassis.calibrate();

    static vision_signature_s_t blue_signature = Vision::signature_from_utility (1, -3775, -3259, -3517, 4809, 7525, 6167, 3.0, 0);
    static vision_signature_s_t red_signature =  Vision::signature_from_utility (2,  7457,  9721,  8589, -611,    1, -305, 3.0, 0);

    vision_sensor.set_signature(BLUE_SIG_ID, &blue_signature);
    vision_sensor.set_signature(RED_SIG_ID,  &red_signature);

    // This code passes a lambda to an object constructor to create an asynchronous routine.
    // C++ is weird.
    Task screen_task([&]() {
        while (true) {
            lemlib::Pose pose = chassis.getPose();

            lcd::print(0, "    X: %f", pose.x);
            lcd::print(1, "    Y: %f", pose.y);
            lcd::print(2, "Theta: %f", pose.theta);

            pros::delay(100);
        }
    });
}

void disabled() {}

void autonomous() {}

void opcontrol() {

    while (true) {

        if (controller.get_digital(DIGITAL_L1)) 
            lift_intake.move_velocity(600);

        else if (controller.get_digital(DIGITAL_L2))
            lift_intake.move_velocity(-600);

        else 
            lift_intake.move_velocity(0);

        if (controller.get_digital(DIGITAL_R1)) {
            stake_grab_left.toggle();
            stake_grab_right.toggle();
        }

        // Controller code
        int leftY = controller.get_analog(E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(E_CONTROLLER_ANALOG_RIGHT_X);

        chassis.curvature(leftY, rightX);

        vision_object blue_obj = vision_sensor.get_by_sig(0, BLUE_SIG_ID);
        vision_object red_obj = vision_sensor.get_by_sig(0, RED_SIG_ID);

        // pros:: is specified, since delay() is ambiguous.
        pros::delay(20);
    }
}