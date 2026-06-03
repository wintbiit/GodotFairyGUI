# Test 09 -- UIPackage: load, query, create, cache, fonts
extends RefCounted

static func run(host: Node) -> bool:
    var TR := load("res://scripts/tests/test_runner.gd")
    var r: RefCounted = TR.new()
    r.init("09_uipackage")

    ClassDB.class_call_static("UIPackage", "remove_all_packages")
    ClassDB.class_call_static("UIPackage", "clear_texture_cache")

    var ok: bool = ClassDB.class_call_static("UIPackage", "add_package", "res://assets/ui/BundleUsage_fui.bytes")
    r.check(ok, "add_package")
    var pkg_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
    r.check(pkg_id != "", "pkg_id")
    r.check(ClassDB.class_call_static("UIPackage", "has_package", pkg_id), "has_package")
    var item_name: String = ClassDB.class_call_static("UIPackage", "get_first_image_item_name", pkg_id)
    r.check(item_name != "", "image_item")
    var img_url: String = ClassDB.class_call_static("UIPackage", "get_item_url", pkg_id, item_name)
    r.check(img_url.begins_with("ui://"), "item_url")

    r.check(ClassDB.class_call_static("UIPackage", "get_image_texture", pkg_id, item_name) != null, "get_image_texture")
    r.check(ClassDB.class_call_static("UIPackage", "get_image_texture_by_url", img_url) != null, "texture_by_url")
    r.check(ClassDB.class_call_static("UIPackage", "get_texture_cache_count") > 0, "cache_count")

    var obj: Variant = ClassDB.class_call_static("UIPackage", "create_object", pkg_id, item_name)
    r.check(obj != null, "create_object")
    if obj: host.add_child(obj); obj.free()

    var s9_name: String = ClassDB.class_call_static("UIPackage", "get_first_scale9_image_item_name", pkg_id)
    if s9_name != "":
        r.check(ClassDB.class_call_static("UIPackage", "has_image_scale9_grid", pkg_id, s9_name), "has_scale9")

    var bundle_bytes: PackedByteArray = FileAccess.get_file_as_bytes("res://assets/ui/BundleUsage_fui.bytes")
    if not bundle_bytes.is_empty():
        ok = ClassDB.class_call_static("UIPackage", "add_package_bytes", bundle_bytes, "mem/BundleUsage")
        r.check(ok, "add_package_bytes")
        var bp_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
        var bp_name: String = ClassDB.class_call_static("UIPackage", "get_first_image_item_name", bp_id)
        r.check(ClassDB.class_call_static("UIPackage", "get_image_texture", bp_id, bp_name) != null, "bytes_texture")

    DirAccess.make_dir_recursive_absolute("user://fgui_smoke")
    _copy_file_bytes("res://assets/ui/BundleUsage_atlas0.png", "user://fgui_smoke/atlas0.png")
    ClassDB.class_call_static("UIPackage", "set_asset_path_override", "hot/atlas0.png", "user://fgui_smoke/atlas0.png")
    r.check(ClassDB.class_call_static("UIPackage", "get_asset_path_override_count") == 1, "path_override")
    r.check(ClassDB.class_call_static("UIPackage", "resolve_asset_path", "hot/atlas0.png") == "user://fgui_smoke/atlas0.png", "resolve_path")
    ClassDB.class_call_static("UIPackage", "remove_asset_path_override", "hot/atlas0.png")

    ClassDB.class_call_static("UIPackage", "register_font_resource", "_test_", "res://nonexistent.ttf")
    var tf: Variant = ClassDB.class_call_static("UIPackage", "get_true_type_font", "_test_", 14)
    r.check(tf == null, "font not found")
    ClassDB.class_call_static("UIPackage", "unregister_font_resource", "_test_")

    ClassDB.class_call_static("UIPackage", "resolve_asset_path_for_image_ubb", "ui://demo/icon")
    ClassDB.class_call_static("UIPackage", "remove_all_packages")
    ClassDB.class_call_static("UIPackage", "clear_texture_cache")
    r.check(ClassDB.class_call_static("UIPackage", "get_package_count") == 0, "cleanup")
    return r.finalize()

static func _copy_file_bytes(from: String, to: String) -> void:
    var bytes: Variant = FileAccess.get_file_as_bytes(from)
    if bytes.is_empty(): return
    var file: Variant = FileAccess.open(to, FileAccess.WRITE)
    if file == null: return
    file.store_buffer(bytes); file.close()
