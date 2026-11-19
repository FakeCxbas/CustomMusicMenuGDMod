#include "MusicManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;
using custom_menu_music::MusicManager;

class $modify(CustomGameManager, GameManager) {
    gd::string getMenuMusicFile() {
        if (MusicManager::shouldOverrideMenuMusic()) {
            return gd::string(geode::utils::string::pathToString(*MusicManager::getValidCustomFilePath()));
        }
        return GameManager::getMenuMusicFile();
    }

    void playMenuMusic() {
        if (MusicManager::shouldOverrideMenuMusic()) {
            MusicManager::playCustomMusic(false, true);
            return;
        }
        GameManager::playMenuMusic();
    }

    void fadeInMenuMusic() {
        if (MusicManager::shouldOverrideMenuMusic()) {
            MusicManager::playCustomMusic(true, true);
            return;
        }
        GameManager::fadeInMenuMusic();
    }
};

class $modify(CustomMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        updateCustomMusicButton();
        return true;
    }

    void updateCustomMusicButton() {
        auto* bottomMenu = getChildByID("bottom-menu");
        if (!bottomMenu) return;
        if (auto* existing = bottomMenu->getChildByID("custom-menu-music-button"_spr)) {
            existing->removeFromParent();
        }
        if (!Mod::get()->getSettingValue<bool>("show-menu-button")) {
            bottomMenu->updateLayout();
            return;
        }

        auto* cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        if (!cache->spriteFrameByName("GJ_noteIcon_001.png") &&
            !cache->spriteFrameByName("GJ_musicIcon_001.png")) {
            cache->addSpriteFramesWithFile("GJ_GameSheet03.plist");
        }
        CircleButtonSprite* btnSpr = nullptr;
        for (auto frameName : {"GJ_noteIcon_001.png", "GJ_musicIcon_001.png"}) {
            if (cache->spriteFrameByName(frameName)) {
                btnSpr = CircleButtonSprite::createWithSpriteFrameName(
                    frameName, 0.92f, CircleBaseColor::Green, CircleBaseSize::MediumAlt
                );
                if (btnSpr) break;
            }
        }

        if (btnSpr) {
            btnSpr->setTopOffset({-3.0f, 2.5f});
            if (auto* gm = GameManager::sharedState()) {
                if (auto* player = SimplePlayer::create(0)) {
                    player->updatePlayerFrame(gm->getPlayerFrame(), IconType::Cube);
                    player->setColor(gm->colorForIdx(gm->getPlayerColor()));
                    player->setSecondColor(gm->colorForIdx(gm->getPlayerColor2()));
                    if (gm->getPlayerGlow()) {
                        player->setGlowOutline(gm->colorForIdx(gm->getPlayerGlowColor()));
                    } else {
                        player->disableGlowOutline();
                        player->updateColors();
                    }
                    player->setScale(0.55f);
                    player->setRotation(-10.0f);
                    player->setPosition(btnSpr->getContentSize() / 2 + ccp(7.5f, -6.5f));
                    btnSpr->addChild(player, 2);
                }
            }
            auto* btn = CCMenuItemSpriteExtra::create(
                btnSpr, this, menu_selector(CustomMenuLayer::onCustomMenuMusicBtn)
            );
            btn->setID("custom-menu-music-button"_spr);
            bottomMenu->addChild(btn);
        }
        bottomMenu->updateLayout();
    }

    void onCustomMenuMusicBtn(cocos2d::CCObject*) {
        geode::openSettingsPopup(geode::Mod::get());
    }
};

$on_mod(Loaded) {
    listenForSettingChanges<std::filesystem::path>("custom-music-file", +[](std::filesystem::path const&) {
        MusicManager::refreshMenuMusic(true);
    });
    listenForSettingChanges<bool>("enabled", +[](bool) {
        MusicManager::refreshMenuMusic(true);
    });
    listenForSettingChanges<bool>("loop", +[](bool) {
        MusicManager::refreshMenuMusic(false);
    });
    listenForSettingChanges<bool>("show-menu-button", +[](bool) {
        if (auto* menu = MenuLayer::get()) {
            static_cast<CustomMenuLayer*>(menu)->updateCustomMusicButton();
        }
    });
}
