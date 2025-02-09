#include "main.h"

using namespace pros;

Controller controller (pros::E_CONTROLLER_MASTER);

MotorGroup left_motors ({1, 2, -3}, v5::MotorGears::blue, v5::MotorUnits::counts);
MotorGroup right_motors ({-4,-5, 6}, v5::MotorGears::blue, v5::MotorUnits::counts);

Motor lift_intake (-7, v5::MotorGears::blue, v5::MotorUnits::counts);
Motor wall_stake (8, v5::MotorGears::red, v5::MotorUnits::degrees); 

Vision vision_sensor (9, E_VISION_ZERO_CENTER);

adi::Pneumatics stake_piston ({1, 'a'}, false, false);
adi::Pneumatics doink_piston ({2, 'b'}, false, false);

adi::Button donut_switch (4);

const int BLUE_SIG_ID = 1;
const int RED_SIG_ID = 2;

int MY_SIG_ID = 1;
int ENEMY_SIG_ID = 2;

#define LEFT 0
#define RIGHT 1

void initialize() {
	pros::lcd::initialize();

	static vision_signature_s_t blue_sig = vision_sensor.signature_from_utility(BLUE_SIG_ID, -4645, -3641, -4143,4431, 9695, 7063, 2.5, 0);
	static vision_signature_s_t red_sig =  vision_sensor.signature_from_utility(RED_SIG_ID,  7935, 9719, 8827,-1261, -289, -775, 2.5, 0);

	vision_sensor.set_signature(BLUE_SIG_ID, &blue_sig);
	vision_sensor.set_signature(RED_SIG_ID, &red_sig);

	vision_sensor.clear_led();
}

void disabled() {}

void competition_initialize() {}

void autonomous() {}

void opcontrol() {
	left_motors.set_brake_mode(MOTOR_BRAKE_BRAKE);
	right_motors.set_brake_mode(MOTOR_BRAKE_BRAKE);
	lift_intake.set_brake_mode(MOTOR_BRAKE_BRAKE);
	wall_stake.set_brake_mode(MOTOR_BRAKE_HOLD);

	while (true) {
		int forwardpower = controller.get_analog(E_CONTROLLER_ANALOG_LEFT_Y);
    	int turnpower = controller.get_analog(E_CONTROLLER_ANALOG_RIGHT_X);

    	int left = forwardpower + turnpower;
    	int right = forwardpower - turnpower; 

		left_motors.move_velocity(left * 6);
		right_motors.move_velocity(right * 6);

		delay(20);
	}

	// keybinds for toggling pneumatics
	if (controller.get_digital_new_press(E_CONTROLLER_DIGITAL_R2))
		stake_piston.toggle();
	
	if (controller.get_digital_new_press(E_CONTROLLER_DIGITAL_R1))
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
            while (!donut_switch.get_new_press()) {
                int save_direction = lift_intake.get_direction();
                delay(20);
                if (donut_switch.get_new_press()) {
                    delay(100);
                    lift_intake.brake();
                    delay(250);

                    lift_intake.move_velocity(600 * (save_direction == FORWARD ? 1 : -1));
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