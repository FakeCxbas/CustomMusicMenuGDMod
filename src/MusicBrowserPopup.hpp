#pragma once
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>

namespace custom_menu_music {
class MusicBrowserPopup : public geode::Popup {
protected:
    geode::TextInput* m_songId = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
    geode::ScrollLayer* m_presets = nullptr;
    geode::async::TaskHolder<geode::utils::file::PickResult> m_picker;
    bool m_busy = false;
    bool m_closed = false;
    bool m_spanish = false;
    bool init() override;
    void onClose(cocos2d::CCObject* sender) override;
    void onDownload(cocos2d::CCObject*);
    void onPreset(cocos2d::CCObject* sender);
    void onLocalFile(cocos2d::CCObject*);
    void onSettings(cocos2d::CCObject*);
    void selectSong(int id);
    void useFile(std::filesystem::path const& path);
    void populatePresets();
    void setStatus(std::string const& text);
public:
    static MusicBrowserPopup* create();
};
}
