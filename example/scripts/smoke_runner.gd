# Smoke test runner for all example scenes (headless CI)
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

func _ready() -> void:
	if not DisplayServer.get_name() == "headless":
		print("Run with --headless for automated smoke test.")
		return
	_run_tests()

func _run_tests() -> void:
	for pkg in EXAMPLES:
		_total += 1
		var path: String = "res://scenes/examples/example_" + pkg + ".tscn"
		var s: PackedScene = load(path)
		if s == null:
			printerr("SMOKE: FAILED to load " + path)
			continue

		var n: Node = s.instantiate()
		add_child(n)
		var ok: bool = false
		if n.has_method("run_headless_test"):
			ok = n.run_headless_test()
		print("[smoke] " + pkg + ": " + ("ok" if ok else "FAILED"))
		if ok: _ok += 1
		n.free()

	print("GodotFairyGUI smoke: %d/%d" % [_ok, _total])
	get_tree().quit(0 if _ok == _total else 1)
