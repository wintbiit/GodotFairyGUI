#pragma once

#include <godot_cpp/classes/control.hpp>

namespace godot {

class GRoot : public Control {
    GDCLASS(GRoot, Control)

    static GRoot *s_instance;

protected:
    static void _bind_methods();

public:
    GRoot();
    ~GRoot() override;

    static GRoot *get_instance();

    void set_design_resolution_x(int p_x);
    int get_design_resolution_x() const;
    void set_design_resolution_y(int p_y);
    int get_design_resolution_y() const;
    float get_content_scale_factor() const;

    void _ready() override;
    void _notification(int p_what);

private:
    int design_resolution_x = 0;
    int design_resolution_y = 0;
    float content_scale_factor = 1.0f;

    void apply_content_scale_factor();
};

} // namespace godot
