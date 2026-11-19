#include "GDText.hpp"
#include <cassert>

int main() {
    using custom_menu_music::cleanGDText;
    assert(cleanGDText("Explorador de Música") == "Explorador de Musica");
    assert(cleanGDText("Canción seleccionada") == "Cancion seleccionada");
    assert(cleanGDText("ÁÉÍÓÚÑÜ áéíóúñü") == "AEIOUNU aeiounu");
    assert(cleanGDText("Dimrain47 - At the Speed of Light") == "Dimrain47 - At the Speed of Light");
    assert(cleanGDText("Mu\xCC\x81sica / N\xCC\x83") == "Musica / N");
    assert(cleanGDText("ça va, déjà vu") == "ca va, deja vu");
    assert(cleanGDText("").empty());
    assert(cleanGDText("\xC3") == "?");
    assert(cleanGDText("\xF0\x9F\x8E\xB5") == "?");
}
