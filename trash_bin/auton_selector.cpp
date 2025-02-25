#include "main.hpp" // IWYU pragma: keep

using namespace std;

class Option {
    
private:
    unsigned index = 0;
    
    string name;
    vector<string> options;
    size_t count;

public:
    Option(string name, vector<string> options, size_t count) : 
        name(name), options(options), count(count) {}

    string get_str() const {
        return name + ": " + options[index];
    }

    string get_name() const {
        return name;
    }

    string get_value() const {
        return options[index];
    }

    void prev() {
        if (index == 0U) 
            index = count - 1;

        else
            index = index - 1;
    }

    void next() {
        index += 1;
        
        if (index == count)
            index = 0;
    }
};

/*

class SelectionMenu:
    def __init__(self):
        self.count = 0
        self.options: list[SelectionMenu._Option] = []

        self.disabled = False
        self.enter_callback: Callable[[dict[str, Any]], None]

        brain.screen.pressed(self._on_brain_screen_press)

        self.add_option("Enter", Color.WHITE, ["", "Are you sure?", "ENTERED"])
    
    def on_enter(self, callback: Callable[[dict[str, Any]], None]):
        self.enter_callback = callback

    def add_option(self, name: str, color: Color | Color.DefinedColor, choices: list[Any]):
        if self.disabled:
            return
        
        self.options.insert(self.count - 1, SelectionMenu._Option(name, color, choices))
        self.count += 1
    
    def _on_brain_screen_press(self):
        if self.disabled:
            return
        
        x = brain.screen.x_position()
        y = brain.screen.y_position()

        if y < 240 - 100:
            return
        
        self.options[x * self.count // 480].next()

        self.draw()

        if self.options[self.count - 1].value() == "ENTERED":
            self.enter_callback(self._get_all())
            self.disabled = True
            return
    
    def force_submit(self):
            self.enter_callback(self._get_all())
            self.disabled = True
            return
    
    def _get_all(self) -> dict[str, Any]:
        if self.disabled:
            return {}
        
        d = {}
        for option in self.options:
            d[option.name] = option.value()
        
        return d

    def draw(self):
        if self.disabled:
            return
        
        # print all
*/

typedef void (*menu_callback_t)(unordered_map<string, string> choices);

class SelectionMenu {

private:
    size_t count = 0;
    vector<Option> options = {};
    bool disabled = false;

    menu_callback_t enter_callback = nullptr;

    auto get_all() {
        unordered_map<string, string> ret;
        
        for (const auto& option : options)
            ret[option.get_name()] = option.get_value();

        return ret;
    }

    void submit() {
        enter_callback(get_all());
        disabled = true;
    }

public:
    SelectionMenu(menu_callback_t enter_callback) : enter_callback(enter_callback) {
        this->add_option("Enter", {"", "Are you sure?", "ENTERED"}, 3);
    }

    void set_enter_callback(menu_callback_t callback) {
        enter_callback = callback;
    }

    void add_option(string name, vector<string> choices, size_t count) {
        if (disabled)
            return;

        options.insert(options.end() - 1, Option(name, choices, count));
        this->count++;
    }

    void force_submit() {
        submit();
    }

    void draw() {

        if (options[count - 1].get_str() == "ENTERED") {
            submit();
        }
    }
};

/*
    menu.add_option("Team color", Color.RED, ["Red", "Blue"])
    menu.add_option("Auton direction", Color.BLUE, ["Left", "Right"])
    menu.add_option("Auton type", Color.PURPLE, ["Quals", "Elims", "Skills"])
    menu.add_option("Testing", Color.CYAN, ["Driver Control", "Auton"])
*/

static void lcd_update() {

}

namespace auton_selector {

}