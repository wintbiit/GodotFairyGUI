#include "ui_panel.h"

#include "g_component.h"
#include "g_root.h"
#include "ui_package.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

void UIPanel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_package_name", "name"), &UIPanel::set_package_name);
    ClassDB::bind_method(D_METHOD("get_package_name"), &UIPanel::get_package_name);
    ClassDB::bind_method(D_METHOD("set_component_name", "name"), &UIPanel::set_component_name);
    ClassDB::bind_method(D_METHOD("get_component_name"), &UIPanel::get_component_name);
    ClassDB::bind_method(D_METHOD("refresh"), &UIPanel::refresh);
    ClassDB::bind_method(D_METHOD("is_loaded"), &UIPanel::is_loaded);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "package_name"), "set_package_name", "get_package_name");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "component_name"), "set_component_name", "get_component_name");
}

void UIPanel::_ready() {
    if (!package_name.is_empty()) {
        refresh();
    }
}

void UIPanel::set_package_name(const String &p_name) {
    package_name = p_name;
}

String UIPanel::get_package_name() const {
    return package_name;
}

void UIPanel::set_component_name(const String &p_name) {
    component_name = p_name;
}

String UIPanel::get_component_name() const {
    return component_name;
}

void UIPanel::refresh() {
    _cleanup();
    if (package_name.is_empty() || component_name.is_empty()) {
        return;
    }

    if (!_ensure_package_loaded()) {
        return;
    }

    GComponent *comp = Object::cast_to<GComponent>(UIPackage::create_object(package_name, component_name));
    if (!comp) {
        return;
    }

    add_child(comp);

    const Vector2 design_size = comp->get_size();
    const Vector2 panel_size = get_size();
    if (design_size.x > 0.0f && design_size.y > 0.0f && panel_size.x > 0.0f && panel_size.y > 0.0f) {
        const float sf = MIN(panel_size.x / design_size.x, panel_size.y / design_size.y);
        comp->set_scale(Vector2(sf, sf));
        comp->set_position(Vector2(
                (panel_size.x - design_size.x * sf) * 0.5f,
                (panel_size.y - design_size.y * sf) * 0.5f));
    }

    if (Engine::get_singleton()->is_editor_hint()) {
        comp->set_owner(nullptr);
    }
    _component = comp;
    _loaded = true;
}

void UIPanel::_cleanup() {
    if (_component) {
        _component->queue_free();
        _component = nullptr;
    }
    _loaded = false;
}

bool UIPanel::_ensure_package_loaded() {
    if (UIPackage::has_package(package_name)) {
        return true;
    }

    const String path = "res://assets/ui/" + package_name + "_fui.bytes";
    if (!FileAccess::file_exists(path)) {
        return false;
    }

    return UIPackage::add_package(path);
}

bool UIPanel::is_loaded() const {
    return _loaded;
}

} // namespace godot
