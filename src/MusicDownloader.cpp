#include "MusicDownloader.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <charconv>
#include <cmath>
#include <mutex>
#include <thread>

namespace custom_menu_music {
namespace {
std::mutex downloadMutex;
std::string decode(std::string const& text, bool form = true) {
    std::string output;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '%' && i + 2 < text.size()) {
            unsigned value;
            auto result = std::from_chars(text.data() + i + 1, text.data() + i + 3, value, 16);
            if (result.ec == std::errc{} && result.ptr == text.data() + i + 3) {
                output += static_cast<char>(value);
                i += 2;
                continue;
            }
        }
        output += form && text[i] == '+' ? ' ' : text[i];
    }
    return output;
}
bool validUrl(std::string const& url) {
    return url.starts_with("https://") || url.starts_with("http://");
}
}
std::vector<PresetSong> const& MusicDownloader::getPresets() {
    static std::vector<PresetSong> const songs = {
        {467339, "At the Speed of Light", "Dimrain47"},
        {261685, "Glorious Morning", "Waterflame"},
        {167466, "Time Machine", "Waterflame"},
        {467140, "Sonic Blaster", "F-777"},
        {624119, "Airborne Robots", "F-777"},
        {697079, "Carnivores", "Creo"},
        {772648, "Epilogue", "Creo"},
        {762464, "Crazy", "Creo"},
        {663672, "Payload", "Dex Arson"},
        {737522, "2011", "ColBreakz"},
        {611849, "Highscore", "Panda Eyes & Teminite"},
        {837946, "The Cyber Grind", "Meganeko"}
    };
    return songs;
}
std::filesystem::path MusicDownloader::getSongsDir() {
    auto path = geode::Mod::get()->getSaveDir() / "downloaded_songs";
    std::filesystem::create_directories(path);
    return path;
}
std::optional<std::filesystem::path> MusicDownloader::getDownloadedPath(int songId) {
    if (songId <= 0) return std::nullopt;
    try {
        auto path = getSongsDir() / (std::to_string(songId) + ".mp3");
        std::error_code ec;
        if (std::filesystem::is_regular_file(path, ec)) {
            auto size = std::filesystem::file_size(path, ec);
            if (!ec && size > 0) return path;
        }
    } catch (std::exception const&) {}
    return std::nullopt;
}
void MusicDownloader::fetchSongInfo(int songId, InfoCallback callback) {
    std::thread([songId, callback = std::move(callback)] {
        auto result = [&]() -> geode::Result<SongInfo> {
            try {
                if (songId <= 0) return geode::Err("Invalid Song ID");
                auto response = geode::utils::web::WebRequest()
                    .timeout(std::chrono::seconds(30))
                    .header("Content-Type", "application/x-www-form-urlencoded")
                    .bodyString(fmt::format("songID={}&secret=Wmfd2893gb7", songId))
                    .postSync("http://www.boomlings.com/database/getGJSongInfo.php");
                if (!response.ok()) return geode::Err("Song lookup failed (HTTP {})", response.code());
                auto body = response.string();
                if (body.isErr()) return geode::Err("Unable to read song information");
                auto text = body.unwrap();
                if (text == "-1") return geode::Err("Song unavailable / Canción no disponible");
                auto fields = geode::utils::string::split(text, "~|~");
                SongInfo song;
                for (size_t i = 0; i + 1 < fields.size(); i += 2) {
                    auto const& key = fields[i];
                    auto const& value = fields[i + 1];
                    if (key == "1") song.id = std::stoi(value);
                    else if (key == "2") song.title = decode(value);
                    else if (key == "4") song.artist = decode(value);
                    else if (key == "5") song.sizeMB = std::stof(value);
                    // Preserve escaping in an already usable URL (including signed URLs).
                    else if (key == "10") song.downloadUrl = validUrl(value) ? value : decode(value, false);
                }
                if (song.id != songId || song.title.empty() || !validUrl(song.downloadUrl) ||
                    !std::isfinite(song.sizeMB) || song.sizeMB < 0)
                    return geode::Err("Invalid song information / Respuesta inválida");
                return geode::Ok(std::move(song));
            } catch (std::exception const& e) { return geode::Err("{}", e.what()); }
        }();
        geode::queueInMainThread([callback = std::move(callback), result = std::move(result)]() mutable { callback(std::move(result)); });
    }).detach();
}
void MusicDownloader::downloadSong(SongInfo const& info, DownloadCallback callback) {
    std::thread([info, callback = std::move(callback)] {
        auto result = [&]() -> geode::Result<std::filesystem::path> {
            // Serialize writes so multiple browser windows cannot corrupt the same cache entry.
            std::lock_guard lock(downloadMutex);
            std::filesystem::path temporary;
            try {
                if (info.id <= 0 || !validUrl(info.downloadUrl)) return geode::Err("Invalid song");
                if (auto cached = getDownloadedPath(info.id)) return geode::Ok(*cached);
                auto destination = getSongsDir() / (std::to_string(info.id) + ".mp3");
                temporary = destination;
                temporary += ".part";
                auto response = geode::utils::web::WebRequest().timeout(std::chrono::seconds(180)).getSync(info.downloadUrl);
                if (!response.ok() || response.data().empty()) return geode::Err("Download failed (HTTP {})", response.code());
                auto const& data = response.data();
                bool id3 = data.size() >= 3 && data[0] == 'I' && data[1] == 'D' && data[2] == '3';
                bool frame = data.size() >= 2 && data[0] == 0xff && (data[1] & 0xe0) == 0xe0;
                if (!id3 && !frame) return geode::Err("Server did not return an MP3 / El servidor no devolvió un MP3");
                auto saved = response.into(temporary);
                if (saved.isErr()) throw std::runtime_error(saved.unwrapErr());
                std::filesystem::rename(temporary, destination);
                return geode::Ok(destination);
            } catch (std::exception const& e) {
                std::error_code ec;
                if (!temporary.empty()) std::filesystem::remove(temporary, ec);
                return geode::Err("{}", e.what());
            }
        }();
        geode::queueInMainThread([callback = std::move(callback), result = std::move(result)]() mutable { callback(std::move(result)); });
    }).detach();
}
}
