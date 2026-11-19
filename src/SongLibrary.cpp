#include "SongLibrary.hpp"
#include "MusicManager.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/file.hpp>
#include <algorithm>
#include <charconv>
#include <chrono>

using namespace geode::prelude;
namespace custom_menu_music {
namespace {
std::string pathText(std::filesystem::path const& path) { return string::pathToString(path); }
std::filesystem::path utf8Path(std::string_view text) {
    return std::filesystem::path(std::u8string(text.begin(), text.end()));
}
bool samePath(std::filesystem::path const& a, std::filesystem::path const& b) {
    if (a.empty() || b.empty()) return false;
    std::error_code ec;
    auto left = std::filesystem::weakly_canonical(a, ec);
    if (ec) return false;
    auto right = std::filesystem::weakly_canonical(b, ec);
    if (ec) return false;
    if (left == right) return true;
    return std::filesystem::equivalent(a, b, ec) && !ec;
}
std::string newId() {
    static unsigned serial = 0;
    return fmt::format("local-{}-{}", std::chrono::system_clock::now().time_since_epoch().count(), ++serial);
}
}
SongLibrary& SongLibrary::get() { static SongLibrary library; return library; }
LibrarySong SongLibrary::makeLocal(std::filesystem::path const& path) const {
    return {newId(), pathText(path.stem()), "", "Local", 0, path, false};
}
Result<> SongLibrary::save() const {
    auto entries = matjson::Value::array();
    for (auto const& song : m_songs) {
        entries.push(matjson::makeObject({{"id", song.id}, {"title", song.title},
            {"artist", song.artist}, {"source", song.source}, {"songId", song.songId},
            {"filePath", pathText(song.filePath)}, {"active", song.active}}));
    }
    return file::writeStringSafe(Mod::get()->getSaveDir() / "library.json", entries.dump());
}
Result<> SongLibrary::initialize() {
    if (m_loaded) return m_error.empty() ? Result<>(Ok()) : Result<>(Err("{}", m_error));
    m_loaded = true;
    try {
        auto jsonPath = Mod::get()->getSaveDir() / "library.json";
        if (std::filesystem::exists(jsonPath) && std::filesystem::file_size(jsonPath) > 0) {
            auto data = file::readString(jsonPath);
            if (data.isErr()) throw std::runtime_error(data.unwrapErr());
            auto parsed = matjson::parse(data.unwrap());
            if (parsed.isErr() || !parsed.unwrap().isArray()) throw std::runtime_error("Invalid library.json; original file preserved");
            for (auto const& value : parsed.unwrap().asArray().unwrap()) {
                auto id = value["id"].asString();
                auto title = value["title"].asString();
                auto artist = value["artist"].asString();
                auto source = value["source"].asString();
                auto path = value["filePath"].asString();
                auto songId = value["songId"].asInt();
                if (!id || !title || !artist || !source || !path || !songId || id.unwrap().empty() ||
                    (source.unwrap() != "Local" && source.unwrap() != "Newgrounds"))
                    throw std::runtime_error("Invalid library entry; original file preserved");
                if (std::ranges::any_of(m_songs, [&](auto const& s) { return s.id == id.unwrap(); }))
                    throw std::runtime_error("Duplicate library ID; original file preserved");
                m_songs.push_back({id.unwrap(), title.unwrap(), artist.unwrap(), source.unwrap(),
                    static_cast<int>(songId.unwrap()), utf8Path(path.unwrap()), false});
            }
        }
        if (m_songs.empty()) {
            auto current = MusicManager::getValidCustomFilePath();
            if (current) m_songs.push_back(makeLocal(*current));
            for (auto const& entry : std::filesystem::directory_iterator(MusicDownloader::getSongsDir())) {
                if (!entry.is_regular_file() || string::toLower(pathText(entry.path().extension())) != ".mp3") continue;
                auto found = std::ranges::find_if(m_songs, [&](auto const& s) { return samePath(s.filePath, entry.path()); });
                if (found == m_songs.end()) { m_songs.push_back(makeLocal(entry.path())); found = std::prev(m_songs.end()); }
                auto stem = pathText(entry.path().stem());
                int id = 0;
                auto parsed = std::from_chars(stem.data(), stem.data() + stem.size(), id);
                if (parsed.ec == std::errc{} && parsed.ptr == stem.data() + stem.size() && id > 0) {
                    found->songId = id; found->source = "Newgrounds";
                    for (auto const& preset : MusicDownloader::getPresets()) if (preset.id == id) {
                        found->title = preset.title; found->artist = preset.artist; break;
                    }
                }
            }
        }
        auto result = syncActive();
        if (result.isErr()) throw std::runtime_error(result.unwrapErr());
        return Ok();
    } catch (std::exception const& e) { m_error = e.what(); return Err("{}", m_error); }
}
Result<> SongLibrary::syncActive() {
    if (!m_loaded) return initialize();
    if (!m_error.empty()) return Err("{}", m_error);
    auto path = Mod::get()->getSettingValue<std::filesystem::path>("custom-music-file");
    for (auto& song : m_songs) song.active = samePath(song.filePath, path);
    return save();
}
Result<std::string> SongLibrary::importLocalFile(std::filesystem::path const& path) {
    GEODE_UNWRAP(initialize());
    try {
        if (!std::filesystem::is_regular_file(path) || std::filesystem::file_size(path) == 0) return Err("Invalid audio file");
        auto ext = string::toLower(pathText(path.extension()));
        if (ext != ".mp3" && ext != ".ogg" && ext != ".wav" && ext != ".flac") return Err("Unsupported audio format");
        for (auto const& song : m_songs) if (samePath(song.filePath, path)) {
            auto id = song.id; GEODE_UNWRAP(setActiveSong(id)); return Ok(id);
        }
        auto dest = MusicDownloader::getSongsDir() / path.filename();
        unsigned suffix = 1;
        // Reserve numeric MP3 filenames for the Newgrounds cache.
        auto stem = pathText(path.stem());
        bool numeric = !stem.empty() && std::ranges::all_of(stem, [](char c) { return c >= '0' && c <= '9'; });
        while ((std::filesystem::exists(dest) && !samePath(dest, path)) || (numeric && dest == MusicDownloader::getSongsDir() / path.filename())) {
            dest = MusicDownloader::getSongsDir() / utf8Path(fmt::format("{} ({}){}", stem, suffix++, ext));
        }
        bool copied = !samePath(path, dest);
        if (copied) std::filesystem::copy_file(path, dest);
        auto song = makeLocal(dest); song.title = pathText(path.stem());
        auto id = song.id;
        m_songs.push_back(std::move(song));
        auto saved = save();
        if (saved.isErr()) {
            m_songs.pop_back();
            if (copied) { std::error_code ec; std::filesystem::remove(dest, ec); }
            return Err("{}", saved.unwrapErr());
        }
        GEODE_UNWRAP(setActiveSong(id));
        return Ok(id);
    } catch (std::exception const& e) { return Err("{}", e.what()); }
}
Result<std::string> SongLibrary::addSong(SongInfo const& info, std::filesystem::path const& path) {
    GEODE_UNWRAP(initialize());
    std::error_code ec;
    if (info.id <= 0 || !std::filesystem::is_regular_file(path, ec)) return Err("Downloaded file is missing");
    auto previous = m_songs;
    auto found = std::ranges::find_if(m_songs, [&](auto const& s) { return s.songId == info.id || samePath(s.filePath, path); });
    if (found == m_songs.end()) { m_songs.push_back(makeLocal(path)); found = std::prev(m_songs.end()); }
    found->title = info.title; found->artist = info.artist; found->source = "Newgrounds";
    found->songId = info.id; found->filePath = path;
    auto id = found->id;
    auto result = save();
    if (result.isErr()) { m_songs = std::move(previous); return Err("{}", result.unwrapErr()); }
    return Ok(id);
}
Result<> SongLibrary::setActiveSong(std::string_view id) {
    GEODE_UNWRAP(initialize());
    auto found = std::ranges::find_if(m_songs, [&](auto const& s) { return s.id == id; });
    if (found == m_songs.end()) return Err("Song not found");
    std::error_code ec;
    if (!std::filesystem::is_regular_file(found->filePath, ec)) return Err("Audio file is missing");
    auto path = found->filePath;
    auto previous = m_songs;
    for (auto& song : m_songs) song.active = song.id == id;
    auto result = save();
    if (result.isErr()) { m_songs = std::move(previous); return result; }
    m_selecting = true;
    Mod::get()->setSettingValue<std::filesystem::path>("custom-music-file", path);
    m_selecting = false;
    MusicManager::refreshMenuMusic(true);
    return Ok();
}
Result<> SongLibrary::removeSong(std::string_view id) {
    GEODE_UNWRAP(initialize());
    auto found = std::ranges::find_if(m_songs, [&](auto const& s) { return s.id == id; });
    if (found == m_songs.end()) return Err("Song not found");
    auto removed = *found;
    auto previous = m_songs;
    m_songs.erase(found);
    auto saved = save();
    if (saved.isErr()) { m_songs = std::move(previous); return saved; }
    if (removed.active) {
        m_selecting = true;
        Mod::get()->setSettingValue<std::filesystem::path>("custom-music-file", {});
        m_selecting = false;
        MusicManager::refreshMenuMusic(false);
    }
    // Canonical parent prevents deleting external files or symlink targets.
    try {
        if (samePath(std::filesystem::weakly_canonical(removed.filePath).parent_path(), MusicDownloader::getSongsDir()) &&
            !std::ranges::any_of(m_songs, [&](auto const& s) { return samePath(s.filePath, removed.filePath); })) {
            std::filesystem::remove(removed.filePath);
        }
    } catch (std::exception const& e) { return Err("Removed from library, but could not delete file: {}", e.what()); }
    return Ok();
}
}
