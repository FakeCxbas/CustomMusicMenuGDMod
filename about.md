# Custom Menu Music

Choose any custom audio file (`.mp3`, `.ogg`, `.wav`, `.flac`) to play on the Geometry Dash main menu.

## Features

- **Online Music Browser & Song ID Downloader:** Download and apply any Newgrounds or Geometry Dash track directly by entering its Song ID, complete with title, artist, and size inspection.
- **Curated Community Presets:** One-click downloads and playback for 12 iconic community tracks (Waterflame, Creo, Dimrain47, F-777, Dex Arson, ColBreakz, etc.).
- **Flexible audio formats:** Supports `.mp3`, `.ogg`, `.wav`, and `.flac` files selected through Geode's native file picker.
- **Respects native game options:** Honors Geometry Dash's built-in **Menu Music** toggle (`GameVar::DisableMenuMusic`), remaining silent when disabled in game options.
- **Instant reload:** Updating the file, toggling the mod, or changing the loop setting applies immediately from either the main menu or Geode's Mods list without restarting the game.
- **Texture pack compatible shortcut:** Renders a native `CircleButtonSprite` overlaid with your active cube icon on the main menu bottom bar.
- **Bilingual interface & notifications:** Spanish and English UI, status messages, and optional Now Playing popups.

## How to use

1. Click the music note shortcut on the bottom bar of the main menu.
2. Enter any **Newgrounds Song ID** to download it directly, click **Use / Download** on any community preset, or click **Local File** to select your own audio track.
3. Playback updates immediately on the main menu.

## Credits

- Developed by **FakeCxbas**
- Built with **Geode SDK**
