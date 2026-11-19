# Changelog

## v1.2.0
* Standardized the interface, notifications, and download errors on English and removed the language setting
* Fixed disappearing song buttons while scrolling by grouping each row into one culling container
* Left the initial status empty after successful library loading
* Added interactive in-game **Music Browser** popup accessible from the main menu shortcut button
* Added persistent **Global Music Library** (`library.json`) with unified management for local and downloaded tracks
* Added **Song Switching** with live active status badges (`GJ_completesIcon_001.png`), instant playback switch, and song deletion
* Added **+ Import File** to import local audio tracks directly into the global library
* Added Newgrounds / Geometry Dash **Song ID direct download** via Boomlings audio endpoint with live title, artist, and size inspection
* Added **Community Presets** catalog featuring 12 iconic GD community tracks (Dimrain47, Waterflame, F-777, Creo, Dex Arson, ColBreakz, Panda Eyes & Teminite, Meganeko)
* Transliterated accented song metadata for GD bitmap fonts (`cleanGDText`) and preserved mixed case for titles and artists using `chatFont.fnt`
* Cached downloaded audio files in mod save storage (`downloaded_songs/`) with automatic format validation and reuse
* Integrated settings shortcut directly inside the browser popup

## v1.1.1
* Separated now playing notifications into independent settings: `show-notification` (on song selection) and `notify-on-menu-return` (on returning to the menu from a level)
* Disabled `notify-on-menu-return` by default to eliminate repetitive notifications when exiting levels
* Bumped mod version to 1.1.1

## v1.1.0
* Fixed custom music continuing to play when Geometry Dash's native **Menu Music** toggle (`0122`) is disabled
* Fixed settings changes not applying immediately when opened from Geode's Mods list (`ModsLayer`)
* Added live update support for the `show-menu-button` setting
* Added `.flac` audio file filter support in `mod.json`
* Modularized audio playback and state logic into `MusicManager` (`src/MusicManager.hpp`, `src/MusicManager.cpp`)
* Added `GameManager::getMenuMusicFile` hook and removed redundant playback/spritesheet reload calls from `MenuLayer::init`
* Replaced `std::locale("")` with safe, non-throwing OS and environment language detection
* Removed tracked `.geode` build binary from repository and added multi-platform GitHub Actions CI (`geode-sdk/build-geode-mod`)
* Corrected `repository` URL and added `issues` metadata in `mod.json`

## v1.0.1
* Rounded the mod logo corners

## v1.0.0
* Initial release
* Custom menu music support (.mp3, .ogg, .wav)
* Texture pack support using native circle buttons
* In-game file picker with real-time reload
* English and Spanish support
* Optional song notification
