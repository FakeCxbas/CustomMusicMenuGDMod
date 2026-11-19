#pragma once
#include <string>
#include <string_view>

namespace custom_menu_music {
// GD bitmap fonts have a limited glyph set. Preserve case while transliterating
// Latin UTF-8, including decomposed accents, before passing text to Cocos.
inline std::string cleanGDText(std::string_view text) {
    std::string out;
    constexpr std::string_view upper = "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYTs";
    constexpr std::string_view lower = "aaaaaaaceeeeiiiidnooooo/ouuuuyty";
    for (size_t i = 0; i < text.size();) {
        auto c = static_cast<unsigned char>(text[i]);
        if (c < 128) { out += c >= 32 ? static_cast<char>(c) : ' '; ++i; continue; }
        if (i + 1 < text.size()) {
            auto next = static_cast<unsigned char>(text[i + 1]);
            if (c == 0xc3 && next >= 0x80 && next <= 0xbf) {
                out += next < 0xa0 ? upper[next - 0x80] : lower[next - 0xa0];
                i += 2; continue;
            }
            if ((c == 0xcc && next >= 0x80 && next <= 0xbf) ||
                (c == 0xcd && next >= 0x80 && next <= 0xaf)) { i += 2; continue; }
        }
        // One visible replacement per unsupported code point, never broken UTF-8.
        out += '?';
        ++i;
        while (i < text.size() && (static_cast<unsigned char>(text[i]) & 0xc0) == 0x80) ++i;
    }
    return out;
}
}
