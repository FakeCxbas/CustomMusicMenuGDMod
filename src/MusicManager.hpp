#pragma once

#include <filesystem>
#include <optional>

namespace custom_menu_music {
    class MusicManager {
    public:
        static bool isModEnabled();
        static bool isGdMenuMusicDisabled();
        static std::optional<std::filesystem::path> getValidCustomFilePath();
        static bool shouldOverrideMenuMusic();
        static bool isInMenuContext();
        static void showSongNotification(std::filesystem::path const& path);
        static bool playCustomMusic(bool fadeIn, bool notifyIfStarted);
        static void refreshMenuMusic(bool notifyOnPlay);
    };
}
