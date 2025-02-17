#include "main.hpp"

void initialize() {

    /* Legacy ports need time to start */
    delay(500);

    /* Disable changing the controller curve */
    chassis.opcontrol_curve_buttons_toggle(false);

    /* This sets the kP for braking (2 is recommended) */
    chassis.opcontrol_drive_activebrake_set(2.0);

    /* Use no special curving when driving. */
    chassis.opcontrol_curve_default_set(0.0, 0.0);
    wall_stake.tare_position();
    
	lift_intake.set_brake_mode(MOTOR_BRAKE_BRAKE);
	wall_stake.set_brake_mode(MOTOR_BRAKE_HOLD);

    /* Vision sensor configurations */
	static vision_signature blue_sig = vision_sensor.signature_from_utility(BLUE_SIG_ID, -4645, -3641, -4143,4431, 9695, 7063, 2.5, 0);
	static vision_signature red_sig =  vision_sensor.signature_from_utility(RED_SIG_ID,  7935, 9719, 8827,-1261, -289, -775, 2.5, 0);

	vision_sensor.set_signature(BLUE_SIG_ID, &blue_sig);
	vision_sensor.set_signature(RED_SIG_ID, &red_sig);

	vision_sensor.clear_led();

    auton_default_params();

    ez::as::auton_selector.autons_add({
        {"Red\n\nLeft", auton_red_left},
        {"Red\n\nRight", auton_red_right},
        {"Blue\n\nLeft", auton_blue_left},
        {"Blue\n\nRight", auton_blue_right},
        {"Skills Auton\n\nPlace on the left", auton_skills}
    });

    lcd::initialize();
    chassis.initialize();
    ez::as::initialize();

    controller.rumble(chassis.drive_imu_calibrated() ? "." : "---");

}

void disabled() {}

void competition_initialize() {}

/* The autonomous code is actually at autons.cpp */
void autonomous() {
    chassis.pid_targets_reset();                // Resets PID targets to 0
    chassis.drive_imu_reset();                  // Reset gyro position to 0
    chassis.drive_sensor_reset();               // Reset drive sensors to 0
    chassis.odom_xyt_set(0_in, 0_in, 0_deg);    // Set the current position, you can start at a specific position with this
    chassis.drive_brake_set(MOTOR_BRAKE_HOLD);  // Set motors to hold.  This helps autonomous consistency

    /* This is equally applicable to autonomous. */

    ez::as::auton_selector.selected_auton_call(); 
}

void opcontrol() {

    /* The color to reject was already set in auton... right? */
    color_sort::start();
    
    chassis.drive_brake_set(MOTOR_BRAKE_BRAKE);

	while (true) {
        
        /* move_absolute is non-blocking. This is the intended behavior, and the controls should not be locked. */

        /* Initial position of the donut */
        if (controller.get_digital_new_press(DIGITAL_Y))
            wall_stake.move_absolute(0, 70);

        /* Pick up the donut and hold it */
        if (controller.get_digital_new_press(DIGITAL_X)) 
            wall_stake.move_absolute(40, 70);

        /* Score the donut */
        if (controller.get_digital_new_press(DIGITAL_A))
            wall_stake.move_absolute(100, 70);
        
        /* Toggle the doinker */
        if (controller.get_digital_new_press(DIGITAL_R1))
            doink_piston.toggle();

        /* Toggle the stake grabber */
        if (controller.get_digital_new_press(DIGITAL_R2))
            stake_piston.toggle();
        
        /* Move the lift intake */
        if (controller.get_digital(DIGITAL_L2)) {
            lift_intake.move_velocity(600);
            color_sort::declare_lift_intake_is_running(true);
        }
        else if (controller.get_digital(DIGITAL_L1)) {
            lift_intake.move_velocity(-600);
            color_sort::declare_lift_intake_is_running(true);
        }
        else {
            lift_intake.move_velocity(0);
            color_sort::declare_lift_intake_is_running(false);
        }

        /* Toggle color sorting */
        if (controller.get_digital_new_press(DIGITAL_DOWN))
            color_sort::toggle();

    	chassis.opcontrol_arcade_standard(ez::SPLIT);

        /* This is required for the chassis.opcontrol_arcade_standard function to work. */
		delay(ez::util::DELAY_TIME);
	}
}