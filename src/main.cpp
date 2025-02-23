#include "main.hpp"
#include "lemlib/api.hpp" // IWYU pragma: keep

using namespace pros;

Controller controller(CONTROLLER_MASTER);

adi::Pneumatics stake_grab ('a', false, false);
adi::Pneumatics doink_piston ('b', false, false);

MotorGroup left_motors ({1, -2, 3}, v5::MotorGears::blue, v5::MotorUnits::counts);
MotorGroup right_motors ({-4, 5, -6}, v5::MotorGears::blue, v5::MotorUnits::counts);

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
v5::Imu imu(10);

lemlib::OdomSensors sensors(nullptr, nullptr, nullptr, nullptr, &imu);

lemlib::Chassis chassis(drivetrain, 
                        lateral_controller, 
                        angular_controller, 
                        sensors);

void initialize() {
    lcd::initialize();
    chassis.calibrate();

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
            lift_intake::spin(FORWARD);

        else if (controller.get_digital(DIGITAL_L2))
            lift_intake::spin(REVERSE);

        else 
            lift_intake::stop();

        if (controller.get_digital(DIGITAL_R1)) {
            stake_grab.toggle();
        }

        if (controller.get_digital_new_press(DIGITAL_R2)) {
            doink_piston.toggle();
        }

        // Controller code
        int leftY = controller.get_analog(ANALOG_LEFT_Y);
        int rightX = controller.get_analog(ANALOG_RIGHT_X);

        chassis.curvature(leftY, rightX);

        // pros:: is specified, since delay() is ambiguous.
        pros::delay(20);
    }
}