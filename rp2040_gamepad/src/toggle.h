// Simple tracking of a key combo being pressed then released.
// Flips its state on buttons release.
class Toggle {
public:
    Toggle() {}

    Toggle(bool val) : state(val) {}

    bool changed(bool buttons) {
        if (buttons)
            pressed = true;
        else if (pressed) {
            pressed = false;
            state = !state;
            return true;
        }
        return false;
    }

    operator bool() const {
        return state;
    }

    void operator =(const bool val) {
        state = val;
    }

private:
    bool state { false };
    bool pressed { false };
};
