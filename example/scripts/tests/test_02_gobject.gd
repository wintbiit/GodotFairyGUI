# Test 02 -- GObject: properties, transforms, relations, signals
extends RefCounted

const Runner := preload("res://scripts/tests/test_runner.gd")

static func run(host: Node) -> bool:
    var r: RefCounted = Runner.new()
    r.init("02_gobject")

    var obj: Variant = ClassDB.instantiate("GObject")
    host.add_child(obj)

    obj.set_id("go_001")
    r.check(obj.get_id() == "go_001", "set_id")
    obj.set_data({"val": 99})
    r.check(obj.get_data()["val"] == 99, "set_data")
    obj.set_enabled(true)
    r.check(obj.is_enabled(), "enabled")
    obj.set_grayed(true)
    r.check(obj.is_grayed() and not obj.is_enabled(), "grayed")
    obj.set_tooltips("hello")
    r.check(obj.get_tooltips() == "hello", "tooltips")

    obj.position = Vector2(10, 20)
    r.check(obj.local_to_global_pos(Vector2(5, 5)).is_equal_approx(Vector2(15, 25)), "local_to_global")

    var other: Variant = ClassDB.instantiate("GObject")
    host.add_child(other)
    obj.add_relation(other, 0, false)
    r.check(obj.get_relation_count() > 0, "add_relation")
    obj.remove_relation(other, 0)
    r.check(obj.get_relation_count() == 0, "remove_relation")
    other.free()

    r.check(obj.has_signal("fgui_click"), "fgui_click")
    r.check(obj.has_signal("fgui_drag_start"), "drag_start")
    r.check(obj.has_signal("fgui_focus_in"), "focus_in")

    obj.free()
    return r.finalize()
