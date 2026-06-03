# ExampleBase -- shared helper for all FairyGUI example scenes
extends Control

func _ready() -> void:
	if DisplayServer.get_name() == "headless":
		_headless_test()
	else:
		_load_and_show()

func _load_and_show() -> void:
	var ok := _example_load()
	if not ok:
		return
	var comp := get_child(get_child_count() - 1)
	comp.set_anchors_preset(PRESET_FULL_RECT)
	comp.set_offsets_preset(PRESET_FULL_RECT)

func _example_load() -> bool:
	var pkg: String = _get_package_name()
	if not ClassDB.class_call_static("UIPackage", "has_package", pkg):
		var path: String = "res://assets/ui/" + pkg + "_fui.bytes"
		if not ClassDB.class_call_static("UIPackage", "add_package", path):
			printerr("FAILED to load " + path)
			return false

	var pkg_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
	var comp_name: String = _get_component_name()
	var comp = ClassDB.class_call_static("UIPackage", "create_object", pkg_id, comp_name)
	if not comp:
		printerr("FAILED to create " + comp_name + " in " + pkg_id)
		return false
	add_child(comp)
	return true

func _headless_test() -> void:
	var ok := _example_load()
	print("[example] " + _get_package_name() + ": " + ("ok" if ok else "FAILED"))
	get_tree().quit(0 if ok else 1)

# Override these:
func _get_package_name() -> String: return ""
func _get_component_name() -> String: return "Main"
