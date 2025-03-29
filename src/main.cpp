#include "main.hpp"
#include "lemlib/api.hpp" // IWYU pragma: keep

enum class MotorDirection {
    REVERSE,
    FORWARD
};

enum class AutonDirection {
    LEFT,
    RIGHT
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
};

class LiftIntake {
private:
    pros::Motor motor;
    pros::Optical optical;
    pros::Color enemy_color;

public:
    LiftIntake(const pros::Motor& motor, const pros::Optical& optical)
        : motor(motor), optical(optical) { 
        this->motor.set_brake_mode(MOTOR_BRAKE_BRAKE);
    }

    void start_sorting() const {
        pros::Task {[]() {
            while (true) {
                pros::delay(20);

                /* TODO: Write that */
            }
        }, "Color sorting task"};
    }

    void start_sorting(pros::Color enemy_color) {
        this->enemy_color = enemy_color;
        start_sorting();
    }

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

    void stop() const {
        motor.brake();
    }
};

using namespace pros;
using namespace pros::v5;

WallStake wall_stake (Motor (-8, MotorGears::red, MotorEncoderUnits::degrees), 
                      Rotation (11));

Controller controller(CONTROLLER_MASTER);

SelectionMenu menu {};

adi::Pneumatics stake_grab ('a', false);
adi::Pneumatics doink_piston ('b', false);

LiftIntake lift_intake (Motor(7, MotorGears::blue, MotorEncoderUnits::degrees), Optical(9));

MotorGroup left_motors ({1, -2, 3}, v5::MotorGears::blue, v5::MotorEncoderUnits::degrees);
MotorGroup right_motors ({-4, 5, -6}, v5::MotorGears::blue, v5::MotorEncoderUnits::degrees);

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

v5::Imu imu(10);
lemlib::OdomSensors sensors(nullptr, nullptr, nullptr, nullptr, &imu);

lemlib::Chassis chassis(drivetrain, 
                        lateral_controller, 
                        angular_controller, 
                        sensors);

void menu_touch_callback() {
    menu.touch_callback();
}

void initialize() {
    pros::screen::touch_callback(menu_touch_callback, TOUCH_PRESSED);
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

    def set_config(self, config: dict[str, Any]):

        if config["Team color"] == "Red":
            self.color = RED_SIG
        else:
            self.color = BLUE_SIG
        
        if config["Auton direction"] == "Left":
            self.direction = LEFT
        else:
            self.direction = RIGHT

        if config["Ring/Goal rush"] == "Ring":
            self.mode = "Ring"
        
        else:
            self.mode = "Goal"
        
        if config["Auton type"] == "Skills":
            self._routine_selected = self._skills

        elif config['Auton type'] == "Quals":
            self._routine_selected = self._quals

        elif config['Auton type'] == "Elims":
            self._routine_selected = self._elims

    def __call__(self):
        wall_stake.start_log()
        self._routine_selected()

#region Parts
BLUE_SIG = Signature(1, -4645, -3641, -4143,4431, 9695, 7063, 2.5, 0)
RED_SIG = Signature(2, 7935, 9719, 8827,-1261, -289, -775, 2.5, 0)

brain = Brain()
controller = Controller()

stake_grabber = DigitalOutToggleable(brain.three_wire_port.a)
doink_piston = DigitalOutToggleable(brain.three_wire_port.b)

drivetrain= SmartDrive(
                MotorGroup(
                    Motor(Ports.PORT1, GearSetting.RATIO_6_1, False), 
                    Motor(Ports.PORT2, GearSetting.RATIO_6_1, True),
                    Motor(Ports.PORT3, GearSetting.RATIO_6_1, False),
                ),

                MotorGroup(
                    Motor(Ports.PORT4, GearSetting.RATIO_6_1, True), 
                    Motor(Ports.PORT5, GearSetting.RATIO_6_1, False), 
                    Motor(Ports.PORT6, GearSetting.RATIO_6_1, True),
                ),

                Inertial(Ports.PORT10), 

                259.34, # wheel travel
                310,    # track width
                205,    # wheel base
                MM,     # unit
                600/360
            )

#endregion Parts

def initialize():
    menu = SelectionMenu()

    menu.add_option("Team color", Color.RED, ["Red", "Blue"])
    menu.add_option("Auton direction", Color.BLUE, ["Left", "Right"])
    menu.add_option("Auton type", Color.PURPLE, ["Quals", "Elims", "Skills"])
    menu.add_option("Ring/Goal rush", Color.CYAN, ["Ring", "Goal"])

    menu.on_enter(auton.set_config)
    
    menu.draw()
    print("\033[2J")
    controller.buttonLeft.pressed(menu.force_submit)

def driver():
    wall_stake.start_log()

    drivetrain.set_drive_velocity(0, PERCENT)
    drivetrain.set_turn_velocity(0, PERCENT)
    
    lift_intake.stop()
    drivetrain.drive(FORWARD)

    drivetrain.set_stopping(COAST)

    controller.buttonL2.pressed(lift_intake.spin, (FORWARD,))
    controller.buttonL2.released(lift_intake.stop)
    controller.buttonL1.pressed(lift_intake.spin, (REVERSE,))
    controller.buttonL1.released(lift_intake.stop)

    controller.buttonX.pressed(wall_stake.pickup)
    controller.buttonB.pressed(wall_stake.reset)

    controller.buttonY.pressed(wall_stake.spin, (REVERSE,))
    controller.buttonY.released(wall_stake.stop)

    controller.buttonA.pressed(wall_stake.spin, (FORWARD,))
    controller.buttonA.released(wall_stake.stop)

    controller.buttonR2.pressed(stake_grabber.toggle)
    controller.buttonR1.pressed(doink_piston.toggle)
    
    while True:
        accel_stick = controller.axis3.position()
        turn_stick = controller.axis1.position()

        drivetrain.lm.set_velocity(accel_stick - turn_stick, PERCENT)
        drivetrain.rm.set_velocity(accel_stick + turn_stick, PERCENT)

        wait(1 / 60, SECONDS)

auton = Auton()

Competition(driver, auton)
initialize()

    def print_pos(self):
        while True:
            wait(200, MSEC)
            brain.screen.clear_screen()
            brain.screen.set_cursor(1, 1)
            brain.screen.print("Intake temp:", lift_intake.motor.temperature())
            brain.screen.set_cursor(2, 1)
            brain.screen.print("Wall stake temp:", wall_stake.motor.temperature())
            brain.screen.set_cursor(3, 1)
            brain.screen.print("Drivetrain temp:", drivetrain.temperature())

    def start_log(self):
        Thread(self.print_pos)
*/