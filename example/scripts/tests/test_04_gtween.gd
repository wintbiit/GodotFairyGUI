# Test 04 -- GTween: all tween types, seek, pause, callbacks
extends RefCounted

static func run(host: Node) -> bool:
    var TR := load("res://scripts/tests/test_runner.gd")
    var r: RefCounted = TR.new()
    r.init("04_gtween")

    var tw: Variant = ClassDB.instantiate("GTween")
    host.add_child(tw)
    var tgt: Variant = ClassDB.instantiate("GObject")
    host.add_child(tgt)

    tgt.position = Vector2.ZERO
    var id: Variant = tw.to_position(tgt, Vector2(100, 40), 1.0)
    tw.set_tween_ease(id, 0)
    r.check(id > 0, "to_position")
    tw.advance(0.5)
    r.check(tgt.position == Vector2(50, 20), "pos 50%")
    tw.advance(0.5)
    r.check(tgt.position == Vector2(100, 40), "pos done")

    tgt.modulate = Color.WHITE
    id = tw.to_alpha(tgt, 0.25, 1.0)
    tw.set_tween_ease(id, 0)
    tw.advance(1.0)
    r.check(is_equal_approx(tgt.modulate.a, 0.25), "to_alpha")

    var val := {"v": 0.0}
    id = tw.to_value(2.0, 10.0, 1.0, func(f: float): val["v"] = f)
    tw.set_tween_ease(id, 0)
    tw.advance(0.25)
    r.check(is_equal_approx(val["v"], 4.0), "to_value 25%")
    tw.advance(0.75)
    r.check(is_equal_approx(val["v"], 10.0), "to_value done")

    tgt.size = Vector2(10, 20)
    id = tw.to_property(tgt, "size", Vector2(30, 40), 1.0)
    tw.set_tween_ease(id, 0)
    tw.advance(0.5)
    r.check(tgt.size == Vector2(20, 30), "to_property")

    tgt.position = Vector2(50, 50)
    id = tw.to_shake(tgt, 10.0, 0.5)
    tw.advance(0.25)
    r.check(tgt.position != Vector2(50, 50), "shake")
    tw.advance(0.5)
    r.check(tgt.position == Vector2(50, 50), "shake restore")

    tgt.position = Vector2.ZERO
    id = tw.to_position(tgt, Vector2(200, 0), 1.0)
    tw.set_tween_ease(id, 0)
    tw.advance(0.2)
    var mid: Variant = tgt.position
    tw.seek(id, 0.0)
    r.check(tgt.position == Vector2.ZERO, "seek 0")
    tw.set_tween_paused(id, true)
    tw.advance(1.0)
    r.check(tgt.position == Vector2.ZERO, "paused")

    id = tw.to_position(tgt, Vector2(100, 0), 0.3)
    tw.set_tween_ease(id, 0)
    tw.set_tween_repeat(id, 3, true)
    tw.advance(0.3 * 4)
    r.check(tgt.position == Vector2.ZERO, "yoyo")

    tw.kill_target(tgt, false)
    id = tw.to_value(0, 1, 1.0, func(_f: float): pass)
    tw.set_tween_user_data(id, {"test": 42})
    tw.kill(id)
    tw.clear()
    tw.free(); tgt.free()
    return r.finalize()
