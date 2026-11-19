# Custom Menu Music

<p align="left">
  <a href="https://github.com/FakeCxbas/CustomMusicMenuGDMod/actions/workflows/build.yml">
    <img src="https://github.com/FakeCxbas/CustomMusicMenuGDMod/actions/workflows/build.yml/badge.svg" alt="Build Geode Mod" />
  </a>
  <img src="https://img.shields.io/badge/Geode_SDK-v5.11.2-purple" alt="Geode SDK" />
  <img src="https://img.shields.io/badge/Geometry_Dash-2.2081-yellow" alt="Geometry Dash" />
  <img src="https://img.shields.io/badge/C%2B%2B-23-00599C?logo=c%2B%2B&logoColor=white" alt="C++23" />
  <a href="LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-84CC16.svg" alt="License" />
  </a>
</p>

A Geode mod for Geometry Dash (2.2081) that lets you choose any custom audio file (`.mp3`, `.ogg`, `.wav`, `.flac`) to play on the main menu while respecting Geometry Dash's native audio toggles.

## Features

- **Multi-format audio support:** Choose `.mp3`, `.ogg`, `.wav`, or `.flac` files via Geode's built-in file picker.
- **Native Menu Music toggle compliance:** Checks `GameManager::getGameVariable("0122")` (`GameVar::DisableMenuMusic`) in all hooks and setting listeners so disabling Menu Music in Geometry Dash options always mutes menu music.
- **Live settings reload:** Changes to the audio file, enabled toggle, loop toggle, and menu shortcut button apply immediately from both `MenuLayer` and Geode's `ModsLayer`.
- **Texture pack friendly UI:** Uses native `CircleButtonSprite` paired with the player's current `SimplePlayer` cube colors and glow.
- **Bilingual notifications:** Optional Now Playing notification supporting `Auto`, `English`, and `Español` without platform-unsafe `std::locale("")` initialization.

## Usage

1. Click the music note shortcut on the bottom bar of the main menu (or open **Geode -> Mods -> Custom Menu Music -> Settings**).
2. Select an audio file from your device.
3. Playback starts immediately in the menu context when both the mod and Geometry Dash's **Menu Music** option are enabled.

## Architecture

- `src/MusicManager.hpp` / `src/MusicManager.cpp`: Encapsulates custom file validation (`std::error_code`), native Menu Music state checks (`0122`), FMOD playback control, menu context guards, and cross-platform locale detection.
- `src/main.cpp`: Hooks `GameManager` (`getMenuMusicFile`, `playMenuMusic`, `fadeInMenuMusic`), `MenuLayer` (`init`, live shortcut button management), and `$on_mod(Loaded)` setting listeners.

## Building from Source

### Prerequisites

- [Geode SDK](https://github.com/geode-sdk/geode) `v5.11.2` with `GEODE_SDK` set in your environment
- [Geode CLI](https://github.com/geode-sdk/cli)
- CMake 3.21+ and a C++23 compiler toolchain for your target platform

### Build Commands

Using Geode CLI:

```bash
geode build
```

Or using CMake directly (example for cross-compiling Windows from Linux with Clang-MSVC):

```bash
cmake -B build-win -S . \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/.local/share/Geode/cross-tools/clang-msvc-sdk/clang-msvc.cmake" \
  -DSPLAT_DIR="$HOME/.local/share/Geode/cross-tools/splat" \
  -DHOST_ARCH=x64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-win --config Release
```

## License & Authors

This project is licensed under the [MIT License](LICENSE).

- **Main Author:** Sebastián Zambrano ([@FakeCxbas](https://github.com/FakeCxbas))
- **Detailed Credits:** See [AUTHORS.md](AUTHORS.md) for full project roles and framework credits.
- **Framework:** Built with [Geode SDK](https://geode-sdk.org/) for Geometry Dash.
