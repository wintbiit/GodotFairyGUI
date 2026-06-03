# Test 10 -- Widgets: GGraph, GImage, GLabel, GComboBox, GProgressBar, GScrollBar, GSlider, GMovieClip
extends RefCounted

static func run(host: Node) -> bool:
    var TR := load("res://scripts/tests/test_runner.gd")
    var r: RefCounted = TR.new()
    r.init("10_widgets")

    # GGraph
    var graph: Variant = ClassDB.instantiate("GGraph")
    host.add_child(graph)
    r.check(graph.is_empty(), "graph empty")
    graph.draw_rect_shape(32, 16, 1, Color.BLACK, Color.RED)
    r.check(graph.get_shape_type() == 1, "graph rect")
    r.check(not graph.is_empty(), "graph not empty")
    graph.clear_shape()
    r.check(graph.is_empty(), "graph cleared")
    graph.free()

    # GImage
    var img: Variant = ClassDB.instantiate("GImage")
    host.add_child(img)
    img.set_scale9_grid(Rect2(4, 5, 12, 13))
    r.check(img.has_scale9_grid(), "image scale9")
    r.check(img.get_scale9_grid() == Rect2(4, 5, 12, 13), "grid match")
    img.clear_scale9_grid()
    r.check(not img.has_scale9_grid(), "image no scale9")
    img.free()

    # GLabel
    var lbl: Variant = ClassDB.instantiate("GLabel")
    host.add_child(lbl)
    r.check(ClassDB.is_parent_class(lbl.get_class(), "GComponent"), "label GComponent")
    lbl.set("title", "MyLabel")
    r.check(lbl.get("title") == "MyLabel", "label title")
    lbl.set("title_color", Color.BLUE)
    r.check(lbl.get("title_color") == Color.BLUE, "label color")
    lbl.free()

    # GComboBox
    var cb: Variant = ClassDB.instantiate("GComboBox")
    host.add_child(cb)
    cb.add_item("Alpha", "a"); cb.add_item("Beta", "b")
    r.check(cb.get_item_count() == 2, "combo items")
    r.check(cb.get("value") == "a", "combo value")
    cb.set("selected_index", 1)
    r.check(cb.get("value") == "b", "combo switch")
    cb.free()

    # GProgressBar
    var pb: Variant = ClassDB.instantiate("GProgressBar")
    host.add_child(pb)
    pb.set("value", 25.0); pb.set("max", 50.0)
    r.check(is_equal_approx(pb.get_percent(), 0.5), "progress 50%")
    r.check(pb.get_title_text() == "50%", "progress title")
    pb.set("title_type", 1)
    r.check(pb.get_title_text() == "25/50", "progress val/max")
    pb.free()

    # GScrollBar
    var sb: Variant = ClassDB.instantiate("GScrollBar")
    host.add_child(sb)
    sb.set("display_perc", 0.25)
    r.check(sb.get("display_perc") == 0.25, "scrollbar display")
    sb.set("scroll_perc", 0.5)
    r.check(sb.get("scroll_perc") == 0.5, "scrollbar scroll")
    sb.free()

    # GSlider
    var sl: Variant = ClassDB.instantiate("GSlider")
    host.add_child(sl)
    sl.set("value", 20.0); sl.set("max", 40.0)
    r.check(is_equal_approx(sl.get_percent(), 0.5), "slider 50%")
    sl.set("whole_numbers", true)
    sl.update_with_percent(0.333, true)
    r.check(sl.get("value") == 13.0, "slider whole")
    sl.free()

    # GMovieClip
    var mc: Variant = ClassDB.instantiate("GMovieClip")
    host.add_child(mc)
    mc.set("playing", false)
    r.check(not mc.get("playing"), "movieclip stopped")
    mc.set("time_scale", 2.0)
    r.check(mc.get("time_scale") == 2.0, "movieclip timescale")
    mc.set("frame", 3)
    r.check(mc.get("frame") == 3, "movieclip frame")
    mc.free()

    return r.finalize()
