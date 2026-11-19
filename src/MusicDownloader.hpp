#pragma once

#include <Geode/Result.hpp>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace custom_menu_music {
struct SongInfo {
    int id = 0;
    std::string title;
    std::string artist;
    std::string downloadUrl;
    float sizeMB = 0;
};
struct PresetSong {
    int id;
    std::string title;
    std::string artist;
};
class MusicDownloader {
public:
    using InfoCallback = std::function<void(geode::Result<SongInfo>)>;
    using DownloadCallback = std::function<void(geode::Result<std::filesystem::path>)>;
    static std::vector<PresetSong> const& getPresets();
    static std::filesystem::path getSongsDir();
    static std::optional<std::filesystem::path> getDownloadedPath(int songId);
    static void fetchSongInfo(int songId, InfoCallback callback);
    static void downloadSong(SongInfo const& info, DownloadCallback callback);
};
}
