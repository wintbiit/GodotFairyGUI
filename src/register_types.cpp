#include "register_types.h"

#include "runtime/g_component.h"
#include "runtime/g_button.h"
#include "runtime/g_combo_box.h"
#include "runtime/g_graph.h"
#include "runtime/g_image.h"
#include "runtime/g_label.h"
#include "runtime/g_list.h"
#include "runtime/g_loader.h"
#include "runtime/g_movie_clip.h"
#include "runtime/g_object.h"
#include "runtime/g_progress_bar.h"
#include "runtime/g_scroll_bar.h"
#include "runtime/g_slider.h"
#include "runtime/g_text_field.h"
#include "runtime/g_text_input.h"
#include "runtime/g_tween.h"
#include "runtime/ui_package.h"

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_godot_fairygui_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    GDREGISTER_CLASS(UIPackage);
    GDREGISTER_CLASS(GObject);
    GDREGISTER_CLASS(GComponent);
    GDREGISTER_CLASS(GButton);
    GDREGISTER_CLASS(GComboBox);
    GDREGISTER_CLASS(GGraph);
    GDREGISTER_CLASS(GImage);
    GDREGISTER_CLASS(GLabel);
    GDREGISTER_CLASS(GTextField);
    GDREGISTER_CLASS(GTextInput);
    GDREGISTER_CLASS(GLoader);
    GDREGISTER_CLASS(GMovieClip);
    GDREGISTER_CLASS(GList);
    GDREGISTER_CLASS(GProgressBar);
    GDREGISTER_CLASS(GScrollBar);
    GDREGISTER_CLASS(GSlider);
    GDREGISTER_CLASS(GTween);
}

void uninitialize_godot_fairygui_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {
GDExtensionBool GDE_EXPORT godot_fairygui_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
        GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_godot_fairygui_module);
    init_obj.register_terminator(uninitialize_godot_fairygui_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
