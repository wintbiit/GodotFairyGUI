@tool
extends EditorPlugin

const PACKAGE_ROOT_SETTING := "godot_fairygui/package_search_root"

func _enter_tree() -> void:
	if not ProjectSettings.has_setting(PACKAGE_ROOT_SETTING):
		ProjectSettings.set_setting(PACKAGE_ROOT_SETTING, "res://assets/ui")
	add_tool_menu_item("FairyGUI/Load Packages", _load_packages_from_project)
	add_tool_menu_item("FairyGUI/Refresh Scene Components", _refresh_scene_components)
	add_tool_menu_item("FairyGUI/Clear Runtime Packages", _clear_runtime_packages)

func _exit_tree() -> void:
	remove_tool_menu_item("FairyGUI/Load Packages")
	remove_tool_menu_item("FairyGUI/Refresh Scene Components")
	remove_tool_menu_item("FairyGUI/Clear Runtime Packages")

func _load_packages_from_project() -> void:
	if not ClassDB.class_exists("UIPackage"):
		push_warning("GodotFairyGUI: UIPackage is not registered. Build and load the GDExtension first.")
		return

	var root: String = str(ProjectSettings.get_setting(PACKAGE_ROOT_SETTING, "res://assets/ui"))
	var files: PackedStringArray = _find_package_files(root)
	var loaded := 0
	for path in files:
		if ClassDB.class_call_static("UIPackage", "add_package", path):
			loaded += 1
	print("GodotFairyGUI editor: loaded %d package(s) from %s." % [loaded, root])
	_refresh_scene_components()

func _refresh_scene_components() -> void:
	var root := get_editor_interface().get_edited_scene_root()
	if root == null:
		return
	var refreshed := _refresh_component_tree(root)
	print("GodotFairyGUI editor: refreshed %d scene component(s)." % refreshed)

func _clear_runtime_packages() -> void:
	if not ClassDB.class_exists("UIPackage"):
		return
	ClassDB.class_call_static("UIPackage", "remove_all_packages")
	ClassDB.class_call_static("UIPackage", "clear_texture_cache")
	print("GodotFairyGUI editor: cleared runtime packages.")

func _refresh_component_tree(node: Node) -> int:
	var count := 0
	if node.has_method("construct_from_resource"):
		var package_name: String = str(node.get("package_name"))
		var component_name: String = str(node.get("component_name"))
		if package_name != "" and component_name != "" and node.call("construct_from_resource"):
			count += 1

	for child in node.get_children():
		count += _refresh_component_tree(child)
	return count

func _find_package_files(root: String) -> PackedStringArray:
	var files := PackedStringArray()
	_collect_package_files(root, files)
	return files

func _collect_package_files(path: String, files: PackedStringArray) -> void:
	var dir := DirAccess.open(path)
	if dir == null:
		return

	dir.list_dir_begin()
	while true:
		var entry := dir.get_next()
		if entry == "":
			break
		if entry == "." or entry == "..":
			continue

		var full_path := path.path_join(entry)
		if dir.current_is_dir():
			_collect_package_files(full_path, files)
		elif entry.ends_with("_fui.bytes") or entry.ends_with(".fui.bytes"):
			files.append(full_path)
	dir.list_dir_end()
