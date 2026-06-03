#pragma once

#include <godot_cpp/classes/control.hpp>

namespace godot {

class GComponent;

class UIPanel : public Control {
    GDCLASS(UIPanel, Control)

protected:
    static void _bind_methods();

public:
    void _ready() override;

    void set_package_name(const String &p_name);
    String get_package_name() const;
    void set_component_name(const String &p_name);
    String get_component_name() const;

    void refresh();
    bool is_loaded() const;

private:
    String package_name;
    String component_name;
    GComponent *_component = nullptr;
    bool _loaded = false;

    void _cleanup();
    bool _ensure_package_loaded();
};

} // namespace godot
