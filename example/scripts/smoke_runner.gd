extends Control

const EXAMPLES := [
	"Basics", "Bag", "BundleUsage", "Cooldown", "Curve",
	"CutScene", "EmitNumbers", "Emoji", "Extension", "Filter",
	"Gesture", "Guide", "HeadBar", "HitTest", "Joystick",
	"LoopList", "ModalWaiting", "PullToRefresh", "ScrollPane",
	"Transition", "TreeView", "TurnCard", "TurnPage", "TypingEffect",
	"VirtualList",
]

var _ok: int = 0
var _total: int = 0
var _frame: int = 0

func _ready() -> void:
	for pkg in EXAMPLES:
		_total += 1
		var panel = ClassDB.instantiate("UIPanel")
		panel.set("package_name", pkg)
		panel.set("component_name", "Main")
		panel.set("size", Vector2(1136, 640))
		panel.set_meta("_pkg", pkg)
		add_child(panel)

func _process(_delta: float) -> void:
	_frame += 1
	if _frame < 2:
		return
	if _frame > 60:
		print("GodotFairyGUI smoke: %d/%d (timeout)" % [_ok, _total])
		get_tree().quit(1)
		return

	var to_remove: Array = []
	for child in get_children():
		var pkg: String = child.get_meta("_pkg", "")
		if pkg == "":
			continue
		var loaded: bool = child.call("is_loaded")
		if loaded:
			print("[smoke] " + pkg + ": ok")
			_ok += 1
			to_remove.append(child)
		elif _frame > 10:
			print("[smoke] " + pkg + ": FAILED (timeout)")
			to_remove.append(child)

	for panel in to_remove:
		remove_child(panel)
		panel.queue_free()

	if get_child_count() == 0:
		print("GodotFairyGUI smoke: %d/%d" % [_ok, _total])
		get_tree().quit(0 if _ok == _total else 1)
