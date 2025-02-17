#include "main.hpp"

const int DRIVE_SPEED = 110;
const int TURN_SPEED = 90;
const int SWING_SPEED = 110;

///
// Constants
///
void auton_default_params() {
    // P, I, D, and Start I
    chassis.pid_drive_constants_set(20.0, 0.0, 100.0);         // Fwd/rev constants, used for odom and non odom motions
    chassis.pid_heading_constants_set(11.0, 0.0, 20.0);        // Holds the robot straight while going forward without odom
    chassis.pid_turn_constants_set(3.0, 0.05, 20.0, 15.0);     // Turn in place constants
    chassis.pid_swing_constants_set(6.0, 0.0, 65.0);           // Swing constants
    chassis.pid_odom_angular_constants_set(6.5, 0.0, 52.5);    // Angular control for odom motions
    chassis.pid_odom_boomerang_constants_set(5.8, 0.0, 32.5);  // Angular control for boomerang motions

    // Exit conditions
    chassis.pid_turn_exit_condition_set(90_ms, 3_deg, 250_ms, 7_deg, 500_ms, 500_ms);
    chassis.pid_swing_exit_condition_set(90_ms, 3_deg, 250_ms, 7_deg, 500_ms, 500_ms);
    chassis.pid_drive_exit_condition_set(90_ms, 1_in, 250_ms, 3_in, 500_ms, 500_ms);
    chassis.pid_odom_turn_exit_condition_set(90_ms, 3_deg, 250_ms, 7_deg, 500_ms, 750_ms);
    chassis.pid_odom_drive_exit_condition_set(90_ms, 1_in, 250_ms, 3_in, 500_ms, 750_ms);
    chassis.pid_turn_chain_constant_set(3_deg);
    chassis.pid_swing_chain_constant_set(5_deg);
    chassis.pid_drive_chain_constant_set(3_in);

    // Slew constants
    chassis.slew_turn_constants_set(3_deg, 70);
    chassis.slew_drive_constants_set(3_in, 70);
    chassis.slew_swing_constants_set(3_in, 80);

    // The amount that turns are prioritized over driving in odom motions
    // - if you have tracking wheels, you can run this higher.  1.0 is the max
    chassis.odom_turn_bias_set(0.9);

    chassis.odom_look_ahead_set(7_in);           // This is how far ahead in the path the robot looks at
    chassis.odom_boomerang_distance_set(16_in);  // This sets the maximum distance away from target that the carrot point can be
    chassis.odom_boomerang_dlead_set(0.625);     // This handles how aggressive the end of boomerang motions are

    chassis.pid_angle_behavior_set(ez::shortest);  // Changes the default behavior for turning, this defaults it to the shortest path there
}

#define TEAM_RED 1
#define TEAM_BLUE 2

#define LEFT_SIDE 3
#define RIGHT_SIDE 4

/*
    Odometry and Pure Pursuit are not magic

    It is possible to get perfectly consistent results without tracking wheels,
    but it is also possible to have extremely inconsistent results without tracking wheels.
    When you don't use tracking wheels, you need to:
    - avoid wheel slip
    - avoid wheelies
    - avoid throwing momentum around (super harsh turns, like in the example below)
    You can do cool curved motions, but you have to give your robot the best chance
    to be consistent
*/

static void _auton_match_generic(int color, int side) {
    static int enemy_sig_id;
    if (color == TEAM_RED) 
        enemy_sig_id = BLUE_SIG_ID;
    else
        enemy_sig_id = RED_SIG_ID;
    
    lift_intake.move_velocity(600);

    color_sort::set_reject_color(color);
    color_sort::declare_lift_intake_is_running(true);
    color_sort::start();
    
    /* TODO: write actual code here. */
    chassis.pid_drive_set(24_in, DRIVE_SPEED, true);
    chassis.pid_wait();

    chassis.pid_turn_set(45_deg, TURN_SPEED);
    chassis.pid_wait();

    chassis.pid_turn_set(-45_deg, TURN_SPEED);
    chassis.pid_wait();

    chassis.pid_turn_set(0_deg, TURN_SPEED);
    chassis.pid_wait();

    chassis.pid_drive_set(-24_in, DRIVE_SPEED, true);
    chassis.pid_wait();
}

void auton_skills() {
    lift_intake.move_velocity(600);

    /* Use no color sorting in skills. */
}

void auton_blue_left() {
    _auton_match_generic(TEAM_BLUE, LEFT_SIDE);
}

void auton_blue_right() {
    _auton_match_generic(TEAM_BLUE, RIGHT_SIDE);
}

void auton_red_left() {
    _auton_match_generic(TEAM_RED, LEFT_SIDE);
}

void auton_red_right() {
    _auton_match_generic(TEAM_RED, RIGHT_SIDE);
}