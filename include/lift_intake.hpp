#define REVERSE -1
#define FORWARD  1

#define REJECT_RED 1
#define REJECT_BLUE 2

/* Vision ID 1 is whatever the enemy color is. */

namespace lift_intake {
    void init(int color_to_reject);
    void stop();
    void spin(int direction);
    
} /* namespace color_sort */