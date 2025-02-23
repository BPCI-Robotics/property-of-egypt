#include "main.hpp"

/* Vision ID 1 is whatever the enemy color is. */

using namespace pros;

static int reject_color;
static bool color_sort_enabled = true;

static Motor motor (7, MotorGears::blue, MotorUnits::counts);
static v5::Vision vision (9);
static adi::Button limit_switch('c');

namespace lift_intake {

static void loop(void *);

void init(int color_to_reject) {
    reject_color = color_to_reject;

    if (color_to_reject == REJECT_RED) {
        static vision_signature_s_t red_signature =  Vision::signature_from_utility (1,  7457,  9721,  8589, -611,    1, -305, 3.0, 0);
        vision.set_signature(1,  &red_signature);
    }

    if (color_to_reject == REJECT_BLUE) {
        static vision_signature_s_t blue_signature = Vision::signature_from_utility (1, -3775, -3259, -3517, 4809, 7525, 6167, 3.0, 0);
        vision.set_signature(1, &blue_signature);
    }
    

    Task task_loop {loop, nullptr, "color_sort::loop"};
}

void stop() {
    motor.brake();
}

void spin(int direction) {
    motor.move_velocity(direction * 600);
}

static void loop(void *) {

    while (true) {
        delay(20);

        if (!color_sort_enabled) 
            continue;

        vision_object enemy_donut = vision.get_by_sig(0, 1);

        if (enemy_donut.height < 30 || enemy_donut.width < 70)
            continue;

        while (!limit_switch.get_new_press()) 
            delay(20);

        delay(10);
        lift_intake::stop();
        delay(250);

        lift_intake::spin(FORWARD);
    }
}

} /* namespace color_sort */