# Custom Menu Music

Choose any custom audio file (`.mp3`, `.ogg`, `.wav`, `.flac`) to play on the Geometry Dash main menu.

## Features

- **Flexible audio formats:** Supports `.mp3`, `.ogg`, `.wav`, and `.flac` files selected through Geode's native file picker.
- **Respects native game options:** Honors Geometry Dash's built-in **Menu Music** toggle (`GameVar::DisableMenuMusic`), remaining silent when disabled in game options.
- **Instant reload:** Updating the file, toggling the mod, or changing the loop setting applies immediately from either the main menu or Geode's Mods list without restarting the game.
- **Texture pack compatible shortcut:** Renders a native `CircleButtonSprite` overlaid with your active cube icon on the main menu bottom bar.
- **Bilingual notifications:** Optional Now Playing popup supporting English and Spanish (`Auto`, `English`, `Español`).

## How to use

1. Click the music note shortcut on the bottom bar of the main menu (or open **Geode -> Mods -> Custom Menu Music -> Settings**).
2. Browse and select your audio file.
3. Playback updates immediately on the main menu.

## Credits

- Developed by **FakeCxbas**
- Built with **Geode SDK**
