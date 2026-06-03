# Test 05 -- GList: layout, selection, virtual, scroll
extends RefCounted

static func run(host: Node) -> bool:
    var TR := load("res://scripts/tests/test_runner.gd")
    var r: RefCounted = TR.new()
    r.init("05_glist")

    var lst: Variant = ClassDB.instantiate("GList")
    host.add_child(lst)
    lst.set_size(Vector2(100, 200))

    r.check(lst.get_layout() == 0, "layout")
    r.check(lst.get_selection_mode() == 0, "selection_mode")
    r.check(lst.is_mouse_wheel_enabled(), "wheel")
    r.check(lst.get_item_count() == 0, "empty")

    lst.select_all(); lst.select_none()
    r.check(lst.get_selected_index() == -1, "no selection")
    lst.set_virtual()
    r.check(lst.is_virtual(), "virtual")

    lst.set_scroll_step(16.0)
    r.check(lst.get_scroll_step() == 16.0, "scroll_step")
    lst.set_align(1)
    r.check(lst.get_align() == 1, "align")
    lst.scroll_to_view(0)
    lst.scroll_to_view_animated(0, true)
    lst.resize_to_fit()
    lst.handle_mouse_wheel(MOUSE_BUTTON_WHEEL_DOWN)
    r.check(lst.get_item_index_for_child_index(0) >= -1, "item_index")
    lst.free()
    return r.finalize()
