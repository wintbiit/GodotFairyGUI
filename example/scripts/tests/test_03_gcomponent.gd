# Test 03 -- GComponent: children, controllers, transitions
extends RefCounted

const Runner := preload("res://scripts/tests/test_runner.gd")

static func run(host: Node) -> bool:
    var r: RefCounted = Runner.new()
    r.init("03_gcomponent")

    var comp: Variant = ClassDB.instantiate("GComponent")
    host.add_child(comp)
    comp.set_package_name("TestPkg")
    r.check(comp.get_package_name() == "TestPkg", "package_name")

    var a: Variant = ClassDB.instantiate("GObject"); a.name = "alpha"
    var b: Variant = ClassDB.instantiate("GObject"); b.name = "beta"
    comp.add_child(a); comp.add_child(b)
    r.check(comp.get_child_by_name("alpha") != null, "get_child_by_name")
    comp.swap_children(0, 1)
    r.check(comp.get_child(0).name == "beta", "swap_children")
    comp.set_child_index_before(a, 1)

    var tgt: Variant = ClassDB.instantiate("GObject")
    comp.add_child(tgt)
    comp.add_transition_xy("t", tgt, Vector2(100, 50), 1.0)
    r.check(comp.get_transition_count() == 1, "add_transition")
    comp.play_transition("t")
    comp.advance_transitions(0.1)
    r.check(comp.is_transition_playing(), "playing")
    comp.set_transition_paused(true)
    r.check(not comp.is_transition_playing(), "paused")
    comp.stop_transition("t")
    r.check(not comp.is_transition_playing(), "stopped")

    comp.free()
    return r.finalize()
