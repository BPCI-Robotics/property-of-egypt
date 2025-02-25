#include "main.hpp" // IWYU pragma: keep

typedef struct {
    size_t idx;
    size_t cnt;

    const char name[50];
    const char *options[3];
} option;

static void prev(option *o) {
    if (o->idx == 0) 
        o->idx = o->cnt - 1;
    else
        o->idx--;
}

static void next(option *o) {
    o->idx++;

    if (o->idx == o->cnt)
        o->idx = 0;
}

static option options[] = {
    {.idx=0, .cnt=2, .name="Team color", .options={"Red", "Blue"}},
    {.idx=0, .cnt=2, .name="Direction", .options={"Left", "Right"}},
    {.idx=0, .cnt=3, .name="Auton type", .options={"Quals", "Elims", "Skills"}},
    {.idx=0, .cnt=2, .name="Testing", .options={"Driver Control", "Auton"}}
};

static unsigned options_idx = 0;
static const size_t options_len = sizeof(options) / sizeof(option);

static void lcd_update() {
    lcd::clear();

    for (unsigned i = 0; i < options_len; i++) {
        lcd::print(i, "%c %s: %s", i == options_idx ? '>' : ' ', options[i].name, options[i].options[options[i].idx]);
    }
}

namespace auton_selector {
    void btn0_cb() {
        prev(options + options_idx);
        lcd_update();
    }

    void btn1_cb() {
        options_idx++;
        if (options_idx == options_len)
            options_idx = 0;

        lcd_update();
    }

    void btn2_cb() {
        next(options + options_idx);
        lcd_update();
    }
}