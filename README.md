# GodotFairyGUI

GodotFairyGUI is a GDExtension runtime for FairyGUI packages on Godot. The project targets Godot 4.5, verified with Godot 4.5.1 and `godot-cpp` tag `godot-4.5-stable`, and keeps the runtime model aligned with FairyGUI's Unity API while using native Godot `Control` nodes.

## Build

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

The debug library is written to:

```text
example/addons/godot_fairygui/bin/godot_fairygui.dll
```

## Smoke Test

```powershell
& "D:\opt\godot\Godot_v4.5.1-stable_mono_win64\Godot_v4.5.1-stable_mono_win64_console.exe" --headless --path example --scene res://scenes/main.tscn
```

The smoke scene loads real FairyGUI `.bytes` packages from `example/assets/ui`, exercises `add_package`, `add_package_bytes`, `add_package_with_loader`, Callable-backed package/texture/audio loading, asset path overrides, texture and audio caches, `has_package`, `remove_package`, `remove_all_packages`, and a `user://` hot-update style package path with external PNG fallback. It parses package metadata, atlas sprite records, sound items, image scale9 grids, component rawData, controller/page data, display/XY/size/look gears, transition records, movieclip frames, bitmap font glyphs, basic UBB text tags, text/input/label/button/list/loader/progress-bar/slider/scrollbar/combobox setup data, GList ScrollPane basics, GGraph shapes, and PixelHitTestData, creates `GImage`/`GMovieClip`/`GComponent`/`GLabel`/`GButton`/`GProgressBar`/`GSlider`/`GScrollBar`/`GComboBox` objects through `UIPackage.create_object`, binds an `AtlasTexture` slice, expands component children, verifies `GImage` direct draw and nine-slice metadata, `GLoader` align/fill/shrink/resize layout for external textures and `ui://` package objects, `GTextField` style properties, bitmap font selection, basic UBB conversion, RichTextLabel-backed rich text mode, and `fgui_link_click`, `GTextInput` prompt/restrict/max-length/password/editable/selection APIs plus `LineEdit`/`TextEdit` native ownership, `GButton` package sound loading and `AudioStreamPlayer` playback, `GLabel` title/icon/input forwarding APIs, `GMovieClip` frame texture/advance APIs, `GTween` position/alpha/value/property/delay/kill behavior, `GComponent` transition XY/size/scale/alpha/rotation/color/visible/text/icon playback, `GSlider` value/percent/signals, `GScrollBar` display/scroll percent and structure, `GComboBox` item/value/title selection, `GList` registration, metadata APIs, `add_item_from_url` item creation, virtual-list visible item reuse, single-selection state, `fgui_click_item`, content size, scroll position clamping, `scroll_to_view`, and mouse-wheel scrolling, verifies `GLoader` external and `ui://` package loading, checks `fgui_click`, switches a real controller page, and checks bitset-backed pixel hit testing before printing `GodotFairyGUI smoke: ok`.

## Runtime Loading

`UIPackage.add_package(path)` uses Godot path semantics for `res://`, `user://`, PCK-mounted files, and external paths. `UIPackage.add_package_with_loader(asset_name_prefix, loader)` accepts a Callable for custom hot-update or encrypted asset systems; the loader receives paths such as `mem/BundleUsage_fui.bytes` and `mem/BundleUsage_atlas0.png`, returning either `PackedByteArray` bytes or a `Texture2D`.

## Editor Helper

The addon includes a lightweight editor plugin at `example/addons/godot_fairygui/plugin.cfg`. It adds FairyGUI menu actions that scan the configured package root, defaulting to `res://assets/ui`, and call the runtime `UIPackage.add_package` API directly. It can also refresh `GComponent` nodes in the edited scene from their `package_name` and `component_name` Inspector properties. Generated preview/runtime children are added without an owner, so they can render in the viewport without being serialized into the `.tscn`. The plugin does not generate intermediate import files, so editor usage keeps the same runtime-first `.bytes` loading model as hot-update builds.
