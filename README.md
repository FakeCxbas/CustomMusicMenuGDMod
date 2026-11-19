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

- **Persistent music library:** Import local files, keep downloads, switch the active song, and remove entries from one library saved in `library.json`.
- **Library and Download / Presets tabs:** Scrollable rows show a music icon, title, artist, source, and a green active indicator. Song metadata preserves capitalization and accents are transliterated for GD bitmap fonts.

- **Multi-format audio support:** Choose `.mp3`, `.ogg`, `.wav`, or `.flac` files via Geode's built-in file picker.
- **Native Menu Music toggle compliance:** Checks `GameManager::getGameVariable("0122")` (`GameVar::DisableMenuMusic`) in all hooks and setting listeners so disabling Menu Music in Geometry Dash options always mutes menu music.
- **Live settings reload:** Changes to the audio file, enabled toggle, loop toggle, and menu shortcut button apply immediately from both `MenuLayer` and Geode's `ModsLayer`.
- **Texture pack friendly UI:** Uses native `CircleButtonSprite` paired with the player's current `SimplePlayer` cube colors and glow.
- **English interface and notifications:** All controls and status messages use English, with optional Now Playing notifications.

## Usage

1. Click the music note shortcut on the bottom bar of the main menu (or open **Geode -> Mods -> Custom Menu Music -> Settings**).
2. In **Library**, select **+ Import File** to copy an audio file into the mod's `downloaded_songs` folder. Use **Download / Presets** for Newgrounds Song IDs and community presets.
3. Select **USE** to switch songs; the active song has a green check. Playback starts when both the mod and Geometry Dash's **Menu Music** option are enabled.
4. The trash button removes an entry and deletes its file only if it belongs to `downloaded_songs`. Removing the active song restores the default menu track.

On first use of an empty library, the mod registers the configured audio file and existing MP3 downloads. External files registered by this migration remain at their original paths and are never deleted by the library.

## Architecture

- `src/SongLibrary.hpp` / `src/SongLibrary.cpp`: Persistent library, initial migration, local copies, Newgrounds metadata, selection, and managed-file deletion.
- `src/MusicBrowserPopup.hpp` / `src/MusicBrowserPopup.cpp`: Library and download tabs, file picker, and scrollable song rows.
- `src/GDText.hpp`: Case-preserving UTF-8 transliteration for GD fonts.

- `src/MusicManager.hpp` / `src/MusicManager.cpp`: Encapsulates custom file validation (`std::error_code`), native Menu Music state checks (`0122`), FMOD playback control, menu context guards, and English Now Playing notifications.
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

## Verification

Run the standalone text regression tests with a native C++23 compiler:

```bash
c++ -std=c++23 -Wall -Wextra -Werror -Isrc tests/GDTextTest.cpp -o /tmp/modgd-text-test
/tmp/modgd-text-test
```

In Geometry Dash, verify migration with an empty library, local imports with duplicate filenames, switching songs, persistence after restart, preset downloads, and deleting both active and inactive songs. Scroll both tabs through multiple rows and back, checking that USE/Get buttons, trash buttons, and active badges stay visible with their rows. The status area should be empty on a successful initial load. Also check that removing a migrated external entry leaves its original file intact.

## License & Authors

This project is licensed under the [MIT License](LICENSE).

- **Main Author:** Sebastián Zambrano ([@FakeCxbas](https://github.com/FakeCxbas))
- **Detailed Credits:** See [AUTHORS.md](AUTHORS.md) for full project roles and framework credits.
- **Framework:** Built with [Geode SDK](https://geode-sdk.org/) for Geometry Dash.
