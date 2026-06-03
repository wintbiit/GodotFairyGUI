#!/usr/bin/env python3
"""
GodotFairyGUI — Addon build & packaging script.

Usage:
    python3 tools/build_addon.py            # build current platform Release
    python3 tools/build_addon.py --debug     # build Debug
    python3 tools/build_addon.py --all       # build all supported platforms

Output goes to example/addons/godot_fairygui/bin/

Supported platforms:
    windows  (MSVC/Clang, x86_64)
    linux    (GCC/Clang, x86_64) – cross or native
    macos    (AppleClang, arm64+x86_64) – native only
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD = os.path.join(ROOT, "build")
ADDON_BIN = os.path.join(ROOT, "example", "addons", "godot_fairygui", "bin")

CMakePresets = {
    "windows": {
        "generator": "Visual Studio 17 2022",
        "arch": "x64",
    },
    "linux": {
        "generator": "Unix Makefiles",
    },
    "macos": {
        "generator": "Xcode",
    },
}


def dll_name(platform_: str, config: str, *, arm64: bool = False) -> str:
    prefix = "lib" if platform_ in ("linux", "macos") else ""
    arch = "arm64" if arm64 else "x86_64"
    ext = {
        "windows": ".dll",
        "linux": ".so",
        "macos": ".framework",
    }[platform_]
    if platform_ == "macos":
        return f"{prefix}godot_fairygui.macos.template_{config}.framework"
    return f"{prefix}godot_fairygui.{platform_}.template_{config}.{arch}{ext}"


def build_platform(plat: str, config: str = "Release") -> str:
    preset = CMakePresets[plat]
    gen = preset["generator"]
    build_dir = os.path.join(BUILD, plat)
    os.makedirs(build_dir, exist_ok=True)

    # Configure
    cmd = [
        "cmake",
        "-B", build_dir,
        "-S", ROOT,
        "-G", gen,
    ]
    if plat == "windows":
        cmd.extend(["-A", "x64"])
    if plat == "linux":
        cmd.append("-DCMAKE_BUILD_TYPE=" + config)
    subprocess.check_call(cmd)

    # Build
    bc = ["cmake", "--build", build_dir, "--config", config, "--target", "godot_fairygui"]
    subprocess.check_call(bc)

    # Locate output DLL
    out = os.path.join(ROOT, "example", "addons", "godot_fairygui", "bin")
    src = os.path.join(out, dll_name(plat, config))
    # The raw CMake output is just "godot_fairygui.dll", rename to template standard
    raw = os.path.join(out, {
        "windows": "godot_fairygui.dll",
        "linux": "libgodot_fairygui.so",
        "macos": "libgodot_fairygui.macos.template_debug.framework",
    }.get(plat, "godot_fairygui"))
    if os.path.exists(raw):
        target = os.path.join(out, dll_name(plat, config.lower()))
        if raw != target and os.path.exists(target):
            os.remove(target)
        shutil.copy2(raw, target)
        print(f"  -> {target}")
    return out


def main():
    ap = argparse.ArgumentParser(description="Build GodotFairyGUI addon")
    ap.add_argument("--debug", action="store_true", help="Build Debug")
    ap.add_argument("--all", action="store_true", help="Build all platforms")
    args = ap.parse_args()

    config = "Debug" if args.debug else "Release"
    host = platform.system().lower()

    platforms = ["windows", "linux", "macos"] if args.all else [host]

    if host == "windows" and "windows" in platforms:
        build_platform("windows", config)
    if host == "linux" and "linux" in platforms:
        build_platform("linux", config)
    if host == "darwin" and "macos" in platforms:
        build_platform("macos", config)

    print("Done — addon ready at:", ADDON_BIN)


if __name__ == "__main__":
    main()
