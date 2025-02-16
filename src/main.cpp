#include "main.hpp"

const int BLUE_SIG_ID = 1;
const int RED_SIG_ID = 2;

int MY_SIG_ID = 1;
int ENEMY_SIG_ID = 2;

#define LEFT 0
#define RIGHT 1

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

    left_motors.set_brake_mode(MOTOR_BRAKE_BRAKE);
	right_motors.set_brake_mode(MOTOR_BRAKE_BRAKE);
	lift_intake.set_brake_mode(MOTOR_BRAKE_BRAKE);
	wall_stake.set_brake_mode(MOTOR_BRAKE_HOLD);
    
}

/* Convert percent velocity to RPM (out of 600). */
/* This is always an integer, because velocity is passed as an integer anyway. */
constexpr unsigned long long operator"" _percent(unsigned long long vel) {
    return vel * 6;
}

void disabled() {}

void competition_initialize() {}

void autonomous() {}

void opcontrol() {

	while (true) {
        
        /* move_absolute is non-blocking. This is the intended behavior, and the controls should not be locked. */

        /* Initial position of the donut */
        if (controller.get_digital_new_press(DIGITAL_Y)){
            wall_stake.move_absolute(0, 70_percent);
        }

        /* Pick up the donut and hold it*/
        if (controller.get_digital_new_press(DIGITAL_X)) {
            wall_stake.move_absolute(40, 70);
        }

        /* Score the donut */
        if (controller.get_digital_new_press(DIGITAL_A)) {
            wall_stake.move_absolute(100, 70);
        }

		int forwardpower = controller.get_analog(ANALOG_LEFT_Y);
    	int turnpower = controller.get_analog(ANALOG_RIGHT_X);

    	chassis.opcontrol_arcade_standard(ez::SPLIT);

		pros::delay(ez::util::DELAY_TIME);
	}

	// keybinds for toggling pneumatics
	if (controller.get_digital_new_press(DIGITAL_R2))
		stake_piston.toggle();
	
	if (controller.get_digital_new_press(DIGITAL_R1))
		doink_piston.toggle();
}


void elevator_loop() {
    static bool lift_intake_running = false;

    if (controller.get_digital(E_CONTROLLER_DIGITAL_L2)) {
            lift_intake.move_velocity(600);
            lift_intake_running = true;
    }
    else if (controller.get_digital(E_CONTROLLER_DIGITAL_L1)) {
            lift_intake.move_velocity(-600);
            lift_intake_running = true;
    }

    else {
        lift_intake_running = false;
    }	

    while (lift_intake_running) {
        vision_object enemy_donut = vision_sensor.get_by_sig(0,1);

        if (enemy_donut.height >= 30 && enemy_donut.width >= 70) {
            while (!donut_presence_sensor.get_new_press()) {
                int save_direction = lift_intake.get_direction();
                delay(20);
                if (donut_presence_sensor.get_new_press()) {
                    delay(100);
                    lift_intake.brake();
                    delay(250);

                    lift_intake.move_velocity(600 * (save_direction == FORWARD ? 1 : -1));
                }
            }
        }
    }
}