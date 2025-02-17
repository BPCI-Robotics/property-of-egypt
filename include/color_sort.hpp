namespace color_sort {

/* Set the reject color of the color sort. Valid values are `BLUE_SIG_ID` and `RED_SIG_ID`. 
   This must be called before `color_sort::start()` or `color_sort::toggle()`. */
void set_reject_color(int color);

/* Start the color sort. I trust that you won't call it more than once in each period, driver and autonomous. */
void start(void);

/* This toggles the color sort. This does not stop or start the thread. It is on by default. */
void toggle(void);

/* This must be run whenever you turn on or off the lift intake. */
void declare_lift_intake_is_running(bool state);

}

const int BLUE_SIG_ID = 1;
const int RED_SIG_ID = 2;