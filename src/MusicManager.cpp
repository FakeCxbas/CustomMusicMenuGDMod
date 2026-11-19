#include "MusicManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/ui/Notification.hpp>

#include <cctype>
#include <cstdlib>
#include <string>
#include <system_error>

#ifdef GEODE_IS_WINDOWS
#include <windows.h>
#endif

namespace custom_menu_music {
    bool MusicManager::isModEnabled() {
        return geode::Mod::get()->getSettingValue<bool>("enabled");
    }

    bool MusicManager::isGdMenuMusicDisabled() {
        auto* gm = GameManager::sharedState();
        return gm && gm->getGameVariable("0122");
    }

    std::optional<std::filesystem::path> MusicManager::getValidCustomFilePath() {
        auto path = geode::Mod::get()->getSettingValue<std::filesystem::path>("custom-music-file");
        std::error_code ec;
        if (!path.empty() && std::filesystem::is_regular_file(path, ec)) return path;
        return std::nullopt;
    }

    bool MusicManager::shouldOverrideMenuMusic() {
        return isModEnabled() && !isGdMenuMusicDisabled() && getValidCustomFilePath().has_value();
    }

    bool MusicManager::isInMenuContext() {
        return PlayLayer::get() == nullptr && LevelEditorLayer::get() == nullptr &&
            cocos2d::CCDirector::sharedDirector()->getRunningScene() != nullptr;
    }

    bool MusicManager::isSpanish() {
        auto language = geode::Mod::get()->getSettingValue<std::string>("language");
        if (language == "Español") return true;
        if (language == "English") return false;

#ifdef GEODE_IS_WINDOWS
        if ((GetUserDefaultUILanguage() & 0x3FF) == 0x0A) return true;
#endif

        for (auto variable : {"LC_ALL", "LANG"}) {
            if (auto* value = std::getenv(variable)) {
                std::string localeName(value);
                for (auto& character : localeName) {
                    character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
                }
                if (localeName.starts_with("es") || localeName.find("spanish") != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }

    void MusicManager::showSongNotification(std::filesystem::path const& path) {
        auto name = geode::utils::string::pathToString(path.filename());
        auto text = isSpanish()
            ? fmt::format("Reproduciendo: {}", name)
            : fmt::format("Now Playing: {}", name);
        geode::Notification::create(text, geode::NotificationIcon::Success, 1.5f)->show();
    }

    bool MusicManager::playCustomMusic(bool fadeIn, bool notifyIfStarted) {
        if (!shouldOverrideMenuMusic()) return false;
        auto path = *getValidCustomFilePath();
        auto* engine = FMODAudioEngine::sharedEngine();
        if (!engine) return false;
        auto pathStr = geode::utils::string::pathToString(path);
        if (engine->isMusicPlaying(pathStr, 0)) return true;

        auto loop = geode::Mod::get()->getSettingValue<bool>("loop");
        engine->playMusic(pathStr, loop, fadeIn ? 1.0f : 0.0f, 0);
        if (notifyIfStarted) {
            geode::queueInMainThread([path]() { showSongNotification(path); });
        }
        return true;
    }

    void MusicManager::refreshMenuMusic(bool notifyOnPlay) {
        if (!isInMenuContext()) return;
        if (shouldOverrideMenuMusic()) {
            auto path = *getValidCustomFilePath();
            if (auto* engine = FMODAudioEngine::sharedEngine()) {
                auto pathStr = geode::utils::string::pathToString(path);
                auto loop = geode::Mod::get()->getSettingValue<bool>("loop");
                engine->playMusic(pathStr, loop, 0.3f, 0);
                if (notifyOnPlay && geode::Mod::get()->getSettingValue<bool>("show-notification")) {
                    showSongNotification(path);
                }
            }
        } else if (auto* gm = GameManager::sharedState()) {
            gm->playMenuMusic();
        }
    }
}
