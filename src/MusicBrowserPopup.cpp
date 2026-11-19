#include "MusicBrowserPopup.hpp"
#include "MusicManager.hpp"
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
void activeBadge(CCNode* parent, float x, float y, bool spanish) {
    if (auto check = icon("GJ_completesIcon_001.png", 18)) {
        check->setPosition({x, y + 7}); parent->addChild(check);
    }
    auto text = label(spanish ? "ACTIVA" : "ACTIVE", parent, {x, y - 12}, .22f, 55);
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
    m_spanish = MusicManager::isSpanish();
    setTitle(m_spanish ? "Explorador de Musica" : "Music Browser");
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("GJ_GameSheet03.plist");
    m_libraryTab = button(m_spanish ? "Biblioteca" : "Library", this, menu_selector(MusicBrowserPopup::onTab), m_buttonMenu, {118, 269}, .6f);
    m_libraryTab->setTag(0);
    m_onlineTab = button(m_spanish ? "Descargar / Presets" : "Download / Presets", this, menu_selector(MusicBrowserPopup::onTab), m_buttonMenu, {310, 269}, .5f);
    m_onlineTab->setTag(1);
    m_online = CCNode::create();
    m_mainLayer->addChild(m_online);
    label("Song ID:", m_online, {64, 226}, .4f, 80);
    m_songId = TextInput::create(150, "467339");
    m_songId->setCommonFilter(CommonFilter::Uint);
    m_songId->setMaxCharCount(10);
    m_songId->setPosition({195, 226});
    m_online->addChild(m_songId);
    auto menu = CCMenu::create(); menu->setPosition({0, 0}); m_online->addChild(menu);
    button(m_spanish ? "Descargar" : "Download", this, menu_selector(MusicBrowserPopup::onDownload), menu, {350, 226});
    m_statusLabel = label("", m_mainLayer, {220, 58}, .3f, 400, true);
    m_import = button(m_spanish ? "+ Importar Archivo" : "+ Import File", this, menu_selector(MusicBrowserPopup::onLocalFile), m_buttonMenu, {139, 27});
    button(m_spanish ? "Ajustes" : "Settings", this, menu_selector(MusicBrowserPopup::onSettings), m_buttonMenu, {345, 27});
    auto loaded = SongLibrary::get().initialize();
    if (loaded.isErr()) setStatus(loaded.unwrapErr());
    else {
        auto sync = SongLibrary::get().syncActive();
        setStatus(sync.isErr() ? sync.unwrapErr() : (m_spanish ? "Biblioteca de Musica" : "Music Library"));
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
    m_showLibrary = static_cast<CCNode*>(sender)->getTag() == 0;
    populateList();
}
void MusicBrowserPopup::populateList() {
    m_online->setVisible(!m_showLibrary);
    m_import->setVisible(m_showLibrary);
    m_libraryTab->setOpacity(m_showLibrary ? 255 : 150);
    m_onlineTab->setOpacity(m_showLibrary ? 150 : 255);
    if (m_list) m_list->removeFromParent();
    float viewport = m_showLibrary ? 174.f : 126.f;
    m_list = ScrollLayer::create(CCSize{400, viewport});
    m_list->setPosition({20, 76}); m_mainLayer->addChild(m_list);
    auto content = m_list->m_contentLayer;
    auto const& library = SongLibrary::get().songs();
    auto const& presets = MusicDownloader::getPresets();
    size_t count = m_showLibrary ? library.size() : presets.size();
    float height = std::max(viewport, count * 58.f);
    content->setContentSize({400, height});
    if (!count) label(m_spanish ? "Importa tu primera cancion" : "Import your first song", content, {200, height / 2}, .75f, 360, true);
    for (size_t i = 0; i < count; ++i) {
        float y = height - (i + .5f) * 58.f;
        auto bg = CCLayerColor::create(i % 2 ? ccc4(0, 0, 0, 65) : ccc4(255, 255, 255, 18), 398, 55);
        bg->setPosition({1, y - 27}); content->addChild(bg);
        auto note = icon("GJ_noteIcon_001.png", 27);
        if (!note) note = icon("GJ_musicIcon_001.png", 27);
        if (note) { note->setPosition({23, y}); content->addChild(note); }
        auto title = m_showLibrary ? library[i].title : presets[i].title;
        auto artist = m_showLibrary ? library[i].artist : presets[i].artist;
        auto source = m_showLibrary ? (library[i].source == "Local" ? "Local" : fmt::format("NG {}", library[i].songId)) : fmt::format("NG {}", presets[i].id);
        auto titleNode = label(title, content, {46, y + 15}, .8f, 235, true); titleNode->setAnchorPoint({0, .5f});
        auto artistNode = label(artist.empty() ? (m_spanish ? "Artista desconocido" : "Unknown artist") : artist, content, {46, y}, .6f, 235, true);
        artistNode->setAnchorPoint({0, .5f}); artistNode->setColor({190, 210, 230});
        auto sourceNode = label(source, content, {46, y - 16}, .48f, 235, true);
        sourceNode->setAnchorPoint({0, .5f}); sourceNode->setColor({150, 185, 210});
        auto rowMenu = CCMenu::create(); rowMenu->setPosition({0, 0}); content->addChild(rowMenu);
        bool active = m_showLibrary ? library[i].active : std::ranges::any_of(library, [&](auto const& s) { return s.songId == presets[i].id && s.active; });
        if (active) activeBadge(content, 318, y, m_spanish);
        else {
            bool cached = m_showLibrary || MusicDownloader::getDownloadedPath(presets[i].id).has_value();
            auto use = button(cached ? (m_spanish ? "USAR" : "USE") : (m_spanish ? "Bajar" : "Get"), this,
                m_showLibrary ? menu_selector(MusicBrowserPopup::onUse) : menu_selector(MusicBrowserPopup::onPreset), rowMenu, {318, y}, .43f);
            if (m_showLibrary) use->setID(library[i].id);
            else use->setTag(presets[i].id);
        }
        if (m_showLibrary) {
            CCMenuItemSpriteExtra* remove = nullptr;
            if (auto trash = icon("GJ_trashBtn_001.png", 27)) {
                remove = CCMenuItemSpriteExtra::create(trash, this, menu_selector(MusicBrowserPopup::onRemove));
                remove->setPosition({377, y}); rowMenu->addChild(remove);
            } else remove = button("X", this, menu_selector(MusicBrowserPopup::onRemove), rowMenu, {377, y}, .4f);
            remove->setID(library[i].id);
        }
    }
    m_list->scrollToTop();
}
void MusicBrowserPopup::onUse(CCObject* sender) {
    auto id = static_cast<CCNode*>(sender)->getID();
    auto result = SongLibrary::get().setActiveSong(id);
    setStatus(result.isErr() ? result.unwrapErr() : (m_spanish ? "Cancion seleccionada" : "Song selected"));
    populateList();
}
void MusicBrowserPopup::onRemove(CCObject* sender) {
    auto id = static_cast<CCNode*>(sender)->getID();
    auto result = SongLibrary::get().removeSong(id);
    setStatus(result.isErr() ? result.unwrapErr() : (m_spanish ? "Cancion eliminada" : "Song removed"));
    populateList();
}
void MusicBrowserPopup::onDownload(CCObject*) {
    auto text = m_songId->getString(); int id = 0;
    auto parsed = std::from_chars(text.data(), text.data() + text.size(), id);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || id <= 0) {
        setStatus(m_spanish ? "Introduce un Song ID valido" : "Enter a valid Song ID"); return;
    }
    selectSong(id);
}
void MusicBrowserPopup::onPreset(CCObject* sender) { selectSong(static_cast<CCNode*>(sender)->getTag()); }
void MusicBrowserPopup::registerSong(SongInfo const& info, std::filesystem::path const& path) {
    auto added = SongLibrary::get().addSong(info, path);
    if (added.isErr()) { setStatus(added.unwrapErr()); return; }
    auto selected = SongLibrary::get().setActiveSong(added.unwrap());
    setStatus(selected.isErr() ? selected.unwrapErr() : (m_spanish ? "Cancion seleccionada" : "Song selected"));
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
    setStatus(m_spanish ? "Consultando cancion..." : "Looking up song...");
    WeakRef<MusicBrowserPopup> weak(this);
    MusicDownloader::fetchSongInfo(id, [weak](Result<SongInfo> result) {
        auto self = weak.lock();
        if (!self || self->m_closed) return;
        if (result.isErr()) { self->m_busy = false; self->setStatus(result.unwrapErr()); return; }
        auto info = result.unwrap();
        self->setStatus(fmt::format("{} {} ({:.1f} MB)", self->m_spanish ? "Descargando" : "Downloading", info.title, info.sizeMB));
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
            popup->setStatus(selected.isErr() ? selected.unwrapErr() : (popup->m_spanish ? "Cancion seleccionada" : "Song selected"));
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
        setStatus(imported.isErr() ? imported.unwrapErr() : (m_spanish ? "Archivo importado y activo" : "File imported and active"));
        populateList();
    });
}
void MusicBrowserPopup::onSettings(CCObject*) { openSettingsPopup(Mod::get()); }
}
