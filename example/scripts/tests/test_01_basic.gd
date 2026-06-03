# Test 01 -- Basic class existence + instantiation
extends RefCounted

const Runner := preload("res://scripts/tests/test_runner.gd")

static func run(host: Node) -> bool:
    var r: RefCounted = Runner.new()
    r.init("01_basic")
    r.check(ClassDB.class_exists("GObject"), "GObject class")
    r.check(ClassDB.class_exists("GComponent"), "GComponent class")
    r.check(ClassDB.class_exists("GButton"), "GButton class")
    r.check(ClassDB.class_exists("GList"), "GList class")
    r.check(ClassDB.class_exists("GTextField"), "GTextField class")
    r.check(ClassDB.class_exists("GTween"), "GTween class")
    r.check(ClassDB.class_exists("GLoader"), "GLoader class")
    r.check(ClassDB.class_exists("UIPackage"), "UIPackage class")

    var obj: Variant = ClassDB.instantiate("GObject")
    r.check(obj is Control, "instantiate GObject")
    obj.free()
    return r.finalize()
