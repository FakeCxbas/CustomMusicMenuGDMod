#pragma once
#include "MusicDownloader.hpp"
#include <string_view>

namespace custom_menu_music {
struct LibrarySong {
    std::string id;
    std::string title;
    std::string artist;
    std::string source;
    int songId = 0;
    std::filesystem::path filePath;
    bool active = false;
};
// Accessed on the game thread; downloader callbacks are dispatched there too.
class SongLibrary {
    std::vector<LibrarySong> m_songs;
    bool m_loaded = false;
    bool m_selecting = false;
    std::string m_error;
    geode::Result<> save() const;
    LibrarySong makeLocal(std::filesystem::path const& path) const;
public:
    static SongLibrary& get();
    geode::Result<> initialize();
    std::vector<LibrarySong> const& songs() const { return m_songs; }
    bool isSelecting() const { return m_selecting; }
    geode::Result<> syncActive();
    geode::Result<std::string> importLocalFile(std::filesystem::path const& path);
    geode::Result<std::string> addSong(SongInfo const& info, std::filesystem::path const& path);
    geode::Result<> setActiveSong(std::string_view id);
    geode::Result<> removeSong(std::string_view id);
};
}
