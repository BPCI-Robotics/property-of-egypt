#include "main.hpp"
#include "lemlib/api.hpp" // IWYU pragma: keep

using namespace pros;

Controller controller(CONTROLLER_MASTER);

/*
a. piston stake grabber (off; on is true)
b. piston doinker (off; on is true)
*/
adi::Pneumatics stake_grab ('a', false, false);
adi::Pneumatics doink_piston ('b', false, false);

/*
drivetrain blue coast:
    1. motor left
    2. motor left reversed
    3. motor left
    
    4. motor right reversed
    5. motor right
    6. motor right reversed
*/
MotorGroup left_motors ({1, -2, 3}, v5::MotorGears::blue, v5::MotorEncoderUnits::degrees);
MotorGroup right_motors ({-4, 5, -6}, v5::MotorGears::blue, v5::MotorEncoderUnits::degrees);

// 10. IMU (inertial sensor)
v5::Imu imu(10);

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

lemlib::OdomSensors sensors(nullptr, nullptr, nullptr, nullptr, &imu);

lemlib::Chassis chassis(drivetrain, 
                        lateral_controller, 
                        angular_controller, 
                        sensors);

void initialize() {
    std::puts("Hello!");
    lcd::initialize();
    // chassis.calibrate();
    chassis.setBrakeMode(MOTOR_BRAKE_COAST);

    Task screen_task([&]() {
        while (false) {
            lemlib::Pose pose = chassis.getPose();

            lcd::print(4, "    X: %f", pose.x);
            lcd::print(5, "    Y: %f", pose.y);
            lcd::print(6, "Theta: %f", pose.theta);

            pros::delay(100);
        }
    });
    
    std::puts("Just before initialization!");
    auton_selector::init(auton::set_config);
}

void disabled() {}

ASSET(example_txt);

void autonomous() {
    auton::start();
}

void opcontrol() {
    lift_intake::init();
    wall_stake::init();

    while (true) {

        if (controller.get_digital(DIGITAL_L1)) 
            lift_intake::spin(FORWARD);

        else if (controller.get_digital(DIGITAL_L2))
            lift_intake::spin(REVERSE);

        else 
            lift_intake::stop();
        
        /* Pneumatics logic */
        if (controller.get_digital_new_press(DIGITAL_R2))
            stake_grab.toggle();
        
        if (controller.get_digital_new_press(DIGITAL_R1))
            doink_piston.toggle();

        /* Wall stake logic */
        if (controller.get_digital_new_press(DIGITAL_X))
            wall_stake::pickup();

        else if (controller.get_digital_new_press(DIGITAL_Y))
            wall_stake::reset();

        
        if (controller.get_digital(DIGITAL_B))
            wall_stake::reverse();
        else
            wall_stake::stop();
        
        if (controller.get_digital(DIGITAL_A))
            wall_stake::forward();
        else
            wall_stake::stop();


        // Controller code
        int leftY = controller.get_analog(ANALOG_LEFT_Y);
        int rightX = -controller.get_analog(ANALOG_RIGHT_X);

        chassis.curvature(leftY, rightX);

        delay(10);
    }
}
