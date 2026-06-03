# Test 07 -- GLoader: layout, animation, color, fill, async HTTP
extends RefCounted

static func run(host: Node) -> bool:
    var TR := load("res://scripts/tests/test_runner.gd")
    var r: RefCounted = TR.new()
    r.init("07_gloader")

    var ld: Variant = ClassDB.instantiate("GLoader")
    host.add_child(ld)
    ld.set_size(Vector2(100, 80))

    ld.set_url("res://assets/ui/BundleUsage_atlas0.png")
    r.check(ld.get_texture() != null, "texture loaded")

    ld.set_align(1)
    r.check(ld.get_align() == 1, "align")
    ld.set_vertical_align(1)
    ld.set_fill(1)
    r.check(ld.get_fill() == 1, "fill")
    ld.set_shrink_only(true)
    r.check(ld.is_shrink_only(), "shrink_only")
    ld.set_auto_size(true)
    r.check(ld.is_auto_size(), "auto_size")
    ld.set_use_resize(true)
    r.check(ld.is_use_resize(), "use_resize")
    var cr: Variant = ld.get_content_rect()
    r.check(cr.size.x > 0, "content_rect")

    ld.set_playing(true)
    r.check(ld.is_playing(), "playing")
    ld.set_frame(0)
    r.check(ld.get_frame() == 0, "frame")
    ld.set_time_scale(2.0)
    r.check(ld.get_time_scale() == 2.0, "time_scale")

    ld.set_color(Color(1, 0, 0, 0.5))
    r.check(ld.get_color() == Color(1, 0, 0, 0.5), "color")
    ld.set_show_error_sign(false)
    r.check(not ld.is_show_error_sign(), "show_error_sign")

    ld.set_fill_method(1)
    r.check(ld.get_fill_method() == 1, "fill_method")
    ld.set_fill_origin(2)
    ld.set_fill_clockwise(false)
    r.check(not ld.is_fill_clockwise(), "fill_clockwise")
    ld.set_fill_amount(0.75)
    r.check(ld.get_fill_amount() == 0.75, "fill_amount")

    r.check(ld.get_state() == GLoader.STATE_IDLE, "state idle")
    r.check(ld.has_signal("fgui_loaded"), "fgui_loaded")
    r.check(ld.has_signal("fgui_load_failed"), "fgui_load_failed")
    ld.set_url("http://127.0.0.1:19999/nonexistent.png")
    r.check(ld.get_state() == GLoader.STATE_LOADING, "state loading")
    ld.cancel_http_request()
    r.check(ld.get_state() == GLoader.STATE_IDLE, "state idle after cancel")

    ld.free()
    return r.finalize()
