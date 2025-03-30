#include "main.hpp"
#include "lemlib/api.hpp" // IWYU pragma: keep

enum class MotorDirection {
    REVERSE,
    FORWARD
};

enum class AutonDirection {
    LEFT,
    RIGHT,
    UNSET
};

enum class TeamColor {
    RED,
    BLUE,
    UNSET
};

enum class AutonMode {
    QUALS,
    ELIMS,
    SKILLS,
    UNSET
};

enum class RushType {
    RING,
    GOAL,
    UNSET
};

class SelectionMenu {
private:
    class Option {
    private:
        std::vector<std::string> choices;
        unsigned index = 0;
        unsigned count = 0;
    
    public:
        std::string name;
        pros::Color color;
    
        Option (std::string name, pros::Color color, std::vector<std::string> choices)
            : choices(choices), name(name), color(color) {
            index = 0;
            count = choices.size();
        }
    
        const std::string value() const {
            return choices[index];
        }
    
        void next() {
            index = (index + 1) % count;
        }
    
        void prev() {
            if (index == 0)
                index = count - 1;
            else
                index--;
        }
    };

    unsigned count = 0;
    std::vector<Option> options;
    bool disabled = false;
    void (*enter_callback)(std::unordered_map<std::string, std::string>) = nullptr;
        
    std::unordered_map<std::string, std::string> get_all() {
        if (disabled)
            return {};
        
        std::unordered_map<std::string, std::string> d = {};

        for (const auto& option : options)
            d[option.name] = option.value();
        
        return d;
    }

public:
    void touch_callback() {
        if (disabled)
            return;

        auto status = pros::c::screen_touch_status();

        auto x = static_cast<int>(status.x);
        auto y = static_cast<int>(status.y);

        if (y < 140)
            return;

        // Integer division
        options[x * count / 480].next();
            
        draw();

        if (options[count - 1].value() == "ENTERED") {
            enter_callback(get_all());
            disabled = true;
            return;
        }
    }

    void on_enter(void (*callback)(std::unordered_map<std::string, std::string>)) {
        enter_callback = callback;
    }

    void force_submit() {
        enter_callback(get_all());
        disabled = true;
        return;
    }

    void add_option(std::string name, pros::Color color, std::vector<std::string> choices) {
        if (disabled)
            return;
        
        options.insert(options.end() - 1, Option(name, color, choices));
        count += 1;
    }

    void draw() const {
        if (disabled)
            return;

        pros::screen::set_eraser(pros::Color::black);
        pros::screen::erase();

        /* Insha Allah this will not cause a segmentation fault */
        for (unsigned i = 0; i < count; i++) {
            pros::screen::set_pen(options[i].color);
            pros::screen::print(pros::E_TEXT_MEDIUM, static_cast<int16_t>(i + 1), "%s: %s", options[i].name, options[i].value());
        }
        
        constexpr int canvas_width = 480;
        constexpr int canvas_height = 240;

        int rect_width = (canvas_width - 10 * (count + 1)) / count;
        constexpr int rect_height = 70;

        for (unsigned i = 0; i < count; i++) {
            pros::screen::set_pen(options[i].color);
            
            int16_t x0 = 10 + (10 + rect_width) * i;
            int16_t y0 = canvas_height - (rect_height * 5);

            pros::screen::draw_rect(
                x0,
                y0,
                x0 + rect_width,
                y0 + rect_height
            );
        }
    }

    SelectionMenu() {
        add_option("Enter", pros::Color::white, std::vector<std::string> {"", "Are you sure?", "ENTERED"});
    }
};

class WallStake {
private:
    pros::Motor motor;
    pros::Rotation rotation;
    
    void init() const {
        spin(MotorDirection::REVERSE);
        pros::delay(800);
        rotation.reset_position();
        stop();
    }

public:
    WallStake(const pros::Motor& motor, const pros::Rotation& rotation) 
        : motor(motor), rotation(rotation) {
        
        motor.set_brake_mode(MOTOR_BRAKE_HOLD);
        init();
    }

    /*
    You can call this function two ways:
    `wall_stake.spin(MotorDirection::FORWARD);`
    `wall_stake.spin(MotorDirection::REVERSE);`
    */
    void spin(MotorDirection direction) const {
        switch (direction) {
        case MotorDirection::REVERSE:
            motor.move(-127 * 6 / 10);
            break;

        case MotorDirection::FORWARD:
            motor.move(127 * 6 / 10);
            break;
        }
    }

    /*
    Only one call:
    `wall_stake.stop();`
    */
    void stop() const {
        motor.move(0);
    }

    void spin_to(int degrees) const {
        int time_spent = 0;

        int position = rotation.get_position() / 100;

        while (abs(degrees - position) > 4 || time_spent > 1000) {
            position = rotation.get_position() / 100;

            if (degrees > position)
                spin(MotorDirection::FORWARD);

            if (degrees < position)
                spin(MotorDirection::REVERSE);

            pros::delay(20);
            time_spent += 20;
        }
        stop();
    }

    void pickup() const { spin_to(36); }
    void score() const { spin_to(192); }
    void reset() const { spin_to(0); }
    auto temperature() const { return motor.get_temperature(); }
};

class LiftIntake {
private:
    pros::Motor motor;
    pros::Optical optical {7};
    TeamColor enemy_color = TeamColor::UNSET;

    void reject() const {
        pros::delay(100);
        spin(MotorDirection::REVERSE);
        std::cout << "Gooned!";
        pros::delay(750);
        std::cout << "Finished gooning!";
        spin(MotorDirection::FORWARD);
    }

public:
    LiftIntake(const pros::Motor& motor, const pros::Optical& optical)
        : motor(motor), optical(optical) { 
        this->motor.set_brake_mode(MOTOR_BRAKE_BRAKE);
    }
    /*
    lift_intake.spin(MotorDirection::REVERSE);
    lift_intake.spin(MotorDirection::FORWARD);
    */
    void spin(MotorDirection direction) const {
        switch (direction) {
        case MotorDirection::REVERSE:
            motor.move(-127);
            break;

        case MotorDirection::FORWARD:
            motor.move(127);
            break;
        }
    }

    /* lift_intake.stop() */
    void stop() const {
        motor.brake();
    }

    /* When a color isn't given, it simply sorts with the last color given. */
    void start_sorting() {
        pros::Task {[this]() {
            while (true) {
                pros::delay(20);

                /* https://kb.vex.com/hc/article_attachments/360074778111 */
                double hue = optical.get_hue();

                switch (enemy_color) {
                case TeamColor::RED:
                    if ((330.0 < hue) || (hue < 30.0)) reject();
                    break;
                
                case TeamColor::BLUE:
                    if ((210.0 < hue) && (hue < 270.0)) reject();
                    break;
                
                case TeamColor::UNSET:
                    std::cout << "LiftIntake: You forgot to set a team color for color sort." << std::endl;
                    return;
                }
            }
        }, "LiftIntake: Color sorting task"};
    }

    /* 
    When a color is given, it starts sorting with the given color. 
    lift_intake.start_sorting(TeamColor::RED);
    lift_intake.start_sorting(TeamColor::BLUE);
    lift_intake.start_sorting();
    */
    void start_sorting(TeamColor enemy_color) {
        this->enemy_color = enemy_color;
        start_sorting();
    }

    auto temperature() const { return motor.get_temperature(); }
};

class Auton {
private:
    AutonDirection direction = AutonDirection::LEFT;
    TeamColor my_color = TeamColor::UNSET;
    TeamColor enemy_color = TeamColor::UNSET;
    AutonMode mode = AutonMode::UNSET;
    RushType rush = RushType::UNSET;

public:
    Auton() {}

    void set_config(std::unordered_map<std::string, std::string>& config) {

        if (config["Team color"] == "Red") {
            my_color = TeamColor::RED;
            enemy_color = TeamColor::BLUE;
        } else {
            my_color = TeamColor::BLUE;
            enemy_color = TeamColor::RED;
        }

        if (config["Auton direction"] == "Left")
            direction = AutonDirection::LEFT;
        else
            direction = AutonDirection::RIGHT;

        if (config["Ring/Goal rush"] == "Ring")
            rush = RushType::RING;
        else
            rush = RushType::GOAL;

        if (config["Auton type"] == "Skills")
            mode = AutonMode::SKILLS;
        else if (config["Auton type"] == "Quals")
            mode = AutonMode::QUALS;
        else
            mode = AutonMode::ELIMS;
    }
};

using namespace pros;
using namespace pros::v5;

WallStake wall_stake (Motor (-8, MotorGears::red, MotorEncoderUnits::degrees), 
                      Rotation (11));

Controller controller(CONTROLLER_MASTER);

SelectionMenu menu {};

adi::Pneumatics stake_grabber ('a', false);
adi::Pneumatics doink_piston ('b', false);

LiftIntake lift_intake (Motor(7, MotorGears::blue, MotorEncoderUnits::degrees), Optical(9));

MotorGroup left_motors ({1, -2, 3}, v5::MotorGears::blue, v5::MotorEncoderUnits::degrees);
MotorGroup right_motors ({-4, 5, -6}, v5::MotorGears::blue, v5::MotorEncoderUnits::degrees);

// drivetrain settings
lemlib::Drivetrain drivetrain(&left_motors,
                              &right_motors,
                              10, // TODO: set track width (inches)
                              lemlib::Omniwheel::NEW_325,
                              360, // TODO: Verify RPM
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

v5::Imu imu(10);
lemlib::OdomSensors sensors(nullptr, nullptr, nullptr, nullptr, &imu);

lemlib::Chassis chassis(drivetrain, 
                        lateral_controller, 
                        angular_controller, 
                        sensors);

void initialize() {
    pros::screen::touch_callback([](){menu.touch_callback();}, TOUCH_PRESSED);
    menu.add_option("Team color", pros::Color::red, {"Red", "Blue"});
    menu.add_option("Auton direction", pros::Color::blue, {"Left", "Right"});
    menu.add_option("Auton type", pros::Color::purple, {"Quals", "Elims", "Skills"});
    menu.add_option("Ring/Goal rush", pros::Color::cyan, {"Ring", "Goal"});

    // menu.on_enter(auton.set_config);
    
    menu.draw();
    std::puts("\033[2J");


    chassis.calibrate();
    // menu.force_submit();
}

void opcontrol() {
    chassis.setBrakeMode(MOTOR_BRAKE_HOLD);

    pros::Task {[](){
        while (true) {
            pros::delay(1000);

            pros::screen::set_eraser(pros::Color::black);
            pros::screen::erase();

            pros::screen::print(TEXT_MEDIUM, 1, "Intake temp: %f", lift_intake.temperature());
            pros::screen::print(TEXT_MEDIUM, 2, "Wall stake temp: %f", wall_stake.temperature());
            pros::screen::print(TEXT_MEDIUM, 3, "Drivetrain temp: %f", 
                (drivetrain.leftMotors->get_temperature(0) + drivetrain.rightMotors->get_temperature(0)) / 2.0);
        }
    }, "opcontrol: Log temps"};

    while (true) {
        pros::delay(20);

        if (controller.get_digital(DIGITAL_L2))
            lift_intake.spin(MotorDirection::FORWARD);

        else if (controller.get_digital(DIGITAL_L1))
            lift_intake.spin(MotorDirection::REVERSE);

        else
            lift_intake.stop();


        if (controller.get_digital_new_press(DIGITAL_X))
            wall_stake.pickup();

        if (controller.get_digital_new_press(DIGITAL_B))
            wall_stake.reset();

        
        if (controller.get_digital(DIGITAL_Y))
            wall_stake.spin(MotorDirection::REVERSE);
        
        else if (controller.get_digital(DIGITAL_A))
            wall_stake.spin(MotorDirection::FORWARD);

        else
            wall_stake.stop();

        
        if (controller.get_digital_new_press(DIGITAL_R2))
            stake_grabber.toggle();
        
        if (controller.get_digital_new_press(DIGITAL_R1))
            doink_piston.toggle();

        auto left_y = controller.get_analog(ANALOG_LEFT_Y);
        auto right_x = controller.get_analog(ANALOG_RIGHT_X);

        chassis.curvature(left_y, right_x);
    }
}

/*

class Auton:
    def __init__(self):
        self.direction = LEFT
        self._routine_selected = self._noop
        self.color = RED_SIG
        self.mode = "Ring"

    def _noop(self):
        pass

    def _quals(self):
        if self.color == RED_SIG:
        elif self.color == BLUE_SIG:

    def _elims(self):
        drivetrain.set_timeout(5, SECONDS)
        if self.color == RED_SIG:
            if self.mode == "Ring":
            elif self.mode == "Goal":

        elif self.color == BLUE_SIG:
            if self.mode == "Ring":

            elif self.mode == "Goal":
            
    def _skills(self):
*/