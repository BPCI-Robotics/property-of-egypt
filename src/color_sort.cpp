#include "main.hpp"

static int reject_color;
static bool color_sort_enabled = true;
static bool lift_intake_running = false;

namespace color_sort {

static void loop(void *enemy_sig_id_ptr);

void set_reject_color(int color) {
    reject_color = color;
}

void start(void) {
    Task task_loop {loop, &reject_color, "color_sort::loop"};
}

void toggle(void) {
    color_sort_enabled = !color_sort_enabled;
}

void declare_lift_intake_is_running(bool state) {
    lift_intake_running = state;
}

static void loop(void *enemy_sig_id_ptr) {
    int enemy_sig_id = *(int *) enemy_sig_id_ptr;

    while (true) {
        delay(20);

        /* Exit: the lift intake isn't running or the user doesn't want to color sort. */
        if (!lift_intake_running || !color_sort_enabled) 
            continue;

        vision_object enemy_donut = vision_sensor.get_by_sig(0, enemy_sig_id);

        /* Exit: the donut is too far away (so it appears small) */
        if (enemy_donut.height < 30 || enemy_donut.width < 70)
            continue;

        /* Hold on, I found something. Let's wait until the switch is hit. */
        int timer = 0;
        bool akita_neru = false;

        while (!donut_presence_sensor.get_new_press()) {
            delay(10);
            timer += 10;

            /* 1. Two seconds have passed and the donut did not make it to the top. */
            /* 2. The lift intake is not spinning anymore. No need to continue waiting. */
            /* 3. The driver has asked to disable color sorting. No need to continue waiting. */
            akita_neru = (timer > 2000 || !lift_intake_running || !color_sort_enabled);
            if (akita_neru)
                break;
        }

        /* Exit: the donut did not make it to the top. */
        if (akita_neru)
            continue;

        int save_direction = lift_intake.get_direction();

        delay(100);
        lift_intake.brake();
        delay(250);

        lift_intake.move_velocity(600 * (save_direction == FORWARD ? 1 : -1));
    }
}

} /* namespace color_sort */