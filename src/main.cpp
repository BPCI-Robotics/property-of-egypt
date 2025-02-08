#include "main.h"
#include "vision.hpp"


using namespace pros;

Controller controller (pros::E_CONTROLLER_MASTER);

Motor lift_intake (-7, v5::MotorGears::blue, v5::MotorUnits::counts);
Motor wall_stake (8, v5::MotorGears::red, v5::MotorUnits::degrees); 

MotorGroup left_motors ({1, 2, -3}, v5::MotorGears::blue, v5::MotorUnits::counts);
MotorGroup right_motors ({-4,-5, 6}, v5::MotorGears::blue, v5::MotorUnits::counts);

adi::Pneumatics stake_piston ({1, 'a'}, false, false);
adi::Pneumatics doink_piston ({2, 'b'}, false, false);

adi::Button donut_switch (4);

Vision vision_sensor (1, E_VISION_ZERO_CENTER);

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */


void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");

	pros::lcd::register_btn1_cb(on_center_button);

	vision_signature_s_t BLUE_SIG = vision_sensor.signature_from_utility(1, -4645, -3641, -4143,4431, 9695, 7063, 2.5, 0);
	vision_signature_s_t RED_SIG =  vision_sensor.signature_from_utility(2,  7935, 9719, 8827,-1261, -289, -775, 2.5, 0);

	vision_sensor.set_signature(1, &BLUE_SIG);
	vision_sensor.set_signature(2, &RED_SIG);

	vision_sensor.clear_led();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	const int speed = 600;
	left_motors.set_brake_mode(MOTOR_BRAKE_BRAKE);
	right_motors.set_brake_mode(MOTOR_BRAKE_BRAKE);
	lift_intake.set_brake_mode(MOTOR_BRAKE_BRAKE);
	wall_stake.set_brake_mode(MOTOR_BRAKE_HOLD);

	while (true) {
		int forwardpower = controller.get_analog(E_CONTROLLER_ANALOG_LEFT_Y);
    	int turnpower = controller.get_analog(E_CONTROLLER_ANALOG_RIGHT_X);

    	int left = forwardpower + turnpower;
    	int right = forwardpower - turnpower; 

    	left = left * speed / 100;
    	right = right * speed / 100;

		left_motors.move_velocity(left);
		right_motors.move_velocity(right);

		delay(20);
	}

	//keybinds for toggling pneumatics
	if (controller.get_digital(E_CONTROLLER_DIGITAL_R2)) {
		stake_piston.toggle();
	}
	if (controller.get_digital(E_CONTROLLER_DIGITAL_R1)) {
		doink_piston.toggle();
	}

	//elevator routine
	void elevator_loop() {
		bool lift_intake_running = false;
		//keybinds for lift intake
				if (controller.get_digital(E_CONTROLLER_DIGITAL_L2)) {
						lift_intake.move_velocity(speed);
						lift_intake_running = true;
				}
				else if (controller.get_digital(E_CONTROLLER_DIGITAL_L1)) {
						lift_intake.move_velocity(-1*speed);
						lift_intake_running = true;
				}

				else {
					lift_intake_running = false;
				}	

		while (lift_intake_is_running) {
			enemy_donut = vision_sensor.get_by_sig(0,1);
			if (enemy_donut.height >= 30 && enemy_donut.width >= 70) {
				while (!donut_switch.get_new_press()) {
					int save_direction = lift_intake.get_direction()
					delay(20);
					if (donut_switch.get_new_press()) {
						delay(100);
						lift_intake.brake();
						delay(250);
						lift_intake.move_velocity(speed * save_direction);
					}
				}
			}

		  }

		}

	void wall_stake_mech() {
		//establish 0-position
		wall_stake.tare_position();

		// keybinds for wall stake mech
		// note that move_absolute() does not wait for the function to finish, so it would be possible to change positions midway through execution
		if (controller.get_digital(E_CONTROLLER_DIGITAL_Y)){
			wall_stake.move_absolute(0, 70); //if pressed, this will bring the donut back to its initial position
		}
		if (controller.get_digital(E_CONTROLLER_DIGITAL_X)) {
			wall_stake.move_absolute(40, 70); //if pressed, this will pick up the donut
		}
		if (controller.get_digital(E_CONTROLLER_DIGITAL_A)) {
			wall_stake.move_absolute(100, 70); //if pressed, this will score the donut
		}

	}

}
