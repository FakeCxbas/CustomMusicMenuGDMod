#include "MusicBrowserPopup.hpp"
#include "SongLibrary.hpp"
#include "GDText.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <algorithm>
#include <charconv>

using namespace geode::prelude;
namespace custom_menu_music {
namespace {
CCMenuItemSpriteExtra* button(char const* title, CCObject* target, SEL_MenuHandler handler, CCMenu* menu, CCPoint position, float scale = .5f) {
    auto text = cleanGDText(title);
    auto sprite = ButtonSprite::create(text.c_str());
    sprite->setScale(scale);
    auto item = CCMenuItemSpriteExtra::create(sprite, target, handler);
    item->setPosition(position);
    menu->addChild(item);
    return item;
}
CCLabelBMFont* label(std::string const& text, CCNode* parent, CCPoint position, float scale, float width, bool chat = false) {
    auto clean = cleanGDText(text);
    auto node = CCLabelBMFont::create(clean.c_str(), chat ? "chatFont.fnt" : "bigFont.fnt");
    node->setPosition(position);
    node->limitLabelWidth(width, scale, .12f);
    parent->addChild(node);
    return node;
}
CCSprite* icon(char const* name, float size) {
    auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
    if (!cache->spriteFrameByName(name)) return nullptr;
    auto sprite = CCSprite::createWithSpriteFrameName(name);
    sprite->setScale(size / std::max(sprite->getContentSize().width, sprite->getContentSize().height));
    return sprite;
}
void activeBadge(CCNode* parent, float x, float y) {
    if (auto check = icon("GJ_completesIcon_001.png", 18)) {
        check->setColor({100, 255, 120});
        check->setPosition({x, y + 7}); parent->addChild(check);
    }
    auto text = label("ACTIVE", parent, {x, y - 12}, .22f, 55);
    text->setColor({100, 255, 120});
}
}
MusicBrowserPopup* MusicBrowserPopup::create() {
    auto popup = new MusicBrowserPopup;
    if (popup->init()) { popup->autorelease(); return popup; }
    delete popup; return nullptr;
}
bool MusicBrowserPopup::init() {
    if (!Popup::init(440.f, 320.f)) return false;
    setTitle("Music Browser");
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("GJ_GameSheet03.plist");
    m_libraryTab = button("Library", this, menu_selector(MusicBrowserPopup::onTab), m_buttonMenu, {80, 269}, .5f);
    m_libraryTab->setTag(0);
    m_jukeboxTab = button("GD Jukebox", this, menu_selector(MusicBrowserPopup::onTab), m_buttonMenu, {220, 269}, .5f);
    m_jukeboxTab->setTag(1);
    m_onlineTab = button("Download NG", this, menu_selector(MusicBrowserPopup::onTab), m_buttonMenu, {360, 269}, .5f);
    m_onlineTab->setTag(2);
    m_search = TextInput::create(398, "Search song or artist...");
    m_search->setPosition({220, 226});
    m_search->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -_.,'!?&()/");
    m_search->setCallback([this](std::string const&) { populateList(); });
    m_mainLayer->addChild(m_search);
    m_online = CCNode::create();
    m_mainLayer->addChild(m_online);
    label("Song ID:", m_online, {64, 226}, .4f, 80);
    m_songId = TextInput::create(150, "467339");
    m_songId->setCommonFilter(CommonFilter::Uint);
    m_songId->setMaxCharCount(10);
    m_songId->setPosition({195, 226});
    m_online->addChild(m_songId);
    auto menu = CCMenu::create(); menu->setPosition({0, 0}); m_online->addChild(menu);
    button("Download", this, menu_selector(MusicBrowserPopup::onDownload), menu, {350, 226});
    label("Community Presets", m_online, {220, 195}, .32f, 250);
    m_statusLabel = label("", m_mainLayer, {220, 58}, .3f, 400, true);
    m_import = button("+ Import File", this, menu_selector(MusicBrowserPopup::onLocalFile), m_buttonMenu, {139, 27});
    button("Settings", this, menu_selector(MusicBrowserPopup::onSettings), m_buttonMenu, {345, 27});
    auto loaded = SongLibrary::get().initialize();
    if (loaded.isErr()) setStatus(loaded.unwrapErr());
    else {
        auto sync = SongLibrary::get().syncActive();
        setStatus(sync.isErr() ? sync.unwrapErr() : "");
    }
    populateList();
    return true;
}
void MusicBrowserPopup::onClose(CCObject* sender) { m_closed = true; Popup::onClose(sender); }
void MusicBrowserPopup::setStatus(std::string const& text) {
    m_statusLabel->setString(cleanGDText(text).c_str());
    m_statusLabel->limitLabelWidth(400, .65f, .15f);
}
void MusicBrowserPopup::onTab(CCObject* sender) {
    auto synced = SongLibrary::get().syncActive();
    if (synced.isErr()) setStatus(synced.unwrapErr());
    m_tab = static_cast<Tab>(static_cast<CCNode*>(sender)->getTag());
    populateList();
}
void MusicBrowserPopup::populateList() {
    bool showingLibrary = m_tab == Tab::Library;
    bool downloading = m_tab == Tab::Download;
    m_online->setVisible(downloading);
    m_search->setVisible(!downloading);
    m_import->setVisible(showingLibrary);
    m_libraryTab->setOpacity(showingLibrary ? 255 : 150);
    m_jukeboxTab->setOpacity(m_tab == Tab::Jukebox ? 255 : 150);
    m_onlineTab->setOpacity(downloading ? 255 : 150);
    if (m_list) m_list->removeFromParent();
    float viewport = downloading ? 108.f : 126.f;
    m_list = ScrollLayer::create(CCSize{400, viewport});
    m_list->setPosition({20, 76}); m_mainLayer->addChild(m_list);
    auto content = m_list->m_contentLayer;
    auto const& library = SongLibrary::get().songs();
    auto const& presets = MusicDownloader::getPresets();
    auto songs = m_tab == Tab::Jukebox ? SongLibrary::get().getJukeboxSongs() : library;
    auto query = string::toLower(cleanGDText(m_search->getString()));
    if (!downloading && !query.empty()) {
        std::erase_if(songs, [&](auto const& song) {
            return string::toLower(cleanGDText(song.title)).find(query) == std::string::npos &&
                string::toLower(cleanGDText(song.artist)).find(query) == std::string::npos;
        });
    }
    size_t count = downloading ? presets.size() : songs.size();
    float height = std::max(viewport, count * 58.f);
    content->setContentSize({400, height});
    if (!count) label(!query.empty() ? "No matching songs" :
        (showingLibrary ? "Import your first song or choose a preset" : "No downloaded GD songs found"),
        content, {200, height / 2}, .75f, 360, true);
    for (size_t i = 0; i < count; ++i) {
        float y = height - (i + .5f) * 58.f;
        // Keep each row together so viewport culling also controls its buttons.
        auto row = CCNode::create();
        row->setContentSize({398.f, 54.f});
        row->setPosition({1.f, y - 27.f});
        content->addChild(row);
        auto bg = CCScale9Sprite::create("GJ_square02.png");
        bg->setContentSize({398, 54});
        bg->setColor(i % 2 ? ccc3(85, 65, 45) : ccc3(105, 80, 55));
        bg->setPosition({199, 27}); row->addChild(bg);
        auto note = icon("GJ_noteIcon_001.png", 27);
        if (!note) note = icon("GJ_musicIcon_001.png", 27);
        if (note) { note->setPosition({23, 27}); row->addChild(note); }
        auto title = downloading ? presets[i].title : songs[i].title;
        auto artist = downloading ? presets[i].artist : songs[i].artist;
        auto source = downloading ? fmt::format("NG {}", presets[i].id) :
            (songs[i].source == "Local" ? "Local" : fmt::format("{} {}", m_tab == Tab::Jukebox ? "GD" : "NG", songs[i].songId));
        auto titleNode = label(title, row, {46, 42}, .8f, 235, true); titleNode->setAnchorPoint({0, .5f});
        auto artistNode = label(artist.empty() ? "Unknown artist" : artist, row, {46, 27}, .6f, 235, true);
        artistNode->setAnchorPoint({0, .5f}); artistNode->setColor({190, 210, 230});
        auto sourceNode = label(source, row, {46, 11}, .48f, 235, true);
        sourceNode->setAnchorPoint({0, .5f}); sourceNode->setColor({150, 185, 210});
        auto rowMenu = CCMenu::create();
        rowMenu->setPosition({0, 0});
        rowMenu->setContentSize({398.f, 54.f});
        row->addChild(rowMenu);
        bool active = !downloading ? songs[i].active : std::ranges::any_of(library, [&](auto const& s) { return s.songId == presets[i].id && s.active; });
        if (active) activeBadge(row, 318, 27);
        else {
            bool cached = !downloading || MusicDownloader::getDownloadedPath(presets[i].id).has_value();
            auto use = button(cached ? "USE" : "Get", this,
                !downloading ? menu_selector(MusicBrowserPopup::onUse) : menu_selector(MusicBrowserPopup::onPreset), rowMenu, {318, 27}, .43f);
            if (!downloading) { use->setID(songs[i].id); use->setTag(songs[i].songId); }
            else use->setTag(presets[i].id);
        }
        if (showingLibrary) {
            CCMenuItemSpriteExtra* remove = nullptr;
            if (auto trash = icon("GJ_trashBtn_001.png", 27)) {
                remove = CCMenuItemSpriteExtra::create(trash, this, menu_selector(MusicBrowserPopup::onRemove));
                remove->setPosition({377, 27}); rowMenu->addChild(remove);
            } else remove = button("X", this, menu_selector(MusicBrowserPopup::onRemove), rowMenu, {377, 27}, .4f);
            remove->setID(songs[i].id);
        }
    }
    m_list->scrollToTop();
}
void MusicBrowserPopup::onUse(CCObject* sender) {
    auto id = static_cast<CCNode*>(sender)->getID();
    auto result = m_tab == Tab::Jukebox ?
        SongLibrary::get().setActiveJukeboxSong(static_cast<CCNode*>(sender)->getTag()) :
        SongLibrary::get().setActiveSong(id);
    setStatus(result.isErr() ? result.unwrapErr() : "Song selected");
    populateList();
}
void MusicBrowserPopup::onRemove(CCObject* sender) {
    auto id = static_cast<CCNode*>(sender)->getID();
    auto result = SongLibrary::get().removeSong(id);
    setStatus(result.isErr() ? result.unwrapErr() : "Song removed");
    populateList();
}
void MusicBrowserPopup::onDownload(CCObject*) {
    auto text = m_songId->getString(); int id = 0;
    auto parsed = std::from_chars(text.data(), text.data() + text.size(), id);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || id <= 0) {
        setStatus("Enter a valid Song ID"); return;
    }
    selectSong(id);
}
void MusicBrowserPopup::onPreset(CCObject* sender) { selectSong(static_cast<CCNode*>(sender)->getTag()); }
void MusicBrowserPopup::registerSong(SongInfo const& info, std::filesystem::path const& path) {
    auto added = SongLibrary::get().addSong(info, path);
    if (added.isErr()) { setStatus(added.unwrapErr()); return; }
    auto selected = SongLibrary::get().setActiveSong(added.unwrap());
    setStatus(selected.isErr() ? selected.unwrapErr() : "Song selected");
    populateList();
}
void MusicBrowserPopup::selectSong(int id) {
    if (m_busy) return;
    if (auto cached = MusicDownloader::getDownloadedPath(id)) {
        SongInfo info; info.id = id; info.title = fmt::format("Song {}", id);
        for (auto const& song : SongLibrary::get().songs()) if (song.songId == id) { info.title = song.title; info.artist = song.artist; break; }
        for (auto const& preset : MusicDownloader::getPresets()) if (preset.id == id) { info.title = preset.title; info.artist = preset.artist; break; }
        registerSong(info, *cached); return;
    }
    m_busy = true;
    setStatus("Looking up song...");
    WeakRef<MusicBrowserPopup> weak(this);
    MusicDownloader::fetchSongInfo(id, [weak](Result<SongInfo> result) {
        auto self = weak.lock();
        if (!self || self->m_closed) return;
        if (result.isErr()) { self->m_busy = false; self->setStatus(result.unwrapErr()); return; }
        auto info = result.unwrap();
        self->setStatus(fmt::format("Downloading {} ({:.1f} MB)...", info.title, info.sizeMB));
        MusicDownloader::downloadSong(info, [weak, info](Result<std::filesystem::path> downloaded) {
            // Persist completed downloads even if the browser was closed meanwhile.
            auto added = downloaded.isOk() ? SongLibrary::get().addSong(info, downloaded.unwrap()) : Result<std::string>(Err("{}", downloaded.unwrapErr()));
            auto selected = added.isOk() ? SongLibrary::get().setActiveSong(added.unwrap()) : Result<>(Err("{}", added.unwrapErr()));
            auto popup = weak.lock();
            if (!popup || popup->m_closed) {
                if (selected.isErr()) log::warn("Song library download: {}", selected.unwrapErr());
                return;
            }
            popup->m_busy = false;
            popup->setStatus(selected.isErr() ? selected.unwrapErr() : "Song selected");
            popup->populateList();
        });
    });
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
        if (!result.unwrap()) return;
        auto imported = SongLibrary::get().importLocalFile(*result.unwrap());
        setStatus(imported.isErr() ? imported.unwrapErr() : "File imported and active");
        populateList();
    });
}
void MusicBrowserPopup::onSettings(CCObject*) { openSettingsPopup(Mod::get()); }
}
