extends RefCounted

var _ok: bool = true
var _count: int = 0
var _failed: int = 0
var _name: String = ""

func init(p_name: String) -> void:
    _name = p_name; _ok = true; _count = 0; _failed = 0

func check(p_condition: bool, p_msg: String = "") -> void:
    _count += 1
    if not p_condition:
        _failed += 1; _ok = false
        if not p_msg.is_empty():
            printerr("  FAIL #%d: %s" % [_count, p_msg])

func finalize() -> bool:
    if _ok: print("  [OK] %s (%d tests)" % [_name, _count])
    else: print("  [FAIL] %s: %d/%d" % [_name, _count - _failed, _count])
    return _ok
