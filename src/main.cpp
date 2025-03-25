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
    
    void on_brain_screen_press() {
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

public:
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
        constexpr int canvas_height = 480;

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
        pros::screen::touch_callback([](){return;}, E_TOUCH_PRESSED);

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
    pros::Vision vision;

public:
    LiftIntake(const pros::Motor& motor, const pros::Vision& vision)
        : motor(motor), vision(vision) { }
}

/*
class LiftIntake:
    def __init__(self, motor: Motor, vision: Vision):
        self.motor = motor
        self.vision = vision
        self.enemy_sig = None

        self.motor.set_stopping(BRAKE)
    
    def set_enemy_sig(self, enemy_sig):
        self.enemy_sig = enemy_sig

    def spin(self, direction: DirectionType.DirectionType):
        self.motor.spin(direction, 100, PERCENT)

    def stop(self):
        self.motor.stop()

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
            #run ring rush with alliance stake scoring
            drivetrain.drive_for(FORWARD, 50, INCHES, 60, PERCENT, wait=True)
            drivetrain.drive_for(FORWARD, 2, INCHES, 60, PERCENT, True)
            #drivetrain.drive_for

            wait(1, SECONDS)

            drivetrain.drive_for(FORWARD, 5, INCHES, 90, PERCENT, True)
            drivetrain.drive_for(FORWARD, 5, INCHES, 90, PERCENT)
            wall_stake.reset()

            drivetrain.turn_for(LEFT, 55, DEGREES, 80, PERCENT)

            drivetrain.drive_for(REVERSE, 55, INCHES, 90, PERCENT)
            drivetrain.drive_for(FORWARD, 5, INCHES, 80, PERCENT)
            stake_grabber.toggle()

            wait(0.2, SECONDS)

            drivetrain.turn_for(RIGHT, 120, DEGREES, 85, PERCENT)

            lift_intake.spin(FORWARD)

            drivetrain.drive_for(FORWARD, 34, INCHES, 90, PERCENT)
            drivetrain.drive_for(REVERSE, 5, INCHES, 80, PERCENT)
            #after testing, we can add the above two lines again to pick up a third donut onto the stake

            drivetrain.turn_for(RIGHT, 90, DEGREES, 80, PERCENT)

            drivetrain.drive_for(FORWARD, 30, INCHES, 90, PERCENT)
            drivetrain.drive_for(REVERSE, 5, INCHES, 80, PERCENT)

            #check the time - if we don't have much time left, then just hit ladder
            drivetrain.turn_for(RIGHT, 90, DEGREES, 85, PERCENT)
            drivetrain.drive_for(FORWARD, 60, INCHES, 90, PERCENT)

            #check the time - if we have time left, then the following code will apply

            #drivetrain.turn_for(LEFT, 90, DEGREES, 75, PERCENT)

            #drivetrain.drive_for(FORWARD, 51, INCHES, 90, PERCENT)
            
            #doink_piston.toggle()
            #drivetrain.drive_for(FORWARD, 10, INCHES, 80, PERCENT)
            #drivetrain.turn_for(LEFT, 70, DEGREES, 90, PERCENT)

        """elif self.color == BLUE_SIG:
            #ts for goal rush
            drivetrain.drive_for(REVERSE, 40, INCHES, 85, PERCENT)
            drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT)
            stake_grabber.toggle()

            wait(0.3, SECONDS)
            #score the preload
            lift_intake.motor.spin_for(REVERSE, 2, TURNS)
            lift_intake.motor.spin_for(REVERSE, 1, TURNS)
            drivetrain.turn_for(LEFT, 90, DEGREES, 80, PERCENT)

            lift_intake.spin(FORWARD)

            drivetrain.drive_for(FORWARD, 35, INCHES, 90, PERCENT)
            wait(0.5, SECONDS)
            drivetrain.drive_for(REVERSE, 5, INCHES, 80, PERCENT)

            drivetrain.turn_for(RIGHT, 90, DEGREES, 85, PERCENT)
            drivetrain.drive_for(FORWARD, 12, INCHES, 90, PERCENT)

            drivetrain.turn_for(LEFT, 90, DEGREES, 85, PERCENT)
            drivetrain.drive_for(FORWARD, 18, INCHES, 90, PERCENT)

            drivetrain.turn_for(RIGHT, 90, DEGREES, 85, PERCENT)

            drivetrain.drive_for(FORWARD, 60, INCHES, 90, PERCENT)
            
            #clear corner
            doink_piston.toggle()
            drivetrain.drive_for(FORWARD, 10, INCHES, 80, PERCENT)
            drivetrain.turn_for(RIGHT, 116.6, DEGREES, 90, PERCENT)"""

    def _elims(self):
        drivetrain.set_timeout(5, SECONDS)
        if self.color == RED_SIG:
            if self.mode == "Ring":
                #its ring rush time
                #RED 
                #without alliance stake
                drivetrain.drive_for(REVERSE, 60, INCHES, 85, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT)
                stake_grabber.toggle()

                wait(0.3, SECONDS)
                #score the preload
                lift_intake.motor.spin_for(REVERSE, 3, TURNS, 100, PERCENT)
                lift_intake.motor.spin_for(REVERSE, 1, TURNS, 100, PERCENT)

                drivetrain.turn_for(LEFT, 105, DEGREES, 80, PERCENT)

                lift_intake.motor.spin(REVERSE, 100, PERCENT)

                drivetrain.drive_for(FORWARD, 46, INCHES, 90, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 6, INCHES, 90, PERCENT, wait=True)

                drivetrain.turn_for(LEFT, 63, DEGREES, 85, PERCENT, wait=True)

                #just to make sure that the lift intake spins properly
                lift_intake.motor.spin(REVERSE, 100, PERCENT)

                drivetrain.drive_for(FORWARD, 47, INCHES, 90, PERCENT, wait=True)
                drivetrain.drive_for(FORWARD, 7, INCHES, 80, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 6, INCHES, 90, PERCENT)

                #drivetrain.turn_for(RIGHT, 62, DEGREES, 85, PERCENT, wait=True)
                #drivetrain.drive_for(FORWARD, 18, INCHES, 80, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT, wait=True)

                drivetrain.stop()
                
                wait(4, SECONDS)

            elif self.mode == "Goal":
                #its goal rush time
                #BLUE 
                #without alliance stake
                drivetrain.drive_for(REVERSE, 60, INCHES, 85, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT)
                stake_grabber.toggle()

                wait(0.3, SECONDS)
                #score the preload
                lift_intake.motor.spin_for(REVERSE, 3, TURNS, 100, PERCENT)
                lift_intake.motor.spin_for(REVERSE, 1, TURNS, 100, PERCENT)

                drivetrain.turn_for(RIGHT, 105, DEGREES, 80, PERCENT)

                lift_intake.motor.spin(REVERSE, 100, PERCENT)

                drivetrain.drive_for(FORWARD, 46, INCHES, 90, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 6, INCHES, 90, PERCENT, wait=True)

                drivetrain.turn_for(RIGHT, 63, DEGREES, 85, PERCENT, wait=True)

                #just to make sure that the lift intake spins properly
                lift_intake.motor.spin(REVERSE, 100, PERCENT)

                drivetrain.drive_for(FORWARD, 47, INCHES, 90, PERCENT, wait=True)
                drivetrain.drive_for(FORWARD, 7, INCHES, 80, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 6, INCHES, 90, PERCENT)

                #drivetrain.turn_for(RIGHT, 62, DEGREES, 85, PERCENT, wait=True)
                #drivetrain.drive_for(FORWARD, 18, INCHES, 80, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT, wait=True)

                drivetrain.stop()
                lift_intake.motor.stop(BRAKE)

        elif self.color == BLUE_SIG:
            if self.mode == "Ring":
                #its ring rush time
                #BLUE 
                #without alliance stake
                drivetrain.drive_for(REVERSE, 60, INCHES, 85, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT)
                stake_grabber.toggle()

                wait(0.3, SECONDS)
                #score the preload
                lift_intake.motor.spin_for(REVERSE, 3, TURNS, 90, PERCENT)
                lift_intake.motor.spin_for(REVERSE, 1, TURNS, 90, PERCENT)
                if lift_intake.motor.velocity() == 0:
                    lift_intake.motor.spin_for(FORWARD, 1,  TURNS, 80, PERCENT, wait=True)

                    lift_intake.motor.spin(REVERSE, 100, PERCENT)
    
                drivetrain.turn_for(RIGHT, 105, DEGREES, 80, PERCENT)

                lift_intake.motor.spin(REVERSE, 100, PERCENT)

                drivetrain.drive_for(FORWARD, 49, INCHES, 90, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 6, INCHES, 90, PERCENT, wait=True)

                drivetrain.turn_for(RIGHT, 63, DEGREES, 85, PERCENT, wait=True)

                #just to make sure that the lift intake spins properly
                lift_intake.motor.spin(REVERSE, 100, PERCENT)

                drivetrain.drive_for(FORWARD, 45, INCHES, 90, PERCENT, wait=True)
                drivetrain.drive_for(FORWARD, 6, INCHES, 80, PERCENT, wait=True)
                lift_intake.motor.spin(REVERSE, 100, PERCENT)
                drivetrain.drive_for(REVERSE, 7, INCHES, 90, PERCENT, wait=True) 

                wait(4, SECONDS)

                #drivetrain.turn_for(LEFT, 62, DEGREES, 85, PERCENT, wait=True)
                #drivetrain.drive_for(FORWARD, 18, INCHES, 80, PERCENT, wait=True)
                #drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT, wait=True)

                #no more moving 
                drivetrain.stop()
                lift_intake.motor.stop(BRAKE)

                #smash into ladder time 
                #drivetrain.turn_for(LEFT, 180, DEGREES, 85, PERCENT, wait=True)
                #drivetrain.drive_for(FORWARD, 56, INCHES, 95, PERCENT)

            elif self.mode == "Goal":
                #its goal rush time
                #BLUE 
                #without alliance stake
                drivetrain.drive_for(REVERSE, 60, INCHES, 85, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT)
                stake_grabber.toggle()


                wait(0.3, SECONDS)
                #score the preload
                lift_intake.motor.spin_for(REVERSE, 3, TURNS, 100, PERCENT)
                lift_intake.motor.spin_for(REVERSE, 1, TURNS, 100, PERCENT)

                drivetrain.turn_for(LEFT, 105, DEGREES, 80, PERCENT)

                lift_intake.motor.spin(REVERSE, 100, PERCENT)

                drivetrain.drive_for(FORWARD, 46, INCHES, 90, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 6, INCHES, 90, PERCENT, wait=True)

                drivetrain.turn_for(LEFT, 63, DEGREES, 85, PERCENT, wait=True)

                #just to make sure that the lift intake spins properly
                lift_intake.motor.spin(REVERSE, 100, PERCENT)

                drivetrain.drive_for(FORWARD, 47, INCHES, 90, PERCENT, wait=True)
                drivetrain.drive_for(FORWARD, 7, INCHES, 80, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 6, INCHES, 90, PERCENT)

                #drivetrain.turn_for(RIGHT, 62, DEGREES, 85, PERCENT, wait=True)
                #drivetrain.drive_for(FORWARD, 18, INCHES, 80, PERCENT, wait=True)
                drivetrain.drive_for(REVERSE, 5, INCHES, 85, PERCENT, wait=True)

                drivetrain.stop()
                lift_intake.motor.stop(BRAKE)
                     
        //#elif self.direction == RIGHT:
            

            

    def _skills(self):
        #initial_time = brain.timer.time(MSEC)
        wall_stake.motor.spin(FORWARD, 90, PERCENT)
        wait(1, SECONDS)
        drivetrain.drive_for(REVERSE, 14, INCHES, 95, PERCENT)
        wall_stake.motor.spin(REVERSE, 90, PERCENT)
        wait(0.9, SECONDS)

        wall_stake.motor.stop(BRAKE)

        drivetrain.drive_for(FORWARD, 8, INCHES, 95, PERCENT)

        drivetrain.turn_for(RIGHT, 80, DEGREES, 90, PERCENT)

        drivetrain.drive_for(REVERSE, 66, INCHES, 95, PERCENT)
        drivetrain.drive_for(REVERSE, 4, INCHES, 95, PERCENT, wait=False)
        stake_grabber.toggle()

        wait(1, SECONDS)

        drivetrain.turn_for(RIGHT, 107, DEGREES, 90, PERCENT)

        lift_intake.spin(REVERSE)

        drivetrain.drive_for(FORWARD, 65, INCHES, 100, PERCENT)

        drivetrain.turn_for(RIGHT, 85, DEGREES, 90, PERCENT)

        drivetrain.drive_for(FORWARD, 62, INCHES, 95, PERCENT)

        drivetrain.turn_for(RIGHT, 85, DEGREES, 90, PERCENT)
        
        drivetrain.drive_for(FORWARD, 74, INCHES, 100, PERCENT)
        


        """#which direction
        drivetrain.drive_for(REVERSE, 62, INCHES, 95, PERCENT, wait=True)
        drivetrain.drive_for(REVERSE, 3, INCHES, 95, PERCENT, wait=False)
        stake_grabber.toggle()

        wait(0.2, SECONDS)
        #score the preload
        lift_intake.motor.spin_for(REVERSE, 3, TURNS, 100, PERCENT)
        lift_intake.motor.spin_for(REVERSE, 1, TURNS, 100, PERCENT)

        drivetrain.turn_for(LEFT, 210, DEGREES, 90, PERCENT)

        lift_intake.motor.spin(REVERSE, 100, PERCENT, wait=False)

        drivetrain.drive_for(FORWARD, 60, INCHES, 95, PERCENT, wait=True)
        wait(0.7, SECONDS)
 
        drivetrain.turn_for(LEFT, 105, DEGREES, 90, PERCENT, wait=True)

        #just to make sure that the lift intake spins properly
        lift_intake.motor.spin(REVERSE, 100, PERCENT, wait=False)

        drivetrain.drive_for(FORWARD, 60, INCHES, 95, PERCENT, wait=True)
        wait(0.7, SECONDS)

        drivetrain.turn_for(LEFT, 105, DEGREES, 90, PERCENT, wait=True)

        lift_intake.motor.spin(REVERSE, 100, PERCENT)

        drivetrain.drive_for(FORWARD, 78, INCHES, 95, PERCENT, wait=True)
        wait(0.3, SECONDS)

        drivetrain.turn_for(LEFT, 305, DEGREES, 90, PERCENT, wait=True)
        drivetrain.drive_for(REVERSE, 60, INCHES, wait=True)

        #ungrab the stake to put into corner
        stake_grabber.toggle()
        wait(0.8, SECONDS)

        drivetrain.drive_for(FORWARD, 60, INCHES, 95, PERCENT, wait=True)

        drivetrain.turn_for(RIGHT, 60, DEGREES, 90, PERCENT, wait=True)

        drivetrain.drive_for(REVERSE, 120, INCHES, 95, PERCENT, wait = True)
        drivetrain.drive_for(REVERSE, 5, INCHES, 95, PERCENT, wait=False)
        
        #to grab second mobile goal
        stake_grabber.toggle()
        
        wait(0.8, SECONDS)

        drivetrain.drive_for(FORWARD, 60, INCHES, 95, PERCENT)
        drivetrain.turn_for(RIGHT, 158, DEGREES, 90, PERCENT, wait=True)

        drivetrain.drive_for(FORWARD, 87, INCHES, 95, PERCENT)
        drivetrain.turn_for(RIGHT, 50, DEGREES, 90, PERCENT)
        drivetrain.drive_for(FORWARD, 15, INCHES, 95, PERCENT)

        drivetrain.turn_for(RIGHT, 305, DEGREES, 90, PERCENT)
        drivetrain.drive_for(REVERSE, 70, INCHES, 95, PERCENT)"""


    def set_config(self, config: dict[str, Any]):
        print(config)

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

lift_intake = LiftIntake(
    Motor(Ports.PORT7, GearSetting.RATIO_6_1, True), 
    Vision(Ports.PORT9, 50, BLUE_SIG, RED_SIG), 
)

wall_stake = WallStake(Motor(Ports.PORT8, GearSetting.RATIO_36_1, True), Rotation(Ports.PORT11))

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