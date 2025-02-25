#include "main.hpp" // IWYU pragma: keep

typedef struct {
    size_t index;
    size_t count;

    const char name[50];
    const char *options[3];
} option;

static void prev(option *o) {
    if (o->index == 0) 
        o->index = o->count - 1;
    else
        o->index--;
}

static void next(option *o) {
    o->index++;

    if (o->index == o->count)
        o->index = 0;
}

static unsigned idx = 0;

option options[] = {
    {.index=0, .count=2, .name="Team color", .options={"Red", "Blue"}},
    {.index=0, .count=2, .name="Direction", .options={"Left", "Right"}},
    {.index=0, .count=3, .name="Auton type", .options={"Quals", "Elims", "Skills"}},
    {.index=0, .count=2, .name="Testing", .options={"Driver Control", "Auton"}}
};

size_t options_len = sizeof(options) / sizeof(option);

static void lcd_update() {
    lcd::clear();

    for (unsigned i = 0; i < options_len; i++) {
        lcd::print(i, "%c %s: %s", i == idx ? '>' : ' ', options[i].name, options[i].options[options[i].index]);
    }
}

namespace auton_selector {
    void btn0_cb() {
        prev(options + idx);
        
        lcd_update();
    }

    void btn1_cb() {
        idx++;
        if (idx == options_len)
            idx = 0;

        lcd_update();
    }

    void btn2_cb() {
        next(options + idx);
        lcd_update();
    }
}