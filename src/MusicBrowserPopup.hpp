#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>
#include "MusicDownloader.hpp"

namespace custom_menu_music {
class MusicBrowserPopup : public geode::Popup {
protected:
    enum class Tab { Library, Jukebox, Download };
    Tab m_tab = Tab::Library;
    geode::TextInput* m_search = nullptr;
    geode::TextInput* m_songId = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
    geode::ScrollLayer* m_list = nullptr;
    cocos2d::CCNode* m_online = nullptr;
    CCMenuItemSpriteExtra* m_libraryTab = nullptr;
    CCMenuItemSpriteExtra* m_jukeboxTab = nullptr;
    CCMenuItemSpriteExtra* m_onlineTab = nullptr;
    CCMenuItemSpriteExtra* m_import = nullptr;
    geode::async::TaskHolder<geode::utils::file::PickResult> m_picker;
    bool m_busy = false;
    bool m_closed = false;
    bool init() override;
    void onClose(cocos2d::CCObject* sender) override;
    void onTab(cocos2d::CCObject* sender);
    void onDownload(cocos2d::CCObject*);
    void onPreset(cocos2d::CCObject* sender);
    void onUse(cocos2d::CCObject* sender);
    void onRemove(cocos2d::CCObject* sender);
    void onLocalFile(cocos2d::CCObject*);
    void onSettings(cocos2d::CCObject*);
    void selectSong(int id);
    void registerSong(SongInfo const& info, std::filesystem::path const& path);
    void populateList();
    void setStatus(std::string const& text);
public:
    static MusicBrowserPopup* create();
};
}
