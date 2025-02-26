#include "main.hpp"

static void auton_skills() {
    
}

static void auton_quals() {

}

static void auton_elims() {

}

static auton_direction config_auton_direction = auton_direction::LEFT;

static void (*auton_routine)();

namespace auton {
    void set_config(std::unordered_map<std::string, std::string> config) {
        if (config["Team color"] == "Red")
            lift_intake::init(REJECT_BLUE);
        else
            lift_intake::init(REJECT_RED);

        if (config["Auton direction"] == "Left")
            config_auton_direction = auton_direction::LEFT;
        else
            config_auton_direction = auton_direction::RIGHT;

        if (config["Auton type"] == "Skills")
            auton_routine = auton_skills;

        if (config["Auton type"] == "Quals")
            auton_routine = auton_quals;

        if (config["Auton type"] == "Elims")
            auton_routine = auton_elims;

        if (config["Testing"] == "Driver Control")
            opcontrol();
        else if (config["Testing"] == "Auton")
            autonomous();
        else
            /* There is nothing to do. */;

    }

    void start() {
        auton_routine();
    }
}
