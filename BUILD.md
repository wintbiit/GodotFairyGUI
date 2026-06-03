# GodotFairyGUI — Build Guide

## Directory Structure

```
GodotFairyGUI/
├── CMakeLists.txt              # CMake build system
├── src/                        # C++ source code
├── godot-cpp/                  # GDExtension bindings (submodule, tag godot-4.5-stable)
├── example/                    # Godot demo project
│   └── addons/godot_fairygui/  # ← Final addon deliverable
│       ├── godot_fairygui.gdextension
│       ├── plugin.cfg
│       ├── editor/plugin.gd
│       ├── shaders/clip_mask.gdshader
│       └── bin/
│           ├── godot_fairygui.dll                              # last-built DLL
│           ├── godot_fairygui.windows.template_debug.x86_64.dll
│           └── godot_fairygui.windows.template_release.x86_64.dll
└── tools/build_addon.py        # Cross-platform build script
```

## Prerequisites

- **CMake** ≥ 3.22
- **C++17** compiler:
  - Windows: MSVC (Visual Studio 2022)
  - Linux: GCC ≥ 11 or Clang ≥ 14
  - macOS: Xcode ≥ 14 / AppleClang ≥ 14
- **Godot 4.5.1** (runtime testing)

## Build

### Windows (MSVC)

```batch
rem One-time configure:
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

rem Build Debug:
cmake --build build --config Debug --target godot_fairygui

rem Build Release:
cmake --build build --config Release --target godot_fairygui
```

Output goes to `example/addons/godot_fairygui/bin/godot_fairygui.dll`.

### Linux (GCC/Clang)

```bash
mkdir -p build/linux && cd build/linux
cmake ../../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --target godot_fairygui
```

Output goes to `example/addons/godot_fairygui/bin/libgodot_fairygui.so`.

### macOS (Xcode)

```bash
mkdir -p build/macos && cd build/macos
cmake ../../ -G "Xcode"
cmake --build . --config Release --target godot_fairygui
```

Output goes to `example/addons/godot_fairygui/bin/libgodot_fairygui.macos.template_release.framework`.

### Cross-platform script

```bash
python3 tools/build_addon.py          # current platform Release
python3 tools/build_addon.py --debug  # Debug
```

## Addon Packaging

After building the desired platform DLLs, copy the output to `example/addons/godot_fairygui/bin/` using the template naming convention:

```
godot_fairygui.windows.template_debug.x86_64.dll
godot_fairygui.windows.template_release.x86_64.dll
godot_fairygui.linux.template_debug.x86_64.so
godot_fairygui.linux.template_release.x86_64.so
godot_fairygui.macos.template_debug.framework
godot_fairygui.macos.template_release.framework
```

The `.gdextension` file references these paths. The entire `addons/godot_fairygui/` directory
is the deliverable — users copy it into their Godot project's `addons/` folder.

## Testing

```bash
# Headless smoke test:
godot --headless --path example --quit
# Expected: "GodotFairyGUI smoke: ok"
```

## Known Issues

- **Release MSVC build**: ByteBuffer seek in package loading may fail in optimized builds.
  Use Debug builds for development. (Root cause: potential UB in index table parsing.)
- **Linux/macOS builds**: Only tested via CI. Native builds recommended.
