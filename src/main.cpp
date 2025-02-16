#include "main.hpp"

const int BLUE_SIG_ID = 1;
const int RED_SIG_ID = 2;

int MY_SIG_ID = 1;
int ENEMY_SIG_ID = 2;

#define LEFT 0
#define RIGHT 1

// Definition is later in file.
void elevator_loop();

void initialize() {
	pros::lcd::initialize();

	static vision_signature blue_sig = vision_sensor.signature_from_utility(BLUE_SIG_ID, -4645, -3641, -4143,4431, 9695, 7063, 2.5, 0);
	static vision_signature red_sig =  vision_sensor.signature_from_utility(RED_SIG_ID,  7935, 9719, 8827,-1261, -289, -775, 2.5, 0);

    wall_stake.tare_position();

	vision_sensor.set_signature(BLUE_SIG_ID, &blue_sig);
	vision_sensor.set_signature(RED_SIG_ID, &red_sig);

	vision_sensor.clear_led();

    ez::as::auton_selector.autons_add({
        {"Red\n\nLeft", auton_red_left},
        {"Red\n\nRight", auton_red_right},
        {"Blue\n\nLeft", auton_blue_left},
        {"Blue\n\nRight", auton_blue_right},
        {"Skills Auton\n\nPlace on the left", auton_skills}
    });

	lift_intake.set_brake_mode(MOTOR_BRAKE_BRAKE);
	wall_stake.set_brake_mode(MOTOR_BRAKE_HOLD);
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
    pros::Task loop_task (elevator_loop);

    ez::as::auton_selector.selected_auton_call(); 
}

bool lift_intake_running;

void opcontrol() {
    pros::Task loop_task (elevator_loop);

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
        

        lift_intake_running = false;

        if (controller.get_digital(E_CONTROLLER_DIGITAL_L2)) {
            lift_intake.move_velocity(600);
            lift_intake_running = true;
        }
        else if (controller.get_digital(E_CONTROLLER_DIGITAL_L1)) {
            lift_intake.move_velocity(-600);
            lift_intake_running = true;
        }

    	chassis.opcontrol_arcade_standard(ez::SPLIT);

        /* This is required for the chassis.opcontrol_arcade_standard function to work. */
		delay(ez::util::DELAY_TIME);
	}
}


void elevator_loop() {
    while (true) {
        pros::delay(20);

        /* Exit: the lift intake isn't running */
        if (!lift_intake_running) 
            continue;

        vision_object enemy_donut = vision_sensor.get_by_sig(0, ENEMY_SIG_ID);

        /* Exit: the donut is too far away (so it appears small) */
        if (enemy_donut.height < 30 || enemy_donut.width < 70)
            continue;

        /* Hold on, I found something. Let's wait until the switch is hit. */
        int timer = 0;
        bool akita_neru = false;

        while (!donut_presence_sensor.get_new_press()) {
            delay(10);
            timer += 10;

            /* Break the loop when for some reason the switch doesn't get a donut, or the lift intake is stopped. */
            /* This may be because the switch literally missed the donut. */
            akita_neru = (timer > 2000 || !lift_intake_running);
            if (akita_neru)
                break;
        }

        /* Exit: the donut did not make it to the top. */
        if (akita_neru)
            continue;


        /* By now, we know that: the donut elevator is running, the donut has reached the top,
           and the donut is the enemy's color. This is enough reason to complete the routine. */

        int save_direction = lift_intake.get_direction();

        delay(100);
        lift_intake.brake();
        delay(250);

        lift_intake.move_velocity(600 * (save_direction == FORWARD ? 1 : -1));
    }
}