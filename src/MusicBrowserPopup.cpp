#include "MusicBrowserPopup.hpp"
#include "MusicDownloader.hpp"
#include "MusicManager.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <charconv>

using namespace geode::prelude;
namespace custom_menu_music {
namespace {
CCMenuItemSpriteExtra* button(char const* title, CCObject* target, SEL_MenuHandler handler, CCMenu* menu, CCPoint position) {
    auto sprite = ButtonSprite::create(title);
    sprite->setScale(.55f);
    auto item = CCMenuItemSpriteExtra::create(sprite, target, handler);
    item->setPosition(position);
    menu->addChild(item);
    return item;
}
CCLabelBMFont* label(std::string const& text, CCNode* parent, CCPoint position, float scale, float width) {
    auto node = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    node->setPosition(position);
    node->limitLabelWidth(width, scale, .1f);
    parent->addChild(node);
    return node;
}
}
MusicBrowserPopup* MusicBrowserPopup::create() {
    auto popup = new MusicBrowserPopup;
    if (popup->init()) { popup->autorelease(); return popup; }
    delete popup;
    return nullptr;
}
bool MusicBrowserPopup::init() {
    if (!Popup::init(420.f, 290.f)) return false;
    m_spanish = MusicManager::isSpanish();
    setTitle(m_spanish ? "Explorador de Música" : "Music Browser");
    label("Song ID:", m_mainLayer, {60, 232}, .45f, 75);
    m_songId = TextInput::create(150, "467339");
    m_songId->setCommonFilter(CommonFilter::Uint);
    m_songId->setMaxCharCount(10);
    m_songId->setPosition({185, 232});
    m_mainLayer->addChild(m_songId);
    button(m_spanish ? "Descargar" : "Download", this, menu_selector(MusicBrowserPopup::onDownload), m_buttonMenu, {335, 232});
    m_statusLabel = label(m_spanish ? "Elige un preset o introduce un Song ID" : "Choose a preset or enter a Song ID", m_mainLayer, {210, 205}, .33f, 375);
    label(m_spanish ? "Presets comunitarios" : "Community Presets", m_mainLayer, {210, 184}, .43f, 375);
    m_presets = ScrollLayer::create(CCSize{380, 130});
    m_presets->setPosition({20, 44});
    m_mainLayer->addChild(m_presets);
    populatePresets();
    button(m_spanish ? "Archivo local" : "Local file", this, menu_selector(MusicBrowserPopup::onLocalFile), m_buttonMenu, {125, 23});
    button(m_spanish ? "Ajustes" : "Settings", this, menu_selector(MusicBrowserPopup::onSettings), m_buttonMenu, {300, 23});
    return true;
}
void MusicBrowserPopup::onClose(CCObject* sender) {
    m_closed = true;
    Popup::onClose(sender);
}
void MusicBrowserPopup::setStatus(std::string const& text) {
    m_statusLabel->setString(text.c_str());
    m_statusLabel->limitLabelWidth(375, .33f, .1f);
}
void MusicBrowserPopup::populatePresets() {
    auto content = m_presets->m_contentLayer;
    content->removeAllChildren();
    auto const& songs = MusicDownloader::getPresets();
    float height = songs.size() * 42.f;
    content->setContentSize({380, height});
    for (size_t i = 0; i < songs.size(); ++i) {
        auto const& song = songs[i];
        float y = height - (i + .5f) * 42.f;
        auto title = label(song.title, content, {8, y + 8}, .42f, 245);
        title->setAnchorPoint({0, .5f});
        auto artist = label(fmt::format("{} | {}", song.artist, song.id), content, {8, y - 8}, .3f, 245);
        artist->setAnchorPoint({0, .5f});
        artist->setColor({190, 210, 230});
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        menu->setContentSize({380, height});
        content->addChild(menu);
        bool cached = MusicDownloader::getDownloadedPath(song.id).has_value();
        auto item = button(cached ? (m_spanish ? "Usar" : "Use") : (m_spanish ? "Descargar" : "Download"),
            this, menu_selector(MusicBrowserPopup::onPreset), menu, {315, y});
        item->setTag(song.id);
    }
    m_presets->scrollToTop();
}
void MusicBrowserPopup::onDownload(CCObject*) {
    auto text = m_songId->getString();
    int id = 0;
    auto parsed = std::from_chars(text.data(), text.data() + text.size(), id);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || id <= 0) {
        setStatus(m_spanish ? "Introduce un Song ID válido" : "Enter a valid Song ID");
        return;
    }
    selectSong(id);
}
void MusicBrowserPopup::onPreset(CCObject* sender) { selectSong(static_cast<CCNode*>(sender)->getTag()); }
void MusicBrowserPopup::selectSong(int id) {
    if (m_busy) return;
    if (auto cached = MusicDownloader::getDownloadedPath(id)) { useFile(*cached); return; }
    m_busy = true;
    setStatus(m_spanish ? "Consultando canción..." : "Looking up song...");
    WeakRef<MusicBrowserPopup> weak(this);
    MusicDownloader::fetchSongInfo(id, [weak](Result<SongInfo> result) {
        auto self = weak.lock();
        if (!self || self->m_closed) return;
        if (result.isErr()) { self->m_busy = false; self->setStatus(result.unwrapErr()); return; }
        auto info = result.unwrap();
        self->setStatus(fmt::format("{} {} ({:.1f} MB)", self->m_spanish ? "Descargando" : "Downloading", info.title, info.sizeMB));
        MusicDownloader::downloadSong(info, [weak](Result<std::filesystem::path> downloaded) {
            auto popup = weak.lock();
            if (!popup || popup->m_closed) return;
            popup->m_busy = false;
            if (downloaded.isErr()) { popup->setStatus(downloaded.unwrapErr()); return; }
            popup->useFile(downloaded.unwrap());
            popup->populatePresets();
        });
    });
}
void MusicBrowserPopup::useFile(std::filesystem::path const& path) {
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec)) {
        setStatus(m_spanish ? "El archivo no existe" : "File does not exist");
        return;
    }
    auto previous = Mod::get()->getSettingValue<std::filesystem::path>("custom-music-file");
    Mod::get()->setSettingValue<std::filesystem::path>("custom-music-file", path);
    // Changed settings already trigger refreshMenuMusic(true) in main.cpp.
    if (previous == path) MusicManager::refreshMenuMusic(true);
    setStatus(m_spanish ? "Canción seleccionada" : "Song selected");
}
void MusicBrowserPopup::onLocalFile(CCObject*) {
    if (m_busy) return;
    m_busy = true;
    m_picker.spawn(file::pick(file::PickMode::OpenFile, {
        std::nullopt, {{"Audio (*.mp3, *.ogg, *.wav, *.flac)", {"*.mp3", "*.ogg", "*.wav", "*.flac"}}}
    }), [this](file::PickResult result) {
        m_busy = false;
        if (m_closed) return;
        if (result.isErr()) { setStatus(result.unwrapErr()); return; }
        if (result.unwrap()) useFile(*result.unwrap());
    });
}
void MusicBrowserPopup::onSettings(CCObject*) { openSettingsPopup(Mod::get()); }
}
