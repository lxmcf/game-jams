package assets

import ls "shared:lspx"
import rl "vendor:raylib"

import "core:strings"

fonts: map[string]rl.Font
sounds: map[string]rl.Sound
dictionary: map[string]cstring

sprites: ls.Bundle
sprites_loaded: bool

load_assets :: proc() {
    sprites, sprites_loaded = ls.LoadBundle("sprite_project/export/bundle.lspx" when ODIN_DEBUG else "bundle.lspx")
    if sprites_loaded {
        ls.SetActiveBundle(sprites)
    }

    // Fonts
    fonts["menu_button"] = rl.LoadFontEx("font/concert_one.ttf", 56, nil, 255)

    sounds["menu_button"] = rl.LoadSound("sound/menu_button.ogg")
    sounds["water_1"] = rl.LoadSound("sound/water_1.ogg")
    sounds["water_2"] = rl.LoadSound("sound/water_2.ogg")
    sounds["water_3"] = rl.LoadSound("sound/water_3.ogg")
}

unload_assets :: proc() {
    ls.UnloadBundle(sprites)

    for _, value in dictionary do delete(value)
    delete(dictionary)

    for _, value in fonts do rl.UnloadFont(value)
    delete(fonts)

    for _, value in sounds do rl.UnloadSound(value)
    delete(sounds)
}

dictionary_lookup :: proc(key: string) -> cstring {
    exists := key in dictionary

    if !exists {
        dictionary[key] = strings.clone_to_cstring(key)
    }

    return dictionary[key]
}
