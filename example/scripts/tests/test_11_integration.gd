# Test 11 -- Integration: package components, controllers, bitmap fonts, sound, movie clips
extends RefCounted

static func run(host: Node) -> bool:
    var TR := load("res://scripts/tests/test_runner.gd")
    var r: RefCounted = TR.new()
    r.init("11_integration")

    ClassDB.class_call_static("UIPackage", "remove_all_packages")
    ClassDB.class_call_static("UIPackage", "clear_texture_cache")
    r.check(ClassDB.class_call_static("UIPackage", "add_package", "res://assets/ui/Basics_fui.bytes"), "load Basics")
    var pkg_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")

    var comp_name: String = ClassDB.class_call_static("UIPackage", "get_first_component_item_name", pkg_id)
    r.check(comp_name != "", "component item")
    var comp: Variant = ClassDB.class_call_static("UIPackage", "create_object", pkg_id, comp_name)
    if comp:
        host.add_child(comp)
        r.check(comp.get_child_count() > 0, "component children")
        comp.free()

    var btn_name: String = ClassDB.class_call_static("UIPackage", "get_first_button_item_name", pkg_id)
    var btn: Variant = ClassDB.class_call_static("UIPackage", "create_object", pkg_id, btn_name)
    if btn:
        host.add_child(btn)
        r.check(btn.get_controller_count() > 0, "button controllers")
        r.check(btn.get_controller_page_count(0) > 0, "button pages")
        var page_id: String = btn.get_controller_selected_page_id(0)
        r.check(page_id != "", "selected_page")
        r.check(btn.has_controller_page(0, page_id), "has_page")
        r.check(btn.count_display_gear_descendants() > 0, "display_gear")
        if btn.get_controller_page_count(0) > 1:
            btn.set_controller_selected_index(0, 1)
            r.check(btn.get_controller_selected_page_id(0) != page_id, "page changed")
            btn.go_back_controller(0)
            r.check(btn.get_controller_selected_page_id(0) == page_id, "go_back")
        btn.free()

    var bf_name: String = ClassDB.class_call_static("UIPackage", "get_first_bitmap_font_item_name", pkg_id)
    if bf_name != "":
        r.check(ClassDB.class_call_static("UIPackage", "has_bitmap_font", pkg_id, bf_name), "has_bitmap")
        r.check(ClassDB.class_call_static("UIPackage", "get_bitmap_font_size", pkg_id, bf_name) > 0, "font_size")

    var snd_name: String = ClassDB.class_call_static("UIPackage", "get_first_sound_item_name", pkg_id)
    if snd_name != "":
        var su: String = ClassDB.class_call_static("UIPackage", "get_item_url", pkg_id, snd_name)
        r.check(ClassDB.class_call_static("UIPackage", "get_sound_audio_stream", pkg_id, snd_name) != null, "sound stream")
        var sb: Variant = ClassDB.instantiate("GButton")
        host.add_child(sb)
        sb.set_sound_url(su)
        r.check(sb.play_sound(), "play_sound")
        sb.free()

    var mc_name: String = ClassDB.class_call_static("UIPackage", "get_first_movie_clip_item_name", pkg_id)
    if mc_name != "":
        var fc: Variant = ClassDB.class_call_static("UIPackage", "get_movie_clip_frame_count", pkg_id, mc_name)
        r.check(fc > 0, "movieclip frames")
        r.check(ClassDB.class_call_static("UIPackage", "get_movie_clip_frame_texture", pkg_id, mc_name, 0) != null, "frame 0")
        var mc2: Variant = ClassDB.class_call_static("UIPackage", "create_object", pkg_id, mc_name)
        if mc2: host.add_child(mc2); mc2.free()

    var ht_ok: bool = ClassDB.class_call_static("UIPackage", "add_package", "res://assets/ui/HitTest_fui.bytes")
    if ht_ok:
        var ht_pkg: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
        r.check(ClassDB.class_call_static("UIPackage", "get_pixel_hit_test_item_count", ht_pkg) > 0, "pixel_hit_test")
        var ht_name: String = ClassDB.class_call_static("UIPackage", "get_first_pixel_hit_test_item_name", ht_pkg)
        if ht_name != "":
            var ht: Variant = ClassDB.class_call_static("UIPackage", "create_object", ht_pkg, ht_name)
            if ht:
                host.add_child(ht)
                r.check(ht.has_pixel_hit_test(), "has_pixel_hit_test")
                r.check(not ht.hit_test_point(Vector2(-1, -1)), "hit_test miss")
                ht.free()

    ClassDB.class_call_static("UIPackage", "remove_all_packages")
    ClassDB.class_call_static("UIPackage", "clear_texture_cache")
    return r.finalize()
