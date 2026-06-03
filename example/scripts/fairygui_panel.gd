# FairyGUIPanel -- Godot equivalent of Unity's UIPanel
# @tool script: loads FairyGUI package+component directly in editor for WYSIWYG preview.
@tool
extends Control

## Package name (without _fui.bytes suffix)
@export var package_name: String = "":
	set(v):
		package_name = v
		if is_inside_tree():
			_refresh()

## Component name to create from the package (default: "Main")
@export var component_name: String = "Main":
	set(v):
		component_name = v
		if is_inside_tree():
			_refresh()

## Auto-load packages when set
@export var auto_load_package: bool = true

var _loaded: bool = false
var _component = null
var _dirty: bool = false

func _ready() -> void:
	if not package_name.is_empty():
		_refresh()

func refresh() -> void:
	_refresh()

func _refresh() -> void:
	if package_name.is_empty():
		return

	_cleanup()

	if not ClassDB.class_exists("UIPackage"):
		if Engine.is_editor_hint():
			return  # silently skip in editor, GDExtension may not be loaded
		push_error("FairyGUIPanel: UIPackage not registered")
		return

	if auto_load_package and not _is_package_loaded():
		var path := "res://assets/ui/" + package_name + "_fui.bytes"
		if not FileAccess.file_exists(path):
			push_warning("FairyGUIPanel: package not found: " + path)
			return
		if not ClassDB.class_call_static("UIPackage", "add_package", path):
			push_warning("FairyGUIPanel: failed to load " + package_name)
			return

	var pkg_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
	var comp = ClassDB.class_call_static("UIPackage", "create_object", pkg_id, component_name)
	if not comp:
		# Some packages use a non-"Main" root component — try finding it
		var first_comp: String = ClassDB.class_call_static("UIPackage", "get_first_component_item_name", pkg_id)
		if first_comp != "" and first_comp != component_name:
			comp = ClassDB.class_call_static("UIPackage", "create_object", pkg_id, first_comp)
			if comp:
				component_name = first_comp
		if not comp:
			push_warning("FairyGUIPanel: component '%s' not found in '%s'" % [component_name, pkg_id])
			return

	add_child(comp)
	_component = comp
	if Engine.is_editor_hint():
		var root := get_tree().edited_scene_root
		if root:
			comp.owner = root
	_loaded = true

func _cleanup() -> void:
	if _component:
		_component.queue_free()
		_component = null
	_loaded = false

func _is_package_loaded() -> bool:
	if not ClassDB.class_exists("UIPackage"):
		return false
	return ClassDB.class_call_static("UIPackage", "has_package", package_name)

func run_headless_test() -> bool:
	_refresh()
	return _loaded
