#include "main.hpp" // IWYU pragma: keep

static void submit();

typedef struct {
    unsigned idx;
    unsigned cnt;

    const char name[50];
    const char *const options[3];
} option;

static void prev(option *o) {
    std::puts("Entering prev");
    if (o->idx == 0) 
        o->idx = o->cnt - 1;
    else
        o->idx--;

    std::puts("Exiting prev");
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
    {.idx=0, .cnt=2, .name="Testing", .options={"Nothing", "Driver Control", "Auton"}},
    {.idx=0, .cnt=3, .name="Submit", .options={"", "Are you sure?", "SUBMITTED"}}
};

static unsigned options_idx = 0;
static const size_t options_len = sizeof(options) / sizeof(option);

static void lcd_update() {
    std::puts("Entered lcd_update");
    lcd::clear();
    std::puts("Exiting lcd_update");

    if (options[4].idx == 2U) {
        std::puts("Entering submit");
        submit();
        std::puts("Exiting submit");
        lcd::print(0, "Submitted configuration.");
        return;
    }

    std::puts("Got to the for loop.");
    for (unsigned i = 0; i < options_len; i++) {
        std::printf("About to print line %d", i);
        pros::delay(1000);
        pros::lcd::print(i, "Testing");
        // lcd::print(i, "%c %s: %s", i == options_idx ? '>' : ' ', options[i].name, options[i].options[options[i].idx]);
    }
}

using uss = std::unordered_map<std::string, std::string>;

static uss get_result(void) {
    uss ret;
    
    for (unsigned i = 0; i < options_len; i++)
        ret[options[i].name] = options[i].options[options[i].idx];
    
    return ret;
}

void (*cb)(uss);

static bool submitted = false;
static void submit() {
    submitted = true;
    cb(get_result());
}

namespace auton_selector {

    void init(void (*submit_callback)(uss)) {
        // return;
        cb = submit_callback;
        lcd::register_btn0_cb(auton_selector::btn0_cb);
        lcd::register_btn1_cb(auton_selector::btn1_cb);
        lcd::register_btn2_cb(auton_selector::btn2_cb);
        lcd_update();

        std::puts("Finished registration");
        auton_selector::btn0_cb();
        std::puts("Finished running btn0_cb");
    }

    void btn0_cb() {
        std::puts("Entered btn0_cb");
        if (submitted)
            return;
        std::puts("Passed submit check");
        std::puts("Entering prev");
        prev(options + options_idx);
        std::puts("Exiting prev");
        std::puts("Decremented pointer");
        std::puts("Entering lcd_update");
        lcd_update();
    }

    void btn1_cb() {
        if (submitted)
            return;

        options_idx++;
        if (options_idx == options_len)
            options_idx = 0;

        lcd_update();
    }

    void btn2_cb() {
        if (submitted)
            return;

        next(options + options_idx);
        lcd_update();
    }
}