#include "main.hpp"
#include "lemlib/api.hpp" // IWYU pragma: keep

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

class Auton {
private:
    // TODO: add the auton configuration parameters here

public:
    Auton() {}
    void set_config(std::unordered_map<std::string, std::string>& config) {
        // TODO: add the code to process the user input here
    }
};

using namespace pros;
using namespace pros::v5;

Controller controller(CONTROLLER_MASTER);

// SelectionMenu menu;
Auton auton;

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

// TODO: tune the PID
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

lemlib::ExpoDriveCurve throttle_curve(3, // joystick deadband out of 127
                                     10, // minimum output where drivetrain will move out of 127
                                     1.019 // expo curve gain
);

lemlib::ExpoDriveCurve steer_curve(3, // joystick deadband out of 127
                                  10, // minimum output where drivetrain will move out of 127
                                  1.019 // expo curve gain
);

lemlib::Chassis chassis(drivetrain, 
                        lateral_controller, 
                        angular_controller, 
                        sensors,
                        &throttle_curve,
                        &steer_curve
);

void initialize() {
    // pros::screen::touch_callback([](){menu.touch_callback();}, TOUCH_PRESSED);
    
    // TODO: add the auton config stuff here
    // menu.add_option(std::string name, pros::Color color, std::vector<std::string> choices)

    // menu.on_enter([](auto config){auton.set_config(config);});
    
    // menu.draw();
    std::puts("\033[2J");

    chassis.calibrate();
    pros::delay(2000);
}

void opcontrol() {
    chassis.setBrakeMode(MOTOR_BRAKE_HOLD);

    while (true) {
        pros::delay(20);

        auto left_y = controller.get_analog(ANALOG_LEFT_Y);
        auto right_x = controller.get_analog(ANALOG_RIGHT_X);

        chassis.curvature(left_y, right_x);
    }
}
