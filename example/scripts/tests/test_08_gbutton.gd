# Test 08 -- GButton: properties, state, sound
extends RefCounted

static func run(host: Node) -> bool:
    var TR := load("res://scripts/tests/test_runner.gd")
    var r: RefCounted = TR.new()
    r.init("08_gbutton")

    var btn: Variant = ClassDB.instantiate("GButton")
    host.add_child(btn)

    btn.set_title("Click me")
    r.check(btn.get_title() == "Click me", "title")
    btn.set_selected_title("On")
    r.check(btn.get_selected_title() == "On", "selected_title")
    btn.set_icon("res://assets/ui/BundleUsage_atlas0.png")
    r.check(btn.get_icon() == "res://assets/ui/BundleUsage_atlas0.png", "icon")
    btn.set_selected_icon("res://icon_sel.png")
    r.check(btn.get_selected_icon() == "res://icon_sel.png", "selected_icon")

    btn.set_title_color(Color(0.2, 0.3, 0.4))
    r.check(btn.get_title_color() == Color(0.2, 0.3, 0.4), "title_color")
    btn.set_title_font_size(20)
    r.check(btn.get_title_font_size() == 20, "title_font_size")

    r.check(not btn.is_selected(), "not selected")
    btn.set_selected(true)
    r.check(btn.is_selected(), "selected")
    btn.set_selected(false)
    r.check(btn.has_signal("fgui_click"), "fgui_click")

    btn.set_related_controller_index(0)
    r.check(btn.get_related_controller_index() == 0, "related_controller")
    btn.set_related_page_id("page_a")
    r.check(btn.get_related_page_id() == "page_a", "related_page")

    var popup: Variant = ClassDB.instantiate("GObject")
    btn.set_linked_popup(popup)
    r.check(btn.get_linked_popup() != null, "linked_popup")
    btn.set_sound_volume_scale(0.5)
    r.check(is_equal_approx(btn.get_sound_volume_scale(), 0.5), "sound_volume")

    btn.free()
    return r.finalize()
